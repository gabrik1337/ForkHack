#include "Utils/SString.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

SString& SString::vFormat(const char* szFormat, va_list vl)
{
#ifdef WIN32

    va_list vlLocal;

    va_copy(vlLocal, vl);
    int iRequiredCapacity;
    __try
    {
        iRequiredCapacity = _vscprintf(szFormat, vlLocal);
    }
    __except (1 /* EXCEPTION_EXECUTE_HANDLER */)
    {

        OnFormatException(szFormat);
        return *this;
    }

    if (iRequiredCapacity < 1)
    {

        clear();
        if (iRequiredCapacity == -1)
            OnInvalidParameter(szFormat);
        return *this;
    }

    char* szDest = static_cast<char*>(malloc(iRequiredCapacity + 2));

    va_copy(vlLocal, vl);
    int iSize;
    __try
    {
        iSize = vsnprintf(szDest, iRequiredCapacity + 1, szFormat, vlLocal);
    }
    __except (1 /* EXCEPTION_EXECUTE_HANDLER */)
    {

        free(szDest);
        OnFormatException(szFormat);
        return *this;
    }

    if (iSize < 1)
    {

        clear();
        if (iSize == -1)
            OnInvalidParameter(szFormat);
    }
    else
    {

        szDest[iSize] = '\0';
        std::string::assign(szDest);
    }

    free(szDest);

    return *this;

#else

    va_list vlLocal;

    int iRequiredCapacity = 220;

    char* szDest = static_cast<char*>(malloc(iRequiredCapacity + 1));

    va_copy(vlLocal, vl);
    int iSize = vsnprintf(szDest, iRequiredCapacity, szFormat, vlLocal);
    if (iSize == -1)
    {

        do
        {
            iRequiredCapacity *= 2;
            szDest = static_cast<char*>(realloc(szDest, iRequiredCapacity + 1));
            va_copy(vlLocal, vl);
            iSize = vsnprintf(szDest, iRequiredCapacity, szFormat, vlLocal);
        } while (iSize == -1);
    }
    else if (iSize > iRequiredCapacity)
    {

        iRequiredCapacity = iSize + 1;
        szDest = static_cast<char*>(realloc(szDest, iRequiredCapacity + 1));

        va_copy(vlLocal, vl);
        iSize = vsnprintf(szDest, iRequiredCapacity, szFormat, vlLocal);
    }

    if (iSize < 1)
    {

        clear();
    }
    else
    {

        szDest[iSize] = '\0';
        std::string::assign(szDest);
    }

    free(szDest);

    return *this;
#endif
}

void SString::OnFormatException(const char* szFormat)
{

    *this = (SStringX("[Format exception] ") + szFormat).Replace("%", "#");
}

void SString::OnInvalidParameter(const char* szFormat)
{

    *this = (SStringX("[Invalid parameter] ") + szFormat).Replace("%", "#");
}

void SString::Split(const SString& strDelim, std::vector<SString>& outResult, unsigned int uiMaxAmount, unsigned int uiMinAmount) const
{
    outResult.clear();
    size_t ulStartPoint = 0;

    while (true)
    {
        size_t ulPos = find(strDelim, ulStartPoint);

        if (ulPos == npos || (uiMaxAmount > 0 && uiMaxAmount <= outResult.size() + 1))
        {
            if (ulStartPoint <= length())
                outResult.push_back(substr(ulStartPoint));
            break;
        }

        outResult.push_back(substr(ulStartPoint, ulPos - ulStartPoint));

        ulStartPoint = ulPos + strDelim.length();
    }

    while (outResult.size() < uiMinAmount)
        outResult.push_back("");
}

bool SString::Split(const SString& strDelim, SString* pstrLeft, SString* pstrRight, int iIndex) const
{

    if (this == pstrLeft || this == pstrRight)
        return SString(*this).Split(strDelim, pstrLeft, pstrRight, iIndex);

    assert(iIndex);
    bool   bFromEnd = iIndex < 0;
    size_t ulPos;
    if (!bFromEnd)
    {
        ulPos = 0;
        for (int i = 0; i < iIndex && ulPos != npos; i++)
        {
            if (i)
                ulPos += strDelim.length();
            if (ulPos < length())
            {
                ulPos = find(strDelim, ulPos);
            }
            else
            {
                ulPos = npos;
                break;
            }
        }
    }
    else
    {
        ulPos = length();
        for (int i = 0; i < -iIndex && ulPos != npos; i++)
        {
            if (ulPos >= strDelim.length())
            {
                ulPos -= strDelim.length();
                ulPos = rfind(strDelim, ulPos);
            }
            else
            {
                ulPos = npos;
                break;
            }
        }
    }

    if (ulPos == npos)
    {
        if (pstrLeft)
            *pstrLeft = bFromEnd ? "" : c_str();
        if (pstrRight)
            *pstrRight = bFromEnd ? c_str() : "";
        return false;
    }

    if (pstrLeft)
        *pstrLeft = substr(0, ulPos);

    if (pstrRight)
        *pstrRight = substr(ulPos + strDelim.length(), length() - (ulPos + strDelim.length()));

    return true;
}

