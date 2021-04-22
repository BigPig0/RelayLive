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
    bool ps_empty();

    void push_flv_frame(char* pBuff, int nLen);
    int get_flv_frame(char **buff);   /** ����˻�ȡ��Ƶ���� */
	void next_flv_frame();

	void close();
private:
	bool is_key(char* pBuff, int nLen);

public:
    void                 *m_pPlay;          // �ⲿ����ʵ��
    ILiveSession         *m_pSession;       // ���ӻỰ
    RequestParam         *m_pParam;         // �û�����Ĳ���
    bool                  m_bWebSocket;     // false:http����true:websocket
    std::string           m_strClientIP;    // ���Ŷ˵�ip
	bool                  m_bConnect;       // �ͻ�������״̬

private:
    ring_buff_t          *m_pPSRing;       // PS���ݶ���
    ring_buff_t          *m_pFlvRing;      // Ŀ���������ݶ���
	bool                  m_bParseKey;     // �Ƿ�ȡ�õ�һ���ؼ�֡
    //������������������������������ݴ���ffmpeg���뻺������
	util::CNetStreamMaker m_PsStream;      // ��PS������ȡ��һ�����ݣ��ȴ浽�ˣ�Ȼ��һ��һ�ε�д��ffmpeg��
	int                   m_nStreamReaded; // PS��������д�˶��ٽ���ffmpeg��ȫ��д���ٴ�PS���л�ȡ��һ��
};

typedef bool(*funOutPlay)(CLiveWorker* worker);

/** ֱ�� */
CLiveWorker* CreatLiveWorker(RequestParam param, bool isWs, ILiveSession *pSession, string clientIP);

namespace Worker {
    /**
     * ��ʼ����Ƶת��
     * @play �ⲿ���ŷ���
     * @stop �ⲿֹͣ����
     * @findFirstKey �ⲿ�����ps���ݣ��Ƿ���Ҫ������һ���ؼ�֡
     */
    void Init(funOutPlay play = NULL, funOutPlay stop = NULL, bool findFirstKey = false);
}
