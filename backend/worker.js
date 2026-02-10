export default {
    async fetch(request, env, ctx) {
        // Test GitHub push
        const url = new URL(request.url);
        if (url.pathname === "/v1/devices/current" && request.method === "GET") {
            const result = await env.tempo_db_binding
                .prepare(`
                    SELECT
                        d.id AS deviceId,
                        d.name,
                        r.temperature,
                        r.timestamp
                    FROM Devices d
                    JOIN Readings r ON r.deviceId = d.id
                    JOIN (
                        SELECT deviceId, MAX(timestamp) AS latest_timestamp
                        FROM Readings
                        GROUP BY deviceId
                    ) latest
                    ON r.deviceId = latest.deviceId
                    AND r.timestamp = latest.latest_timestamp
                    ORDER BY d.id
                `)
                .all();

            return jsonResponse(result.results);
        }

        if (url.pathname === "/v1/devices/history" && request.method === "GET") {
            const minutes = Number(url.searchParams.get("minutes") ?? 60);

            if (!Number.isFinite(minutes) || minutes <= 0 || minutes > 1440) {
                return new Response("Bad Request: invalid minutes", { status: 400 });
            }

            const cutoff = new Date(Date.now() - minutes * 60 * 1000).toISOString();

            const result = await env.tempo_db_binding
                .prepare(`
                    SELECT
                        d.id AS deviceId,
                        d.name,
                        r.temperature,
                        r.timestamp
                    FROM Readings r
                    JOIN Devices d ON d.id = r.deviceId
                    WHERE r.timestamp >= ?1
                    ORDER BY r.timestamp
                `)
                .bind(cutoff)
                .all();

            return jsonResponse(result.results);
        }

        if (url.pathname !== "/v1/temperature") {
            return new Response("Not Found", { status: 404 });
        }

        if (request.method !== "POST") {
            return new Response("Method Not Allowed", {
                status: 405,
                headers: { "Allow": "POST" },
            });
        }

        // --- Auth ---
        const apiKey = request.headers.get("X-API-Key");
        if (!apiKey || apiKey !== env.API_KEY) {
            return new Response("Unauthorized", { status: 401 });
        }

        // --- Content-Type ---
        const contentType = request.headers.get("Content-Type") || "";
        if (!contentType.startsWith("application/json")) {
            return new Response("Bad Request: expected application/json", {
                status: 400,
            });
        }

        // --- Parse JSON ---
        let body;
        try {
            body = await request.json();
        } catch {
            return new Response("Bad Request: invalid JSON", { status: 400 });
        }

        // --- Validate payload ---
        const validationError = validateTemperaturePayload(body);
        if (validationError) {
            return new Response(`Bad Request: ${validationError}`, {
                status: 400,
            });
        }

        const deviceId = body.deviceId;
        const temperature = body.temperature;
        const timestamp = new Date().toISOString();

        // --- Verify device exists ---
        const device = await env.tempo_db_binding
            .prepare("SELECT id FROM Devices WHERE id = ?1")
            .bind(deviceId)
            .first();

        if (!device) {
            return new Response("Bad Request: unknown deviceId", {
                status: 400,
            });
        }

        // --- Insert reading ---
        await env.tempo_db_binding
            .prepare(
                `INSERT INTO Readings (deviceId, temperature, timestamp)
         VALUES (?1, ?2, ?3)`
            )
            .bind(deviceId, temperature, timestamp)
            .run();

        const record = {
            deviceId,
            temperature,
            timestamp,
        };

        return new Response(JSON.stringify(record), {
            status: 200,
            headers: { "Content-Type": "application/json" },
        });
    },
};

// ----------------------------
// Helpers
// ----------------------------

function validateTemperaturePayload(body) {
    if (typeof body !== "object" || body === null) {
        return "body must be an object";
    }

    if (!Object.prototype.hasOwnProperty.call(body, "deviceId")) {
        return "missing 'deviceId'";
    }

    if (!Object.prototype.hasOwnProperty.call(body, "temperature")) {
        return "missing 'temperature'";
    }

    if (!Number.isInteger(body.deviceId) || body.deviceId <= 0) {
        return "'deviceId' must be a positive integer";
    }

    if (typeof body.temperature !== "number" || !Number.isFinite(body.temperature)) {
        return "'temperature' must be a finite number";
    }

    return null;
}

function jsonResponse(body, status = 200) {
    return new Response(JSON.stringify(body), {
        status,
        headers: {
            "Content-Type": "application/json",
            "Access-Control-Allow-Origin": "*",
        },
    });
}