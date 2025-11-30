# BTHome BLE Scanner Example

This example demonstrates how to scan for and decode BTHome BLE advertisements using the ESP-IDF BLE stack.

## How to use

### Hardware Required

* An ESP32 development board (ESP32, ESP32-C3, ESP32-S3, etc.)
* One or more BTHome BLE devices broadcasting sensor data

### Build and Flash

Navigate to the scanner example directory and build:

```bash
cd examples/scanner
idf.py set-target esp32
idf.py build
idf.py -p PORT flash -b 115200 monitor
```

Replace `PORT` with your ESP32's serial port and `esp32` with your target chip (esp32, esp32c3, esp32s3, etc.).

## Example Output

```
I (320) bthome_scanner: Starting BTHome BLE Scanner
I (330) bthome_scanner: BTHome BLE scanner initialized
I (340) bthome_scanner: Scan parameters set successfully
I (350) bthome_scanner: Scan started successfully
I (360) bthome_scanner: BLE scanner started, listening for BTHome advertisements...
I (2450) bthome_scanner: BTHome packet from A4:C1:38:12:34:56 (RSSI: -65 dBm)
I (2451) bthome_scanner:   Version: 2, Encrypted: 0, Trigger-based: 0
I (2452) bthome_scanner:   Packet ID: 42
I (2453) bthome_scanner:   Measurement 0x02: 23.45
I (2454) bthome_scanner:     Temperature: 23.45 °C
I (2455) bthome_scanner:   Measurement 0x03: 55.20
I (2456) bthome_scanner:     Humidity: 55.20 %
```

## Troubleshooting

* Make sure Bluetooth is enabled in menuconfig (`idf.py menuconfig`)
* Verify you have BTHome devices nearby that are broadcasting
* Check that the BLE controller has enough memory allocated
