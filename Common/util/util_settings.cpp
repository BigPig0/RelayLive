#include "util_settings.h"
#include "util_mutex.h"
#include <unordered_map>
#include <string.h>
using namespace std;

namespace util {
namespace Settings
{
#ifdef WINDOWS_IMPL
#include <windows.h>
#else
#define SECTION_MAX_LEN 256
#define STRVALUE_MAX_LEN 256
#define LINE_CONTENT_MAX_LEN 256

static int GetPrivateProfileStringA( char* lpAppName, char* lpKeyName, char* lpDefault, char* lpReturnedString, int nSize, char* lpFileName )
{
    if (lpAppName == NULL || lpKeyName == NULL || lpReturnedString == NULL || lpFileName == NULL)
        return -1;

    char sect[SECTION_MAX_LEN] = {0};
    sprintf(sect, "[%s]", lpAppName);

    FILE* fp;
    int i = 0;
    int lineContentLen = 0;
    int position = 0;
    char lineContent[LINE_CONTENT_MAX_LEN];
    bool bFoundSection = false;
    bool bFoundKey = false;
    fp = fopen(lpFileName, "r");
    if(fp == NULL) {
        printf("%s: Opent file %s failed.\n", __FILE__, lpFileName);
        return -1;
    }
    while(feof(fp) == 0) {
        memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
        fgets(lineContent, LINE_CONTENT_MAX_LEN, fp);
        if((lineContent[0] == ';') || (lineContent[0] == '\0') || (lineContent[0] == '\r') || (lineContent[0] == '\n')) {
            continue;
        }

        //check section
        if(strncmp(lineContent, sect, strlen(sect)) == 0) {
            bFoundSection = true;
            //printf("Found section = %s\n", lineContent);
            while(feof(fp) == 0) {
                memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
                fgets(lineContent, LINE_CONTENT_MAX_LEN, fp);
                //check key
                if(strncmp(lineContent, lpKeyName, strlen(lpKeyName)) == 0) {
                    bFoundKey = true;
                    lineContentLen = strlen(lineContent);
                    //find value
                    for(i = strlen(lpKeyName); i < lineContentLen; i++) {
                        if(lineContent[i] == '=') {
                            position = i + 1;
                            break;
                        }
                    }
                    if(i >= lineContentLen) break;
                    strncpy(lpReturnedString, lineContent + position, strlen(lineContent + position));
                    lineContentLen = strlen(lpReturnedString);
                    for(i = 0; i < lineContentLen; i++) {
                        if((lineContent[i] == '\0') || (lineContent[i] == '\r') || (lineContent[i] == '\n')) {
                            lpReturnedString[i] = '\0';
                            break;
                        }
                    }  
                } else if(lineContent[0] == '[') {
                    break;
                }
            }
            break;
        }
    }
    fclose(fp);
    if(!bFoundSection){
        printf("No section = %s\n", sect);
        strncpy(lpReturnedString, lpDefault, nSize);
        return -1;
    } else if(!bFoundKey){
        printf("No key = %s\n", lpKeyName);
        strncpy(lpReturnedString, lpDefault, nSize);
        return -1;
    }

    return 0;
}

static int GetPrivateProfileIntA( char* lpAppName, char* lpKeyName, int nDefault, char* lpFileName )
{
    char strValue[STRVALUE_MAX_LEN];
    memset(strValue, '\0', STRVALUE_MAX_LEN);
    if(GetPrivateProfileStringA(lpAppName, lpKeyName, (char*)to_string(nDefault).c_str(), strValue, STRVALUE_MAX_LEN, lpFileName) != 0)
    {
        printf("%s: error", __func__);
        return nDefault;
    }
    return(atoi(strValue));
}
#endif

static unordered_map<string, string> g_settings;

static SlimRWLock g_rwLock;

static string g_strProfile;

bool loadFromProfile(const string &strFileName)
{
    g_strProfile = strFileName;
    return true;
}

string getValue(const string &section,const string &key)
{
    return getValue(section, key, "");
}

string getValue(const string &section,const string &key,const string &default_value)
{
    string mapkey = section + "\\" + key;

    g_rwLock.sharedLock();
    auto it = g_settings.find(mapkey);
    auto itEnd = g_settings.end();
    g_rwLock.sharedUnlock();
    if (it != itEnd)
        return it->second;

    char strVal[1024] = {0};
    GetPrivateProfileStringA((char*)section.c_str(), (char*)key.c_str(), (char*)default_value.c_str(), strVal, 1024, (char*)g_strProfile.c_str());
    MutexLock lock(&g_rwLock);
    g_settings.insert(make_pair(mapkey, strVal));
    return strVal;
}

int getValue(const string &section,const string &key,const int &default_value)
{
    string mapkey = section + "\\" + key;

    g_rwLock.sharedLock();
    auto it = g_settings.find(mapkey);
    auto itEnd = g_settings.end();
    g_rwLock.sharedUnlock();
    if (it != itEnd)
        return stoi(it->second);

    int value = GetPrivateProfileIntA((char*)section.c_str(), (char*)key.c_str(), default_value, (char*)g_strProfile.c_str());
    MutexLock lock(&g_rwLock);
    g_settings.insert(make_pair(mapkey, to_string(value)));
    return value;
}
}
}