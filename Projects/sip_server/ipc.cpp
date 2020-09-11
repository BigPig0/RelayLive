#include "util.h"
#include "ipc.h"
#include "uvIpc.h"
#include "SipServer.h"
#include <map>
#include <list>
#include <sstream>

using namespace util;

extern std::string GetDevsJson();

namespace IPC {
    uv_ipc_handle_t* h = NULL;

    static void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len)
    {
        if (!strcmp(msg,"live_init")) {
            //������������ devcode=123&id=0
            data[len] = 0;
            char        szDevCode[30] = {0}; // �豸����
            uint32_t    nID = 0;

            sscanf(data, "devcode=%[^&]&id=%d",szDevCode, &nID);

            bool bplay = SipServer::PlayInit(name, nID, szDevCode);
            if(!bplay) {
                stringstream ss;
                ss << "id=" <<nID << "&port=0&ret=-1&error=sip play failed";
                string str = ss.str();
                uv_ipc_send(h, name, "live_init_answer", (char*)str.c_str(), str.size());
            }
        } else if (!strcmp(msg,"live_play")) {
            // ���󲥷� devcode=123&id=0
            data[len] = 0;
            uint32_t    nPort = 0;
            uint32_t    nID = 0;

            sscanf(data, "port=%d&id=%d", &nPort, &nID);

            bool bplay = SipServer::RealPlay(name, nID, nPort);
            if(!bplay){
                stringstream ss;
                ss << "id=" <<nID << "&port=0&ret=-1&error=sip play failed";
                string str = ss.str();
                uv_ipc_send(h, name, "live_play_answer", (char*)str.c_str(), str.size());
            }
        } else if(!strcmp(msg,"stop_play")) {
            //�ر�ָ����������
            string port(data, len);
            SipServer::StopPlay(stoi(port));
        } else if(!strcmp(msg,"close")) {
            //�ر��������ڽ��еĲ���
            SipServer::StopPlayAll(name);
        } else if(!strcmp(msg,"dev_fresh")) {
            //��ѯĿ¼�豸
            SipServer::QueryDirtionary();
        } else if(!strcmp(msg, "dev_ctrl")) {
            //�����̨����
            data[len] = 0;
            char szDevCode[30]={0};
            int nInOut=0, nUpDown=0, nLeftRight=0;
            sscanf(data, "dev=%[^&]&io=%d&ud=%d&lr=%d", szDevCode, &nInOut, &nUpDown, &nLeftRight);
            SipServer::DeviceControl(szDevCode, nInOut, nUpDown, nLeftRight);
        } else if(!strcmp(msg, "dev_get")) {
			data[len] = 0;
			uint32_t    nID = 0;
			sscanf(data, "id=%d", &nID);
			stringstream ss;
			ss << "id=" <<nID << "&json=" << GetDevsJson();
			string str = ss.str();
			uv_ipc_send(h, name, "dev_get_answer", (char*)str.c_str(), str.size());
		}
    }

    void on_play_init_cb(string strProName, uint32_t nID, uint32_t nPort) {
        stringstream ss;
        ss << "id=" << nID << "&port=" << nPort;
        string str = ss.str();
        uv_ipc_send(h, (char*)strProName.c_str(), "live_init_answer", (char*)str.c_str(), str.size());
    }

    void on_play_cb(string strProName, bool bRet, uint32_t nID, uint32_t nPort, string strInfo) {
        if(bRet) {
            std::stringstream ss;
            ss << "id=" <<nID << "&port=" << nPort << "&ret=0&error=" << strInfo;
            string str = ss.str();
            uv_ipc_send(h, (char*)strProName.c_str(), "live_play_answer", (char*)str.c_str(), str.size());
        } else {
            stringstream ss;
            ss << "id=" <<nID << "&port=0&ret=-1&error=sip play failed";
            string str = ss.str();
            uv_ipc_send(h, (char*)strProName.c_str(), "live_play_answer", (char*)str.c_str(), str.size());
        }
    }
    
    bool Init() {
        /** ���̼�ͨ�� */
        string ipc_name = Settings::getValue("IPC","name","ipcsvr");
        int ret = uv_ipc_client(&h, (char*)ipc_name.c_str(), NULL, "sipsvr", on_ipc_recv, NULL);
        if(ret < 0) {
            Log::error("ipc server err: %s", uv_ipc_strerr(ret));
            return false;
        }

        return true;
    }

    void Cleanup() {
        uv_ipc_close(h);
    }

}