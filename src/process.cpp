#include <tuxproc/process.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>


namespace TuxProc {

const char unknownDescriptor[] = "[dynamic]";

Process::Process(uint32_t _processID)
{
    regions.reserve(100);
    processID = _processID;
}

pid_t Process::attach(const char* processName)
{
    // ** search processes for one with processName by inspecting each /proc/<uid>/exe path

    // open proc directory
    processID = 0;
    DIR* procDirectory = opendir("/proc");
    if (!procDirectory) {
        return processID;
    }

    // parse directory files
    char* error;
    struct dirent* entry;
    while ((entry = readdir(procDirectory))) {
        // name of file is the pid
        uint32_t id = strtol(entry->d_name, &error, 10);
        if (id == 0 || *error != 0) {
            continue;
        }
        char symbolicPath[128];
        char truePath[FILENAME_MAX];

        // resolve path of exe symlink
        snprintf(symbolicPath, sizeof(symbolicPath), "/proc/%u/exe", id);
        size_t pathLength = readlink(symbolicPath, truePath, sizeof(truePath));
        if (pathLength > 0) {
            truePath[pathLength] = 0;
            // check if executable name matches supplied processName
            const char* szFileName = basename(truePath);
            if (strcmp(szFileName, processName) == 0) {
                processID = id;
                break;
            }
        }
    }
    closedir(procDirectory);
    return processID;
}

pid_t Process::getPID()
{
    return processID;
}


bool Process::isRunning()
{
    kill(processID, 0); // sig 0 => nothing sent, but error checking is performed
    return (errno != ESRCH);
}

uint32_t Process::parseMaps()
{
    // create maps file path
    regions.clear();
    char mapsFilePath[128];
    snprintf(mapsFilePath, sizeof(mapsFilePath), "/proc/%u/maps", processID);

    // open file
    FILE* mapsFile = fopen(mapsFilePath, "r");
    if (!mapsFile) {
        fprintf(stderr, "ParseMaps: Failed to open maps file\n");
        return 0;
    }

    // read line by line
    char line[1024];
    while (fgets(line, sizeof(line), mapsFile)) {
        uintptr_t regionStart;
        uintptr_t regionEnd;
        char modestr[5];
        int inode;
        int pathstrOffset = -1;

        // scan line for information
        int scanned = sscanf(line, "%lx-%lx %4s %*x %*x:%*x %u %n",
                &regionStart, &regionEnd, modestr, &inode, &pathstrOffset);

        if (scanned < 4) {
            continue;
        }

        // convert modestr to bitmask r = 1, w = 2, x = 4, p = 8
        uint8_t mode = 0;
        for (int i = 0; i < 4; i++) {
            if (modestr[i] != '-') {
                mode += std::pow(2, i);
            }
        }

        // check if region was mapped from file
        char* regionDescriptor = (inode == 0) ? (char*) unknownDescriptor : line + pathstrOffset;

        line[strcspn(line, "\n")] = 0;

        regions.push_back(Region(regionDescriptor, mode, regionStart, regionEnd));
    }
    fclose(mapsFile);
    return regions.size();
}

Region* Process::getRegion(const char* regionName, int index, uint8_t filter)
{
    int counter = 0;
    for (Region& region : regions) {
        if (((filter & region.getMode()) == filter) && (strcmp(regionName, region.getFileName()) == 0)) {
            if (counter == index) return &region;
            else counter++;
        }
    }
    return nullptr;
}

size_t Process::convertHex2Bin(const char* pattern, uint8_t* hexBytes, uint8_t* patternMask)
{
    // hexBytes and patternMask must be initialized beforehand and must be atleast the size of the amount of hex bytes in the pattern
    size_t byteCounter = 0;
    size_t patternLength = strlen(pattern);
    for (size_t i = 0; i < patternLength; ++i) {
        if (isxdigit(pattern[i]) && isxdigit(pattern[i + 1])) {
            sscanf(pattern + i, "%2x", reinterpret_cast<unsigned int*>(&hexBytes[byteCounter]));
            patternMask[byteCounter] = true;
            i++;
        } else if (pattern[i] == '?') {
            patternMask[byteCounter] = false;
        } else if (pattern[i] == ' ') {
            continue;
        } else {
            int errorPos = patternLength + 9;
            fprintf(stderr, "findPattern: Invalid hex detected\nPattern: %s\n",
                    pattern);
            fprintf(stderr, "%*s\n", errorPos, "^");
            return 0;
        }
        ++byteCounter;
    }
    // return amount of bytes parsed
    return byteCounter;
}

uintptr_t Process::findPattern(Region* region, const char* pattern, bool reload)
{
    if (region == nullptr) {
        return 0;
    }

    uint8_t hexBytes[64], patternMask[64];
    size_t  bytesParsed = convertHex2Bin(pattern, hexBytes, patternMask);
    if (bytesParsed == 0) {
        return 0;
    }

    uintptr_t regionSize = region->getSize();
    if (readBuffer.size() < regionSize) {
        readBuffer.resize(regionSize);
    }

    uintptr_t regionStart = region->getRegionStart();

    ssize_t readSize = regionSize;

    // only reload buffer if specified or new region being read
    if (reload || lastRegionRead != region) {
        readSize = readMemory(regionStart, readBuffer.data(), regionSize);
        lastRegionRead = region;
    }

    // pattern scan
    size_t patternIndex = 0;
    for (ssize_t i = 0; i < readSize; ++i) {
        if (readBuffer[i] == hexBytes[patternIndex] || !patternMask[patternIndex]) patternIndex++;
        else patternIndex = 0;

        if (patternIndex == bytesParsed) return regionStart + i - patternIndex + 1;
    }

    return 0;
}

ssize_t Process::readMemory(uintptr_t address, void* result, size_t size)
{
    struct iovec local = {result, size};
    struct iovec remote = {reinterpret_cast<void*>(address), size};
    return process_vm_readv(processID, &local, 1, &remote, 1, 0);
}

ssize_t Process::writeMemory(uintptr_t address, void* value, size_t size)
{
    struct iovec local = {value, size};
    struct iovec remote = {reinterpret_cast<void*>(address), size};
    return process_vm_writev(processID, &local, 1, &remote, 1, 0);
}

template<typename T>
T Process::read(uintptr_t address, size_t size)
{
    T result;
    readMemory(address, &result, size);
    return result;
}

template<typename T>
bool Process::write(uintptr_t address, T value, size_t size)
{
    return writeMemory(address, &value, size) > 0;
}

}
