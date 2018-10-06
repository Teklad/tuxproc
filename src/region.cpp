#include <tuxproc/region.h>
#include <cstdio>
TuxProc::Region::Region(std::string szPath, uintptr_t nStart, uintptr_t nEnd)
{
    m_szAbsolutePath = szPath;
    size_t nBase = m_szAbsolutePath.find_last_of('/');
    if (nBase != std::string::npos) {
        m_szFileName = m_szAbsolutePath.substr(nBase + 1);
    } else {
        m_szFileName = m_szAbsolutePath;
    }
    m_nStartAddress = nStart;
    m_nEndAddress = nEnd;
}
