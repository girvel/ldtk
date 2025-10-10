#include <stddef.h>


typedef struct {
    int x;
    int y;
    char *rails_name;
} Position;

typedef struct {
    Position *items;
    size_t count;
    size_t capacity;
} Positions;

typedef enum {
    Identifier_string,
    Identifier_integer,
} IdentifierType;

typedef union {
    char *string;
    int integer;
} IdentifierValue;

typedef struct {
    IdentifierType type;
    IdentifierValue value;
} Identifier;

typedef struct {
    int x;
    int y;
    char *grid_layer;
    Identifier identifier;
    char *rails_name;
    char *args;
} Entity;

typedef struct {
    Entity *items;
    size_t count;
    size_t capacity;
} Entities;

typedef struct {
    int w;
    int h;
    Positions positions;
    Entities entities;
} Level;

Level read_level(const char *path, char **error);
void free_level(Level level);
