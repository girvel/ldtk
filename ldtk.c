#include <assert.h>
#include <stdio.h>
#include <jansson.h>
#include <string.h>
#include "nob.h"
#include "ldtk.h"
#include "macros.h"
#include "json_toolkit.c"


char *clone(const char *source) {
    char *result = malloc(strlen(source) + 1);
    strcpy(result, source);
    return result;
}

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
    result.rails_name = NULL;

    json_t *field_instances = $(field_array(entity, "fieldInstances", error));
    size_t fields_n = json_array_size(field_instances);
    for (size_t j = 0; j < fields_n; ++j) {
        json_t *field = $(item_object(field_instances, j, error));
        const char *identifier = $(field_string(field, "__identifier", error));

        if (strcmp(identifier, "rails_name") == 0) {
            //if (json_is_null(field, 
            result.rails_name = clone($(field_string(field, "__value", error)));
            break;
        }
    }

    // if (result.rails_name == NULL) {
    //     *error = "Position has no .rails_name field";
    //     goto end;
    // }

    result.x = $(field_int(entity, "__worldX", error)) / 16 + 1;
    result.y = $(field_int(entity, "__worldY", error)) / 16 + 1;

end:
    return result;
}

typedef struct {
    int x;
    int y;
    char *grid_layer;
    char *rails_name;
} Capture;

typedef struct {
    Capture *items;
    size_t count;
    size_t capacity;
} Captures;

Capture read_capture(json_t *entity, char **error) {
    Capture result;

    json_t *field_instances = $(field_array(entity, "fieldInstances", error));
    size_t fields_n = json_array_size(field_instances);
    for (size_t j = 0; j < fields_n; ++j) {
        json_t *field = $(item_object(field_instances, j, error));
        const char *identifier = $(field_string(field, "__identifier", error));

        if (strcmp(identifier, "rails_name") == 0) {
            result.rails_name = clone($(field_string(field, "__value", error)));
        } else if (strcmp(identifier, "layer") == 0) {
            result.grid_layer = clone($(field_string(field, "__value", error)));
        }
    }

    result.x = $(field_int(entity, "__worldX", error)) / 16 + 1;
    result.y = $(field_int(entity, "__worldY", error)) / 16 + 1;

end:
    return result;
}


void use_captures(Entity *e, Captures *captures, char **error) {
    for (size_t i = 0; i < captures->count; ++i) {
        Capture *c = captures->items + i;
        if (e->x == c->x
            && e->y == c->y
            && strcmp(e->grid_layer, c->grid_layer) == 0
        ) {
            if (e->rails_name != NULL) {
                Nob_String_Builder sb = {0};
                nob_sb_append_cstr(&sb, "Attempt to capture an entity as ");
                nob_sb_append_cstr(&sb, c->rails_name);
                nob_sb_append_cstr(&sb, ", when it already has rails_name ");
                nob_sb_append_cstr(&sb, e->rails_name);
                nob_sb_append_null(&sb);

                *error = sb.items;
                goto end;
            }

            e->rails_name = c->rails_name;
            nob_da_remove_unordered(captures, i);
            break;
        }
    }

end:
    return;
}

void put_positions(
    json_t *layer, Positions *positions, Captures *captures, char **error
) {
    json_t *entity_instances = $(field_array(layer, "entityInstances", error));

    size_t instances_n = json_array_size(entity_instances);
    for (size_t i = 0; i < instances_n; ++i) {
        json_t *entity = $(item_object(entity_instances, i, error));
        const char *identifier = $(field_string(entity, "__identifier", error));

        if (strcmp(identifier, "position") == 0) {
            Position p = $(read_position(entity, error));
            nob_da_append(positions, p);
        } else if (strcmp(identifier, "entity_capture") == 0) {
            Capture c = $(read_capture(entity, error));
            nob_da_append(captures, c);
        } else {
            Nob_String_Builder sb = {0};
            nob_sb_append_cstr(&sb, "Unknown position layer entity ");
            nob_sb_append_cstr(&sb, identifier);
            nob_sb_append_null(&sb);

            *error = sb.items;
            goto end;
        }
    }

end:
    return;
}

void put_entities(json_t *layer, Captures *captures, Entities *entities, char **error) {
    char *grid_layer = NULL;  // for free to work

    json_t *entity_instances = $(field_array(layer, "entityInstances", error));
    grid_layer = clone($(field_string(layer, "__identifier", error)));

    Nob_String_View sv = nob_sv_from_cstr(grid_layer);
    if (!nob_sv_end_with(sv, "_entities")) {
        Nob_String_Builder sb = {0};
        nob_sb_append_cstr(&sb, "Expected layer ");
        nob_sb_append_cstr(&sb, grid_layer);
        nob_sb_append_cstr(&sb, " to end with '_entities'");
        nob_sb_append_null(&sb);

        *error = sb.items;
        goto end;
    } else {
        grid_layer[sv.count - 9] = '\0';
    }

    size_t instances_n = json_array_size(entity_instances);
    for (size_t i = 0; i < instances_n; ++i) {
        json_t *entity = $(item_object(entity_instances, i, error));
        Entity e;

        e.x = $(field_int(entity, "__worldX", error)) / 16 + 1;
        e.y = $(field_int(entity, "__worldY", error)) / 16 + 1;
        e.grid_layer = clone(grid_layer);

        e.identifier.type = Identifier_string;
        e.identifier.value.string = clone($(field_string(entity, "__identifier", error)));

        e.args = NULL;
        e.rails_name = NULL;

        json_t *field_instances = $(field_array(entity, "fieldInstances", error));
        size_t fields_n = json_array_size(field_instances);
        for (size_t j = 0; j < fields_n; ++j) {
            json_t *field = $(item_object(field_instances, j, error));
            const char *field_name = $(field_string(field, "__identifier", error));

            if (strcmp(field_name, "rails_name") == 0) {
                if (!json_is_null(json_object_get(field, "__value"))) {
                    e.rails_name = clone($(field_string(field, "__value", error)));
                }
            } else if (strcmp(field_name, "args") == 0) {
                e.args = clone($(field_string(field, "__value", error)));
            }
        }

        MUST(use_captures(&e, captures, error));

        nob_da_append(entities, e);
    }

end:
    free(grid_layer); return; }

