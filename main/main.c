#include <stdio.h>
#include "exosite_pal.h"
#include "exosite.h"

int main()
{
    EXO_STATE activation_results;
    char* vendorStr = "h";
    char* modelStr = "h";
    activation_results = exosite_init();
    if (activation_results == EXO_STATE_INIT_COMPLETE)
    {
            printf("Activated\n");
            char * dataToWrite = "data_in=\"yepnope\"";
            exosite_write(dataToWrite, exoPal_strlen(dataToWrite));
    }
}
