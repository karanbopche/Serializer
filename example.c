#include <stdio.h>
#include <string.h>
#include "serializer.h"


#define SERIALIZER_STRUCT_NAME Stream1_Struct_t
#define SERIALIZER_STREAM_ID 1u
typedef char uint8_array_20[20];
#define STREAM_1_FEILDS(X)        \
    X(ENABLE, 1, int32_t, field1) \
    X(DISABLE, 2, float, field2)  \
    X(ENABLE, 3, uint8_array_20, field3)

SERIALIZER_CREATE_TYPEDEF_STRUCT(SERIALIZER_STRUCT_NAME, STREAM_1_FEILDS, SERIALIZER_STREAM_ID)

#undef SERIALIZER_STRUCT_NAME


#define SERIALIZER_STRUCT_NAME Stream2_Struct_t

typedef char uint8_array_40[40];
#define STREAM_2_FEILDS(X)               \
    X(ENABLE, 1, int32_t, field1)        \
    X(DISABLE, 2, float, field2)         \
    X(ENABLE, 3, uint8_array_40, field3) \
    X(ENABLE, 4, uint8_array_20, field4)

SERIALIZER_CREATE_TYPEDEF_STRUCT(SERIALIZER_STRUCT_NAME, STREAM_2_FEILDS, 2u)

#undef SERIALIZER_STRUCT_NAME



void print_array(const uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

void print_meta_table(const Serializer_Field_Meta_t *meta, size_t count)
{
    printf("Meta Table:\n");
    for (size_t i = 0; i < count; i++)
        printf("Field ID: %u, Offset: %u, Size: %u\n", meta[i].type_id, meta[i].offset, meta[i].size);
}

void print_stream1_struct(const Stream1_Struct_t *obj)
{
    printf("Stream1_Struct_t:\n");
    printf("  field1: %d\n", obj->field1);
    printf("  field3: %s\n", obj->field3);
}

void test_serializer_deserializer()
{
    Stream1_Struct_t obj1 = {0};
    Stream1_Struct_t obj2 = {0};
    obj1.field1 = 42;
    strcpy(obj1.field3, "Hello, World!");
    uint8_t buffer[256] = {0};
    Serialize_Stream1_Struct_t(buffer, &obj1);
    printf("obj1 data:\n");
    print_stream1_struct(&obj1);


    Deserializer_Error_t err = Deserialize_Stream1_Struct_t(buffer, &obj2);
    if (err == SERIALIZER_DESERIALIZER_ERROR_NONE)
    {
        printf("obj2 data:\n");
        print_stream1_struct(&obj2);
    }
    else
    {
        printf("Deserialization failed with error code: %d\n", err);
    }
}

int main()
{
    test_serializer_deserializer();
    return 0;
}