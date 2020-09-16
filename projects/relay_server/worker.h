#pragma once
#include "ring_buff.h"
#include "util.h"
#include "util_netstream.h"
#include <string>
#include <list>

class ILiveSession;
struct RequestParam;

class CLiveWorker
{
public:
    CLiveWorker();
    ~CLiveWorker();

    bool Play();

    void push_flv_frame(char* pBuff, int nLen);
    int get_flv_frame(char **buff);   /** ����˻�ȡ��Ƶ���� */
	void next_flv_frame();

	void close();


public:
    ILiveSession         *m_pSession;       // ���ӻỰ
    RequestParam         *m_pParam;         // �û�����Ĳ���
    bool                  m_bWebSocket;     // false:http����true:websocket
    std::string           m_strClientIP;    // ���Ŷ˵�ip

private:
    ring_buff_t          *m_pFlvRing;       //< Ŀ���������ݶ���
	bool                  m_bConnect;       //< �ͻ�������״̬
};

/** ֱ�� */
CLiveWorker* CreatLiveWorker(RequestParam param, bool isWs, ILiveSession *pSession, string clientIP);

void InitFFmpeg();
