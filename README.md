# sensor-nodes

ESP32-based environmental sensor nodes for home automation, publishing MQTT state updates for room-level monitoring.

This project is designed for a small cluster of battery/USB-powered ESP32-C6 nodes that measure indoor or outdoor conditions and report them to an MQTT broker for use in dashboards, automations, or home assistant-style workflows.

## Features

- ESP32-C6 firmware built with PlatformIO and Arduino
- MQTT publishing over Wi-Fi
- OTA firmware updates for selected nodes
- Watchdog-based reboot protection for silent failures
- Support for multiple sensor combinations in separate build environments
- Native host-side unit tests for payload logic

## Hardware configurations

The repository currently includes two primary node variants:

### 1. Bedroom node: BME688 + SCD41

- BME688 environmental sensor
- SCD41 CO2 sensor
- Publishes to: `flat/bedroom/state`
- Build target: `node_bme688_scd41`
- OTA target: `node_bme688_scd41_ota`

### 2. Balcony node: BME280 + PMSA003I

- BME280 temperature / humidity / pressure sensor
- PMSA003I particulate matter sensor
- Publishes to: `flat/balcony/state`
- Build target: `node_pmsa_bme280`
- OTA target: `node_pmsa_bme280_ota`

## Project structure

```text
.
├── .vscode/
├── include/
├── lib/
├── src/
│   ├── connection_handler.cpp
│   ├── connection_handler.h
│   ├── i2c_scanner.cpp
│   ├── main.cpp
│   ├── ota_handler.cpp
│   ├── ota_handler.h
│   ├── payload.cpp
│   ├── payload.h
│   ├── sensors.h
│   ├── sensors_bme280_pmsa003i.cpp
│   ├── sensors_bme688_scd41.cpp
│   ├── watchdog.cpp
│   └── watchdog.h
├── test/
├── .gitignore
├── LICENSE
├── README.md
├── platformio.ini
└── ...
```

## Build and run

### Prerequisites

- PlatformIO Core or VS Code + PlatformIO extension
- An ESP32-C6 compatible board
- Appropriate sensor hardware wired to the board
- A Wi-Fi network and MQTT broker

### Install dependencies

```bash
pio pkg install
```

### Build a specific environment

```bash
pio run -e node_bme688_scd41
pio run -e node_pmsa_bme280
```

### Upload over USB

```bash
pio run -e node_bme688_scd41 -t upload
pio run -e node_pmsa_bme280 -t upload
```

### OTA upload

For OTA builds, create an environment variable with your password before flashing:

```bash
export OTA_PASS="your-ota-password"
pio run -e node_bme688_scd41_ota -t upload
pio run -e node_pmsa_bme280_ota -t upload
```

The project sets OTA host names in `platformio.ini` using the node names:

- `bedroom.local`
- `balcony.local`

## MQTT and node behavior

Each node publishes a structured reading containing the values currently available from its sensors. The common payload model includes:

- PM1, PM2.5, PM10
- CO2
- temperature
- humidity
- pressure

The firmware loops on a 30-second publish interval and reboots after 30 minutes without a successful publish to recover from persistent communication faults.

## Configuration notes

The environment-specific settings live in `platformio.ini` and define:

- board type (`esp32-c6-devkitc-1`)
- sensor libraries
- MQTT topic names
- OTA upload parameters
- build filters for each node variant

Example topic definitions:

```ini
-D NODE_TOPIC='"flat/bedroom/state"'
-D NODE_NAME='"bedroom"'
```

## Testing

The project includes a native host build for unit testing payload logic without a physical board.

```bash
pio test -e native
```

## License

This project is licensed under the GNU General Public License v2.0. See the [LICENSE](LICENSE) file for details.

## Notes

- The `scanner` environment is intended for I2C bus diagnostics during bring-up.
- The `native` environment is helpful for validating logic that does not require board hardware.
- The firmware is intended for local home automation and experimentation, and is configured to fit a small ESP32 sensor network rather than a generic IoT platform.

## Example usage

Once the devices are online, you can subscribe to the topic stream and integrate the readings into dashboards or automation logic.

```bash
mosquitto_sub -h localhost -t 'flat/+/state'
```

This gives you a simple MQTT data source for climate, air quality, and room-state monitoring.

## Repository

- GitHub: https://github.com/AimrayX/sensor-nodes
- Description: ESP32 cluster with environment sensors for home automation via MQTT
