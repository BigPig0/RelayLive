// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "util.h"
#include "utilc.h"
#include "ipc.h"
#include "SipServer.h"
#include "script.h"
#include <stdio.h>
#include <map>
#include <sstream>

using namespace util;

std::map<string, SipServer::DevInfo*> g_mapDevs;
CriticalSection                       g_csDevs;
bool                                  _useScript = false;   //�Ƿ�����lua�ű�

// ÿ��Сʱ����һ�εĴ����¼�
void on_hour_event(time_t t) {
    struct tm * timeinfo = localtime(&t);
    if(_useScript)
        Script::HourEvent(timeinfo->tm_hour);
}

// ��ѯĿ¼�õ��豸��ϢӦ��
void on_device(SipServer::DevInfo* dev) {
	MutexLock lock(&g_csDevs);
	if(g_mapDevs.count(dev->strDevID) == 0) {
	    g_mapDevs.insert(make_pair(dev->strDevID, dev));
		if(_useScript)
			Script::InsertDev(dev);
	} else {
		if(_useScript && !dev->strLatitude.empty() && dev->strLatitude != "0" && dev->strLatitude != g_mapDevs[dev->strDevID]->strLatitude
			&& !dev->strLongitude.empty() && dev->strLongitude != "0" && dev->strLongitude != g_mapDevs[dev->strDevID]->strLongitude)
			Script::UpdatePos(dev->strDevID, dev->strLatitude, dev->strLongitude);
		if(_useScript && !dev->strStatus.empty() && dev->strStatus != g_mapDevs[dev->strDevID]->strStatus)
			Script::UpdateStatus(dev->strDevID, dev->strStatus);
	    delete g_mapDevs[dev->strDevID];
	    g_mapDevs[dev->strDevID] = dev;
	}
}

// �����豸����״̬
void on_update_status(string strDevID, string strStatus) {
    if(_useScript)
	    Script::UpdateStatus(strDevID, strStatus);

	MutexLock lock(&g_csDevs);
	auto fit = g_mapDevs.find(strDevID);
	if(fit != g_mapDevs.end()) {
		fit->second->strStatus = strStatus;
	}
}

// �����豸gps
void on_update_postion(string strDevID, string log, string lat) {
    if(_useScript)
	    Script::UpdatePos(strDevID, lat, log);

	MutexLock lock(&g_csDevs);
	auto fit = g_mapDevs.find(strDevID);
	if(fit != g_mapDevs.end()) {
		fit->second->strLongitude = log;
		fit->second->strLatitude = lat;
	}
}

int main()
{
    /** Dump���� */
    CMiniDump dump("sipServer.dmp");

    /** ������־�ļ� */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\sipServer.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** ���������ļ� */
    if (!Settings::loadFromProfile(".\\config.txt"))
        Log::error("Settings::loadFromProfile failed");
    else
        Log::debug("Settings::loadFromProfile ok");

    /** ���̼�ͨ�� */
    IPC::Init();

	/** ���ݿ�ű� */
    string use = Settings::getValue("Script", "use", "false");
    if(use == "yes" || use == "1")
        _useScript = true;
    if(_useScript)
	    Script::Init();

    /** ��ʼ��SIP������ */
    if (!SipServer::Init())
    {
        Log::error("SipInstance init failed");
        return -1;
    }
    Log::debug("SipInstance::Init ok");
    
    sleep(INFINITE);
    return 0;
}

std::string GetDevsJson() {
	MutexLock lock(&g_csDevs);
	stringstream ss;
	ss << "{\"root\":[";
	bool first = true;
	for(auto c:g_mapDevs){  
		if(!first) {
			ss << ",";
		} else {
			first = false;
		}
		ss << FormatDevInfo(c.second, true);
	}
	ss << "]}";
	return ss.str();
}