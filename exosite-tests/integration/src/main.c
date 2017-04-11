
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>

#include "exosite_async.h"
#include "exosite_pal_async.h"



Exosite_state_t exoLib;
exoPal_state_t exoPal;

int intgr8_on_start(Exosite_state_t *state, int status)
{
    printf("Started: %d\n", status);
    return 0;
}

int main(int argc, char **argv)
{
    int ret;

    exosite_init(&exoLib);
    exoLib.exoPal = &exoPal;
    exoLib.ops.on_start_complete = intgr8_on_start;

    setenv("INTGR8_VENDOR", "1234567890", 0);
    setenv("INTGR8_MODEL", "1234567890", 0);
    setenv("INTGR8_SN", "1234567890", 0);

    exosite_start(&exoLib);


    while(1) {
        ret = exoPal_processEvents(&exoPal);
        if(ret != 0) {
            fprintf(stderr, "XXX Processed failed: %d\n", ret);
            return ret;
        }
    }

    return 0;
}

/* vim: set ai cin et sw=4 ts=4 : */
