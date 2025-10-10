#include <jansson.h>

typedef struct {
    int w, h;
} Level;

Level read_level(const char *path, char **error);
