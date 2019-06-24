#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <inttypes.h>

#include "tuxproc/process.h"

const char* pattern = "68 65 6c 6c 6f"; // "hello" in hex
const char* replace = "works"; // string to replace hello with
const char* targetRegion = "test";

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Please specify PID of process\n");
        return 1;
    }

    if (getuid() != 0) {
        printf("Please restart the program with su rights\n");
        return 2;
    }

    char* pEnd;
    long int pid = strtol(argv[1], &pEnd, 10);

    TuxProc::Process process(pid);

    while (!process.isRunning()) usleep(100000);

    printf("attached!\n");

    process.parseMaps();

    TuxProc::Region* region = process.getRegion(targetRegion, 0);
    if (!region) {
        printf("Failed to find region 'test' ... probably using PID of a different process\n");
        return 3;
    }

    int regionCounter = 0;
    bool found = false;
    do {
        uintptr_t p = process.findPattern(region, pattern);
        if (p) {
            found = true;
            printf("Found instance at %" PRIxPTR "\n", p);
            if (process.writeMemory(p, (char*) replace, sizeof(replace)) != -1) {
                printf("> Successfully modified remote value!\n");
            } else {
                printf("> Failed to modify remote value.\n");
            }
        }
        regionCounter++;
        region = process.getRegion(targetRegion, regionCounter);
    } while (region);

    if (!found) printf("No instances found!\n");

    return 0;
}
