#ifndef BTHOME_H
#define BTHOME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// BTHome Version
#define BTHOME_VERSION 2

// BTHome UUID (little endian: 0xFCD2)
#define BTHOME_UUID_LE 0xFCD2
#define BTHOME_UUID_BE 0xD2FC

// BTHome Device Information flags
#define BTHOME_DEVICE_INFO_ENCRYPTED        (1 << 0)
#define BTHOME_DEVICE_INFO_TRIGGER_BASED    (1 << 2)
#define BTHOME_DEVICE_INFO_VERSION_SHIFT    5
#define BTHOME_DEVICE_INFO_VERSION_MASK     (0x07 << BTHOME_DEVICE_INFO_VERSION_SHIFT)

// Object IDs for sensor data
typedef enum {
    BTHOME_SENSOR_PACKET_ID = 0x00,
    BTHOME_SENSOR_BATTERY = 0x01,
    BTHOME_SENSOR_TEMPERATURE = 0x02,
    BTHOME_SENSOR_HUMIDITY = 0x03,
    BTHOME_SENSOR_PRESSURE = 0x04,
    BTHOME_SENSOR_ILLUMINANCE = 0x05,
    BTHOME_SENSOR_MASS_KG = 0x06,
    BTHOME_SENSOR_MASS_LB = 0x07,
    BTHOME_SENSOR_DEWPOINT = 0x08,
    BTHOME_SENSOR_COUNT_UINT8 = 0x09,
    BTHOME_SENSOR_ENERGY_UINT24 = 0x0A,
    BTHOME_SENSOR_POWER_UINT24 = 0x0B,
    BTHOME_SENSOR_VOLTAGE = 0x0C,
    BTHOME_SENSOR_PM25 = 0x0D,
    BTHOME_SENSOR_PM10 = 0x0E,
    BTHOME_SENSOR_CO2 = 0x12,
    BTHOME_SENSOR_TVOC = 0x13,
    BTHOME_SENSOR_MOISTURE_UINT16 = 0x14,
    BTHOME_SENSOR_HUMIDITY_UINT8 = 0x2E,
    BTHOME_SENSOR_MOISTURE_UINT8 = 0x2F,
    BTHOME_SENSOR_COUNT_UINT16 = 0x3D,
    BTHOME_SENSOR_COUNT_UINT32 = 0x3E,
    BTHOME_SENSOR_ROTATION = 0x3F,
    BTHOME_SENSOR_DISTANCE_MM = 0x40,
    BTHOME_SENSOR_DISTANCE_M = 0x41,
    BTHOME_SENSOR_DURATION = 0x42,
    BTHOME_SENSOR_CURRENT = 0x43,
    BTHOME_SENSOR_SPEED = 0x44,
    BTHOME_SENSOR_TEMPERATURE_SINT16_1 = 0x45,
    BTHOME_SENSOR_UV_INDEX = 0x46,
    BTHOME_SENSOR_VOLUME_UINT16_1 = 0x47,
    BTHOME_SENSOR_VOLUME_ML = 0x48,
    BTHOME_SENSOR_VOLUME_FLOW_RATE = 0x49,
    BTHOME_SENSOR_VOLTAGE_1 = 0x4A,
    BTHOME_SENSOR_GAS_UINT24 = 0x4B,
    BTHOME_SENSOR_GAS_UINT32 = 0x4C,
    BTHOME_SENSOR_ENERGY_UINT32 = 0x4D,
    BTHOME_SENSOR_VOLUME_UINT32 = 0x4E,
    BTHOME_SENSOR_WATER = 0x4F,
    BTHOME_SENSOR_TIMESTAMP = 0x50,
    BTHOME_SENSOR_ACCELERATION = 0x51,
    BTHOME_SENSOR_GYROSCOPE = 0x52,
    BTHOME_SENSOR_TEXT = 0x53,
    BTHOME_SENSOR_RAW = 0x54,
    BTHOME_SENSOR_VOLUME_STORAGE = 0x55,
    BTHOME_SENSOR_CONDUCTIVITY = 0x56,
    BTHOME_SENSOR_TEMPERATURE_SINT8 = 0x57,
    BTHOME_SENSOR_TEMPERATURE_SINT8_035 = 0x58,
    BTHOME_SENSOR_COUNT_SINT8 = 0x59,
    BTHOME_SENSOR_COUNT_SINT16 = 0x5A,
    BTHOME_SENSOR_COUNT_SINT32 = 0x5B,
    BTHOME_SENSOR_POWER_SINT32 = 0x5C,
    BTHOME_SENSOR_CURRENT_SINT16 = 0x5D,
    BTHOME_SENSOR_DIRECTION = 0x5E,
    BTHOME_SENSOR_PRECIPITATION = 0x5F,
    BTHOME_SENSOR_CHANNEL = 0x60,
    BTHOME_SENSOR_ROTATIONAL_SPEED = 0x61,
} bthome_sensor_id_t;

