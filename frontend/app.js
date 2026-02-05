const API_URL = "https://tempoapi.wglnd.workers.dev/v1/devices/current";
const HISTORY_URL =
  "https://tempoapi.wglnd.workers.dev/v1/devices/history?minutes=60";

let chart;

async function loadTemps() {
  const res = await fetch(API_URL);
  const data = await res.json();

  const grid = document.getElementById("grid");
  grid.innerHTML = "";

  data.forEach(device => {
    const card = document.createElement("div");
    card.className = "card";

    card.innerHTML = `
      <div class="name">${device.name}</div>
      <div class="temp">${device.temperature.toFixed(1)} °C</div>
      <div class="time">
        ${new Date(device.timestamp).toLocaleTimeString()}
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
}, 10000);