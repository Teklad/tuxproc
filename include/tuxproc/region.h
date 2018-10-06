#ifndef  __TUXPROC_REGION_H__
#define  __TUXPROC_REGION_H__
#include <cstdint>
#include <string>

namespace TuxProc {

/**
 * @brief Class which holds information about a memory region.
 */
class Region {
    friend class Process;
    public:
        Region(std::string szPath, uintptr_t start, uintptr_t end);
        Region(const Region&) = delete;
        Region& operator=(const Region&) = delete;
        Region(Region&&) = default;
        Region& operator=(Region&&) = default;

        /**
         * @brief Retrieves the absolute path of the region
         *
         * @return absolute file path for the region
         */
        inline const std::string& GetAbsolutePath() const;
        inline std::string& GetAbsolutePath();

        /**
         * @brief Retrieves the file name of the region
         *
         * @return file name of the region
         */
        inline const std::string& GetFileName() const;
        inline std::string& GetFileName();

        /**
         * @brief Retrieves the address at which the region's memory ends
         *
         * @return end address of the region
         */
        inline uintptr_t GetEndAddress() const;

        /**
         * @brief Retrieves the address at which the region's memory starts
         *
         * @return start address of the region
         */
        inline uintptr_t GetStartAddress() const;

        /**
         * @brief (Currently unimplemented) Retrieves the text address
         *        of the region's ELF-formatted file.
         *
         * @return .text address of the region file
         */
        inline uintptr_t GetTextAddress() const;

        /**
         * @brief Retrieves the total size of the region.  This is the same as
         *        doing GetEndAddress()-GetStartAddress().
         *
         * @return total size of the region's memory
         */
        inline uintptr_t GetSize() const;
    private:
        std::string m_szAbsolutePath;
        std::string m_szFileName;
        uintptr_t m_nEndAddress;
        uintptr_t m_nStartAddress;
        uintptr_t m_nTextAddress; //Unimplemented
};

inline const std::string& Region::GetAbsolutePath() const
{
    return m_szAbsolutePath;
}

inline std::string& Region::GetAbsolutePath()
{
    return m_szAbsolutePath;
}

inline const std::string& Region::GetFileName() const
{
    return m_szFileName;
}

inline std::string& Region::GetFileName()
{
    return m_szFileName;
}

inline uintptr_t Region::GetEndAddress() const
{
    return m_nEndAddress;
}

inline uintptr_t Region::GetStartAddress() const
{
    return m_nStartAddress;
}

inline uintptr_t Region::GetTextAddress() const
{
    return m_nTextAddress;
}

inline uintptr_t Region::GetSize() const
{
    return m_nEndAddress - m_nStartAddress;
}

}

#endif //__TUXPROC_REGION_H__