// Object IDs for binary sensor data
typedef enum {
    BTHOME_BINARY_GENERIC_BOOLEAN = 0x0F,
    BTHOME_BINARY_POWER = 0x10,
    BTHOME_BINARY_OPENING = 0x11,
    BTHOME_BINARY_BATTERY = 0x15,
    BTHOME_BINARY_BATTERY_CHARGING = 0x16,
    BTHOME_BINARY_CO = 0x17,
    BTHOME_BINARY_COLD = 0x18,
    BTHOME_BINARY_CONNECTIVITY = 0x19,
    BTHOME_BINARY_DOOR = 0x1A,
    BTHOME_BINARY_GARAGE_DOOR = 0x1B,
    BTHOME_BINARY_GAS = 0x1C,
    BTHOME_BINARY_HEAT = 0x1D,
    BTHOME_BINARY_LIGHT = 0x1E,
    BTHOME_BINARY_LOCK = 0x1F,
    BTHOME_BINARY_MOISTURE = 0x20,
    BTHOME_BINARY_MOTION = 0x21,
    BTHOME_BINARY_MOVING = 0x22,
    BTHOME_BINARY_OCCUPANCY = 0x23,
    BTHOME_BINARY_PLUG = 0x24,
    BTHOME_BINARY_PRESENCE = 0x25,
    BTHOME_BINARY_PROBLEM = 0x26,
    BTHOME_BINARY_RUNNING = 0x27,
    BTHOME_BINARY_SAFETY = 0x28,
    BTHOME_BINARY_SMOKE = 0x29,
    BTHOME_BINARY_SOUND = 0x2A,
    BTHOME_BINARY_TAMPER = 0x2B,
    BTHOME_BINARY_VIBRATION = 0x2C,
    BTHOME_BINARY_WINDOW = 0x2D,
} bthome_binary_sensor_id_t;

// Object IDs for events
typedef enum {
    BTHOME_EVENT_BUTTON = 0x3A,
    BTHOME_EVENT_DIMMER = 0x3C,
} bthome_event_id_t;

// Button event types
typedef enum {
    BTHOME_BUTTON_NONE = 0x00,
    BTHOME_BUTTON_PRESS = 0x01,
    BTHOME_BUTTON_DOUBLE_PRESS = 0x02,
    BTHOME_BUTTON_TRIPLE_PRESS = 0x03,
    BTHOME_BUTTON_LONG_PRESS = 0x04,
    BTHOME_BUTTON_LONG_DOUBLE_PRESS = 0x05,
    BTHOME_BUTTON_LONG_TRIPLE_PRESS = 0x06,
    BTHOME_BUTTON_HOLD_PRESS = 0x80,
} bthome_button_event_t;

// Dimmer event types
typedef enum {
    BTHOME_DIMMER_NONE = 0x00,
    BTHOME_DIMMER_ROTATE_LEFT = 0x01,
    BTHOME_DIMMER_ROTATE_RIGHT = 0x02,
} bthome_dimmer_event_t;

// Device information object IDs
typedef enum {
    BTHOME_DEVICE_TYPE_ID = 0xF0,
    BTHOME_DEVICE_FIRMWARE_VERSION_UINT32 = 0xF1,
    BTHOME_DEVICE_FIRMWARE_VERSION_UINT24 = 0xF2,
} bthome_device_info_id_t;

