import { Hono } from "hono";

const app = new Hono();

app.get("/v1/devices/current", devicesCurrentHandler);
app.get("/v1/devices/history", devicesHistoryHandler);
app.post("/v1/temperature", temperatureHandler);

async function devicesCurrentHandler(c) {
    const result = await c.env.tempo_db_binding
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

async function devicesHistoryHandler(c) {

    const minutes = Number(c.req.query("minutes") ?? 60);

    if (!Number.isFinite(minutes) || minutes <= 0 || minutes > 1440) {
        return new Response("Bad Request: invalid minutes", { status: 400 });
    }

    const cutoff = new Date(Date.now() - minutes * 60 * 1000).toISOString();

    const result = await c.env.tempo_db_binding
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

async function temperatureHandler(c) {
    // --- Auth ---
    const apiKey = c.req.header("X-API-Key");
    if (!apiKey || apiKey !== c.env.API_KEY) {
        return new Response("Unauthorized", { status: 401 });
    }

    // --- Content-Type ---
    const contentType = c.req.header("Content-Type") || "";
    if (!contentType.startsWith("application/json")) {
        return new Response("Bad Request: expected application/json", {
            status: 400,
        });
    }

    // --- Parse JSON ---
    let body;
    try {
        body = await c.req.json();
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
    const device = await c.env.tempo_db_binding
        .prepare("SELECT id FROM Devices WHERE id = ?1")
        .bind(deviceId)
        .first();

    if (!device) {
        return new Response("Bad Request: unknown deviceId", {
            status: 400,
        });
    }

    // --- Insert reading ---
    await c.env.tempo_db_binding
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
}

export default app;

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