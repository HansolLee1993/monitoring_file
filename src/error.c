#include "../include/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


_Noreturn void error_errno(const char *file, const char *func, const size_t line, int err_code, int exit_code) {
    const char *msg;

    msg = strerror(err_code);
    fprintf(stderr, "Error (%s @ %s:%zu %d) - %s\n", file, func, line, err_code, msg);
    exit(exit_code);
}


_Noreturn void error_message(const char *file, const char *func, const size_t line, const char *msg, int exit_code) {
    fprintf(stderr, "Error (%s @ %s:%zu) - %s\n", file, func, line, msg);
    exit(exit_code);
}
