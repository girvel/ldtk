#include "ldtk.h"
#include "nob.h"
#include <stdio.h>

int main() {
    char *error = NULL;
    Level fallen_level = read_level("../dot/level/main.ldtk", &error);
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

    free_level(fallen_level);
    
    return 0;
}
