#include "simdjson_unity_wrapper.h"
#include "simdjson.h"
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

// DOM backend: random-access safe for Unity.
// (ondemand is single-pass — ObjectCount then FindField / double GetString crashed the Editor.)

using namespace simdjson;

struct ParserWrapper {
    // Kept for API symmetry; each Document owns its own parser.
    int unused = 0;
};

struct DocumentWrapper {
    std::unique_ptr<dom::parser> owned_parser;
    dom::element root;
};

struct ElementWrapper {
    dom::element el;
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

    auto* dw = new (std::nothrow) DocumentWrapper();
    if (!dw) return SIMDJSON_ERROR_MEMALLOC;

    dw->owned_parser = std::make_unique<dom::parser>();
    auto err = dw->owned_parser->parse(json, length).get(dw->root);
    if (err) {
        delete dw;
        return ToErrorCode(err);
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
    auto* ew = new (std::nothrow) ElementWrapper();
    if (!ew) return SIMDJSON_ERROR_MEMALLOC;
    ew->el = dw->root;
    *out_elem = ew;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API void SimdJson_DestroyElement(SimdJsonElementHandle elem) {
    delete static_cast<ElementWrapper*>(elem);
}

SIMDJSON_UNITY_API SimdJsonType SimdJson_GetType(SimdJsonElementHandle elem) {
    if (!elem) return SIMDJSON_TYPE_NULL;
    auto* ew = static_cast<ElementWrapper*>(elem);
    switch (ew->el.type()) {
        case dom::element_type::NULL_VALUE: return SIMDJSON_TYPE_NULL;
        case dom::element_type::BOOL: return SIMDJSON_TYPE_BOOL;
        case dom::element_type::INT64:
        case dom::element_type::UINT64:
        case dom::element_type::DOUBLE: return SIMDJSON_TYPE_NUMBER;
        case dom::element_type::STRING: return SIMDJSON_TYPE_STRING;
        case dom::element_type::ARRAY: return SIMDJSON_TYPE_ARRAY;
        case dom::element_type::OBJECT: return SIMDJSON_TYPE_OBJECT;
        default: return SIMDJSON_TYPE_NULL;
    }
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetBool(
    SimdJsonElementHandle elem, int* out_value)
{
    if (!elem || !out_value) return SIMDJSON_ERROR_INVALID;
    bool b;
    auto error = static_cast<ElementWrapper*>(elem)->el.get(b);
    if (error) return ToErrorCode(error);
    *out_value = b ? 1 : 0;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetInt64(
    SimdJsonElementHandle elem, int64_t* out_value)
{
    if (!elem || !out_value) return SIMDJSON_ERROR_INVALID;
    int64_t v;
    auto error = static_cast<ElementWrapper*>(elem)->el.get(v);
    if (error) return ToErrorCode(error);
    *out_value = v;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetUInt64(
    SimdJsonElementHandle elem, uint64_t* out_value)
{
    if (!elem || !out_value) return SIMDJSON_ERROR_INVALID;
    uint64_t v;
    auto error = static_cast<ElementWrapper*>(elem)->el.get(v);
    if (error) return ToErrorCode(error);
    *out_value = v;
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_GetDouble(
    SimdJsonElementHandle elem, double* out_value)
{
    if (!elem || !out_value) return SIMDJSON_ERROR_INVALID;
    double v;
    auto error = static_cast<ElementWrapper*>(elem)->el.get(v);
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
    std::string_view sv;
    auto error = static_cast<ElementWrapper*>(elem)->el.get(sv);
    if (error) return ToErrorCode(error);
    *out_length = sv.size();
    if (buffer && buffer_capacity > 0) {
        size_t copy_len = sv.size() < (buffer_capacity - 1) ? sv.size() : (buffer_capacity - 1);
        std::memcpy(buffer, sv.data(), copy_len);
        buffer[copy_len] = '\0';
    }
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ObjectCount(
    SimdJsonElementHandle elem, size_t* out_count)
{
    if (!elem || !out_count) return SIMDJSON_ERROR_INVALID;
    dom::object obj;
    auto error = static_cast<ElementWrapper*>(elem)->el.get(obj);
    if (error) return ToErrorCode(error);
    *out_count = obj.size();
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ObjectFindField(
    SimdJsonElementHandle obj,
    const char* key,
    size_t key_len,
    SimdJsonElementHandle* out_value)
{
    if (!obj || !key || !out_value) return SIMDJSON_ERROR_INVALID;
    dom::object o;
    auto error = static_cast<ElementWrapper*>(obj)->el.get(o);
    if (error) return ToErrorCode(error);

    dom::element val;
    error = o[std::string_view(key, key_len)].get(val);
    if (error) return ToErrorCode(error);

    auto* new_ew = new (std::nothrow) ElementWrapper();
    if (!new_ew) return SIMDJSON_ERROR_MEMALLOC;
    new_ew->el = val;
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
    dom::object o;
    auto error = static_cast<ElementWrapper*>(obj)->el.get(o);
    if (error) return ToErrorCode(error);

    size_t i = 0;
    for (auto field : o) {
        if (i == index) {
            std::string_view key = field.key;
            *out_key_len = key.size();
            if (key_buffer && key_capacity > 0) {
                size_t copy_len = key.size() < (key_capacity - 1) ? key.size() : (key_capacity - 1);
                std::memcpy(key_buffer, key.data(), copy_len);
                key_buffer[copy_len] = '\0';
            }
            auto* new_ew = new (std::nothrow) ElementWrapper();
            if (!new_ew) return SIMDJSON_ERROR_MEMALLOC;
            new_ew->el = field.value;
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
    dom::array arr;
    auto error = static_cast<ElementWrapper*>(elem)->el.get(arr);
    if (error) return ToErrorCode(error);
    *out_count = arr.size();
    return SIMDJSON_OK;
}

SIMDJSON_UNITY_API SimdJsonErrorCode SimdJson_ArrayAt(
    SimdJsonElementHandle arr,
    size_t index,
    SimdJsonElementHandle* out_value)
{
    if (!arr || !out_value) return SIMDJSON_ERROR_INVALID;
    dom::array a;
    auto error = static_cast<ElementWrapper*>(arr)->el.get(a);
    if (error) return ToErrorCode(error);
    if (index >= a.size()) return SIMDJSON_ERROR_INDEX_OUT_OF_BOUNDS;

    auto* new_ew = new (std::nothrow) ElementWrapper();
    if (!new_ew) return SIMDJSON_ERROR_MEMALLOC;
    new_ew->el = a.at(index);
    *out_value = new_ew;
    return SIMDJSON_OK;
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
        *out_length = length;
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
    dom::parser parser;
    dom::element el;
    auto error = parser.parse(json, length).get(el);
    return ToErrorCode(error);
}

SIMDJSON_UNITY_API const char* SimdJson_Version(void) {
    return SIMDJSON_VERSION;
}

} // extern "C"
