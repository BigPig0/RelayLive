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

    void push_ps_data(char* pBuff, int nLen);
    int get_ps_data(char* pBuff, int &nLen);

    void push_flv_frame(char* pBuff, int nLen);
    int get_flv_frame(char **buff);   /** ����˻�ȡ��Ƶ���� */
	void next_flv_frame();

	void close();
private:
	bool is_key(char* pBuff, int nLen);

public:
    ILiveSession         *m_pSession;       // ���ӻỰ
    RequestParam         *m_pParam;         // �û�����Ĳ���
    bool                  m_bWebSocket;     // false:http����true:websocket
    std::string           m_strClientIP;    // ���Ŷ˵�ip

private:
    ring_buff_t          *m_pPSRing;       // PS���ݶ���
    ring_buff_t          *m_pFlvRing;      // Ŀ���������ݶ���
	bool                  m_bConnect;      // �ͻ�������״̬
	bool                  m_bParseKey;     // �Ƿ�ȡ�õ�һ���ؼ�֡
	util::CNetStreamMaker m_PsStream;      // ��PS������ȡ��һ�����ݣ��ȴ浽�ˣ�Ȼ��һ��һ�ε�д��ffmpeg��
	int                   m_nStreamReaded; // PS��������д�˶��ٽ���ffmpeg��ȫ��д���ٴ�PS���л�ȡ��һ��
};

/** ֱ�� */
CLiveWorker* CreatLiveWorker(RequestParam param, bool isWs, ILiveSession *pSession, string clientIP);

void InitFFmpeg();
