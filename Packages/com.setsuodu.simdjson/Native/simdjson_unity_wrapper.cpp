#include "simdjson_unity_wrapper.h"
#include "simdjson.h"
#include <string>
#include <memory>
#include <vector>
#include <cstring>

using namespace simdjson;

struct ParserWrapper {
    ondemand::parser parser;
};

struct DocumentWrapper {
    padded_string json;
    ondemand::document doc;
    // Keep elements alive via shared ownership of document
    std::shared_ptr<void> keep_alive;
};

struct ElementWrapper {
    ondemand::value value;
    std::shared_ptr<DocumentWrapper> doc_owner; // keep document alive
    // For object/array iteration we may need to store more
};

static SimdJsonErrorCode ToErrorCode(error_code ec) {
    switch (ec) {
        case SUCCESS: return SIMDJSON_OK;
        case CAPACITY: return SIMDJSON_ERROR_CAPACITY;
        case MEMALLOC: return SIMDJSON_ERROR_MEMALLOC;
        case TAPE_ERROR: return SIMDJSON_ERROR_TAPE_ERROR;
        case DEPTH_ERROR: return SIMDJSON_ERROR_DEPTH_ERROR;
        case STRING_ERROR: return SIMDJSON_ERROR_STRING_ERROR;
        case T_ATOM_ERROR: return SIMDJSON_ERROR_T_ATOM_ERROR;
        case F_ATOM_ERROR: return SIMDJSON_ERROR_F_ATOM_ERROR;
        case N_ATOM_ERROR: return SIMDJSON_ERROR_N_ATOM_ERROR;
        case NUMBER_ERROR: return SIMDJSON_ERROR_NUMBER_ERROR;
        case UTF8_ERROR: return SIMDJSON_ERROR_UTF8_ERROR;
        case UNINITIALIZED: return SIMDJSON_ERROR_UNINITIALIZED;
        case EMPTY: return SIMDJSON_ERROR_EMPTY;
        case UNESCAPED_CHARS: return SIMDJSON_ERROR_UNESCAPED_CHARS;
        case UNEXPECTED_ERROR: return SIMDJSON_ERROR_UNEXPECTED_ERROR;
        case PARSER_IN_USE: return SIMDJSON_ERROR_PARSER_IN_USE;
        case INSUFFICIENT_PADDING: return SIMDJSON_ERROR_INSUFFICIENT_PADDING;
        case INCOMPLETE_ARRAY_OR_OBJECT: return SIMDJSON_ERROR_INCOMPLETE_ARRAY_OR_OBJECT;
        case SCALAR_DOCUMENT_AS_VALUE: return SIMDJSON_ERROR_SCALAR_DOCUMENT_AS_VALUE;
        case OUT_OF_ORDER_ITERATION: return SIMDJSON_ERROR_OUT_OF_ORDER_ITERATION;
        case NO_SUCH_FIELD: return SIMDJSON_ERROR_NO_SUCH_FIELD;
        case INDEX_OUT_OF_BOUNDS: return SIMDJSON_ERROR_INDEX_OUT_OF_BOUNDS;
        case INCORRECT_TYPE: return SIMDJSON_ERROR_INCORRECT_TYPE;
        case NUMBER_OUT_OF_RANGE: return SIMDJSON_ERROR_NUMBER_OUT_OF_RANGE;
        default: return SIMDJSON_ERROR_INVALID;
    }
}

