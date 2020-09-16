/**
* ���ļ���libLiveģ��Ψһ���⵼����ͷ�ļ�
*/
#pragma once

#include "avtypes.h"

#ifdef LIVECLIENT_EXPORTS
#define LIVECLIENT_API __declspec(dllexport)
#else
#define LIVECLIENT_API
#endif

enum HandleType
{
    unknown_handle,
    flv_handle,
    fmp4_handle,
    h264_handle,
    ts_handle,
    rtp_handle,
};



namespace LiveClient
{
    struct ClientInfo {
        string devCode; //�豸id
        string connect; //����Э�� http/websocket/rtsp/hls
        string media;   //ý������ h264/flv/mp4/ts
        string clientIP; //�ͻ���ip
        int    channel; //ͨ����
    };
    /**
    * ���ݻص�����ӿ�
    * ���ϲ�ʵ��һ���̳иýӿڵ��࣬live workerͨ���ýӿڷ�����ʵ���ķ���
    */
    struct ILiveHandle
    {
		virtual ~ILiveHandle(){};

        /**
         * �����������ص�
         * @param ret ������ �ɹ���0
         * @param error_info �ɹ�Ϊ�¼�ƽ̨���ص�sdp��ʧ����ʧ��ԭ��
         */
        virtual void play_answer(int ret, string error_info) = 0;

        /**
         * ��Ƶ���ݷ��ͻص�
         */
        virtual void push_video_stream(AV_BUFF buff) = 0;

       /**
        * rtp���ն˽�����Ŀǰ��ֻ֪�н��ճ�ʱ����
        */
        virtual void stop() = 0;

        /**
         * ��ȡ�ͻ�����Ϣ
         */
        virtual ClientInfo get_clients_info() = 0;
    };

    struct ILiveHandleRtp : public ILiveHandle {
        virtual void push_rtcp_stream(char*, int) = 0;
    };

    /**
     * ���ϲ㵼��live worker�Ľӿ�
     */
    struct ILiveWorker
    {
        /**
        * ��liveworker�����һ���ص������
        * liveworker��Ӧ���livehandle
        * @param pWorker Liveworker�������rtp���ݲ������ɸ��ָ�ʽ
        * @param h ��Ҫ��Ƶ��������ִ�и��ִ�������Ϊ�����͵��ͻ���
        */
        virtual bool AddHandle(ILiveHandle* h, HandleType t, int c) = 0;

        /**
        * ��liveworker���Ƴ�һ��livehandle��
        * ��liveworker�е�handleȫ���Ƴ�ʱ��liveworker������ɱ
        */
        virtual bool RemoveHandle(ILiveHandle* h) = 0;

        //virtual string GetSDP() = 0;
    };

    /**
    * liveclient��ʼ��
    */
    void LIVECLIENT_API Init(void* uv);

    /**
    * ��ȡ�ͻ������ݡ������첽�������˴�ֻ��������
    */
    void LIVECLIENT_API GetClientsInfo();

    /**
    * ��ȡ�豸��Ϣ�������첽�������˴�ֻ��������
    * @param h ��������ľ��
    */
    void LIVECLIENT_API GetDevList();

    /**
     * ֪ͨsipģ�鶩���豸Ŀ¼
     */
    void LIVECLIENT_API QueryDirtionary();

	/**
	 * ֪ͨsipģ�鷢���豸������Ϣ
	 */
	 void LIVECLIENT_API DeviceControl(string strDev,
        int nInOut = 0, int nUpDown = 0, int nLeftRight = 0);

    /**
     * ���ûص�����ȡ��Ϣ��ɺ�,֪ͨ������
     * ����1����������
     * ����2����������������ֵ
     */
    typedef void(*LIVECLIENT_CB)(string, string);
    void LIVECLIENT_API SetCallBack(LIVECLIENT_CB cb);

    /**
    * ��ȡһ���豸��ֱ����������Ѵ���ֱ��ʹ�ã����������򴴽�һ���µ�
    * @param strCode �豸�Ĺ������
    */
    ILiveWorker* LIVECLIENT_API GetWorker(string strCode);
}