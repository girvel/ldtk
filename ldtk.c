#include <assert.h>
#include <stdio.h>
#include <jansson.h>
#include <string.h>
#include "nob.h"
#include "ldtk.h"
#include "macros.h"
#include "json_toolkit.c"


void put_level_size(json_t *levels, int *w, int *h, char **error) {
    size_t levels_n = json_array_size(levels);
    for (size_t i = 0; i < levels_n; ++i) {
        json_t *level = json_array_get(levels, i);
        if (!json_is_object(level)) {
            *error = "Expected the level to be an object";
            goto end;
        }

        int offset_x = $(field_int(level, "worldX", error)) / 16;
        int offset_y = $(field_int(level, "worldY", error)) / 16;
        int level_w = $(field_int(level, "pxWid", error)) / 16;
        int level_h = $(field_int(level, "pxHei", error)) / 16;

        *w = MAX2(*w, offset_x + level_w);
        *h = MAX2(*h, offset_y + level_h);
    }

end:
    return;
}

void put_positions(json_t *layer, Positions *positions, char **error) {
    json_t *entity_instances = $(field_array(layer, "entityInstances", error));

    size_t instances_n = json_array_size(entity_instances);
    for (size_t k = 0; k < instances_n; ++k) {
        json_t *entity = $(item_object(entity_instances, k, error));
        const char *identifier = $(field_string(entity, "__identifier", error));

        if (strcmp(identifier, "position") == 0) {
            Position p = {
              .name = "hello",
              .x = 0,
              .y = 0,
            };
            nob_da_append(positions, p);
        }
    }

end:
    return;
}

Level read_level(const char *path, char **error) {
    Level result = {0};

    json_error_t json_error;
    json_t *root = json_load_file(path, 0, &json_error);
    if (!json_is_object(root)) {
        *error = "Expected the level to contain an object";
        goto end;
    }

    json_t *levels = $(field_array(root, "levels", error));

    MUST(put_level_size(levels, &result.w, &result.h, error));

    size_t levels_n = json_array_size(levels);
    for (size_t i = 0; i < levels_n; ++i) {
        json_t *level = $(item_object(levels, i, error));
        json_t *layers = $(field_array(level, "layerInstances", error));

        size_t layers_n = json_array_size(layers);
        for (size_t j = 0; j < layers_n; ++j) {
            json_t *layer = $(item_object(layers, i, error));

            const char *identifier = $(field_string(layer, "__identifier", error));

            if (strcmp(identifier, "positions") == 0) {
                MUST(put_positions(layer, &result.positions, error));
            }
        }
    }
end:
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
    for (size_t i = 0; i < fallen_level.positions.count; ++i) {
        Position p = fallen_level.positions.items[i];
        printf("- %s: (%d, %d)\n", p.name, p.x, p.y);
    }
    
    return 0;
}
