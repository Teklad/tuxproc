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
        Process(uint32_t nProcessID = 0);
        ~Process() = default;

        /**
         * @brief Finds the process with the given name and sets m_nProcessID
         *        internally.
         *
         * @param szProcessName The name of the process you'd like to attach to
         *                      this is the same as if you were using pidof
         *
         * @return PID of the process, or 0 if none was found
         */
        uint32_t Attach(const std::string& szProcessName);
        uint32_t Attach(const char* szProcessName);
        
        /**
         * @brief Checks to see if the currently "attached" process is running.
         *
         * @return true if it is, otherwise false
         */
        bool IsRunning();

        /**
         * @brief Parses the /proc/PID/maps file for memory regions which can
         *        be used when reading/writing memory.
         *
         * @return Number of regions found, or 0 on failure
         */
        uint32_t ParseMaps();

        /**
         * @brief Finds a binary pattern in memory using hex-based input.
         *
         * @param pRegion pointer to a TuxProc::Region object
         * @param szPattern hex pattern string
         * @param nOffset number of bytes to offset the return value
         *
         * @return address of found pattern + offset, or 0 on failure
         */
        uintptr_t FindPattern(Region* pRegion, const std::string& szPattern, int nOffset);

        /**
         * @brief Finds a binary pattern in memory using hex-based input.
         *
         * @param szRegionName string containing the filename of the region
         * @param szPattern hex pattern string
         * @param nOffset number of bytes to offset the return value
         *
         * @return address of found pattern + offset, or 0 on failure
         */
        uintptr_t FindPattern(const std::string& szRegionName, const std::string& szPattern, int nOffset);

        /**
         * @brief Reads the provided address and returns the address it points
         *        to.
         *
         * @param nAddress original address
         * @param nOffset number of bytes to offset the address prior to reading
         * @param nExtra number of bytes to offset the address after reading
         *
         * @return new memory address
         */
        uintptr_t GetAbsoluteAddress(uintptr_t nAddress, uint32_t nOffset, uint32_t nExtra);

        /**
         * @brief Reads the provided address and returns the address it points
         *        to, skipping the opcode.  This is implemented using
         *        GetAbsoluteAddress.
         *
         * @param nAddress original address
         *
         * @return new memory address
         */
        uintptr_t GetCallAddress(uintptr_t nAddress);

        /**
         * @brief Looks through m_regions and finds the region that matches the
         *        given region file name.  ParseMaps should be used prior to
         *        calling this function.
         *
         * @param szRegionName filename of the region
         *
         * @return pointer to a region, or nullptr if no region matches
         */
        Region* GetRegion(const std::string& szRegionName);

        /**
         * @brief Reads the given memory address and puts the result in
         *        pResult.
         *
         * @param nAddress address to read
         * @param pResult pointer to a storage object/variable
         * @param nReadSize number of bytes to read
         *
         * @return number of bytes read, or -1 on failure
         */
        inline ssize_t ReadMemory(uintptr_t nAddress, void* pResult, size_t nReadSize);

        /**
         * @brief Writes the data provided in pValue into the memory address
         *        provided.
         *
         * @param nAddress address to write
         * @param pValue pointer to a storage object/variable
         * @param nWriteSize number of bytes to write
         *
         * @return number of bytes written, or -1 on failure
         */
        inline ssize_t WriteMemory(uintptr_t nAddress, void* pValue, size_t nWriteSize);
        
        /**
         * @brief Reads the given memory address and returns the result
         *        directly.  This function assumes the memory address is valid
         *        and the provided type "T" is correct.
         *
         * @tparam T type of variable to read
         * @param nAddress address to read
         * @param nReadSize number of bytes to read (optional except when T is
         *                  of a string type)
         *
         * @return T containing the value of the read address
         */
        template<typename T>
        inline T Read(uintptr_t nAddress, size_t nReadSize = sizeof(T));

        /**
         * @brief Writes to the given memory address.
         *
         * @tparam T type of variable to write
         * @param nAddress address to write
         * @param pValue variable or value to write
         * @param nWriteSize number of bytes to write (optional except when T is
         *                   of a string type)
         *
         * @return true if any bytes were written, otherwise false
         */
        template<typename T>
        inline bool Write(uintptr_t nAddress, T pValue, size_t nWriteSize = sizeof(T));

    private:
        size_t HexToBinary(const std::string& szPattern, uint8_t* pByte, uint8_t* pMask);
        uint32_t m_nProcessID;
        Region* m_pLastRegion = nullptr;
        std::vector<uint8_t> m_pReadBuffer;
        std::vector<Region> m_regions;
};

inline ssize_t Process::ReadMemory(uintptr_t nAddress, void* pResult, size_t nReadSize)
{
    struct iovec mLocal = {pResult, nReadSize};
    struct iovec mRemote = {reinterpret_cast<void*>(nAddress), nReadSize};
    return process_vm_readv(m_nProcessID, &mLocal, 1, &mRemote, 1, 0);
}

inline ssize_t Process::WriteMemory(uintptr_t nAddress, void* pValue, size_t nWriteSize)
{
    struct iovec mLocal = {pValue, nWriteSize};
    struct iovec mRemote = {reinterpret_cast<void*>(nAddress), nWriteSize};
    return process_vm_writev(m_nProcessID, &mLocal, 1, &mRemote, 1, 0);
}

template<typename T>
inline T Process::Read(uintptr_t nAddress, size_t nReadSize)
{
    T pResult;
    ReadMemory(nAddress, &pResult, nReadSize);
    return pResult;
}

template<>
inline std::string Process::Read(uintptr_t nAddress, size_t nReadSize)
{
    std::string pResult(nReadSize, 0);
    ReadMemory(nAddress, &pResult[0], nReadSize);
    return pResult;
}

template<typename T>
inline bool Process::Write(uintptr_t nAddress, T pValue, size_t nWriteSize)
{
    return WriteMemory(nAddress, &pValue, nWriteSize) > 0;
}


}
#endif //__TUXPROC_PROCESS_H__
