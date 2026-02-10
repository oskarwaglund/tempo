-- Devices table
CREATE TABLE IF NOT EXISTS Devices (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL UNIQUE
);

-- Readings table
CREATE TABLE IF NOT EXISTS Readings (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  deviceId INTEGER NOT NULL,
  temperature REAL NOT NULL,
  timestamp TEXT NOT NULL,
  FOREIGN KEY (deviceId) REFERENCES Devices(id)
);

CREATE INDEX IF NOT EXISTS idx_readings_timestamp_device ON Readings (timestamp, deviceId);
CREATE INDEX IF NOT EXISTS idx_readings_device_timestamp_desc ON Readings (deviceId, timestamp DESC);