#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bthome.h"

// Object size lookup table (0 = variable length, requires length byte)
static const uint8_t object_sizes[] = {
    [0x00] = 1,   // packet_id
    [0x01] = 1,   // battery
    [0x02] = 2,   // temperature
    [0x03] = 2,   // humidity
    [0x04] = 3,   // pressure
    [0x05] = 3,   // illuminance
    [0x06] = 2,   // mass (kg)
    [0x07] = 2,   // mass (lb)
    [0x08] = 2,   // dewpoint
    [0x09] = 1,   // count
    [0x0A] = 3,   // energy
    [0x0B] = 3,   // power
    [0x0C] = 2,   // voltage
    [0x0D] = 2,   // pm2.5
    [0x0E] = 2,   // pm10
    [0x0F] = 1,   // generic boolean
    [0x10] = 1,   // power (binary)
    [0x11] = 1,   // opening
    [0x12] = 2,   // co2
    [0x13] = 2,   // tvoc
    [0x14] = 2,   // moisture
    [0x15] = 1,   // battery (binary)
    [0x16] = 1,   // battery charging
    [0x17] = 1,   // co
    [0x18] = 1,   // cold
    [0x19] = 1,   // connectivity
    [0x1A] = 1,   // door
    [0x1B] = 1,   // garage door
    [0x1C] = 1,   // gas (binary)
    [0x1D] = 1,   // heat
    [0x1E] = 1,   // light
    [0x1F] = 1,   // lock
    [0x20] = 1,   // moisture (binary)
    [0x21] = 1,   // motion
    [0x22] = 1,   // moving
    [0x23] = 1,   // occupancy
    [0x24] = 1,   // plug
    [0x25] = 1,   // presence
    [0x26] = 1,   // problem
    [0x27] = 1,   // running
    [0x28] = 1,   // safety
    [0x29] = 1,   // smoke
    [0x2A] = 1,   // sound
    [0x2B] = 1,   // tamper
    [0x2C] = 1,   // vibration
    [0x2D] = 1,   // window
    [0x2E] = 1,   // humidity (uint8)
    [0x2F] = 1,   // moisture (uint8)
    [0x3A] = 1,   // button
    [0x3C] = 2,   // dimmer
    [0x3D] = 2,   // count (uint16)
    [0x3E] = 4,   // count (uint32)
    [0x3F] = 2,   // rotation
    [0x40] = 2,   // distance (mm)
    [0x41] = 2,   // distance (m)
    [0x42] = 3,   // duration
    [0x43] = 2,   // current
    [0x44] = 2,   // speed
    [0x45] = 2,   // temperature (0.1)
    [0x46] = 1,   // uv index
    [0x47] = 2,   // volume (L)
    [0x48] = 2,   // volume (mL)
    [0x49] = 2,   // volume flow rate
    [0x4A] = 2,   // voltage (0.1V)
    [0x4B] = 3,   // gas
    [0x4C] = 4,   // gas (uint32)
    [0x4D] = 4,   // energy (uint32)
    [0x4E] = 4,   // volume (uint32)
    [0x4F] = 4,   // water
    [0x50] = 4,   // timestamp
    [0x51] = 2,   // acceleration
    [0x52] = 2,   // gyroscope
    [0x53] = 0,   // text (variable)
    [0x54] = 0,   // raw (variable)
    [0x55] = 4,   // volume storage
    [0x56] = 2,   // conductivity
    [0x57] = 1,   // temperature (sint8)
    [0x58] = 1,   // temperature (sint8 0.35)
    [0x59] = 1,   // count (sint8)
    [0x5A] = 2,   // count (sint16)
    [0x5B] = 4,   // count (sint32)
    [0x5C] = 4,   // power (sint32)
    [0x5D] = 2,   // current (sint16)
    [0x5E] = 2,   // direction
    [0x5F] = 2,   // precipitation
    [0x60] = 1,   // channel
    [0x61] = 2,   // rotational speed
    [0xF0] = 2,   // device type id
    [0xF1] = 4,   // firmware version (uint32)
    [0xF2] = 3,   // firmware version (uint24)
};

