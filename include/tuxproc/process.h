#ifndef __TUXPROC_PROCESS_H__
#define __TUXPROC_PROCESS_H__

#include "region.h"

#include <vector>
#include <sys/uio.h>

namespace TuxProc {

/**
 * @brief Class which provides a memory interface to a process for easier
 *        reading/writing
 */
class Process {
    public:
        Process(uint32_t processID = 0);
        ~Process() = default;

        /**
         * @brief Finds the process with the given name and sets processID
         *        internally.
         *
         * @param processName The name of the process you'd like to attach to
         *
         * @return PID of the process, or 0 if none was found
         */
        pid_t attach(const char* processName);

        /**
         * @brief Gets the current process ID
         *
         * @return PID
         */
        pid_t getPID();
        /**
         * @brief Checks to see if the currently "attached" process is running.
         *
         * @return true if it is, otherwise false
         */
        bool isRunning();

        /**
         * @brief Parses the /proc/PID/maps file for memory regions which can
         *        be used when reading/writing memory.
         *
         * @return Number of regions found, or 0 on failure
         */
        uint32_t parseMaps();

        /**
         * @brief Finds a binary pattern in memory using hex-based input.
         *
         * @param region pointer to a TuxProc::Region object
         * @param pattern hex pattern string
         * @param offset number of bytes to offset the return value
         *
         * @return address of found pattern + offset, or 0 on failure
         */
        uintptr_t findPattern(Region* pRegion, const char* pattern, bool reload = false);

        /**
         * @brief Finds the region that matches the given region file name.  
         * ParseMaps should be used prior to calling this function.
         *
         * @param regionName name of file mapped the region
         *
         * @return pointer to a region, or nullptr if no region matches
         */
        Region* getRegion(const char* regionName, int index, uint8_t filter = 0); // 0 -> nothing filtered

        /**
         * @brief Reads the given memory address and saves it in result.
         *
         * @param address address to read
         * @param result pointer to a storage object/variable
         * @param size number of bytes to read
         *
         * @return number of bytes read, or -1 on failure
         */
        ssize_t readMemory(uintptr_t address, void* result, size_t size);

        /**
         * @brief Writes the data provided in the value parameter into the memory address
         *        provided.
         *
         * @param address address to write
         * @param value pointer to a storage object/variable
         * @param size number of bytes to write
         *
         * @return number of bytes written, or -1 on failure
         */
        ssize_t writeMemory(uintptr_t address, void* value, size_t size);

        /**
         * @brief Reads the given memory address and returns the result
         *        directly.  This function assumes the memory address is valid
         *        and the provided type "T" is correct.
         *
         * @tparam T type of variable to read
         * @param address address to read
         * @param size number of bytes to read (optional except when T is
         *                  a cstring)
         *
         * @return T containing the value of the read address
         */
        template<typename T>
        T read(uintptr_t address, size_t size = sizeof(T));

        /**
         * @brief Writes to the given memory address.
         *
         * @tparam T type of variable to write
         * @param address address to write
         * @param value variable or value to write
         * @param size number of bytes to write (optional except when T is
         *                   a cstring)
         *
         * @return true if any bytes were written, otherwise false
         */
        template<typename T>
        bool write(uintptr_t address, T value, size_t size = sizeof(T));

    private:
        size_t convertHex2Bin(const char* pattern, uint8_t* bytes, uint8_t* byteMask);
        pid_t processID;
        Region* lastRegionRead = nullptr;
        std::vector<uint8_t> readBuffer;
        std::vector<Region> regions;
};

}

#endif //__TUXPROC_PROCESS_H__
