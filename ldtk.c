#include <stdio.h>
#include "nob.h"

int main() {
    Nob_String_Builder sb = {0};
    nob_read_entire_file("../dot/levels/main/main.ldtk", &sb);

    printf("%s\n", sb.items);
}
