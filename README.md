# BTHome ESP IDF Implementation

A C library for encoding and decoding BTHome protocol advertisements on ESP32 devices. BTHome is a BLE advertising format for broadcasting sensor data from IoT devices.

## Usage

Add this component to your ESP-IDF project via the component manager or as a local component. Initialize a packet, add sensor measurements, encode to a BLE advertisement buffer, and broadcast using ESP-IDF's BLE APIs.

## Examples

### Encoding BTHome Advertisements

```c
// Encoding example
bthome_packet_t packet;
bthome_packet_init(&packet);
bthome_set_device_info(&packet, false, false);
bthome_add_sensor_sint16(&packet, BTHOME_SENSOR_TEMPERATURE, 2500); // 25.00°C
bthome_add_sensor_uint16(&packet, BTHOME_SENSOR_HUMIDITY, 5055);    // 50.55%

uint8_t buffer[64];
int len = bthome_encode_advertisement(&packet, buffer, sizeof(buffer), true);
bthome_packet_free(&packet);
```

### Decoding BTHome Advertisements

```c
// Decoding example
bthome_packet_t decoded;
int result = bthome_decode_advertisement(buffer, len, &decoded);
if (result == 0) {
    for (size_t i = 0; i < decoded.measurement_count; i++) {
        float value = bthome_get_scaled_value(&decoded.measurements[i], 
                      bthome_get_scaling_factor(decoded.measurements[i].object_id));
        // Use the value...
    }
}
bthome_packet_free(&decoded);
```

### BLE Scanning for BTHome Devices

```c
#include "bthome_ble.h"

// Callback function for received BTHome packets
void bthome_callback(esp_bd_addr_t addr, int rssi, 
                     const bthome_packet_t *packet, void *user_data) {
    // Process the received packet
    for (size_t i = 0; i < packet->measurement_count; i++) {
        float value = bthome_get_scaled_value(&packet->measurements[i],
                      bthome_get_scaling_factor(packet->measurements[i].object_id));
        // Handle sensor data...
    }
}

// Initialize and start scanning
esp_err_t ret = bthome_ble_scanner_init();
if (ret == ESP_OK) {
    bthome_ble_scanner_config_t config;
    bthome_ble_scanner_get_default_config(&config);
    config.callback = bthome_callback;
    bthome_ble_scanner_start(&config);
}
```

## Features

- **Encoding**: Create BTHome advertisement packets with sensor measurements and events
- **Decoding**: Parse BTHome advertisements from raw BLE data
- **BLE Scanning**: Integrated ESP-IDF BLE scanner to detect and decode BTHome devices
- **Comprehensive sensor support**: Temperature, humidity, pressure, illuminance, and many more
- **Event support**: Button presses, dimmer controls
- **Memory efficient**: Dynamic allocation for flexible packet sizes
