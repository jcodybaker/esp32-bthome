# BTHome ESP IDF Implementation

```
// Encoding example
bthome_packet_t packet;
bthome_packet_init(&packet);
bthome_set_device_info(&packet, false, false);
bthome_add_sensor_sint16(&packet, BTHOME_SENSOR_TEMPERATURE, 2500); // 25.00°C
bthome_add_sensor_uint16(&packet, BTHOME_SENSOR_HUMIDITY, 5055);    // 50.55%

uint8_t buffer[64];
int len = bthome_encode_advertisement(&packet, buffer, sizeof(buffer), true);
bthome_packet_free(&packet);

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
