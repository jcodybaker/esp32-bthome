# BTHome ESP IDF Implementation

A C library for encoding and decoding BTHome protocol advertisements on ESP32 devices. [BTHome](https://bthome.io) is a BLE advertising format for broadcasting sensor data from IoT devices.

## Note
This code was partially built with Claude.ai. I have reviewed the code and verified it works, but it comes without warranty.

## Usage

Add this component to your ESP-IDF project via the component manager or as a local component. Initialize a packet, add sensor measurements, encode to a BLE advertisement buffer, and broadcast using ESP-IDF's BLE APIs.

## Examples

### Encoding BTHome Advertisements

```c
// Encoding example
bthome_packet_t packet;
bthome_packet_init(&packet);
bthome_set_device_info(&packet, false, false);

// Optional: Set device name (will be included in the BLE advertisement)
bthome_set_device_name(&packet, "DIY-sensor", 10, true);

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
    // Check if device has a name
    if (decoded.device_name != NULL && decoded.device_name_len > 0) {
        printf("Device name: %.*s\n", (int)decoded.device_name_len, decoded.device_name);
    }
    
    // Process measurements
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
- **Device Names**: Support for Complete and Shortened Local Name (UTF-8 encoded)
- **Comprehensive sensor support**: Temperature, humidity, pressure, illuminance, and many more
- **Event support**: Button presses, dimmer controls
- **Memory efficient**: Dynamic allocation for flexible packet sizes

## Device Name Support

BTHome devices can include a human-readable name in advertisements using the Local Name AD element.

### Setting a Device Name

```c
bthome_packet_t packet;
bthome_packet_init(&packet);

// Set complete local name (0x09) - use false for shortened name (0x08)
bthome_set_device_name(&packet, "My-Sensor", 9, true);

// Add measurements and encode as usual
bthome_add_sensor_sint16(&packet, BTHOME_SENSOR_TEMPERATURE, 2500);
uint8_t buffer[128];
int len = bthome_encode_advertisement(&packet, buffer, sizeof(buffer), true);
bthome_packet_free(&packet);
```

### Reading Device Names

When decoding, device names are automatically extracted:

```c
bthome_packet_t packet;
bthome_decode_advertisement(data, len, &packet);

if (packet.device_name != NULL) {
    printf("Device: %.*s\n", (int)packet.device_name_len, packet.device_name);
    // Note: device_name is not null-terminated, use device_name_len
}
bthome_packet_free(&packet);
```

**Note**: Device names must be UTF-8 encoded and ≤255 bytes. The name is optional and transmitted as a separate AD element alongside BTHome service data.
