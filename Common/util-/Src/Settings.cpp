#include "stdafx.h"
#include "Profile.h"
#include "Settings.h"
#include "Mutex.h"

namespace Settings
{

static HashMap<string, string> g_settings;

static SlimRWLock g_rwLock;

static string g_strProfile;

bool loadFromProfile(const string &strFileName)
{
    g_strProfile = strFileName;
    return true;
}

string getValue(const string &section,const string &key)
{
    string mapkey = section + "\\" + key;

    g_rwLock.sharedLock();
    auto it = g_settings.find(mapkey);
    auto itEnd = g_settings.end();
    g_rwLock.sharedUnlock();
    if (it != itEnd)
        return it->second;

    Profile profile(g_strProfile.c_str());
    char strVal[1024];
    if (profile.readValue(section.c_str(), key.c_str(), strVal, 1024))
    {
        MutexLock lock(&g_rwLock);
        g_settings.insert(make_pair(mapkey, strVal));
        return strVal;
    }
    Log::error("’“≤ªµΩ %s",mapkey.c_str());

    return "";
}

string getValue(const string &section,const string &key,const string &default)
{
    string res = getValue(section, key);
    if (res.empty())
    {
        return default;
    }
    return res;
}

int getValue(const string &section,const string &key,const int &default)
{
    string res = getValue(section, key);
    if (res.empty())
    {
        return default;
    }
    return stoi(res);
}
}
