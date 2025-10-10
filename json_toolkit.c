#include <jansson.h>
#include "nob.h"


int field_int(json_t *object, const char *identifier, char **error) {
    json_t *ptr = json_object_get(object, identifier);
    if (!json_is_integer(ptr)) {
        Nob_String_Builder sb = {0};
        nob_sb_append_cstr(&sb, "Expected .");
        nob_sb_append_cstr(&sb, identifier);
        nob_sb_append_cstr(&sb, " to be an integer");
        nob_sb_append_null(&sb);
        *error = sb.items;
        return 0;
    }
    return (int) json_integer_value(ptr);
}

const char *field_string(json_t *object, const char *identifier, char **error) {
    json_t *ptr = json_object_get(object, identifier);
    if (!json_is_string(ptr)) {
        Nob_String_Builder sb = {0};
        nob_sb_append_cstr(&sb, "Expected .");
        nob_sb_append_cstr(&sb, identifier);
        nob_sb_append_cstr(&sb, " to be a string");
        nob_sb_append_null(&sb);
        *error = sb.items;
        return 0;
    }
    return json_string_value(ptr);
}

json_t *field_array(json_t *object, const char *identifier, char **error) {
    json_t *ptr = json_object_get(object, identifier);
    if (!json_is_array(ptr)) {
        Nob_String_Builder sb = {0};
        nob_sb_append_cstr(&sb, "Expected .");
        nob_sb_append_cstr(&sb, identifier);
        nob_sb_append_cstr(&sb, " to be an array");
        nob_sb_append_null(&sb);
        *error = sb.items;
        return 0;
    }
    return ptr;
}

json_t *item_object(json_t *array, size_t i, char **error) {
    json_t *ptr = json_array_get(array, i);
    if (!json_is_object(ptr)) {
        *error = "Expected array item to be an object";
        return 0;
    }
    return ptr;
}

