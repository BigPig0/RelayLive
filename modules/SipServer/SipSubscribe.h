/**
 * ���ĶԷ�ƽ̨
 */

#pragma once

namespace SipServer {

class CSipSubscribe
{
public:

    // Ŀ¼״̬����
    void SubscribeDirectory(const int expires);

    // �¼�����
    void SubscribeAlarm(const int expires);

    // �ƶ��豸λ����Ϣ����
    void SubscribeMobilepostion(const int expires);

	// �ƶ��豸λ����Ϣ����[���Ķ���豸]
    void SubscribeMobilepostion(const int expires, vector<string> devs);

	// �ƶ��豸λ����Ϣ����[���ĵ����豸]
    void SubscribeMobilepostion(const int expires, string strDevCode);

};

};