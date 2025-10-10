#include <assert.h>
#define NOB_IMPLEMENTATION
#include "nob.h"

#define MUST(X) do { if (!X) return false; } while (0)

#define BUILD_FOLDER ".build/"

bool run(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "nob.h");

    MUST(nob_mkdir_if_not_exists(BUILD_FOLDER));

    Nob_Cmd cmd = {0};

    nob_cc(&cmd);
    nob_cc_flags(&cmd);
    nob_cmd_append(&cmd, "-c", "-fPIC");
    nob_cc_inputs(&cmd, "ldtk.c");
    nob_cc_output(&cmd, BUILD_FOLDER"ldtk.o");

    MUST(nob_cmd_run(&cmd));

    nob_cc(&cmd);
    nob_cc_flags(&cmd);
    nob_cmd_append(&cmd, "-c", "-fPIC", "-x", "c", "-DNOB_IMPLEMENTATION");
    nob_cc_inputs(&cmd, "nob.h");
    nob_cc_output(&cmd, BUILD_FOLDER"nob.o");

    MUST(nob_cmd_run(&cmd));

    nob_cc(&cmd);
    nob_cc_inputs(&cmd, BUILD_FOLDER"ldtk.o", BUILD_FOLDER"nob.o");
    nob_cc_output(&cmd, BUILD_FOLDER"ldtk");
    nob_cmd_append(&cmd, "-ljansson");

    MUST(nob_cmd_run(&cmd));

    return true;
}

int main(int argc, char **argv) {
    return !run(argc, argv);
}
