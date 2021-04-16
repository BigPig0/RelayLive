#pragma once
#include "util.h"
#include "utilc.h"
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

    void push_ps_data(char* pBuff, int nLen);
    int get_ps_data(char* pBuff, int &nLen);

    void push_flv_frame(char* pBuff, int nLen);
    int get_flv_frame(char **buff);   /** ����˻�ȡ��Ƶ���� */
	void next_flv_frame();

	void close();
public:
    int32_t               m_nLoginID;       //< �����豸��½���
    int32_t               m_nPlayID;        //< ���Ž��
    ILiveSession         *m_pSession;       // ���ӻỰ
    RequestParam         *m_pParam;         // �û�����Ĳ���
    bool                  m_bWebSocket;     // false:http����true:websocket
    std::string           m_strClientIP;    // ���Ŷ˵�ip

private:
    ring_buff_t          *m_pPSRing;        //< PS���ݶ���
    ring_buff_t          *m_pFlvRing;       //< Ŀ���������ݶ���
	bool                  m_bConnect;       //< �ͻ�������״̬
};

/** ֱ�� */
CLiveWorker* CreatLiveWorker(RequestParam param, bool isWs, ILiveSession *pSession, string clientIP);

void InitFFmpeg();
