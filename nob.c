#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define MUJS_FOLDER "mujs-1.3.7/"

#define PROJECT "vitadeck"
#define TITLE "VitaDeck"
#define TITLE_ID "PGRI00001"
#define OUT_DIR "out"

#define ELF_FILE OUT_DIR"/"PROJECT".elf"
#define VELF_FILE OUT_DIR"/"PROJECT".velf"
#define SFO_FILE OUT_DIR"/param.sfo"
#define EBOOT_FILE OUT_DIR"/eboot.bin"
#define VPK_FILE OUT_DIR"/"PROJECT".vpk"
#define MUJS_OBJECT_FILE OUT_DIR"/mujs.o"

#define GCC_CMD "arm-vita-eabi-gcc"
#define GCC_FLAGS "-Wall", "-Wextra", "-Wno-unused-parameter", "-Wl,-q"
#define INCLUDE_DIRS "-I./common", "-I./"MUJS_FOLDER
// NOTE: debugScreen is no longer used; switch to Raylib rendering
#define SOURCE_FILES "src/main.c"
// Link Raylib and common Vita stubs required by the Raylib VitaGL backend
#define LIBRARIES \
    "-lraylib", \
    "-lvitaGL", \
    "-lSceGxm_stub", \
    "-lSceDisplay_stub", \
    "-lSceCtrl_stub", \
    "-lSceTouch_stub", \
    "-lSceCommonDialog_stub", \
    "-lScePgf_stub", \
    "-lSceGxt_stub", \
    "-lSceSysmodule_stub", \
    "-lm"

Cmd cmd = {0};

int build_mujs_object() {
    if(needs_rebuild1(MUJS_OBJECT_FILE, MUJS_FOLDER"one.c")) {
        nob_log(NOB_INFO, "Building MuJS object\n");
        cmd_append(&cmd, GCC_CMD, GCC_FLAGS, INCLUDE_DIRS, MUJS_FOLDER"one.c", "-c", "-o", MUJS_OBJECT_FILE);
        if (!cmd_run(&cmd)) return 1;
    } else {
        nob_log(NOB_INFO, "MuJS object already built\n");
    }
    return 0;
}

int main(int argc, char *argv[]) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (argc > 1 && strcmp(argv[1], "clean") == 0) {
        delete_file(ELF_FILE);
        delete_file(VELF_FILE);
        delete_file(SFO_FILE);
        delete_file(EBOOT_FILE);
        delete_file(VPK_FILE);
        return 0;
    }

    // Ensure out directory exists
    mkdir_if_not_exists(OUT_DIR);

    if(build_mujs_object()) return 1;

    // Build ELF
    cmd_append(&cmd,
        GCC_CMD,
        GCC_FLAGS,
        INCLUDE_DIRS,
        SOURCE_FILES,
        MUJS_OBJECT_FILE,
        LIBRARIES,
        "-o", ELF_FILE
    );
    if (!cmd_run(&cmd)) return 1;

    // Strip symbols
    cmd_append(&cmd, "arm-vita-eabi-strip", "-g", ELF_FILE);
    if (!cmd_run(&cmd)) return 1;

    // Create VELF
    cmd_append(&cmd, "vita-elf-create", ELF_FILE, VELF_FILE);
    if (!cmd_run(&cmd)) return 1;

    // Create SFO
    cmd_append(&cmd, "vita-mksfoex", "-s", temp_sprintf("TITLE_ID=%s", TITLE_ID), TITLE, SFO_FILE);
    if (!cmd_run(&cmd)) return 1;

    // Create EBOOT
    cmd_append(&cmd, "vita-make-fself", VELF_FILE, EBOOT_FILE);
    if (!cmd_run(&cmd)) return 1;

    // Pack VPK
    cmd_append(&cmd, "vita-pack-vpk",
        "-s", SFO_FILE,
        "-b", EBOOT_FILE,
        "--add", "sce_sys/icon0.png=sce_sys/icon0.png",
        "--add", "sce_sys/livearea/contents/bg.png=sce_sys/livearea/contents/bg.png",
        "--add", "sce_sys/livearea/contents/startup.png=sce_sys/livearea/contents/startup.png",
        "--add", "sce_sys/livearea/contents/template.xml=sce_sys/livearea/contents/template.xml",
        VPK_FILE
    );
    if (!cmd_run(&cmd)) return 1;

    return 0;
}