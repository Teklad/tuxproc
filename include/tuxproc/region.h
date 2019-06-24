#ifndef  __TUXPROC_REGION_H__
#define  __TUXPROC_REGION_H__

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace TuxProc {

const uint8_t READ = 1;
const uint8_t WRITE = 2;
const uint8_t EXEC = 4;
const uint8_t PRIVATE = 8;

/**
 * @brief Class which holds information about a memory region.
 */
class Region {
    public:
        Region(char* filePath, uint8_t mode, uintptr_t start, uintptr_t end);
        Region(const Region&) = delete;
        Region& operator=(const Region&) = delete;
        Region(Region&&) = default;
        Region& operator=(Region&&) = default;

        /**
         * @brief Retrieves the absolute path of the region
         *
         * @return absolute file path for the region
         */
        char* getFilePath();

        /**
         * @brief Retrieves the file name of the region
         *
         * @return file name of the region
         */
        char* getFileName();

        /**
         * @brief Retrieves the address at which the region's memory ends
         *
         * @return end address of the region
         */
        uintptr_t getRegionEnd();

        /**
         * @brief Retrieves the address at which the region's memory starts
         *
         * @return start address of the region
         */
        uintptr_t getRegionStart();

        /**
         * @brief Retrieves the total size of the region.  This is the same as
         *        doing GetEndAddress()-GetStartAddress().
         *
         * @return total size of the region's memory
         */
        uintptr_t getSize();

        /**
         * @brief Retrieves the bitmask mode of the region [rwxp].
         *
         * @return mode byte
         */
        uint8_t getMode();


    private:
        char filePath[200];
        int fileNameOffset; // offset in filePath
        uint8_t mode;
        uintptr_t regionEnd;
        uintptr_t regionStart;
};

}

#endif //__TUXPROC_REGION_H__