extern "C" {

SIMDJSON_UNITY_API SimdJsonParserHandle SimdJson_CreateParser(void) {
    return new (std::nothrow) ParserWrapper();
}

SIMDJSON_UNITY_API void SimdJson_DestroyParser(SimdJsonParserHandle parser) {
    delete static_cast<ParserWrapper*>(parser);
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_Parse(
    SimdJsonParserHandle parser,
    const char* json,
    size_t length,
    SimdJsonDocumentHandle* out_doc)
{
    if (!parser || !json || !out_doc) return SIMDJSON_ERROR_INVALID;
    auto* p = static_cast<ParserWrapper*>(parser);
    auto* dw = new (std::nothrow) DocumentWrapper();
    if (!dw) return SIMDJSON_ERROR_MEMALLOC;
    dw->json = padded_string(json, length);
    auto error = p->parser.iterate(dw->json).get(dw->doc);
    if (error) {
        delete dw;
        return ToErrorCode(error);
    }
    *out_doc = dw;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API void SimdJson_DestroyDocument(SimdJsonDocumentHandle doc) {
    delete static_cast<DocumentWrapper*>(doc);
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetRoot(
    SimdJsonDocumentHandle doc,
    SimdJsonElementHandle* out_elem)
{
    if (!doc || !out_elem) return SIMDJSON_ERROR_INVALID;
    auto* dw = static_cast<DocumentWrapper*>(doc);
    ondemand::value root;
    auto error = dw->doc.get_value().get(root);
    if (error) return ToErrorCode(error);

    auto* ew = new (std::nothrow) ElementWrapper();
    if (!ew) return SIMDJSON_ERROR_MEMALLOC;
    ew->value = std::move(root);
    *out_elem = ew;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API void SimdJson_DestroyElement(SimdJsonElementHandle elem) {
    delete static_cast<ElementWrapper*>(elem);
}

SIMDJSON_UNITY_API SimdJsonType SimdJson_GetType(SimdJsonElementHandle elem) {
    if (!elem) return SIMDJSON_TYPE_NULL;
    auto* ew = static_cast<ElementWrapper*>(elem);
    ondemand::json_type t;
    auto error = ew->value.type().get(t);
    if (error) return SIMDJSON_TYPE_NULL;
    switch (t) {
        case ondemand::json_type::null: return SIMDJSON_TYPE_NULL;
        case ondemand::json_type::boolean: return SIMDJSON_TYPE_BOOL;
        case ondemand::json_type::number: return SIMDJSON_TYPE_NUMBER;
        case ondemand::json_type::string: return SIMDJSON_TYPE_STRING;
        case ondemand::json_type::array: return SIMDJSON_TYPE_ARRAY;
        case ondemand::json_type::object: return SIMDJSON_TYPE_OBJECT;
        default: return SIMDJSON_TYPE_NULL;
    }
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetBool(
    SimdJsonElementHandle elem, int* out_value)
{
    if (!elem || !out_value) return SIMDJSON_ERROR_INVALID;
    auto* ew = static_cast<ElementWrapper*>(elem);
    bool b;
    auto error = ew->value.get_bool().get(b);
    if (error) return ToErrorCode(error);
    *out_value = b ? 1 : 0;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetInt64(
    SimdJsonElementHandle elem, int64_t* out_value)
{
    if (!elem || !out_value) return SIMDJSON_ERROR_INVALID;
    auto* ew = static_cast<ElementWrapper*>(elem);
    int64_t v;
    auto error = ew->value.get_int64().get(v);
    if (error) return ToErrorCode(error);
    *out_value = v;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetUInt64(
    SimdJsonElementHandle elem, uint64_t* out_value)
{
    if (!elem || !out_value) return SIMDJSON_ERROR_INVALID;
    auto* ew = static_cast<ElementWrapper*>(elem);
    uint64_t v;
    auto error = ew->value.get_uint64().get(v);
    if (error) return ToErrorCode(error);
    *out_value = v;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetDouble(
    SimdJsonElementHandle elem, double* out_value)
{
    if (!elem || !out_value) return SIMDJSON_ERROR_INVALID;
    auto* ew = static_cast<ElementWrapper*>(elem);
    double v;
    auto error = ew->value.get_double().get(v);
    if (error) return ToErrorCode(error);
    *out_value = v;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetString(
    SimdJsonElementHandle elem,
    char* buffer,
    size_t buffer_capacity,
    size_t* out_length)
{
    if (!elem || !out_length) return SIMDJSON_ERROR_INVALID;
    auto* ew = static_cast<ElementWrapper*>(elem);
    std::string_view sv;
    auto error = ew->value.get_string().get(sv);
    if (error) return ToErrorCode(error);
    *out_length = sv.size();
    if (buffer && buffer_capacity > 0) {
        size_t copy_len = sv.size() < (buffer_capacity - 1) ? sv.size() : (buffer_capacity - 1);
        memcpy(buffer, sv.data(), copy_len);
        buffer[copy_len] = '\0';
    }
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ObjectCount(
    SimdJsonElementHandle elem, size_t* out_count)
{
    if (!elem || !out_count) return SIMDJSON_ERROR_INVALID;
    auto* ew = static_cast<ElementWrapper*>(elem);
    ondemand::object obj;
    auto error = ew->value.get_object().get(obj);
    if (error) return ToErrorCode(error);
    size_t count = 0;
    for (auto field : obj) {
        (void)field;
        ++count;
    }
    *out_count = count;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ObjectFindField(
    SimdJsonElementHandle obj,
    const char* key,
    size_t key_len,
    SimdJsonElementHandle* out_value)
{
    if (!obj || !key || !out_value) return SIMDJSON_ERROR_INVALID;
    auto* ew = static_cast<ElementWrapper*>(obj);
    ondemand::object o;
    auto error = ew->value.get_object().get(o);
    if (error) return ToErrorCode(error);

    ondemand::value val;
    error = o.find_field(std::string_view(key, key_len)).get(val);
    if (error) return ToErrorCode(error);

    auto* new_ew = new ElementWrapper();
    new_ew->value = std::move(val);
    *out_value = new_ew;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ObjectAt(
    SimdJsonElementHandle obj,
    size_t index,
    char* key_buffer,
    size_t key_capacity,
    size_t* out_key_len,
    SimdJsonElementHandle* out_value)
{
    if (!obj || !out_value || !out_key_len) return SIMDJSON_ERROR_INVALID;
    auto* ew = static_cast<ElementWrapper*>(obj);
    ondemand::object o;
    auto error = ew->value.get_object().get(o);
    if (error) return ToErrorCode(error);

    size_t i = 0;
    for (auto field : o) {
        if (i == index) {
            std::string_view key;
            ondemand::value val;
            error = field.unescaped_key().get(key);
            if (error) return ToErrorCode(error);
            error = field.value().get(val);
            if (error) return ToErrorCode(error);

            *out_key_len = key.size();
            if (key_buffer && key_capacity > 0) {
                size_t copy_len = key.size() < (key_capacity - 1) ? key.size() : (key_capacity - 1);
                memcpy(key_buffer, key.data(), copy_len);
                key_buffer[copy_len] = '\0';
            }

            auto* new_ew = new ElementWrapper();
            new_ew->value = std::move(val);
            *out_value = new_ew;
            return SIMDJSON_OK;
        }
        ++i;
    }
    return SIMDJSON_ERROR_INDEX_OUT_OF_BOUNDS;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ArrayCount(
    SimdJsonElementHandle elem, size_t* out_count)
{
    if (!elem || !out_count) return SIMDJSON_ERROR_INVALID;
    auto* ew = static_cast<ElementWrapper*>(elem);
    ondemand::array arr;
    auto error = ew->value.get_array().get(arr);
    if (error) return ToErrorCode(error);
    size_t count = 0;
    for (auto v : arr) {
        (void)v;
        ++count;
    }
    *out_count = count;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ArrayAt(
    SimdJsonElementHandle arr,
    size_t index,
    SimdJsonElementHandle* out_value)
{
    if (!arr || !out_value) return SIMDJSON_ERROR_INVALID;
    auto* ew = static_cast<ElementWrapper*>(arr);
    ondemand::array a;
    auto error = ew->value.get_array().get(a);
    if (error) return ToErrorCode(error);

    size_t i = 0;
    for (auto v : a) {
        if (i == index) {
            ondemand::value val;
            error = v.get(val);
            if (error) return ToErrorCode(error);
            auto* new_ew = new ElementWrapper();
            new_ew->value = std::move(val);
            *out_value = new_ew;
            return SIMDJSON_OK;
        }
        ++i;
    }
    return SIMDJSON_ERROR_INDEX_OUT_OF_BOUNDS;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_Minify(
    const char* json,
    size_t length,
    char* out_buffer,
    size_t out_capacity,
    size_t* out_length)
{
    if (!json || !out_length) return SIMDJSON_ERROR_INVALID;
    if (!out_buffer || out_capacity == 0) {
        // just compute length roughly
        *out_length = length; // upper bound
        return SIMDJSON_OK;
    }
    size_t dst_len = out_capacity;
    auto error = minify(json, length, out_buffer, dst_len);
    if (error) return ToErrorCode(error);
    *out_length = dst_len;
    if (dst_len < out_capacity) out_buffer[dst_len] = '\0';
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_Validate(
    const char* json,
    size_t length)
{
    if (!json) return SIMDJSON_ERROR_INVALID;
    ondemand::parser parser;
    padded_string ps(json, length);
    ondemand::document doc;
    auto error = parser.iterate(ps).get(doc);
    return ToErrorCode(error);
}

SIMDJSON_UNITY_API const char* SimdJson_Version(void) {
    return SIMDJSON_VERSION;
}

} // extern "C"
