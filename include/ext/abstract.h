#ifndef ABSTRACT_H
#define ABSTRACT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int cpath(const char *input, char *output, size_t output_size, int to_absolute);

char* read_out(const char *filename);
int write_to(const char *filename, const char *data);
int copy_to(const char *target, const char *location);
int move_to(const char *target, const char *location);
int delete_from(const char *target);
int link_to(const char *target, const char *location);

int make_dir(const char *dirname);
int switch_to(const char *location);
char* list_out(const char *directory);

int cmdexec(const char *file, char *const argv[]);

#ifdef __cplusplus
}
#endif

#endif
