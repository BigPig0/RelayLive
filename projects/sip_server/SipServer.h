/**
 * Sip������
 */
#pragma once
#include "utilc_api.h"
#include <string>
using namespace std;


namespace SipServer {

    /** �豸Ŀ¼��ѯ��ϢӦ���е��豸��Ϣ */
    struct DevInfo {
        string strDevID;       //< �������
        string strName;        //< �豸����
        string strManuf;       //< ����
        string strModel;       //< �ͺ�
        string strOwner;       //< ����
        string strCivilCode;   //< �����������
        string strBlock;       //< ����
        string strAddress;     //< ��װ��ַ
        string strParental;    //< �Ƿ������豸 1:�� 0:��
        string strParentID;    //< ���豸ID
        string strSafetyWay;   //< ���ȫģʽ 0(Ĭ��):������ 2:S/MIMEǩ����ʽ 3:S/MIME����ǩ��ͬʱ���÷�ʽ 4:����ժҪ��ʽ
        string strRegisterWay; //< ע�᷽ʽ 1(Ĭ��):����RETF RFC3261��׼����֤ע��ģʽ 2:���ڿ����˫����֤ע��ģʽ 3:��������֤���˫����֤ע��ģʽ
        string strCertNum;     //< ֤�����к�
        string strCertifiable; //< ֤����Ч��ʶ 0(Ĭ��):��Ч 1:��Ч
        string strErrCode;     //< ֤����Чԭ����
        string strEndTime;     //< ֤����ֹ��Ч��
        string strSecrecy;     //< �������� 0(Ĭ��):������ 1:����
        string strIPAddress;   //< IP��ַ
        string strPort;        //< �˿�
        string strPassword;    //< ����
        string strStatus;      //< ״̬
        string strLongitude;   //< ����
        string strLatitude;    //< γ��

        // ������չ��Ϣ
        string strPTZType;             //< ��������� 1:��� 2:���� 3:�̶�ǹ�� 4:ң��ǹ��
        string strPositionType;        //< λ������ 1:ʡ�ʼ��վ 2:�������� 3:��վ��ͷ 4:���Ĺ㳡 5:�������� 6:��ҵ���� 7:�ڽ̳��� 8:ѧУ�ܱ� 9:�ΰ��������� 10:��ͨ����
        string strRoomType;            //< 1(Ĭ��):���� 2:����
        string strUseType;             //< ��;����  1:�ΰ� 2:��ͨ  3:�ص�
        string strSupplyLightType;     //< �������� 1(Ĭ��):�޲��� 2:���ⲹ�� 3:�׹ⲹ��
        string strDirectionType;       //< �������� 1:�� 2:�� 3:�� 4:�� 5:���� 6:���� 7:���� 8:����
        string strResolution;          //< �ֱ���
        string strBusinessGroupID;     //< ������֯������ҵ�����ID
        string strDownloadSpeed;       //< ���ر��ٷ�Χ
        string strSVCSpaceSupportType; //< ����������� 0:��֧�� 1:һ����ǿ 2:������ǿ 3:������ǿ
        string strSVCTimeSupportType;  //< ʱ��������� 0:��֧�� 1:һ����ǿ 2:������ǿ 3:������ǿ
    };

    string FormatDevInfo(DevInfo *dev, bool ansi2utf8 = false);

    void TransDevInfo(string json, DevInfo *dev, bool utf82ansi = false);

    /** ��ʼ�� */
    bool Init();

    /** ������ */
    void Cleanup();

    /**
     * ����һ������׼��
     * @param[in] strProName ��������
     * @param[in] nID ����ID
     * @param[in] strDev �豸����
     * @return true:�ɹ� false:ʧ��
     */
    bool PlayInit(string strProName, uint32_t nID, string strDev);

    /**
     * ����һ��ʵʱ����
     * @param[in] strProName ��������
     * @param[in] nID ����ID
     * @param[in] nPort ���Ŷ˿�
     * @return true:�ɹ� false:ʧ��
     */
    bool RealPlay(string strProName, uint32_t nID, uint32_t nPort);

    /**
     * �ر�һ��ʵʱ����
     * @param[in] rtpPort �ö˿���Ϊid
     * @return true:�ɹ� false:ʧ��
     */
    bool StopPlay(uint32_t rtpPort);

    /**
     * �ر����в���
     */
    bool StopPlayAll(string strProName);

    /**
     * ����һ����ʷ��Ƶ����
     * @param[in] strDev �豸����
     * @param startTime ��ʼʱ��
     * @param endTime ����ʱ��
     * @param session �Ựsession
     * @return true:�ɹ� false:ʧ��
     */
    bool RecordPlay(string strProName, uint32_t nPort, string startTime, string endTime);

    /**
     * ��̨����
     * @param strDev[in] �豸����
     * @param nInOut[in]     ��ͷ�Ŵ���С 0:ֹͣ 1:��С 2:�Ŵ�
     * @param nUpDown[in]    ��ͷ�������� 0:ֹͣ 1:���� 2:����
     * @param nLeftRight[in] ��ͷ�������� 0:ֹͣ 1:���� 2:����
     */
    bool DeviceControl(string strDev, int nInOut = 0, int nUpDown = 0, int nLeftRight = 0);

    /**
     * ��ѯ�豸Ŀ¼
     */
	bool QueryDirtionary();

};

