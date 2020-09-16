#pragma once

namespace SipServer {

/**
 * ������յ���Ϣ��������Ӧ�𡣷���message����(��ѯĿ¼����̨����)
 */
class CSipMessage
{
public:

    /**
     * ����Message�¼�
     */
    void OnMessage(eXosip_event_t *osipEvent);

    /**
     * Ŀ¼��ѯ
     */
    void QueryDirtionary();

    /**
     * �豸״̬��ѯ
     * @param devID[in] �豸id
     */
    void QueryDeviceStatus(string devID);

    /**
     * �豸��Ϣ��ѯ����
     * @param devID[in] �豸id
     */
    void QueryDeviceInfo(string devID);

    /**
     * �ļ�Ŀ¼��������
     * @param devID[in] �豸id
     */
    void QueryRecordInfo(string devID, string strStartTime, string strEndTime);

    /**
     * �ƶ��豸λ�ò�ѯ
     * @param devID[in] �豸id
     */
    void QueryMobilePosition(string devID);

    /**
     * ��̨����
     * @param strDevCode[in] �豸����
     * @param nInOut[in]     ��ͷ�Ŵ���С 0:ֹͣ 1:��С 2:�Ŵ�
     * @param nUpDown[in]    ��ͷ�������� 0:ֹͣ 1:���� 2:����
     * @param nLeftRight[in] ��ͷ�������� 0:ֹͣ 1:���� 2:����
     * @param cMoveSpeed[in] ��ͷ�����ٶ�
     * @param cMoveSpeed[in] ��ͷ�ƶ��ٶ�
     */
    void DeviceControl(string strDevCode,
        int nInOut = 0, int nUpDown = 0, int nLeftRight = 0, 
        uint8_t cInOutSpeed = 0X1, uint8_t cMoveSpeed = 0XFF);

};

};