SString SString::SplitLeft(const SString& strDelim, SString* pstrRight, int iIndex) const
{
    SString strLeft;
    Split(strDelim, &strLeft, pstrRight, iIndex);
    return strLeft;
}

SString SString::SplitRight(const SString& strDelim, SString* pstrLeft, int iIndex) const
{
    SString strRight;
    Split(strDelim, pstrLeft, &strRight, iIndex);
    return strRight;
}

SString SString::Replace(const char* szOld, const char* szNew, bool bSearchJustReplaced) const
{

    size_t idx = 0;
    if ((idx = this->find(szOld, idx)) == npos)
        return *this;

    size_t  iOldLength = strlen(szOld);
    size_t  iNewLength = strlen(szNew);
    SString strResult = *this;
    do
    {
        strResult.replace(idx, iOldLength, szNew);
        if (!bSearchJustReplaced)
            idx += iNewLength;
    } while ((idx = strResult.find(szOld, idx)) != npos);
    return strResult;
}

SString SString::ReplaceI(const char* szOld, const char* szNew, bool bSearchJustReplaced) const
{
    SString strOldUpper = SStringX(szOld).ToUpper();

    size_t idx = 0;
    if ((idx = this->ToUpper().find(strOldUpper, idx)) == npos)
        return *this;

    size_t  iOldLength = strlen(szOld);
    size_t  iNewLength = strlen(szNew);
    SString strResult = *this;
    do
    {
        strResult.replace(idx, iOldLength, szNew);
        if (!bSearchJustReplaced)
            idx += iNewLength;
    } while ((idx = strResult.ToUpper().find(strOldUpper, idx)) != npos);
    return strResult;
}

SString SString::TrimStart(const char* szOld) const
{
    const size_t uiOldLength = strlen(szOld);
    SString      strResult = *this;
    while (strResult.substr(0, uiOldLength) == szOld)
        strResult = strResult.substr(uiOldLength);
    return strResult;
}

SString SString::TrimEnd(const char* szOld) const
{
    const size_t uiOldLength = strlen(szOld);
    SString      strResult = *this;
    while (strResult.length() >= uiOldLength && strResult.substr(strResult.length() - uiOldLength) == szOld)
        strResult = strResult.substr(0, strResult.length() - uiOldLength);
    return strResult;
}

SString SString::ToLower() const
{
    SString strResult = *this;
    std::transform(strResult.begin(), strResult.end(), strResult.begin(), ::tolower);
    return strResult;
}

SString SString::ToUpper() const
{
    SString strResult = *this;
    std::transform(strResult.begin(), strResult.end(), strResult.begin(), ::toupper);
    return strResult;
}

SString SString::ConformLineEndings() const
{
    assert('\n' == '\x0A');
    if (std::count(begin(), end(), '\n'))
        return Replace("\x0D", "");
    else
        return Replace("\x0D", "\n");
}

bool SString::Contains(const SString& strOther) const
{
    return find(strOther) != std::string::npos;
}

bool SString::ContainsI(const SString& strOther) const
{
    return ToUpper().find(strOther.ToUpper()) != std::string::npos;
}

bool SString::CompareI(const SString& strOther) const
{
    return _stricmp(*this, strOther) == 0;
}

SString SString::SubStr(int iPos, int iCount) const
{
    if (iPos < 0)
    {
        iCount += iPos;
        iPos = 0;
    }
    iCount = std::max(0, iCount);
    if (iPos + iCount > (int)length())
    {
        iCount = length() - iPos;
    }
    if (iCount < 1)
        return "";
    return substr(iPos, iCount);
}

SString SString::Left(int iCount) const
{
    return SubStr(0, iCount);
}

SString SString::Right(int iCount) const
{
    return SubStr((int)length() - iCount, iCount);
}

bool SString::EndsWith(const SString& strOther) const
{
    return Right((int)strOther.length()) == strOther;
}

bool SString::EndsWithI(const SString& strOther) const
{
    return _stricmp(Right((int)strOther.length()), strOther) == 0;
}

bool SString::BeginsWith(const SString& strOther) const
{
    return Left((int)strOther.length()) == strOther;
}

bool SString::BeginsWithI(const SString& strOther) const
{
    return _stricmp(Right((int)strOther.length()), strOther) == 0;
}

SString SString::Join(const SString& strDelim, const std::vector<SString>& parts, int iFirst, int iCount)
{
    SString strResult;
    int     iLast = std::min<int>(iFirst + iCount, parts.size()) - 1;
    iFirst = std::max<int>(iFirst, 0);
    for (int i = iFirst; i <= iLast; i++)
    {
        if (i != iFirst)
            strResult += strDelim;
        strResult += parts[i];
    }
    return strResult;
}

void SString::AssignLeft(const char* szOther, unsigned int uiMaxLength)
{
    assign(SStringX(szOther).Left(uiMaxLength));
}
