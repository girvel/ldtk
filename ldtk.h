#include <jansson.h>
#include <stddef.h>


typedef struct {
    int x;
    int y;
    char *name;
} Position;

typedef struct {
    Position *items;
    size_t count;
    size_t capacity;
} Positions;

typedef struct {
    int w;
    int h;
    Positions positions;
} Level;

Level read_level(const char *path, char **error);
