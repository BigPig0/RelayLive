#pragma once

#include <windows.h>

class Profile
{
public:
    explicit Profile(const char *fileName)
    {
        strcpy_s(m_fileName, 32, fileName);
        strcpy_s(m_defValue, 32, "n/a");
    }

    void setDefaultValue(const char *defValue)
    {
        strcpy_s(m_defValue, 32, defValue);
    }

    bool readValue(const char *section, const char *key, char *value, int size) const;

private:
    char m_fileName[32], m_defValue[32];
};

inline
bool Profile::readValue(const char *section, const char *key, char *value, int size) const
{
    // If neither lpAppName nor lpKeyName is NULL and the supplied destination buffer is too small to hold the requested string,
    // the string is truncated and followed by a null character, and the return value is equal to nSize minus one.
    GetPrivateProfileString(section, key, m_defValue, value, size, m_fileName);
    return strcmp(value, m_defValue) != 0;
}