// Scaling factors for sensors
static const float scaling_factors[] = {
    [0x02] = 0.01f,   // temperature
    [0x03] = 0.01f,   // humidity
    [0x04] = 0.01f,   // pressure
    [0x05] = 0.01f,   // illuminance
    [0x06] = 0.01f,   // mass (kg)
    [0x07] = 0.01f,   // mass (lb)
    [0x08] = 0.01f,   // dewpoint
    [0x0A] = 0.001f,  // energy (uint24)
    [0x0B] = 0.01f,   // power (uint24)
    [0x0C] = 0.001f,  // voltage
    [0x14] = 0.01f,   // moisture
    [0x3F] = 0.1f,    // rotation
    [0x42] = 0.001f,  // duration
    [0x43] = 0.001f,  // current
    [0x44] = 0.01f,   // speed
    [0x45] = 0.1f,    // temperature (0.1)
    [0x46] = 0.1f,    // uv index
    [0x47] = 0.1f,    // volume (L)
    [0x49] = 0.001f,  // volume flow rate
    [0x4A] = 0.1f,    // voltage (0.1V)
    [0x4B] = 0.001f,  // gas (uint24)
    [0x4C] = 0.001f,  // gas (uint32)
    [0x4D] = 0.001f,  // energy (uint32)
    [0x4E] = 0.001f,  // volume (uint32)
    [0x4F] = 0.001f,  // water
    [0x51] = 0.001f,  // acceleration
    [0x52] = 0.001f,  // gyroscope
    [0x55] = 0.001f,  // volume storage
    [0x58] = 0.35f,   // temperature (sint8 0.35)
    [0x5C] = 0.01f,   // power (sint32)
    [0x5D] = 0.001f,  // current (sint16)
    [0x5E] = 0.01f,   // direction
    [0x5F] = 0.1f,    // precipitation
};

// Helper functions

static uint16_t read_uint16_le(const uint8_t *data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint32_t read_uint24_le(const uint8_t *data) {
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16);
}

