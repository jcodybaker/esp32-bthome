#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unity.h"
#include "bthome.h"

void setUp(void) {
    // Set up code if needed
}

void tearDown(void) {
    // Clean up code if needed
}

// Helper function to compare floats with tolerance
static bool float_equal(float a, float b, float tolerance) {
    return fabs(a - b) < tolerance;
}

// Test the example payload from BTHome documentation
// https://bthome.io/format/
// Payload: "020106 0B094449592D73656E736F72 0A16D2FC4002C40903BF13"
void test_decode_example_payload(void) {
    // Complete advertising payload from the BTHome documentation
    const uint8_t adv_data[] = {
        // Flags AD element
        0x02, 0x01, 0x06,
        
        // Complete local name AD element: "DIY-sensor"
        0x0B, 0x09, 0x44, 0x49, 0x59, 0x2D, 0x73, 0x65, 0x6E, 0x73, 0x6F, 0x72,
        
        // Service Data (16-bit UUID) - BTHome data
        0x0A, 0x16,
        0xD2, 0xFC,  // UUID (little endian)
        0x40,        // Device info: version 2, not encrypted, not trigger-based
        0x02, 0xC4, 0x09,  // Temperature: 2500 = 25.00°C
        0x03, 0xBF, 0x13   // Humidity: 5055 = 50.55%
    };
    
    bthome_packet_t packet;
    int result = bthome_decode_advertisement(adv_data, sizeof(adv_data), &packet);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    
    // Verify device info
    TEST_ASSERT_FALSE(packet.device_info.encrypted);
    TEST_ASSERT_FALSE(packet.device_info.trigger_based);
    TEST_ASSERT_EQUAL_UINT8(2, packet.device_info.version);
    
    // Verify we have 2 measurements
    TEST_ASSERT_EQUAL_size_t(2, packet.measurement_count);
    
    // Verify first measurement (temperature)
    TEST_ASSERT_EQUAL_UINT8(BTHOME_SENSOR_TEMPERATURE, packet.measurements[0].object_id);
    TEST_ASSERT_TRUE(packet.measurements[0].is_signed);
    TEST_ASSERT_EQUAL_UINT8(2, packet.measurements[0].size);
    TEST_ASSERT_EQUAL_INT16(2500, packet.measurements[0].value.sint16_val);
    
    float temp = bthome_get_scaled_value(&packet.measurements[0], 
                                         bthome_get_scaling_factor(BTHOME_SENSOR_TEMPERATURE));
    TEST_ASSERT_TRUE(float_equal(25.00f, temp, 0.01f));
    
    // Verify second measurement (humidity)
    TEST_ASSERT_EQUAL_UINT8(BTHOME_SENSOR_HUMIDITY, packet.measurements[1].object_id);
    TEST_ASSERT_FALSE(packet.measurements[1].is_signed);
    TEST_ASSERT_EQUAL_UINT8(2, packet.measurements[1].size);
    TEST_ASSERT_EQUAL_UINT16(5055, packet.measurements[1].value.uint16_val);
    
    float humidity = bthome_get_scaled_value(&packet.measurements[1], 
                                             bthome_get_scaling_factor(BTHOME_SENSOR_HUMIDITY));
    TEST_ASSERT_TRUE(float_equal(50.55f, humidity, 0.01f));
    
    bthome_packet_free(&packet);
}

// Test decoding just the service data (without AD elements)
void test_decode_service_data_only(void) {
    const uint8_t service_data[] = {
        0xD2, 0xFC,        // UUID
        0x40,              // Device info
        0x02, 0xC4, 0x09,  // Temperature: 2500 = 25.00°C
        0x03, 0xBF, 0x13   // Humidity: 5055 = 50.55%
    };
    
    bthome_packet_t packet;
    int result = bthome_decode(service_data, sizeof(service_data), &packet);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_size_t(2, packet.measurement_count);
    TEST_ASSERT_EQUAL_UINT8(BTHOME_SENSOR_TEMPERATURE, packet.measurements[0].object_id);
    TEST_ASSERT_EQUAL_UINT8(BTHOME_SENSOR_HUMIDITY, packet.measurements[1].object_id);
    
    bthome_packet_free(&packet);
}

