#include <assert.h>
#include <stdio.h>
#include <jansson.h>
#include "nob.h"

int main() {
    json_error_t error;
    json_t *root = json_load_file("../dot/levels/main/main.ldtk", 0, &error);

    if (!root) {
        fprintf(stderr, "error: on line %d: %s", error.line, error.text);
        return 1;
    }

    assert(json_is_object(root));

    json_t *world_w_ptr = json_object_get(root, "worldGridWidth");
    assert(json_is_integer(world_w_ptr));

    printf("worldGridWidth is %lld\n", json_integer_value(world_w_ptr));
}
