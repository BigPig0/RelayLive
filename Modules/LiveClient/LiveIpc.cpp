#include "common.h"
#include "uvIpc.h"
#include "LiveIpc.h"
#include "LiveClient.h"
#include "LiveWorker.h"

namespace LiveClient
{
    extern uv_loop_t     *g_uv_loop;
    extern LIVECLIENT_CB liveclient_respond;

namespace LiveIpc
{
    static uv_ipc_handle_t* h = NULL;

    static PlayAnswerList   *_pPlayAnswerList = NULL;
    static CriticalSection   _csPlayAnswerList;
    static uv_async_t        _uvAsyncPlay;    //< 异步回调播放结果

    static void async_cb(uv_async_t* handle){
        CLiveReceiver* obj = (CLiveReceiver*)handle->data;
        MutexLock lock(&_csPlayAnswerList);
        PlayAnswerList *pa = _pPlayAnswerList;
        while(pa){
            PlayAnswerList *tmp = pa;
            pa = pa->pNext;
            AsyncPlayCB(tmp);
            delete tmp;
        }
        _pPlayAnswerList = NULL;
    }

    static void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len) {
        if (!strcmp(msg,"live_play_answer")) {
            //播放请求回调 devcode=123rtpport=80&ret=0&error=XXXX
            data[len] = 0;
            char szDevCode[30] = {0}; // 设备编码
            //char szInfo[256]={0};     
            PlayAnswerList *pNewPA = new PlayAnswerList;

			int rtpport = 0;
            sscanf(data, "devcode=%[^&]&rtpport=%d&ret=%d&error=",szDevCode, &rtpport, &pNewPA->nRet);
			char *pInfo = strstr(data, "&error="); //// 成功时sdp信息，失败时错误描述
			pInfo += 7;
            pNewPA->strDevCode = szDevCode;
            pNewPA->strMark = pInfo;

            MutexLock lock(&_csPlayAnswerList);
            pNewPA->pNext = _pPlayAnswerList;
            _pPlayAnswerList = pNewPA;
            uv_async_send(&_uvAsyncPlay);
        } else if (!strcmp(msg,"dev_list_answer")) {
            //获取设备列表回调
            string devjson(data, len);
            if(liveclient_respond)
                liveclient_respond("devlist", devjson);
        }
    }

    void Init(){
        int ret = uv_ipc_client(&h, "relay_live", NULL, "liveDest", on_ipc_recv, NULL);
        if(ret < 0) {
            printf("ipc server err: %s\n", uv_ipc_strerr(ret));
        }
        uv_async_init(g_uv_loop, &_uvAsyncPlay, async_cb);
    }

    int RealPlay(string dev_code, string rtp_ip, int rtp_port){
        // devcode=123&rtpip=1.1.1.1&rtpport=50000
        stringstream ss;
        ss << "devcode=" << dev_code << "&rtpip=" << rtp_ip << "&rtpport=" << rtp_port;
        int ret = uv_ipc_send(h, "liveSrc", "live_play", (char*)ss.str().c_str(), ss.str().size());
        if(ret) {
            Log::error("ipc send real play error");
            return ret;
        }
        return 0;
    }

    int StopPlay(int rtp_port){
        string strPort = StringHandle::toStr<int>(rtp_port);
        int ret = uv_ipc_send(h, "liveSrc", "stop_play", (char*)strPort.c_str(), strPort.size());
        if(ret) {
            Log::error("ipc send stop error");
            return ret;
        }
        return 0;
    }

    int GetDevList()
    {
        int ret = uv_ipc_send(h, "liveSrc", "devices_list", NULL, 0);
        if(ret) {
            Log::error("ipc send devices_list error");
            return ret;
        }
        return 0;
    }

    int QueryDirtionary()
    {
        int ret = uv_ipc_send(h, "liveSrc", "QueryDirtionary", NULL, 0);
        if(ret) {
            Log::error("ipc send devices_list error");
            return ret;
        }
        return 0;
    }

	int DeviceControl(string strDev, int nInOut, int nUpDown, int nLeftRight) {
		stringstream ss;
        ss << "dev=" << strDev << "&io=" << nInOut << "&ud=" << nUpDown << "&lr=" << nLeftRight;
		int ret = uv_ipc_send(h, "liveSrc", "DeviceControl", (char*)ss.str().c_str(), ss.str().size());
        if(ret) {
            Log::error("ipc send devices_list error");
            return ret;
        }
        return 0;
	}
}
}