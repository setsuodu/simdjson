#pragma once

#ifdef _WIN32
  #define SIMDJSON_UNITY_API __declspec(dllexport)
#else
  #define SIMDJSON_UNITY_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// Error codes matching simdjson roughly
typedef enum {
  SIMDJSON_OK = 0,
  SIMDJSON_ERROR_CAPACITY = 1,
  SIMDJSON_ERROR_MEMALLOC = 2,
  SIMDJSON_ERROR_TAPE_ERROR = 3,
  SIMDJSON_ERROR_DEPTH_ERROR = 4,
  SIMDJSON_ERROR_STRING_ERROR = 5,
  SIMDJSON_ERROR_T_ATOM_ERROR = 6,
  SIMDJSON_ERROR_F_ATOM_ERROR = 7,
  SIMDJSON_ERROR_N_ATOM_ERROR = 8,
  SIMDJSON_ERROR_NUMBER_ERROR = 9,
  SIMDJSON_ERROR_UTF8_ERROR = 10,
  SIMDJSON_ERROR_UNINITIALIZED = 11,
  SIMDJSON_ERROR_EMPTY = 12,
  SIMDJSON_ERROR_UNESCAPED_CHARS = 13,
  SIMDJSON_ERROR_UNEXPECTED_ERROR = 14,
  SIMDJSON_ERROR_PARSER_IN_USE = 15,
  SIMDJSON_ERROR_INSUFFICIENT_PADDING = 16,
  SIMDJSON_ERROR_INCOMPLETE_ARRAY_OR_OBJECT = 17,
  SIMDJSON_ERROR_SCALAR_DOCUMENT_AS_VALUE = 18,
  SIMDJSON_ERROR_OUT_OF_ORDER_ITERATION = 19,
  SIMDJSON_ERROR_OBJECT_ITER_ERROR = 20,
  SIMDJSON_ERROR_FIND_ERROR = 21,
  SIMDJSON_ERROR_NO_SUCH_FIELD = 22,
  SIMDJSON_ERROR_INDEX_OUT_OF_BOUNDS = 23,
  SIMDJSON_ERROR_INCORRECT_TYPE = 24,
  SIMDJSON_ERROR_NUMBER_OUT_OF_RANGE = 25,
  SIMDJSON_ERROR_INVALID = 26
} SimdJsonErrorCode;

typedef enum {
  SIMDJSON_TYPE_NULL = 0,
  SIMDJSON_TYPE_BOOL = 1,
  SIMDJSON_TYPE_NUMBER = 2,
  SIMDJSON_TYPE_STRING = 3,
  SIMDJSON_TYPE_ARRAY = 4,
  SIMDJSON_TYPE_OBJECT = 5
} SimdJsonType;

// Opaque handles
typedef void* SimdJsonParserHandle;
typedef void* SimdJsonDocumentHandle;
typedef void* SimdJsonElementHandle;

// Parser lifecycle
SIMDJSON_UNITY_API SimdJsonParserHandle SimdJson_CreateParser(void);
SIMDJSON_UNITY_API void SimdJson_DestroyParser(SimdJsonParserHandle parser);

// Parse
SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_Parse(
    SimdJsonParserHandle parser,
    const char* json,
    size_t length,
    SimdJsonDocumentHandle* out_doc);

SIMDJSON_UNITY_API void SimdJson_DestroyDocument(SimdJsonDocumentHandle doc);

// Root element
SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetRoot(
    SimdJsonDocumentHandle doc,
    SimdJsonElementHandle* out_elem);

// Element type & value access
SIMDJSON_UNITY_API SimdJsonType SimdJson_GetType(SimdJsonElementHandle elem);

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetBool(
    SimdJsonElementHandle elem, int* out_value);

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetInt64(
    SimdJsonElementHandle elem, int64_t* out_value);

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetUInt64(
    SimdJsonElementHandle elem, uint64_t* out_value);

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetDouble(
    SimdJsonElementHandle elem, double* out_value);

// String: returns length, copies to buffer if provided (null-terminated if capacity allows)
SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetString(
    SimdJsonElementHandle elem,
    char* buffer,
    size_t buffer_capacity,
    size_t* out_length);

// Object
SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ObjectCount(
    SimdJsonElementHandle elem, size_t* out_count);

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ObjectFindField(
    SimdJsonElementHandle obj,
    const char* key,
    size_t key_len,
    SimdJsonElementHandle* out_value);

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ObjectAt(
    SimdJsonElementHandle obj,
    size_t index,
    char* key_buffer,
    size_t key_capacity,
    size_t* out_key_len,
    SimdJsonElementHandle* out_value);

// Array
SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ArrayCount(
    SimdJsonElementHandle elem, size_t* out_count);

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ArrayAt(
    SimdJsonElementHandle arr,
    size_t index,
    SimdJsonElementHandle* out_value);

// Convenience: minify
SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_Minify(
    const char* json,
    size_t length,
    char* out_buffer,
    size_t out_capacity,
    size_t* out_length);

// Validate only
SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_Validate(
    const char* json,
    size_t length);

// Version string
SIMDJSON_UNITY_API const char* SimdJson_Version(void);

// Free element handle (elements are owned by document, but we track ref for safety)
SIMDJSON_UNITY_API void SimdJson_DestroyElement(SimdJsonElementHandle elem);

#ifdef __cplusplus
}
#endif
