#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <sys/stat.h>
#include "ext/abstract.h"

#define MAX_PATH 262144

static char STORE_PATH[MAX_PATH];

void init_paths() {
    const char *pandora = getenv("PANDORA");
    if (!pandora) pandora = ".pandora";
    snprintf(STORE_PATH, sizeof(STORE_PATH), "%s/store", pandora);
    make_dir(pandora);
    make_dir(STORE_PATH);
}

int exists(const char *path) {
    struct stat s;
    return stat(path, &s) == 0;
}

void run_build(const char *pkg_path) {
    char script[MAX_PATH];
    snprintf(script, sizeof(script), "%s/build.sh", pkg_path);
    if (!exists(script)) return;
    char *argv[] = {"sh", script, NULL};
    cmdexec("sh", argv);
}

void install_package(const char *pkg);

void install_deps(const char *pkg_path) {
    char req_path[MAX_PATH];
    snprintf(req_path, sizeof(req_path), "%s/require.txt", pkg_path);
    if (!exists(req_path)) return;
    char *content = read_out(req_path);
    if (!content) return;
    char *line = strtok(content, "\n");
    while (line) {
        if (strlen(line) > 0) {
            install_package(line);
        }
        line = strtok(NULL, "\n");
    }
}

void install_package(const char *pkg) {
    char pkg_path[MAX_PATH];
    snprintf(pkg_path, sizeof(pkg_path), "%s/%s", STORE_PATH, pkg);
    if (exists(pkg_path)) {
        printf("[=] '%s' is already installed.\n", pkg);
        return;
    }
    printf("[+] Installing %s...\n", pkg);
    char repo_url[512];
    snprintf(repo_url, sizeof(repo_url),
        "https://github.com/AtlasLinux/panda-registry.git");
    char branch_arg[256];
    snprintf(branch_arg, sizeof(branch_arg), "%s", pkg);
    char *argv[] = {
        "git", "clone",
        "-b", branch_arg,
        "--single-branch",
        repo_url,
        pkg_path,
        NULL
    };
    if (cmdexec("git", argv) != 0) {
        fprintf(stderr, "[-] Failed to clone %s\n", pkg);
        return;
    }
    install_deps(pkg_path);
    run_build(pkg_path);
    printf("[+] Installed '%s'.\n", pkg);
}

void remove_package(const char *pkg) {
    char pkg_path[MAX_PATH];
    snprintf(pkg_path, sizeof(pkg_path), "%s/%s", STORE_PATH, pkg);
    if (!exists(pkg_path)) {
        printf("[-] '%s' not installed.\n", pkg);
        return;
    }
    delete_from(pkg_path);
    printf("[+] Removed '%s'.\n", pkg);
}

void update_package(const char *pkg) {
    printf("[~] Updating '%s'...\n", pkg);
    remove_package(pkg);
    install_package(pkg);
}

void list_packages(const char *pattern) {
    char glob_pattern[MAX_PATH];
    if (!pattern) {
        snprintf(glob_pattern, sizeof(glob_pattern), "%s/*", STORE_PATH);
    } else {
        snprintf(glob_pattern, sizeof(glob_pattern), "%s/%s*", STORE_PATH, pattern);
    }
    glob_t g;
    if (glob(glob_pattern, 0, NULL, &g) == 0) {
        for (size_t i = 0; i < g.gl_pathc; i++) {
            char *name = strrchr(g.gl_pathv[i], '/');
            printf("%s\n", name ? name + 1 : g.gl_pathv[i]);
        }
    }
    globfree(&g);
}

void show_help() {
    char help_path[MAX_PATH];
    snprintf(help_path, sizeof(help_path),
        "%s/doc/user/usage.txt", getenv("PANDORA"));
    char *content = read_out(help_path);
    if (content) {
        printf("%s\n", content);
    } else {
        printf("Help file not found.\n");
    }
}

int main(int argc, char *argv[]) {
    init_paths();
    if (argc < 2) {
        printf("Usage: pandora <command> [args]\n");
        return 0;
    }
    const char *cmd = argv[1];
    if (strcmp(cmd, "install") == 0) {
        if (argc < 3) {
            printf("Usage: install <package>\n");
            return 1;
        }
        install_package(argv[2]);
    } else if (strcmp(cmd, "remove") == 0) {
        if (argc < 3) {
            printf("Usage: remove <package>\n");
            return 1;
        }
        remove_package(argv[2]);
    } else if (strcmp(cmd, "update") == 0) {
        if (argc < 3) {
            printf("Usage: update <package>\n");
            return 1;
        }
        update_package(argv[2]);
    } else if (strcmp(cmd, "list") == 0) {
        if (argc >= 3)
            list_packages(argv[2]);
        else
            list_packages(NULL);
    } else if (strcmp(cmd, "help") == 0) {
        show_help();
    } else {
        printf("Unknown command at '%s'.\n", cmd);
    }
    return 0;
}