// Data types and their sizes
typedef enum {
    BTHOME_TYPE_UINT8 = 1,
    BTHOME_TYPE_UINT16 = 2,
    BTHOME_TYPE_UINT24 = 3,
    BTHOME_TYPE_UINT32 = 4,
    BTHOME_TYPE_SINT8 = 1,
    BTHOME_TYPE_SINT16 = 2,
    BTHOME_TYPE_SINT24 = 3,
    BTHOME_TYPE_SINT32 = 4,
} bthome_data_type_t;

// Measurement value union
typedef union {
    uint8_t uint8_val;
    uint16_t uint16_val;
    uint32_t uint32_val;
    int8_t sint8_val;
    int16_t sint16_val;
    int32_t sint32_val;
    float float_val;
    struct {
        const uint8_t *data;
        size_t len;
    } bytes_val;
} bthome_value_t;

// Measurement structure
typedef struct {
    uint8_t object_id;
    bthome_value_t value;
    bool is_signed;
    uint8_t size;  // in bytes
} bthome_measurement_t;

// Event structure
typedef struct {
    uint8_t event_type;  // BTHOME_EVENT_BUTTON or BTHOME_EVENT_DIMMER
    uint8_t event_value;
    uint8_t steps;  // for dimmer events
} bthome_event_t;

// Device info structure
typedef struct {
    bool encrypted;
    bool trigger_based;
    uint8_t version;
} bthome_device_info_t;

// BTHome packet structure
typedef struct {
    bthome_device_info_t device_info;
    bthome_measurement_t *measurements;
    size_t measurement_count;
    bthome_event_t *events;
    size_t event_count;
    uint8_t packet_id;
    bool has_packet_id;
    const char *device_name;
    size_t device_name_len;
    bool use_complete_name;  // true = 0x09 (complete), false = 0x08 (shortened)
    bool owns_data;  // true if packet owns and should free device_name and text/raw data buffers
} bthome_packet_t;

// Encoder functions

/**
 * Initialize a BTHome packet structure
 */
void bthome_packet_init(bthome_packet_t *packet);

/**
 * Free resources allocated for a BTHome packet
 */
void bthome_packet_free(bthome_packet_t *packet);

/**
 * Deep copy a BTHome packet
 * Allocates new memory for all dynamic data (measurements, events, device name, text/raw data)
 * so the copy remains valid even after the source packet is freed
 * @param dest Destination packet (must be initialized)
 * @param src Source packet to copy from
 * @return 0 on success, negative error code on failure (out of memory)
 */
int bthome_packet_copy(bthome_packet_t *dest, const bthome_packet_t *src);

/**
 * Set device information in packet
 */
void bthome_set_device_info(bthome_packet_t *packet, bool encrypted, bool trigger_based);

/**
 * Add a uint8 sensor measurement to packet
 */
int bthome_add_sensor_uint8(bthome_packet_t *packet, uint8_t object_id, uint8_t value);

/**
 * Add a uint16 sensor measurement to packet
 */
int bthome_add_sensor_uint16(bthome_packet_t *packet, uint8_t object_id, uint16_t value);

/**
 * Add a uint24 sensor measurement to packet
 */
int bthome_add_sensor_uint24(bthome_packet_t *packet, uint8_t object_id, uint32_t value);

/**
 * Add a uint32 sensor measurement to packet
 */
int bthome_add_sensor_uint32(bthome_packet_t *packet, uint8_t object_id, uint32_t value);

/**
 * Add a sint8 sensor measurement to packet
 */
int bthome_add_sensor_sint8(bthome_packet_t *packet, uint8_t object_id, int8_t value);

/**
 * Add a sint16 sensor measurement to packet
 */
int bthome_add_sensor_sint16(bthome_packet_t *packet, uint8_t object_id, int16_t value);

/**
 * Add a sint32 sensor measurement to packet
 */
int bthome_add_sensor_sint32(bthome_packet_t *packet, uint8_t object_id, int32_t value);

/**
 * Add a binary sensor measurement to packet
 */
