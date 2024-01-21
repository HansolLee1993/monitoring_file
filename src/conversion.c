#include "../include/conversion.h"
#include "../include/error.h"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>


in_port_t parse_port(const char *buff, int radix) {
    char *end;
    long sl;
    in_port_t port;
    const char *msg;

    errno = 0;
    sl = strtol(buff, &end, radix);

    if (end == buff) {
        msg = "port is not a decimal number";
    }
    else if (*end != '\0') {
        msg = "%s: port is extra characters at end of input";
    }
    else if ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno) {
        msg = "port is out of range of type long";
    }
    else if (sl > UINT16_MAX) {
        msg = "port is greater than UINT16_MAX";
    }
    else if (sl < 0) {
        msg = "port is less than 0";
    }
    else {
        msg = NULL;
    }

    if (msg) {
        printf("[-]error %s\n", msg);
        error_message(__FILE__, __func__ , __LINE__, msg, 6);
    }

    port = (in_port_t)sl;

    return port;
}
