// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "util.h"
#include "utilc.h"
#include "easylog.h"
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
    //CMiniDump dump("sipServer.dmp");

    /** ������־�ļ� */
    char path[MAX_PATH];
#ifdef WINDOWS_IMPL
    sprintf(path, "./log/sipServer/log.txt");
#else
    sprintf(path, "/var/log/relaylive/sipServer/log.txt");
#endif
    Log::open(Log::Print::both, uvLogPlus::Level::Debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** ���������ļ� */
#ifdef WINDOWS_IMPL
    const char* conf = "./config.txt";
#else
    const char* conf = "/etc/relaylive/config.txt";
#endif
    if (!Settings::loadFromProfile(conf))
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
	// for(int i=0; i<10000; i++) {
	// 	SipServer::DevInfo *dev = new SipServer::DevInfo();
	// 	dev->strDevID = "12345678912345678" + to_string(i);
	// 	dev->strName = "TEST    " + to_string(i);
	// 	dev->strManuf = "strManuf";
	// 	dev->strModel = "ModelModelModelModelModel";
	// 	dev->strStatus = "ON";
	// 	dev->strLongitude = "120.123456";
	// 	dev->strLatitude = "90.225366";
	// 	g_mapDevs.insert(make_pair(dev->strDevID, dev));
	// }
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