#include <tuxproc/region.h>

namespace TuxProc {

Region::Region(char* _filePath, uint8_t _mode, uintptr_t 
        _regionStart, uintptr_t _regionEnd)
{
    strcpy(filePath, _filePath);
    mode = _mode;

    char* lslashPointer = strrchr(filePath, (int) '/');
    if (lslashPointer != nullptr) {
        fileNameOffset = lslashPointer - filePath + 1;
    } else {
        fileNameOffset = 0;
    }

    regionStart = _regionStart;
    regionEnd = _regionEnd;
}

char* Region::getFilePath()
{
    return filePath;
}

char* Region::getFileName()
{
    return filePath + fileNameOffset;
}

uintptr_t Region::getRegionEnd()
{
    return regionEnd;
}

uintptr_t Region::getRegionStart()
{
    return regionStart;
}

uintptr_t Region::getSize()
{
    return regionEnd - regionStart;
}

uint8_t Region::getMode()
{
    return mode;
}

}
