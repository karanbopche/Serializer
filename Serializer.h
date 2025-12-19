#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdint.h>
#include <stddef.h>
#include <stddef.h>
#include <string.h>

/**
 * @file Serializer.h
 * @brief Serializer library for structured data serialization
 *
 * @details This library provides functionality to serialize structured data into a
 * binary frame format. The serializer frame structure enables efficient data
 * transmission and storage with metadata information.
 *
 * SERIALIZER FRAME FORMAT:
 * ========================
 *
 * +------------------+------------------+------------------+------------------+
 * |   Stream ID      |  Meta Table      |   Meta Table     |   Actual Data    |
 * |   (4 bytes)      |  Entry Count     |   Entries        |   (Variable)     |
 * |                  |   (4 bytes)      |   (Variable)     |                  |
 * +------------------+------------------+------------------+------------------+
 *
 * FRAME STRUCTURE DETAILS:
 * ------------------------
 *
 * 1. STREAM ID (4 bytes):
 *    - Unique identifier for the data stream
 *    - Used to distinguish between different serialization contexts
 *    - Range: 0x00000000 to 0xFFFFFFFF
 *
 * 2. META TABLE ENTRY COUNT (4 bytes):
 *    - Number of metadata entries in the meta table
 *    - Indicates how many fields are being serialized
 *    - Range: 0 to 0xFFFFFFFF entries
 *
 * 3. META TABLE ENTRIES (Variable length):
 *    - Each entry describes a field in the actual data
 *    - Contains field information such as:
 *      * Field name/identifier
 *      * Data type
 *      * Field size/length
 *      * Offset position
 *
 * 4. ACTUAL DATA (Variable length):
 *    - The serialized payload data
 *    - Organized according to the meta table descriptions
 *    - Size depends on the data being serialized
 *
 * EXAMPLE FRAME:
 * --------------
 * +----------+----------+------------------+------------------+
 * | StreamID | Count: 2 | Meta Entry 1     | Data Field 1     |
 * | 0x0001   | 0x0002   | (name, type,...) | (binary data)    |
 * |          |          | Meta Entry 2     | Data Field 2     |
 * |          |          | (name, type,...) | (binary data)    |
 * +----------+----------+------------------+------------------+
 */

typedef struct
{
    uint32_t type_id;
    uint16_t offset;
    uint16_t size;
} Serializer_Field_Meta_t;

typedef enum
{
    SERIALIZER_DESERIALIZER_ERROR_NONE = 0,
    SERIALIZER_DESERIALIZER_ERROR_STREAM_ID_MISMATCH,
} Deserializer_Error_t;

/* Helper macros for conditional field inclusion */
#define SERIALIZER_STRUCT_FIELD(enable, id, type, name) SERIALIZER_STRUCT_FIELD_##enable(struct_name, type, name)
#define SERIALIZER_STRUCT_FIELD_ENABLE(struct_name, type, name) type name;
#define SERIALIZER_STRUCT_FIELD_DISABLE(struct_name, type, name) /* field disabled */

/* Helper macros for conditional metadata inclusion */
#define SERIALIZER_META_FIELD(enable, id, type, name) SERIALIZER_META_FIELD_##enable(id, type, name)
#define SERIALIZER_META_FIELD_ENABLE(id, type, name) {id, offsetof(SERIALIZER_STRUCT_NAME, name), sizeof(type)},
#define SERIALIZER_META_FIELD_DISABLE(id, type, name) /* metadata disabled */

#define SERIALIZER_CREATE_STRUCT_IMPL(struct_name, struct_fields) \
    struct struct_name                                            \
    {                                                             \
        struct_fields(SERIALIZER_STRUCT_FIELD)                    \
    };

#define SERIALIZER_CREATE_TYPEDEF_STRUCT_IMPL(struct_name, struct_fields) \
    typedef struct                                                        \
    {                                                                     \
        struct_fields(SERIALIZER_STRUCT_FIELD)                            \
    } struct_name;

#define SERIALIZER_CREATE_META_IMPL(struct_name, struct_fields) \
    static const Serializer_Field_Meta_t struct_name##_meta[] = {struct_fields(SERIALIZER_META_FIELD)};

