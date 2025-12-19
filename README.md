# C Serializer Library

A lightweight, macro-based C library for efficient structured data serialization with metadata support.

## Overview

This library provides a robust framework for serializing C structures into a binary format with embedded metadata. It's designed for applications that need efficient data transmission, storage, or inter-process communication with version compatibility and field flexibility.

## Key Features

- **Macro-based Code Generation**: Automatically generates serialization/deserialization functions
- **Metadata-Driven**: Self-describing binary format with embedded field information
- **Conditional Fields**: Enable/disable fields at compile-time without breaking compatibility
- **Stream Identification**: Multiple data stream support with unique IDs
- **Type Safety**: Compile-time type checking and field validation
- **Efficient Binary Format**: Compact representation with minimal overhead
- **Cross-Platform**: Standard C99 compatible

## Frame Format

The serializer uses a structured binary frame format:

```
+------------------+------------------+------------------+------------------+
|   Stream ID      |  Meta Table      |   Meta Table     |   Actual Data    |
|   (4 bytes)      |  Entry Count     |   Entries        |   (Variable)     |
|                  |   (4 bytes)      |   (Variable)     |                  |
+------------------+------------------+------------------+------------------+
```

### Frame Components

1. **Stream ID (4 bytes)**: Unique identifier for the data stream
2. **Meta Table Entry Count (4 bytes)**: Number of field metadata entries
3. **Meta Table Entries (Variable)**: Field descriptions containing:
   - Field ID/Type identifier
   - Offset within the data
   - Field size in bytes
4. **Actual Data (Variable)**: Serialized structure data

## Quick Start

### 1. Define Your Structure

```c
#include "serializer.h"

#define SERIALIZER_STRUCT_NAME MyStruct_t
#define STREAM_FIELDS(X)              \
  X(ENABLE,  1, int32_t, field1)    \
  X(ENABLE,  2, float,   field2)    \
  X(DISABLE, 3, double,  field3)    \
  X(ENABLE,  4, char, field4)

SERIALIZER_CREATE_TYPEDEF_STRUCT(SERIALIZER_STRUCT_NAME, STREAM_FIELDS, 0x1001);

#undef SERIALIZER_STRUCT_NAME
#undef STREAM_FIELDS

#define SERIALIZER_STRUCT_NAME MyAnotherStruct_t
typedef char char_array_10[10];
#define STREAM_FIELDS(X)              \
  X(ENABLE,  1, int32_t, field1)    \
  X(DISABLE, 3, double,  field3)    \
  X(ENABLE,  4, char_array_10, field4)

SERIALIZER_CREATE_TYPEDEF_STRUCT(SERIALIZER_STRUCT_NAME, STREAM_FIELDS, 0x1002);

#undef SERIALIZER_STRUCT_NAME
#undef STREAM_FIELDS
```

### 2. Use Serialization

```c
#include <stdio.h>

int main() {
    MyStruct_t data = {
        .field1 = 42,
        .field2 = 3.14f,
        .field4 = "Hello World"
    };
    
    uint8_t buffer[256];
    Serialize_MyStruct_t(buffer, &data);
    
    MyStruct_t restored = {0};
    Deserializer_Error_t result = Deserialize_MyStruct_t(buffer, &restored);
    
    if (result == SERIALIZER_DESERIALIZER_ERROR_NONE) {
        printf("Deserialization successful!\n");
        printf("field1: %d\n", restored.field1);
        printf("field2: %f\n", restored.field2);
        printf("field4: %s\n", restored.field4);
    }
    
    return 0;
}
```

## Building

### Prerequisites
- GCC or MinGW compiler
- Make utility

### Compile

```bash
# Using the provided Makefile
mingw32-make all

# Or manually with GCC
gcc -o serializer_test serializer.c -Wall -Wextra -g
```

### Run Example

```bash
./serializer_test.exe
```

## API Reference

### Core Macros

#### `SERIALIZER_CREATE_TYPEDEF_STRUCT(struct_name, fields_macro, stream_id)`
Creates a typedef struct with auto-generated serialization functions.

#### `SERIALIZER_CREATE_STRUCT(struct_name, fields_macro, stream_id)`
Creates a regular struct with auto-generated serialization functions.

#### Field Definition Macro Format
```c
#define YOUR_FIELDS(X) \
    X(ENABLE/DISABLE, field_id, type, field_name)
```

### Generated Functions

For each structure `MyStruct_t`, the library generates:

- **`void Serialize_MyStruct_t(uint8_t *buffer, const MyStruct_t *obj)`**
  - Serializes structure to binary buffer
  
- **`Deserializer_Error_t Deserialize_MyStruct_t(const uint8_t *buffer, MyStruct_t *obj)`**
  - Deserializes binary buffer to structure
  - Returns error code for validation failures

### Error Codes

```c
typedef enum {
    SERIALIZER_DESERIALIZER_ERROR_NONE = 0,
    SERIALIZER_DESERIALIZER_ERROR_STREAM_ID_MISMATCH,
} Deserializer_Error_t;
```

## Advanced Features

### Conditional Field Compilation

Fields can be enabled or disabled at compile-time:

```c
#define MY_FIELDS(X)                    \
    X(ENABLE,  1, int32_t, always_present) \
    X(DISABLE, 2, float,   optional_field)  \
    X(ENABLE,  3, char[10], debug_info)
```

Disabled fields are excluded from the struct definition and serialization but maintain compatibility through the metadata system.

### Multiple Stream Types

Different structures can use different stream IDs for identification:

```c
SERIALIZER_CREATE_TYPEDEF_STRUCT(SensorData_t, SENSOR_FIELDS, 0x2001)
SERIALIZER_CREATE_TYPEDEF_STRUCT(ConfigData_t, CONFIG_FIELDS, 0x2002)
```

### Version Compatibility

The metadata-driven approach allows structures to evolve while maintaining backward compatibility:

- New fields can be added without breaking existing data
- Field order can change without affecting deserialization
- Missing fields in older data are simply skipped

## Example Output

```
obj1 data:
Stream1_Struct_t:
  field1: 42
  field3: Hello, World!

obj2 data:
Stream1_Struct_t:
  field1: 42
  field3: Hello, World!
```

## File Structure

```
Serializer_Lib/
├── serializer.h      # Main library header with macros
├── serializer.c      # Example/test implementation
├── stream_def.h      # Structure definitions
├── makefile          # Build configuration
└── README.md         # This file
```

## License

This project is provided as-is for educational and development purposes.

## Contributing

Feel free to submit issues and enhancement requests!