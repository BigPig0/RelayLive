#include "util.h"
#include "tmcp_interface_sdk.h"
#include "worker.h"

#pragma comment(lib,"tmcp_interface_sdk.lib")

using namespace Platform ;

namespace HikPlat {

	static void __stdcall StreamCallback(int handle,const char* data,int size,void *pUser){
		Server::CLiveWorker* lw = (Server::CLiveWorker*)pUser;
		lw->m_bCbStream = !lw->m_bCbStream;
		if(lw->m_bCbStream) {
			if(lw->m_bFirstStream) {
				lw->m_bFirstStream = false;
			} else {
				lw->push_ps_data((char*)data, size);
			}
		}
	}

	static int _loginHandle = 0;

	bool Init() {
		int ret = Plat_Init();
		if(ret < 0) {
			Log::error("SDK init failed");
			return false;
		}

        string strIP = Settings::getValue("HikSDK", "IP");
        string strUsr = Settings::getValue("HikSDK", "User");
        string strPwd = Settings::getValue("HikSDK", "Pwd");
        string strPort= Settings::getValue("HikSDK", "Port");
		_loginHandle = Plat_LoginCMS(strIP.c_str(), strUsr.c_str(), strPwd.c_str(), strPort.c_str());
		if(_loginHandle <= 0) {
			Log::error("login failed %d", Plat_GetLastError());
			return false;
		}

		return true;
	}

	void Cleanup() {
		if(_loginHandle > 0) {
			Plat_LogoutCMS(_loginHandle);
			_loginHandle = 0;
		}

		Plat_Free();
	}

	int Play(void* user, string devid) {
		const char* url = Plat_QueryRealStreamURL(devid.c_str(), _loginHandle, MAIN);
		if(url == NULL) {
			Log::error("get url failed: %d", Plat_GetLastError());
			return -1;
		}
		Log::debug(url);

		int ret = Plat_PlayVideo(url, NULL, _loginHandle, StreamCallback, user);
		if(ret < 0) {
			Log::debug("play video failed: %d", Plat_GetLastError());
		}

		return ret;
	}

    void Stop(int ret) {
        Plat_StopVideo(ret);
    }
}