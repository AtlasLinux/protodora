#include <stdio.h>
#include <stdlib.h>
#include "ext/abstract.h"

int main(int argc, char *argv[]) {
    const char *pandora = getenv("PANDORA");
    if (!pandora) pandora = ".pandora";
    char include_path[1024];
    char lib_path[1024];
    snprintf(include_path, sizeof(include_path), "%s/include", pandora);
    snprintf(lib_path, sizeof(lib_path), "%s/lib", pandora);
    const char *ld_lib = getenv("LD_LIBRARY_PATH");
    char ld_path[2048];
    if (ld_lib && ld_lib[0] != '\0')
        snprintf(ld_path, sizeof(ld_path), "%s:%s", lib_path, ld_lib);
    else
        snprintf(ld_path, sizeof(ld_path), "%s", lib_path);
    setenv("LD_LIBRARY_PATH", ld_path, 1);
    char **new_argv = malloc(sizeof(char*) * (argc + 2));
    int i = 0;
    new_argv[i++] = "c++";
    new_argv[i++] = "-I";
    new_argv[i++] = include_path;
    for (int j = 1; j < argc; j++)
        new_argv[i++] = argv[j];
    new_argv[i] = NULL;
    return cmdexec("c++", new_argv);
}