void put_tiles(
    json_t *layer, int offset_x, int offset_y, bool is_auto,
    Captures *captures, Entities *entities, char **error
) {
    char *grid_layer = NULL;  // for free to work

    json_t *instances = $(field_array(layer, is_auto ? "autoLayerTiles" : "gridTiles", error));
    grid_layer = clone($(field_string(layer, "__identifier", error)));

    if (is_auto) {
        Nob_String_View sv = nob_sv_from_cstr(grid_layer);
        if (!nob_sv_end_with(sv, "_auto")) {
            Nob_String_Builder sb = {0};
            nob_sb_append_cstr(&sb, "Expected layer ");
            nob_sb_append_cstr(&sb, grid_layer);
            nob_sb_append_cstr(&sb, " to end with '_auto'");
            nob_sb_append_null(&sb);

            *error = sb.items;
            goto end;
        } else {
            grid_layer[sv.count - 5] = '\0';
        }
    }

    size_t instances_n = json_array_size(instances);
    for (size_t i = 0; i < instances_n; ++i) {
        json_t *entity = $(item_object(instances, i, error));
        Entity e;

        json_t *px = $(field_array(entity, "px", error));
        e.x = $(item_int(px, 0, error)) / 16 + 1 + offset_x;
        e.y = $(item_int(px, 1, error)) / 16 + 1 + offset_y;
        e.grid_layer = clone(grid_layer);

        e.identifier.type = Identifier_integer;
        e.identifier.value.integer = $(field_int(entity, "t", error)) + 1;

        e.args = NULL;
        e.rails_name = NULL;

        MUST(use_captures(&e, captures, error));

        nob_da_append(entities, e);
    }

end:
    free(grid_layer);
    return;
}

/* PLAN:
 *
 * read total grid size
 * read all layers; entity captures are getting cached
 * apply entity captures, setting `.name`s
 */

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
        int offset_x = $(field_int(level, "worldX", error)) / 16;
        int offset_y = $(field_int(level, "worldY", error)) / 16;
        json_t *layers = $(field_array(level, "layerInstances", error));

        Captures captures = {0};
        size_t layers_n = json_array_size(layers);
        for (size_t j = 0; j < layers_n; ++j) {
            json_t *layer = $(item_object(layers, j, error));

            const char *identifier = $(field_string(layer, "__identifier", error));

            if (strcmp(identifier, "positions") == 0) {
                MUST(put_positions(layer, &result.positions, &captures, error));
                continue;
            }

            const char *type = $(field_string(layer, "__type", error));

            if (strcmp(type, "Entities") == 0) {
                MUST(put_entities(layer, &captures, &result.entities, error));
            } else if (strcmp(type, "Tiles") == 0) {
                MUST(put_tiles(layer, offset_x, offset_y, false, &captures, &result.entities, error));
            } else if (strcmp(type, "IntGrid") == 0) {
                MUST(put_tiles(layer, offset_x, offset_y, true, &captures, &result.entities, error));
            } else {
                Nob_String_Builder sb = {0};
                nob_sb_append_cstr(&sb, "Unknown layer type ");
                nob_sb_append_cstr(&sb, type);
                nob_sb_append_null(&sb);
                *error = sb.items;
                goto end;
            }
        }

        if (captures.count > 0) {
            Nob_String_Builder sb = {0};
            nob_sb_append_cstr(&sb, "Entity capture misses: ");
            nob_da_foreach(Capture, c, &captures) {
                nob_sb_append_cstr(&sb, c->rails_name);
                nob_sb_append_cstr(&sb, ", ");
            }

            *error = sb.items;
            goto end;
        }

        nob_da_free(captures);
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

    printf("Size: (%d, %d)\n", fallen_level.w, fallen_level.h);

    printf("\nPositions:\n");
    for (size_t i = 0; i < fallen_level.positions.count; ++i) {
        Position p = fallen_level.positions.items[i];
        printf("- %s: (%d, %d)\n", p.rails_name, p.x, p.y);
    }

    printf("\nEntities (%zu):\n", fallen_level.entities.count);
    nob_da_foreach(Entity, e, &fallen_level.entities) {
        const char *rails_name = "";
        if (e->rails_name != NULL) rails_name = e->rails_name;

        const char *args = "";
        if (e->args != NULL) args = e->args;

        if (e->identifier.type == Identifier_string) {
            printf("- %s: %s@(%d, %d) %s %s\n", e->identifier.value.string, e->grid_layer, e->x, e->y, rails_name, args);
        } else {
            printf("- %d: %s@(%d, %d) %s %s\n", e->identifier.value.integer, e->grid_layer, e->x, e->y, rails_name, args);
        }
    }
    
    return 0;
}
