#include <tuxproc/process.h>

#include <cstdio>
#include <cstring>

#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <signal.h>
#include <unistd.h>

namespace TuxProc {

Process::Process(uint32_t nProcessID)
{
    m_regions.reserve(100);
    m_nProcessID = nProcessID;
}

uint32_t Process::Attach(const char* szProcessName)
{
    m_nProcessID = 0;
    DIR* pDirectory = opendir("/proc");
    if (!pDirectory) {
        return m_nProcessID;
    }
    char* pError;
    struct dirent* pEntry;
    while ((pEntry = readdir(pDirectory))) {
        uint32_t id = strtol(pEntry->d_name, &pError, 10);
        if (id == 0 || *pError != 0) {
            continue;
        }
        char szSymbolicPath[128];
        char szAbsolutePath[FILENAME_MAX];

        snprintf(szSymbolicPath, sizeof(szSymbolicPath), "/proc/%u/exe", id);
        ssize_t nBytesRead = readlink(szSymbolicPath, szAbsolutePath, sizeof(szAbsolutePath));
        if (nBytesRead > 0) {
            szAbsolutePath[nBytesRead] = 0;
            const char* szFileName = basename(szAbsolutePath);
            if (!strcmp(szFileName, szProcessName)) {
                m_nProcessID = id;
                break;
            }
        }
    }
    closedir(pDirectory);
    return m_nProcessID;
}

uint32_t Process::GetPID()
{
    return m_nProcessID;
}

uint32_t Process::Attach(const std::string& szProcessName)
{
    return Attach(szProcessName.c_str());
}

bool Process::IsRunning()
{
    kill(m_nProcessID, 0);
    return (errno != ESRCH);
}

uint32_t Process::ParseMaps()
{
    m_regions.clear();
    char szMapsFile[128];
    snprintf(szMapsFile, sizeof(szMapsFile), "/proc/%u/maps", m_nProcessID);

    FILE* pMapsFile = fopen(szMapsFile, "r");
    if (!pMapsFile) {
        fprintf(stderr, "ParseMaps: Failed to open maps file\n");
        return 0;
    }

    char szLine[1024];
    while (fgets(szLine, sizeof(szLine), pMapsFile)) {
        int m = -1;
        uintptr_t nStart;
        uintptr_t nEnd;

        int result = sscanf(szLine, "%lx-%lx %*4s %*x %*x:%*x %*u %n",
                &nStart, &nEnd, &m);

        if (result != 2 || m < 0 || szLine[m] != '/') {
            continue;
        }

        szLine[strcspn(szLine, "\n")] = 0;
        if (m_regions.empty() || m_regions.back().GetAbsolutePath().compare(&szLine[m])) {
            m_regions.push_back(Region(&szLine[m], nStart, nEnd));
        } else {
            m_regions.back().m_nEndAddress = nEnd;
        }
    }
    fclose(pMapsFile);
    return m_regions.size();
}

Region* Process::GetRegion(const std::string& szRegionName)
{
    for (Region& region : m_regions) {
        if (!szRegionName.compare(region.GetFileName())) {
                return &region;
        }
    }
    return nullptr;
}

size_t Process::HexToBinary(const std::string& szPattern, uint8_t* pByte, uint8_t* pMask)
{
    size_t nTotalBytes = 0;
    size_t nPatternSize = szPattern.size();
    for (size_t i = 0; i < nPatternSize; ++i) {
        if (isxdigit(szPattern[i]) && isxdigit(szPattern[i + 1])) {
            sscanf(szPattern.c_str() + i, "%2x", reinterpret_cast<unsigned int*>(&pByte[nTotalBytes]));
            pMask[nTotalBytes] = true;
            i++;
        } else if (szPattern[i] == '?') {
            pMask[nTotalBytes] = false;
        } else if (szPattern[i] == ' ') {
            continue;
        } else {
            int nErrorPos = szPattern.size() + 9;
            fprintf(stderr, "FindPattern: Invalid hex detected\nPattern: %s\n",
                    szPattern.c_str());
            fprintf(stderr, "%*s\n", nErrorPos, "^");
            return 0;
        }
        ++nTotalBytes;
    }
    return nTotalBytes;
}

uintptr_t Process::FindPattern(Region* region, const std::string& szPattern, int nOffset)
{
    if (!region) {
        return 0;
    }

    uint8_t byte[64], mask[64];
    size_t  nHexCount = HexToBinary(szPattern, byte, mask);
    if (!nHexCount) {
        return 0;
    }

    uintptr_t nRegionSize = region->GetSize();
    if (m_pReadBuffer.size() < nRegionSize) {
        m_pReadBuffer.resize(nRegionSize);
    }

    uintptr_t nRegionStart = region->GetStartAddress();

    ssize_t nReadSize = region->GetSize();

    if (m_pLastRegion != region) {
        nReadSize = ReadMemory(nRegionStart, m_pReadBuffer.data(), nRegionSize);
        m_pLastRegion = region;
    }

    for (ssize_t i = 0; i < nReadSize; ++i) {
        bool found = true;
        for (size_t match = 0; match < nHexCount; ++match) {
            found = (!mask[match]) || byte[match] == m_pReadBuffer[match + i];
            if (!found) {
                break;
            }
        }
        if (found) {
            return nRegionStart + i + nOffset;
        }
    }
    return 0;
}

uintptr_t Process::FindPattern(const std::string& szRegionName, const std::string& szPattern, int nOffset)
{
    return FindPattern(GetRegion(szRegionName), szPattern, nOffset);
}

uintptr_t Process::GetAbsoluteAddress(uintptr_t nAddress, uint32_t nOffset, uint32_t nExtra)
{
    uint32_t nCode = 0;
    if (ReadMemory(nAddress + nOffset, &nCode, sizeof(nCode)) > 0) {
        return nAddress + nCode + nExtra;
    }
    return 0;
}

uintptr_t Process::GetCallAddress(uintptr_t nAddress)
{
    return GetAbsoluteAddress(nAddress, 1, 5);
}

}
