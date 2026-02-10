const API_BASE =
  location.hostname === "localhost" || location.hostname === "127.0.0.1"
    ? "http://localhost:8787"
    : "https://tempoapi.wglnd.workers.dev";

const API_URL = API_BASE + "/v1/devices/current";
const HISTORY_URL = API_BASE + "/v1/devices/history?minutes=60";

let chart;

async function loadTemps() {
  const res = await fetch(API_URL);
  const data = await res.json();

  const grid = document.getElementById("grid");
  grid.innerHTML = "";

  data.forEach(device => {
    const card = document.createElement("div");
    card.className = "card";

    const now = new Date(Date.now())
    const latest = new Date(device.timestamp);
    const seconds = Math.round((now.getTime() - latest.getTime()) / 1000);
    const timeDiffString = seconds > 300 ? `>300` : `${seconds}`;
    card.innerHTML = `
      <div class="name">${device.name} (${device.deviceId})</div>
      <div class="temp">${device.temperature.toFixed(1)} °C</div>
      <div class="time">
        ${latest.toLocaleTimeString()} (${timeDiffString}s ago)
      </div>
    `;

    grid.appendChild(card);
  });
}

async function loadHistory() {
  const res = await fetch(HISTORY_URL);
  const data = await res.json();

  // Group by device
  const series = {};
  data.forEach(row => {
    if (!series[row.deviceId]) {
      series[row.deviceId] = {
        label: row.name,
        data: [],
      };
    }

    series[row.deviceId].data.push({
      x: new Date(row.timestamp),
      y: row.temperature,
    });
  });

  const datasets = Object.values(series).map((s, i) => ({
    label: s.label,
    data: s.data,
    borderWidth: 2,
    tension: 0.3,
  }));

  const ctx = document.getElementById("chart").getContext("2d");

  if (chart) {
    chart.destroy();
  }

  chart = new Chart(ctx, {
    type: "line",
    data: { datasets },
    options: {
      responsive: true,
      scales: {
        x: {
          type: "time",
          time: {
            unit: "minute",
          },
          title: {
            display: true,
            text: "Time",
          },
        },
        y: {
          title: {
            display: true,
            text: "°C",
          },
        },
      },
    },
  });
}

// Load once
loadTemps();
loadHistory();

setInterval(() => {
  loadTemps();
  loadHistory();
}, 60000);