// Test encoding and decoding round trip
void test_encode_decode_round_trip(void) {
    // Create a packet
    bthome_packet_t original;
    bthome_packet_init(&original);
    bthome_set_device_info(&original, false, false);
    
    bthome_add_sensor_sint16(&original, BTHOME_SENSOR_TEMPERATURE, 2500);
    bthome_add_sensor_uint16(&original, BTHOME_SENSOR_HUMIDITY, 5055);
    
    // Encode
    uint8_t buffer[64];
    int encoded_len = bthome_encode_advertisement(&original, buffer, sizeof(buffer), true);
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    // Decode
    bthome_packet_t decoded;
    int result = bthome_decode_advertisement(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT8(original.device_info.version, decoded.device_info.version);
    TEST_ASSERT_EQUAL(original.device_info.encrypted, decoded.device_info.encrypted);
    TEST_ASSERT_EQUAL(original.device_info.trigger_based, decoded.device_info.trigger_based);
    TEST_ASSERT_EQUAL_size_t(original.measurement_count, decoded.measurement_count);
    
    for (size_t i = 0; i < decoded.measurement_count; i++) {
        TEST_ASSERT_EQUAL_UINT8(original.measurements[i].object_id, 
                               decoded.measurements[i].object_id);
        TEST_ASSERT_EQUAL(original.measurements[i].is_signed, 
                         decoded.measurements[i].is_signed);
        TEST_ASSERT_EQUAL_UINT8(original.measurements[i].size, 
                               decoded.measurements[i].size);
    }
    
    bthome_packet_free(&original);
    bthome_packet_free(&decoded);
}

// Test various sensor types
void test_various_sensor_types(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    // Add various sensor types
    bthome_add_sensor_uint8(&packet, BTHOME_SENSOR_BATTERY, 97);
    bthome_add_sensor_uint16(&packet, BTHOME_SENSOR_CO2, 1250);
    bthome_add_sensor_uint24(&packet, BTHOME_SENSOR_ILLUMINANCE, 1346067);
    bthome_add_sensor_sint8(&packet, BTHOME_SENSOR_TEMPERATURE_SINT8, -22);
    
    // Encode and decode
    uint8_t buffer[128];
    int encoded_len = bthome_encode(&packet, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    int result = bthome_decode(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_size_t(4, decoded.measurement_count);
    
    TEST_ASSERT_EQUAL_UINT8(97, decoded.measurements[0].value.uint8_val);
    TEST_ASSERT_EQUAL_UINT16(1250, decoded.measurements[1].value.uint16_val);
    TEST_ASSERT_EQUAL_UINT32(1346067, decoded.measurements[2].value.uint32_val);
    TEST_ASSERT_EQUAL_INT8(-22, decoded.measurements[3].value.sint8_val);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test binary sensors
void test_binary_sensors(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    bthome_add_binary_sensor(&packet, BTHOME_BINARY_MOTION, true);
    bthome_add_binary_sensor(&packet, BTHOME_BINARY_DOOR, false);
    bthome_add_binary_sensor(&packet, BTHOME_BINARY_BATTERY, true);
    
    uint8_t buffer[64];
    int encoded_len = bthome_encode(&packet, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    int result = bthome_decode(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_size_t(3, decoded.measurement_count);
    
    TEST_ASSERT_EQUAL_UINT8(BTHOME_BINARY_MOTION, decoded.measurements[0].object_id);
    TEST_ASSERT_EQUAL_UINT8(1, decoded.measurements[0].value.uint8_val);
    
    TEST_ASSERT_EQUAL_UINT8(BTHOME_BINARY_DOOR, decoded.measurements[1].object_id);
    TEST_ASSERT_EQUAL_UINT8(0, decoded.measurements[1].value.uint8_val);
    
    TEST_ASSERT_EQUAL_UINT8(BTHOME_BINARY_BATTERY, decoded.measurements[2].object_id);
    TEST_ASSERT_EQUAL_UINT8(1, decoded.measurements[2].value.uint8_val);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test button events
void test_button_events(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    bthome_add_button_event(&packet, BTHOME_BUTTON_PRESS);
    
    uint8_t buffer[64];
    int encoded_len = bthome_encode(&packet, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    int result = bthome_decode(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_size_t(1, decoded.event_count);
    TEST_ASSERT_EQUAL_UINT8(BTHOME_EVENT_BUTTON, decoded.events[0].event_type);
    TEST_ASSERT_EQUAL_UINT8(BTHOME_BUTTON_PRESS, decoded.events[0].event_value);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test dimmer events
void test_dimmer_events(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    bthome_add_dimmer_event(&packet, BTHOME_DIMMER_ROTATE_LEFT, 3);
    
    uint8_t buffer[64];
    int encoded_len = bthome_encode(&packet, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    int result = bthome_decode(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_size_t(1, decoded.event_count);
    TEST_ASSERT_EQUAL_UINT8(BTHOME_EVENT_DIMMER, decoded.events[0].event_type);
    TEST_ASSERT_EQUAL_UINT8(BTHOME_DIMMER_ROTATE_LEFT, decoded.events[0].event_value);
    TEST_ASSERT_EQUAL_UINT8(3, decoded.events[0].steps);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test packet ID
void test_packet_id(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    bthome_set_packet_id(&packet, 42);
    bthome_add_sensor_uint8(&packet, BTHOME_SENSOR_BATTERY, 80);
    
    uint8_t buffer[64];
    int encoded_len = bthome_encode(&packet, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    int result = bthome_decode(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_TRUE(decoded.has_packet_id);
    TEST_ASSERT_EQUAL_UINT8(42, decoded.packet_id);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test text sensor
void test_text_sensor(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    const char *text = "Hello World!";
    bthome_add_sensor_text(&packet, text, strlen(text));
    
    uint8_t buffer[64];
    int encoded_len = bthome_encode(&packet, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    int result = bthome_decode(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_size_t(1, decoded.measurement_count);
    TEST_ASSERT_EQUAL_UINT8(BTHOME_SENSOR_TEXT, decoded.measurements[0].object_id);
    TEST_ASSERT_EQUAL_size_t(strlen(text), decoded.measurements[0].value.bytes_val.len);
    TEST_ASSERT_EQUAL_MEMORY(text, decoded.measurements[0].value.bytes_val.data, strlen(text));
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test raw sensor
void test_raw_sensor(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    const uint8_t raw_data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x21};
    bthome_add_sensor_raw(&packet, raw_data, sizeof(raw_data));
    
    uint8_t buffer[64];
    int encoded_len = bthome_encode(&packet, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    int result = bthome_decode(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_size_t(1, decoded.measurement_count);
    TEST_ASSERT_EQUAL_UINT8(BTHOME_SENSOR_RAW, decoded.measurements[0].object_id);
    TEST_ASSERT_EQUAL_size_t(sizeof(raw_data), decoded.measurements[0].value.bytes_val.len);
    TEST_ASSERT_EQUAL_MEMORY(raw_data, decoded.measurements[0].value.bytes_val.data, sizeof(raw_data));
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test device info flags
void test_device_info_flags(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, true);  // trigger-based
    bthome_add_sensor_uint8(&packet, BTHOME_SENSOR_BATTERY, 90);
    
    uint8_t buffer[64];
    int encoded_len = bthome_encode(&packet, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    int result = bthome_decode(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_FALSE(decoded.device_info.encrypted);
    TEST_ASSERT_TRUE(decoded.device_info.trigger_based);
    TEST_ASSERT_EQUAL_UINT8(2, decoded.device_info.version);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test scaling factors
void test_scaling_factors(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.01f, bthome_get_scaling_factor(BTHOME_SENSOR_TEMPERATURE));
    TEST_ASSERT_EQUAL_FLOAT(0.01f, bthome_get_scaling_factor(BTHOME_SENSOR_HUMIDITY));
    TEST_ASSERT_EQUAL_FLOAT(0.001f, bthome_get_scaling_factor(BTHOME_SENSOR_VOLTAGE));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, bthome_get_scaling_factor(BTHOME_SENSOR_BATTERY));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, bthome_get_scaling_factor(BTHOME_SENSOR_CO2));
}

// Test invalid UUID
void test_invalid_uuid(void) {
    const uint8_t invalid_data[] = {
        0xFF, 0xFF,  // Wrong UUID
        0x40,
        0x02, 0xC4, 0x09
    };
    
    bthome_packet_t packet;
    int result = bthome_decode(invalid_data, sizeof(invalid_data), &packet);
    
    TEST_ASSERT_EQUAL_INT(-2, result);  // Invalid UUID error
}

// Test empty packet
void test_empty_packet(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    uint8_t buffer[64];
    int encoded_len = bthome_encode(&packet, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    int result = bthome_decode(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_size_t(0, decoded.measurement_count);
    TEST_ASSERT_EQUAL_size_t(0, decoded.event_count);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test device name (complete local name)
void test_device_name_complete(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    const char *device_name = "DIY-sensor";
    int result = bthome_set_device_name(&packet, device_name, strlen(device_name), true);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    bthome_add_sensor_sint16(&packet, BTHOME_SENSOR_TEMPERATURE, 2500);
    bthome_add_sensor_uint16(&packet, BTHOME_SENSOR_HUMIDITY, 5055);
    
    uint8_t buffer[128];
    int encoded_len = bthome_encode_advertisement(&packet, buffer, sizeof(buffer), true);
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    result = bthome_decode_advertisement(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_NULL(decoded.device_name);
    TEST_ASSERT_EQUAL_size_t(strlen(device_name), decoded.device_name_len);
    TEST_ASSERT_EQUAL_MEMORY(device_name, decoded.device_name, decoded.device_name_len);
    TEST_ASSERT_TRUE(decoded.use_complete_name);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test device name (shortened local name)
void test_device_name_shortened(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    const char *device_name = "Sensor1";
    int result = bthome_set_device_name(&packet, device_name, strlen(device_name), false);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    bthome_add_sensor_uint8(&packet, BTHOME_SENSOR_BATTERY, 85);
    
    uint8_t buffer[128];
    int encoded_len = bthome_encode_advertisement(&packet, buffer, sizeof(buffer), true);
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    result = bthome_decode_advertisement(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_NULL(decoded.device_name);
    TEST_ASSERT_EQUAL_size_t(strlen(device_name), decoded.device_name_len);
    TEST_ASSERT_EQUAL_MEMORY(device_name, decoded.device_name, decoded.device_name_len);
    TEST_ASSERT_FALSE(decoded.use_complete_name);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test device name with UTF-8 characters
void test_device_name_utf8(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    // UTF-8 encoded string: "Temp™"
    const char *device_name = "Temp\xE2\x84\xA2";
    size_t name_len = strlen(device_name);
    
    int result = bthome_set_device_name(&packet, device_name, name_len, true);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    bthome_add_sensor_sint16(&packet, BTHOME_SENSOR_TEMPERATURE, 2100);
    
    uint8_t buffer[128];
    int encoded_len = bthome_encode_advertisement(&packet, buffer, sizeof(buffer), true);
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    result = bthome_decode_advertisement(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NOT_NULL(decoded.device_name);
    TEST_ASSERT_EQUAL_size_t(name_len, decoded.device_name_len);
    TEST_ASSERT_EQUAL_MEMORY(device_name, decoded.device_name, decoded.device_name_len);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test advertisement without device name
void test_no_device_name(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    bthome_add_sensor_uint8(&packet, BTHOME_SENSOR_BATTERY, 75);
    
    uint8_t buffer[128];
    int encoded_len = bthome_encode_advertisement(&packet, buffer, sizeof(buffer), true);
    TEST_ASSERT_GREATER_THAN(0, encoded_len);
    
    bthome_packet_t decoded;
    int result = bthome_decode_advertisement(buffer, encoded_len, &decoded);
    
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_NULL(decoded.device_name);
    TEST_ASSERT_EQUAL_size_t(0, decoded.device_name_len);
    
    bthome_packet_free(&packet);
    bthome_packet_free(&decoded);
}

// Test device name too long (>255 bytes)
void test_device_name_too_long(void) {
    bthome_packet_t packet;
    bthome_packet_init(&packet);
    bthome_set_device_info(&packet, false, false);
    
    char long_name[300];
    memset(long_name, 'A', sizeof(long_name));
    
    int result = bthome_set_device_name(&packet, long_name, sizeof(long_name), true);
    TEST_ASSERT_EQUAL_INT(-1, result);  // Should fail with name too long error
    
    bthome_packet_free(&packet);
}

// Test case group for running all tests together
TEST_CASE("BTHome: All tests", "[bthome]") {
    printf("=== Running BTHome tests ===\n");
    printf("Test: decode example payload\n");
    test_decode_example_payload();
    printf("Test: decode service data only\n");
    test_decode_service_data_only();
    printf("Test: encode decode round trip\n");
    test_encode_decode_round_trip();
    printf("Test: various sensor types\n");
    test_various_sensor_types();
    printf("Test: binary sensors\n");
    test_binary_sensors();
    printf("Test: button events\n");
    test_button_events();
    printf("Test: dimmer events\n");
    test_dimmer_events();
    printf("Test: packet id\n");
    test_packet_id();
    printf("Test: text sensor\n");
    test_text_sensor();
    printf("Test: raw sensor\n");
    test_raw_sensor();
    printf("Test: device info flags\n");
    test_device_info_flags();
    printf("Test: scaling factors\n");
    test_scaling_factors();
    printf("Test: invalid uuid\n");
    test_invalid_uuid();
    printf("Test: empty packet\n");
    test_empty_packet();
    printf("Test: device name complete\n");
    test_device_name_complete();
    printf("Test: device name shortened\n");
    test_device_name_shortened();
    printf("Test: device name UTF-8\n");
    test_device_name_utf8();
    printf("Test: no device name\n");
    test_no_device_name();
    printf("Test: device name too long\n");
    test_device_name_too_long();
    printf("=== All BTHome tests completed ===\n");
}

// Individual test cases that can be run separately
TEST_CASE("BTHome: decode example payload from docs", "[bthome]") {
    printf("Testing decode of example payload: 020106 0B094449592D73656E736F72 0A16D2FC4002C40903BF13\n");
    test_decode_example_payload();
}

TEST_CASE("BTHome: decode service data only", "[bthome]") {
    test_decode_service_data_only();
}

TEST_CASE("BTHome: encode decode round trip", "[bthome]") {
    test_encode_decode_round_trip();
}

TEST_CASE("BTHome: various sensor types", "[bthome]") {
    test_various_sensor_types();
}

TEST_CASE("BTHome: binary sensors", "[bthome]") {
    test_binary_sensors();
}

TEST_CASE("BTHome: button events", "[bthome]") {
    test_button_events();
}

TEST_CASE("BTHome: dimmer events", "[bthome]") {
    test_dimmer_events();
}

TEST_CASE("BTHome: packet id", "[bthome]") {
    test_packet_id();
}

TEST_CASE("BTHome: text sensor", "[bthome]") {
    test_text_sensor();
}

TEST_CASE("BTHome: raw sensor", "[bthome]") {
    test_raw_sensor();
}

TEST_CASE("BTHome: device info flags", "[bthome]") {
    test_device_info_flags();
}

TEST_CASE("BTHome: scaling factors", "[bthome]") {
    test_scaling_factors();
}

TEST_CASE("BTHome: invalid uuid", "[bthome]") {
    test_invalid_uuid();
}

TEST_CASE("BTHome: empty packet", "[bthome]") {
    test_empty_packet();
}

TEST_CASE("BTHome: device name complete", "[bthome]") {
    test_device_name_complete();
}

TEST_CASE("BTHome: device name shortened", "[bthome]") {
    test_device_name_shortened();
}

TEST_CASE("BTHome: device name UTF-8", "[bthome]") {
    test_device_name_utf8();
}

TEST_CASE("BTHome: no device name", "[bthome]") {
    test_no_device_name();
}

TEST_CASE("BTHome: device name too long", "[bthome]") {
    test_device_name_too_long();
}