int bthome_add_binary_sensor(bthome_packet_t *packet, uint8_t object_id, bool value);

/**
 * Add a text sensor to packet
 */
int bthome_add_sensor_text(bthome_packet_t *packet, const char *text, size_t len);

/**
 * Add raw sensor data to packet
 */
int bthome_add_sensor_raw(bthome_packet_t *packet, const uint8_t *data, size_t len);

/**
 * Add a button event to packet
 */
int bthome_add_button_event(bthome_packet_t *packet, bthome_button_event_t event);

/**
 * Add a dimmer event to packet
 */
int bthome_add_dimmer_event(bthome_packet_t *packet, bthome_dimmer_event_t event, uint8_t steps);

/**
 * Set packet ID
 */
void bthome_set_packet_id(bthome_packet_t *packet, uint8_t packet_id);

/**
 * Set device name (local name)
 * @param packet The packet to set the name on
 * @param name The device name (UTF-8 encoded string)
 * @param len Length of the name (max 255 bytes)
 * @param complete true for Complete Local Name (0x09), false for Shortened Local Name (0x08)
 * @return 0 on success, negative error code on failure
 */
int bthome_set_device_name(bthome_packet_t *packet, const char *name, size_t len, bool complete);

/**
 * Encode a BTHome packet into service data payload
 * @param packet The packet to encode
 * @param buffer Output buffer for the encoded data
 * @param buffer_size Size of the output buffer
 * @return Number of bytes written, or negative error code
 */
int bthome_encode(const bthome_packet_t *packet, uint8_t *buffer, size_t buffer_size);

/**
 * Encode BTHome service data with flags AD element
 * @param packet The packet to encode
 * @param buffer Output buffer for the complete advertising data
 * @param buffer_size Size of the output buffer
 * @param include_flags Include the flags AD element
 * @return Number of bytes written, or negative error code
 */
int bthome_encode_advertisement(const bthome_packet_t *packet, uint8_t *buffer, 
                                 size_t buffer_size, bool include_flags);

// Decoder functions

/**
 * Decode BTHome service data
 * @param data The service data payload (starting with UUID)
 * @param len Length of the service data
 * @param packet Output packet structure
 * @return 0 on success, negative error code on failure
 */
int bthome_decode(const uint8_t *data, size_t len, bthome_packet_t *packet);

/**
 * Decode BTHome advertisement data (including AD elements)
 * @param data The complete advertising data
 * @param len Length of the advertising data
 * @param packet Output packet structure
 * @return 0 on success, negative error code on failure
 */
int bthome_decode_advertisement(const uint8_t *data, size_t len, bthome_packet_t *packet);

/**
 * Get the scaled float value for a measurement based on its factor
 * @param measurement The measurement to get the value for
 * @param factor The scaling factor for this measurement type
 * @return The scaled float value
 */
float bthome_get_scaled_value(const bthome_measurement_t *measurement, float factor);

// Helper functions

/**
 * Get the size in bytes for an object ID
 * Returns 0 for variable-length objects (text, raw)
 */
uint8_t bthome_get_object_size(uint8_t object_id);

/**
 * Check if an object ID is a binary sensor
 */
bool bthome_is_binary_sensor(uint8_t object_id);

/**
 * Check if an object ID is an event
 */
bool bthome_is_event(uint8_t object_id);

/**
 * Get the scaling factor for a sensor object ID
 * Returns 1.0 for objects without scaling
 */
float bthome_get_scaling_factor(uint8_t object_id);

/**
 * Get the name string for an object ID
 * Returns NULL for unknown object IDs
 */
const char* bthome_get_object_name(uint8_t object_id);

/**
 * Get the unit string for an object ID
 * Returns NULL for unknown object IDs, empty string for dimensionless objects
 */
const char* bthome_get_object_unit(uint8_t object_id);

/**
 * Get the ASCII text description of the unit for an object ID
 * Returns NULL for unknown object IDs, empty string for dimensionless objects
 */
const char* bthome_get_object_unit_description(uint8_t object_id);

#ifdef __cplusplus
}
#endif

#endif // BTHOME_H
