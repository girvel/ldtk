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

Position read_position(json_t *entity, char **error) {
    Position result;

    json_t *field_instances = $(field_array(entity, "fieldInstances", error));
    size_t fields_n = json_array_size(field_instances);
    for (size_t j = 0; j < fields_n; ++j) {
        json_t *field = $(item_object(field_instances, j, error));
        const char *identifier = $(field_string(field, "__identifier", error));

        if (strcmp(identifier, "rails_name") == 0) {
            const char *value = $(field_string(field, "__value", error));

            result.name = malloc(strlen(value));
            strcpy(result.name, value);
            break;
        }
    }

    result.x = $(field_int(entity, "__worldX", error)) / 16 + 1;
    result.y = $(field_int(entity, "__worldY", error)) / 16 + 1;

end:
    return result;
}

void put_positions(json_t *layer, Positions *positions, char **error) {
    json_t *entity_instances = $(field_array(layer, "entityInstances", error));

    size_t instances_n = json_array_size(entity_instances);
    for (size_t i = 0; i < instances_n; ++i) {
        json_t *entity = $(item_object(entity_instances, i, error));
        const char *identifier = $(field_string(entity, "__identifier", error));

        if (strcmp(identifier, "position") == 0) {
            nob_da_append(positions, $(read_position(entity, error)));
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
            json_t *layer = $(item_object(layers, j, error));

            const char *identifier = $(field_string(layer, "__identifier", error));

            if (strcmp(identifier, "positions") == 0) {
                MUST(put_positions(layer, &result.positions, error));
            }
        }
    }

end:
    json_decref(root);
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