#define SERIALIZER_CREATE_SERIALIZER_IMPL(struct_name, stream_id)                                \
    static inline void Serialize_##struct_name(uint8_t *buffer, const struct_name *const obj)    \
    {                                                                                            \
        const uint32_t meta_size = sizeof(struct_name##_meta) / sizeof(Serializer_Field_Meta_t); \
        const uint32_t stream_id_le = stream_id;                                                 \
        memcpy(buffer, &stream_id_le, sizeof(uint32_t));                                         \
        buffer += sizeof(uint32_t);                                                              \
        memcpy(buffer, &meta_size, sizeof(uint32_t));                                            \
        buffer += sizeof(uint32_t);                                                              \
        memcpy(buffer, struct_name##_meta, sizeof(struct_name##_meta));                          \
        buffer += sizeof(struct_name##_meta);                                                    \
        memcpy(buffer, obj, sizeof(struct_name));                                                \
    }

#define SERIALIZER_STRUCT_GET_META_FIELD_BY_ID_IMPL(struct_name)                                                   \
    static inline const Serializer_Field_Meta_t *Serializer_Get_##struct_name##_Meta_Field_By_ID(uint32_t type_id) \
    {                                                                                                              \
        const uint32_t meta_size = sizeof(struct_name##_meta) / sizeof(Serializer_Field_Meta_t);                   \
        for (uint32_t i = 0; i < meta_size; i++)                                                                   \
            if (struct_name##_meta[i].type_id == type_id)                                                          \
                return &struct_name##_meta[i];                                                                     \
        return NULL;                                                                                               \
    }

#define SERIALIZER_CREATE_DESERIALIZER_IMPL(struct_name, stream_id)                                                                             \
    static inline Deserializer_Error_t Deserialize_##struct_name(const uint8_t *buffer, struct_name *const obj)                                 \
    {                                                                                                                                           \
        uint32_t received_stream_id;                                                                                                            \
        uint32_t received_meta_size;                                                                                                            \
        memcpy(&received_stream_id, buffer, sizeof(uint32_t));                                                                                  \
        if (received_stream_id != stream_id)                                                                                                    \
            return SERIALIZER_DESERIALIZER_ERROR_STREAM_ID_MISMATCH;                                                                            \
        buffer += sizeof(uint32_t);                                                                                                             \
        memcpy(&received_meta_size, buffer, sizeof(uint32_t));                                                                                  \
        buffer += sizeof(uint32_t);                                                                                                             \
        uint8_t const *meta_field_ptr = buffer;                                                                                                 \
        uint8_t const *received_obj_ptr = buffer + (sizeof(Serializer_Field_Meta_t) * received_meta_size);                                      \
        for (uint32_t i = 0; i < received_meta_size; i++)                                                                                       \
        {                                                                                                                                       \
            Serializer_Field_Meta_t received_meta_field;                                                                                        \
            memcpy(&received_meta_field, meta_field_ptr, sizeof(Serializer_Field_Meta_t));                                                      \
            const Serializer_Field_Meta_t *const our_meta_field = Serializer_Get_##struct_name##_Meta_Field_By_ID(received_meta_field.type_id); \
            if (our_meta_field != NULL)                                                                                                         \
                memcpy((uint8_t *)obj + our_meta_field->offset, received_obj_ptr + received_meta_field.offset, our_meta_field->size);           \
            meta_field_ptr += sizeof(Serializer_Field_Meta_t);                                                                                  \
        }                                                                                                                                       \
        return SERIALIZER_DESERIALIZER_ERROR_NONE;                                                                                              \
    }

#define SERIALIZER_CREATE_STRUCT(struct_name, struct_fields, stream_id) \
    SERIALIZER_CREATE_META_IMPL(struct_name, struct_fields)             \
    SERIALIZER_CREATE_STRUCT_IMPL(struct_name, struct_fields)           \
    SERIALIZER_CREATE_SERIALIZER_IMPL(struct_name, stream_id)           \
    SERIALIZER_STRUCT_GET_META_FIELD_BY_ID_IMPL(struct_name)            \
    SERIALIZER_CREATE_DESERIALIZER_IMPL(struct_name, stream_id)

#define SERIALIZER_CREATE_TYPEDEF_STRUCT(struct_name, struct_fields, stream_id) \
    SERIALIZER_CREATE_TYPEDEF_STRUCT_IMPL(struct_name, struct_fields)           \
    SERIALIZER_CREATE_META_IMPL(struct_name, struct_fields)                     \
    SERIALIZER_CREATE_SERIALIZER_IMPL(struct_name, stream_id)                   \
    SERIALIZER_STRUCT_GET_META_FIELD_BY_ID_IMPL(struct_name)                    \
    SERIALIZER_CREATE_DESERIALIZER_IMPL(struct_name, stream_id)

#endif // SERIALIZER_H
