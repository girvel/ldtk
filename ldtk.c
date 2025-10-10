#include <assert.h>
#include <stdio.h>
#include <jansson.h>
#include <string.h>
#include "nob.h"
#include "ldtk.h"
#include "macros.h"


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

void put_level_size(json_t *levels, Level *fallen_level, char **error) {
    size_t levels_n = json_array_size(levels);
    for (size_t i = 0; i < levels_n; ++i) {
        json_t *level = json_array_get(levels, i);
        if (!json_is_object(level)) {
            *error = "Expected the level to be an object";
            goto end;
        }

        int offset_x = $(field_int(level, "worldX", error)) / 16;
        int offset_y = $(field_int(level, "worldY", error)) / 16;
        int w = $(field_int(level, "pxWid", error)) / 16;
        int h = $(field_int(level, "pxHei", error)) / 16;

        fallen_level->w = MAX2(fallen_level->w, offset_x + w);
        fallen_level->h = MAX2(fallen_level->h, offset_y + h);
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

    json_t *levels = json_object_get(root, "levels");
    if (!json_is_array(levels)) {
        *error = "Expected .levels to be an array";
        goto end;
    }

    MUST(put_level_size(levels, &result, error));

    size_t levels_n = json_array_size(levels);
    for (size_t i = 0; i < levels_n; ++i) {
        json_t *level = json_array_get(levels, i);
        if (!json_is_object(level)) {
            *error = "Expected the level to be an object";
            goto end;
        }

        json_t *layers = json_object_get(level, "layerInstances");
        if (!json_is_array(layers)) {
            *error = "Expected level's .layerInstances to be an array";
            goto end;
        }

        size_t layers_n = json_array_size(layers);
        for (size_t j = 0; j < layers_n; ++j) {
            json_t *layer = json_array_get(layers, j);
            if (!json_is_object(layer)) {
                *error = "Expected layer to be an object";
                goto end;
            }

            json_t *entity_instances = json_object_get(layer, "entityInstances");
            if (!json_is_array(entity_instances)) {
                *error = "Expected entityInstances to be an array";
                goto end;
            }

            size_t instances_n = json_array_size(entity_instances);
            for (size_t k = 0; k < instances_n; ++k) {
                json_t *entity = json_array_get(entity_instances, k);
                if (!json_is_object(entity)) {
                    *error = "...";
                    goto end;
                }

                json_t *identifier_ptr = json_object_get(entity, "__identifier");
                if (!json_is_string(identifier_ptr)) {
                    *error = "...";
                    goto end;
                }

                const char *identifier = json_string_value(identifier_ptr);

                if (strcmp(identifier, "position") == 0) {
                    Position p = {
                      .name = "hello",
                      .x = 0,
                      .y = 0,
                    };
                    nob_da_append(&result.positions, p);
                }
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
