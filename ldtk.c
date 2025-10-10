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

int main() {
    json_error_t error;
    json_t *root = json_load_file("../dot/levels/main/main.ldtk", 0, &error);
    assert(json_is_object(root));

    json_t *levels = json_object_get(root, "levels");
    assert(json_is_array(levels));

    Level fallen_level = {0};

    size_t levels_n = json_array_size(levels);
    for (size_t i = 0; i < levels_n; ++i) {
        json_t *level = json_array_get(levels, i);
        assert(json_is_object(level));

        int offset_x = (int) ({
            json_t *world_x_ptr = json_object_get(level, "worldX");
            assert(json_is_integer(world_x_ptr));
            json_integer_value(world_x_ptr);
        }) / 16;

        int w = (int) ({
            json_t *world_x_ptr = json_object_get(level, "pxWid");
            assert(json_is_integer(world_x_ptr));
            json_integer_value(world_x_ptr);
        }) / 16;

        fallen_level.w = MAX2(fallen_level.w, offset_x + w);
    }

    printf("(%d, %d)\n", fallen_level.w, fallen_level.h);
}
