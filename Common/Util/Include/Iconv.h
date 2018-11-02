#pragma once

namespace Iconv
{

inline
bool convertEncoding(UINT fromCode, UINT toCode, const char *from, const char **to)
{
    int convertedBytes;
    wchar_t *wideChars = NULL;

    do
    {
        convertedBytes = MultiByteToWideChar(fromCode, 0, from, -1, NULL, 0);
        if (!convertedBytes)
            break;

        wideChars = (wchar_t *)malloc(sizeof(wchar_t) * convertedBytes);
        if (!wideChars)
            break;
        convertedBytes = MultiByteToWideChar(fromCode, 0, from, -1, wideChars, convertedBytes);
        if (!convertedBytes)
            break;

        convertedBytes = WideCharToMultiByte(toCode, 0, wideChars, -1, NULL, 0, NULL, NULL);
        if (!convertedBytes)
            break;

        char *mbsChars = (char *)malloc(convertedBytes);
        if(!mbsChars)
            break;
        convertedBytes = WideCharToMultiByte(toCode, 0, wideChars, -1, mbsChars, convertedBytes, NULL, NULL);
        if (!convertedBytes)
        {
            free(mbsChars);
            break;
        }

        *to = mbsChars;
    }
    while (0);

    if (wideChars)
        free(wideChars);
    return convertedBytes != 0;
}

class Utf8
{
public:
    Utf8(const char *gbk)
        : m_utf8(NULL)
    {
        convertEncoding(CP_ACP, CP_UTF8, gbk, &m_utf8);
    }

    operator const char *()
    {
        return m_utf8;
    }

private:
    const char *m_utf8;
};

class Gbk
{
public:
    Gbk(const char *utf8)
        : m_gbk(NULL)
    {
        convertEncoding(CP_UTF8, CP_ACP, utf8, &m_gbk);
    }

    operator const char *()
    {
        return m_gbk;
    }

private:
    const char *m_gbk;
};

inline
const wchar_t *ctow(const char *mbs)
{
    int convertedBytes = 0;

    convertedBytes = MultiByteToWideChar(CP_ACP, 0, mbs, -1, NULL, 0);
    if (!convertedBytes)
        return NULL;

    wchar_t *wideChars = (wchar_t *)malloc(sizeof(wchar_t) * convertedBytes);
    if(!wideChars)
        return NULL;
    convertedBytes = MultiByteToWideChar(CP_ACP, 0, mbs, -1, wideChars, convertedBytes);
    if (!convertedBytes)
    {
        free(wideChars);
        return NULL;
    }
    return wideChars;
}

inline
const char *wtoc(const wchar_t *wstr, int size)
{
    int convertedBytes = 0;

    convertedBytes = WideCharToMultiByte(CP_ACP, 0, wstr, size, NULL, 0, NULL, NULL);
    if (!convertedBytes)
        return NULL;

    char *mbs = (char *)malloc(convertedBytes + 1);
    if(!mbs)
        return NULL;
    convertedBytes = WideCharToMultiByte(CP_ACP, 0, wstr, size, mbs, convertedBytes, NULL, NULL);
    if (!convertedBytes)
    {
        free(mbs);
        return NULL;
    }
    mbs[convertedBytes] = '\0';
    return mbs;
}

}
