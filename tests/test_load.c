#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FERSlib.h"

int main(int argc, char **argv)
{
    char err[1024];
    int ret;

    printf("=== FERS LIB TEST START ===\n");

    // 1) Version check (prima cosa, sempre)
    printf("Lib version: %s\n", FERS_GetLibReleaseNum());
    printf("Lib date   : %s\n", FERS_GetLibReleaseDate());

    // 2) Check error system base
    memset(err, 0, sizeof(err));
    FERS_GetLastError(err);
    printf("Last error (clean state): %s\n", err);

    // 3) Try a harmless call (non sempre serve hardware vero)
    printf("\nTrying GetNumBrdConnected...\n");
    int n = FERS_GetNumBrdConnected();
    printf("Boards detected: %d\n", n);

    // 4) Optional: try opening fake or real device
    // ATTENZIONE: cambia path con uno vero se vuoi test serio
    const char *path = "usb:0";

    printf("\nTrying OpenDevice(%s)...\n", path);

    int h;
    ret = FERS_OpenDevice(path, &h);

    if (ret == 0)
    {
        printf("OpenDevice OK\n");

        FERS_BoardInfo_t info;
        ret = FERS_GetBoardInfo(h, &info);

        if (ret == 0)
        {
            printf("Board PID: %d\n", info.pid);
            printf("FPGA FW  : %08X\n", info.FPGA_FWrev);
            printf("Model    : %s\n", info.ModelName);
        }
        else
        {
            FERS_GetLastError(err);
            printf("GetBoardInfo failed: %s\n", err);
        }
    }
    else
    {
        FERS_GetLastError(err);
        printf("OpenDevice failed: %s\n", err);
    }

    printf("\n=== END TEST ===\n");
    return 0;
}