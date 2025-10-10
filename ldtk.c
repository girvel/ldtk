#include <assert.h>
#include <stdio.h>
#include <jansson.h>
#include "nob.h"

typedef struct {
    int w, h;
} Level;

#define MAX2(A, B) ({ \
    __typeof__ (A) _a = (A); \
    __typeof__ (B) _b = (B); \
    _a > _b ? _a : _b; \
})

#define MIN2(A, B) ({ \
    __typeof__ (A) _a = (A); \
    __typeof__ (B) _b = (B); \
    _a < _b ? _a : _b; \
}

#define $(EXPR) ({ \
    __typeof__ (EXPR) _eval_result = (EXPR); \
    if (*error != NULL) return result; \
    _eval_result; \
})

int field_int(json_t *base, const char *identifier, char **error) {
    json_t *ptr = json_object_get(base, identifier);
    if (!json_is_integer(ptr)) {
        Nob_String_Builder sb = {0};
        nob_sb_append_cstr(&sb, "Expected .");
        nob_sb_append_cstr(&sb, identifier);
        nob_sb_append_cstr(&sb, " to be integer");
        nob_sb_append_null(&sb);
        *error = sb.items;
        return 0;
    }
    return (int) json_integer_value(ptr);
}

Level read_level(const char *path, char **error) {
    Level result = {0};

    json_error_t json_error;
    json_t *root = json_load_file(path, 0, &json_error);
    if (!json_is_object(root)) {
        *error = "Expected the level to contain an object";
        return result;
    }

    json_t *levels = json_object_get(root, "levels");
    if (!json_is_array(levels)) {
        *error = "Expected .levels to be an array";
        return result;
    }

    size_t levels_n = json_array_size(levels);
    for (size_t i = 0; i < levels_n; ++i) {
        json_t *level = json_array_get(levels, i);
        if (!json_is_object(level)) {
            *error = "Expected the level to be an object";
            return result;
        }

        int offset_x = $(field_int(level, "worldX", error)) / 16;
        int offset_y = $(field_int(level, "worldY", error)) / 16;
        int w = $(field_int(level, "pxWid", error)) / 16;
        int h = $(field_int(level, "pxHei", error)) / 16;

        result.w = MAX2(result.w, offset_x + w);
        result.h = MAX2(result.h, offset_y + h);
    }

    return result;
}

int main() {
    char *error = NULL;
    Level fallen_level = read_level("../dot/levels/main/main.ldtk", &error);
    if (error != NULL) {
        printf("error: %s\n", error);
        return 1;
    }

    printf("(%d, %d)\n", fallen_level.w, fallen_level.h);
    return 0;
}
