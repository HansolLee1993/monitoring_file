#include "../include/util.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

void create_directory(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);
}

char * append(char * string1, char * string2) {
    char * result = NULL;
    sprintf(&result, "%s%s", string1, string2);
    return result;
}