static uint32_t read_uint32_le(const uint8_t *data) {
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | 
           ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

static void write_uint16_le(uint8_t *data, uint16_t value) {
    data[0] = value & 0xFF;
    data[1] = (value >> 8) & 0xFF;
}

static void write_uint24_le(uint8_t *data, uint32_t value) {
    data[0] = value & 0xFF;
    data[1] = (value >> 8) & 0xFF;
    data[2] = (value >> 16) & 0xFF;
}

static void write_uint32_le(uint8_t *data, uint32_t value) {
    data[0] = value & 0xFF;
    data[1] = (value >> 8) & 0xFF;
    data[2] = (value >> 16) & 0xFF;
    data[3] = (value >> 24) & 0xFF;
}

static int16_t read_sint16_le(const uint8_t *data) {
    return (int16_t)read_uint16_le(data);
}

static int32_t read_sint32_le(const uint8_t *data) {
    return (int32_t)read_uint32_le(data);
}

static void write_sint16_le(uint8_t *data, int16_t value) {
    write_uint16_le(data, (uint16_t)value);
}

static void write_sint32_le(uint8_t *data, int32_t value) {
    write_uint32_le(data, (uint32_t)value);
}

uint8_t bthome_get_object_size(uint8_t object_id) {
    if (object_id < sizeof(object_sizes)) {
        return object_sizes[object_id];
    }
    return 0;
}

bool bthome_is_binary_sensor(uint8_t object_id) {
    return (object_id >= 0x0F && object_id <= 0x2D) || object_id == 0x10 || object_id == 0x11;
}

bool bthome_is_event(uint8_t object_id) {
    return object_id == 0x3A || object_id == 0x3C;
}

float bthome_get_scaling_factor(uint8_t object_id) {
    if (object_id < sizeof(scaling_factors) / sizeof(scaling_factors[0])) {
        float factor = scaling_factors[object_id];
        return factor == 0.0f ? 1.0f : factor;
    }
    return 1.0f;
}

float bthome_get_scaled_value(const bthome_measurement_t *measurement, float factor) {
    if (measurement->is_signed) {
        switch (measurement->size) {
            case 1:
                return measurement->value.sint8_val * factor;
            case 2:
                return measurement->value.sint16_val * factor;
            case 4:
                return measurement->value.sint32_val * factor;
        }
    } else {
        switch (measurement->size) {
            case 1:
                return measurement->value.uint8_val * factor;
            case 2:
                return measurement->value.uint16_val * factor;
            case 3:
            case 4:
                return measurement->value.uint32_val * factor;
        }
    }
    return 0.0f;
}

// Packet management

void bthome_packet_init(bthome_packet_t *packet) {
    memset(packet, 0, sizeof(bthome_packet_t));
    packet->device_info.version = BTHOME_VERSION;
    packet->measurements = NULL;
    packet->events = NULL;
    packet->measurement_count = 0;
    packet->event_count = 0;
    packet->has_packet_id = false;
}

void bthome_packet_free(bthome_packet_t *packet) {
    if (packet->measurements) {
        free(packet->measurements);
        packet->measurements = NULL;
    }
    if (packet->events) {
        free(packet->events);
        packet->events = NULL;
    }
    packet->measurement_count = 0;
    packet->event_count = 0;
}

void bthome_set_device_info(bthome_packet_t *packet, bool encrypted, bool trigger_based) {
    packet->device_info.encrypted = encrypted;
    packet->device_info.trigger_based = trigger_based;
    packet->device_info.version = BTHOME_VERSION;
}

void bthome_set_packet_id(bthome_packet_t *packet, uint8_t packet_id) {
    packet->packet_id = packet_id;
    packet->has_packet_id = true;
}

// Add measurement helper
static int add_measurement(bthome_packet_t *packet, uint8_t object_id, 
                          bthome_value_t value, bool is_signed, uint8_t size) {
    size_t new_size = (packet->measurement_count + 1) * sizeof(bthome_measurement_t);
    bthome_measurement_t *new_measurements = realloc(packet->measurements, new_size);
    if (!new_measurements) {
        return -1;  // Out of memory
    }
    
    packet->measurements = new_measurements;
    packet->measurements[packet->measurement_count].object_id = object_id;
    packet->measurements[packet->measurement_count].value = value;
    packet->measurements[packet->measurement_count].is_signed = is_signed;
    packet->measurements[packet->measurement_count].size = size;
    packet->measurement_count++;
    
    return 0;
}

int bthome_add_sensor_uint8(bthome_packet_t *packet, uint8_t object_id, uint8_t value) {
    bthome_value_t val = { .uint8_val = value };
    return add_measurement(packet, object_id, val, false, 1);
}

int bthome_add_sensor_uint16(bthome_packet_t *packet, uint8_t object_id, uint16_t value) {
    bthome_value_t val = { .uint16_val = value };
    return add_measurement(packet, object_id, val, false, 2);
}

int bthome_add_sensor_uint24(bthome_packet_t *packet, uint8_t object_id, uint32_t value) {
    bthome_value_t val = { .uint32_val = value };
    return add_measurement(packet, object_id, val, false, 3);
}

int bthome_add_sensor_uint32(bthome_packet_t *packet, uint8_t object_id, uint32_t value) {
    bthome_value_t val = { .uint32_val = value };
    return add_measurement(packet, object_id, val, false, 4);
}

int bthome_add_sensor_sint8(bthome_packet_t *packet, uint8_t object_id, int8_t value) {
    bthome_value_t val = { .sint8_val = value };
    return add_measurement(packet, object_id, val, true, 1);
}

int bthome_add_sensor_sint16(bthome_packet_t *packet, uint8_t object_id, int16_t value) {
    bthome_value_t val = { .sint16_val = value };
    return add_measurement(packet, object_id, val, true, 2);
}

int bthome_add_sensor_sint32(bthome_packet_t *packet, uint8_t object_id, int32_t value) {
    bthome_value_t val = { .sint32_val = value };
    return add_measurement(packet, object_id, val, true, 4);
}

int bthome_add_binary_sensor(bthome_packet_t *packet, uint8_t object_id, bool value) {
    return bthome_add_sensor_uint8(packet, object_id, value ? 1 : 0);
}

int bthome_add_sensor_text(bthome_packet_t *packet, const char *text, size_t len) {
    if (len > 255) {
        return -2;  // Text too long
    }
    
    bthome_value_t val;
    val.bytes_val.data = (const uint8_t *)text;
    val.bytes_val.len = len;
    return add_measurement(packet, BTHOME_SENSOR_TEXT, val, false, 0);
}

int bthome_add_sensor_raw(bthome_packet_t *packet, const uint8_t *data, size_t len) {
    if (len > 255) {
        return -2;  // Data too long
    }
    
    bthome_value_t val;
    val.bytes_val.data = data;
    val.bytes_val.len = len;
    return add_measurement(packet, BTHOME_SENSOR_RAW, val, false, 0);
}

int bthome_add_button_event(bthome_packet_t *packet, bthome_button_event_t event) {
    size_t new_size = (packet->event_count + 1) * sizeof(bthome_event_t);
    bthome_event_t *new_events = realloc(packet->events, new_size);
    if (!new_events) {
        return -1;  // Out of memory
    }
    
    packet->events = new_events;
    packet->events[packet->event_count].event_type = BTHOME_EVENT_BUTTON;
    packet->events[packet->event_count].event_value = event;
    packet->events[packet->event_count].steps = 0;
    packet->event_count++;
    
    return 0;
}

int bthome_add_dimmer_event(bthome_packet_t *packet, bthome_dimmer_event_t event, uint8_t steps) {
    size_t new_size = (packet->event_count + 1) * sizeof(bthome_event_t);
    bthome_event_t *new_events = realloc(packet->events, new_size);
    if (!new_events) {
        return -1;  // Out of memory
    }
    
    packet->events = new_events;
    packet->events[packet->event_count].event_type = BTHOME_EVENT_DIMMER;
    packet->events[packet->event_count].event_value = event;
    packet->events[packet->event_count].steps = steps;
    packet->event_count++;
    
    return 0;
}

// Encoding functions

int bthome_encode(const bthome_packet_t *packet, uint8_t *buffer, size_t buffer_size) {
    if (buffer_size < 3) {
        return -1;  // Buffer too small
    }
    
    size_t offset = 0;
    
    // Write UUID (little endian)
    write_uint16_le(buffer + offset, BTHOME_UUID_LE);
    offset += 2;
    
    // Write device info byte
    uint8_t device_info = 0;
    if (packet->device_info.encrypted) {
        device_info |= BTHOME_DEVICE_INFO_ENCRYPTED;
    }
    if (packet->device_info.trigger_based) {
        device_info |= BTHOME_DEVICE_INFO_TRIGGER_BASED;
    }
    device_info |= (packet->device_info.version << BTHOME_DEVICE_INFO_VERSION_SHIFT) & 
                   BTHOME_DEVICE_INFO_VERSION_MASK;
    buffer[offset++] = device_info;
    
    // Write packet ID if present
    if (packet->has_packet_id) {
        if (offset + 2 > buffer_size) return -1;
        buffer[offset++] = BTHOME_SENSOR_PACKET_ID;
        buffer[offset++] = packet->packet_id;
    }
    
    // Write measurements
    for (size_t i = 0; i < packet->measurement_count; i++) {
        const bthome_measurement_t *m = &packet->measurements[i];
        
        if (offset + 1 > buffer_size) return -1;
        buffer[offset++] = m->object_id;
        
        // Variable length data (text, raw)
        if (m->size == 0) {
            if (offset + 1 + m->value.bytes_val.len > buffer_size) return -1;
            buffer[offset++] = m->value.bytes_val.len;
            memcpy(buffer + offset, m->value.bytes_val.data, m->value.bytes_val.len);
            offset += m->value.bytes_val.len;
            continue;
        }
        
        // Fixed length data
        if (offset + m->size > buffer_size) return -1;
        
        if (m->is_signed) {
            switch (m->size) {
                case 1:
                    buffer[offset++] = (uint8_t)m->value.sint8_val;
                    break;
                case 2:
                    write_sint16_le(buffer + offset, m->value.sint16_val);
                    offset += 2;
                    break;
                case 4:
                    write_sint32_le(buffer + offset, m->value.sint32_val);
                    offset += 4;
                    break;
            }
        } else {
            switch (m->size) {
                case 1:
                    buffer[offset++] = m->value.uint8_val;
                    break;
                case 2:
                    write_uint16_le(buffer + offset, m->value.uint16_val);
                    offset += 2;
                    break;
                case 3:
                    write_uint24_le(buffer + offset, m->value.uint32_val);
                    offset += 3;
                    break;
                case 4:
                    write_uint32_le(buffer + offset, m->value.uint32_val);
                    offset += 4;
                    break;
            }
        }
    }
    
    // Write events
    for (size_t i = 0; i < packet->event_count; i++) {
        const bthome_event_t *e = &packet->events[i];
        
        if (offset + 2 > buffer_size) return -1;
        buffer[offset++] = e->event_type;
        buffer[offset++] = e->event_value;
        
        if (e->event_type == BTHOME_EVENT_DIMMER && e->event_value != BTHOME_DIMMER_NONE) {
            if (offset + 1 > buffer_size) return -1;
            buffer[offset++] = e->steps;
        }
    }
    
    return offset;
}

int bthome_encode_advertisement(const bthome_packet_t *packet, uint8_t *buffer, 
                                 size_t buffer_size, bool include_flags) {
    size_t offset = 0;
    
    // Add flags AD element
    if (include_flags) {
        if (offset + 3 > buffer_size) return -1;
        buffer[offset++] = 0x02;  // Length
        buffer[offset++] = 0x01;  // Flags
        buffer[offset++] = 0x06;  // LE General Discoverable Mode, BR/EDR Not Supported
    }
    
    // Encode service data
    if (offset + 2 > buffer_size) return -1;
    size_t service_data_len_offset = offset;
    offset++;  // Reserve space for length
    buffer[offset++] = 0x16;  // Service Data - 16-bit UUID
    
    int service_data_len = bthome_encode(packet, buffer + offset, buffer_size - offset);
    if (service_data_len < 0) {
        return service_data_len;
    }
    
    // Write service data length (UUID + device info + data - excluding length and type bytes)
    buffer[service_data_len_offset] = service_data_len + 1;  // +1 for the type byte
    
    return offset + service_data_len;
}

// Decoding functions

int bthome_decode(const uint8_t *data, size_t len, bthome_packet_t *packet) {
    if (len < 3) {
        return -1;  // Data too short
    }
    
    bthome_packet_init(packet);
    
    size_t offset = 0;
    
    // Read and verify UUID
    uint16_t uuid = read_uint16_le(data + offset);
    if (uuid != BTHOME_UUID_LE) {
        return -2;  // Invalid UUID
    }
    offset += 2;
    
    // Read device info
    uint8_t device_info = data[offset++];
    packet->device_info.encrypted = (device_info & BTHOME_DEVICE_INFO_ENCRYPTED) != 0;
    packet->device_info.trigger_based = (device_info & BTHOME_DEVICE_INFO_TRIGGER_BASED) != 0;
    packet->device_info.version = (device_info & BTHOME_DEVICE_INFO_VERSION_MASK) >> 
                                   BTHOME_DEVICE_INFO_VERSION_SHIFT;
    
    if (packet->device_info.encrypted) {
        return -3;  // Encrypted data not supported in this decoder
    }
    
    // Parse measurements and events
    while (offset < len) {
        uint8_t object_id = data[offset++];
        
        if (offset >= len) {
            return -4;  // Incomplete data
        }
        
        // Handle packet ID
        if (object_id == BTHOME_SENSOR_PACKET_ID) {
            packet->packet_id = data[offset++];
            packet->has_packet_id = true;
            continue;
        }
        
        // Handle events
        if (bthome_is_event(object_id)) {
            uint8_t event_value = data[offset++];
            
            bthome_event_t *new_events = realloc(packet->events, 
                                                  (packet->event_count + 1) * sizeof(bthome_event_t));
            if (!new_events) {
                bthome_packet_free(packet);
                return -5;  // Out of memory
            }
            
            packet->events = new_events;
            packet->events[packet->event_count].event_type = object_id;
            packet->events[packet->event_count].event_value = event_value;
            
            if (object_id == BTHOME_EVENT_DIMMER && event_value != BTHOME_DIMMER_NONE) {
                if (offset >= len) {
                    bthome_packet_free(packet);
                    return -4;
                }
                packet->events[packet->event_count].steps = data[offset++];
            } else {
                packet->events[packet->event_count].steps = 0;
            }
            
            packet->event_count++;
            continue;
        }
        
        // Handle measurements
        uint8_t size = bthome_get_object_size(object_id);
        
        // Variable length (text, raw)
        if (size == 0) {
            if (offset >= len) {
                bthome_packet_free(packet);
                return -4;
            }
            size = data[offset++];
        }
        
        if (offset + size > len) {
            bthome_packet_free(packet);
            return -4;  // Incomplete data
        }
        
        bthome_measurement_t *new_measurements = realloc(packet->measurements,
                                                         (packet->measurement_count + 1) * sizeof(bthome_measurement_t));
        if (!new_measurements) {
            bthome_packet_free(packet);
            return -5;  // Out of memory
        }
        
        packet->measurements = new_measurements;
        bthome_measurement_t *m = &packet->measurements[packet->measurement_count];
        m->object_id = object_id;
        m->size = size;
        
        // Determine if signed based on object ID
        bool is_signed = (object_id == 0x02 || object_id == 0x08 || 
                         object_id == 0x3F || object_id == 0x45 ||
                         (object_id >= 0x57 && object_id <= 0x5D));
        m->is_signed = is_signed;
        
        // Variable length data
        if (object_id == BTHOME_SENSOR_TEXT || object_id == BTHOME_SENSOR_RAW) {
            m->value.bytes_val.data = data + offset;
            m->value.bytes_val.len = size;
            offset += size;
        } else {
            // Fixed length data
            if (is_signed) {
                switch (size) {
                    case 1:
                        m->value.sint8_val = (int8_t)data[offset];
                        offset++;
                        break;
                    case 2:
                        m->value.sint16_val = read_sint16_le(data + offset);
                        offset += 2;
                        break;
                    case 4:
                        m->value.sint32_val = read_sint32_le(data + offset);
                        offset += 4;
                        break;
                }
            } else {
                switch (size) {
                    case 1:
                        m->value.uint8_val = data[offset];
                        offset++;
                        break;
                    case 2:
                        m->value.uint16_val = read_uint16_le(data + offset);
                        offset += 2;
                        break;
                    case 3:
                        m->value.uint32_val = read_uint24_le(data + offset);
                        offset += 3;
                        break;
                    case 4:
                        m->value.uint32_val = read_uint32_le(data + offset);
                        offset += 4;
                        break;
                }
            }
        }
        
        packet->measurement_count++;
    }
    
    return 0;
}

int bthome_decode_advertisement(const uint8_t *data, size_t len, bthome_packet_t *packet) {
    size_t offset = 0;
    
    while (offset < len) {
        if (offset + 2 > len) {
            return -1;  // Invalid AD structure
        }
        
        uint8_t ad_len = data[offset++];
        if (ad_len == 0) {
            continue;  // Skip empty elements
        }
        
        if (offset + ad_len > len) {
            return -1;  // AD element extends beyond data
        }
        
        uint8_t ad_type = data[offset++];
        ad_len--;  // Subtract type byte from length
        
        // Look for Service Data - 16-bit UUID
        if (ad_type == 0x16) {
            int result = bthome_decode(data + offset, ad_len, packet);
            return result;
        }
        
        offset += ad_len;
    }
    
    return -2;  // No BTHome service data found
}
