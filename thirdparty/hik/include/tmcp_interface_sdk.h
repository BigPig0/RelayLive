/********************************************************************
    created:    2013/03/19
    created:    19:3:2013   11:33
    filename:   tmcp_interface_sdk.h
    author:     xukaijun@hikvision.com.cn at its

    @note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
    purpose:
*********************************************************************/
#pragma once

#ifdef TMCP_INTERFACE_SDK_EXPORTS
#define TMCP_INTERFACE_SDK_API(type) extern "C" __declspec(dllexport) type __cdecl /*__stdcall*/
#else
#define TMCP_INTERFACE_SDK_API(type) extern "C" __declspec(dllimport) type __cdecl /*__stdcall*/
#endif

#include "tmcp_sdk_defines.h"

// ����5000�ӿ�
namespace Platform
{
	/*******************************************************************************
	*	Plat_Init
	*	�ӿ�˵������ʼ���ӿ�
	*	����˵������
	*	����ֵ���ɹ�����0������ʱ����-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_Init();

	/*******************************************************************************
	*   Plat_Free
	*	�ӿ�˵������Դ�ͷŽӿڣ��ر����е�����
	*	����˵������
	*	����ֵ���ɹ�����0������ʱ����-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_Free();

	/*******************************************************************************
	*	Plat_LoginCMS
	*	�ӿ�˵������¼��ָ��ƽ̨
	*	����˵����
	*	cscmsURL		���Ĺ����������ַ
	*	csUserName		�û���
	*	csPSW			�û�����
	*	csPort			�˿�
	*	iLoginType		��¼���� Ԥ��������Ĭ��Ϊ0
	*	����ֵ��>0 �ɹ�������һ���Ự���������ʱ����-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_LoginCMS(const char * cscmsURL, 
        const char * csUserName, 
        const char * csPSW, 
        const char * csPort,
        int iLoginType = 0);

	/*******************************************************************************
	*	Plat_LogoutCMS
	*	�ӿ�˵�����˳���¼
	*	����˵����
	*	iUserhandle	Plat_LoginCMS ���صľ��
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_LogoutCMS(int iUserhandle);

    	/*******************************************************************************
	*	Plat_LoginCMS
	*	�ӿ�˵������¼��ָ��ƽ̨
	*	����˵����
	*	cscmsURL		���Ĺ����������ַ
	*	csUserName		�û���
	*	csPSW			�û�����
	*	csPort			�˿�
	*	iLoginType		��¼���� Ԥ��������Ĭ��Ϊ0
	*	����ֵ��>0 �ɹ�������һ���Ự���������ʱ����-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetResource(int iUserHandle);

	/*******************************************************************************
	*	Plat_QueryResource
	*	�ӿ�˵������ѯ�û���Ȩ�޵�����ָ�����͵��豸��Ϣ
	*	����˵����
	*	iResourceType �豸���� 
	*	iUserHandle	Plat_LoginCMS ���صľ��
	*	����ֵ��  �ɹ�ʱ����0 ����ʱ����-1����ѯ�ɹ���ʹ�� Plat_MoveNext()�� 
	*				Plat_GetValueStr()�� Plat_GetValueInt()������ȡ�豸��Ϣ
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_QueryResource(int iResourceType, int iUserHandle, int iExtraParam = 0);

	/*******************************************************************************
	*	Plat_MoveNext
	*	�ӿ�˵��������ƶ����ݷ����α�
	*	����˵����
	*	iUserHandle			Plat_LoginCMS ���صľ��
	*	����ֵ�� 0 ����δ��ȡ�꣬ -1 ���ݶ�ȡ��ɣ�
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_MoveNext(int iUserHandle);

	/*******************************************************************************
	*	Plat_GetValueStr
	*	�ӿ�˵������ȡ��ѯ��Ϣ���ַ�������
	*	����˵����
	*	propertyName �������� 
	*	iUserHandle	 Plat_LoginCMS ���صľ��
	*	����ֵ���ַ�������ֵ�����û�з��ء���
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(char*) Plat_GetValueStr(const char* propertyName, int iUserHandle,bool * bCascad=NULL);

	/*******************************************************************************
	*	Plat_GetValueInt
	*	�ӿ�˵������ȡ��ѯƬ����Ϣ��int����
	*	����˵����
	*	propertyName �������� 
	*	iUserHandle	 Plat_LoginCMS ���صľ��
	*	����ֵ��int�������ԣ���ȡ����ʧ�ܷ���INT_MIN
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetValueInt(const char* propertyName, int iUserHandle);

	/*******************************************************************************
	*	Plat_QueryRecordFile
	*	�ӿ�˵������ѯָ������ͷĳһʱ����ڵ�¼���ļ���Ϣ
	*	����˵����
	*	csCameraID			����ͷID
	*	lStartTime			��ѯ¼��Ŀ�ʼʱ��
	*	lEndtime			��ѯ¼��Ľ���ʱ��
	*	csQueryCondition	��ѯ���� 0ΪIPSAN  1Ϊ�豸  2ΪPCNVR
	*	iUserHandle			Plat_LoginCMS ���صľ��
	*	����ֵ���ɹ�ʱ����0 ��ѯ�ɹ� ����ʱ����-1����ѯ�ɹ���ʹ�� MoveNext()�� GetValueStr()�� 
	*		GetValueInt()������ȡ¼����Ϣ(¼��Ƭ�ε� ��ʼʱ�� ����ʱ�� ¼��URL) 
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_QueryRecordFile(const char* csCameraID, 
        const long lStartTime, 
        const long lEndtime, 
        const char* csQueryCondition, 
        int iUserHandle);

	/*******************************************************************************
	*	Plat_GetPlayBackUrl
	*	�ӿ�˵������ȡ�ط�URL
	*	����˵����
	*	iUserHandle			Plat_LoginCMS ���صľ��
	*	����ֵ��¼��ĵ�ַ
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(const char*) Plat_GetPlayBackUrl(int iUserHandle);

	/*******************************************************************************
	*	Plat_QueryRealStreamURL
	*	�ӿ�˵������ѯʵʱ����Ƶ��URL 
	*	����˵����
    *	csCameraID			����ͷID
    *	iUserHandle			Plat_LoginCMS ���صľ��
    *   streamType           ������/������
                                        NONE�� ����ƽ̨������
                                        MAIN�� ������
                                        SUB   �� ������
    *   streamLink             TCP/UDP
	*	����ֵ���ɹ�ʱ����URL�ַ��� ����ʱ���ؿ��ַ���
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(const char *) Plat_QueryRealStreamURL(const char* csCameraID, int iUserHandle\
        , TMCP_STREAM_TYPE streamType = DEFAULT, TMCP_STREAM_LINK streamLink = TCP);

	/*******************************************************************************
	*	Plat_PlayVideo
	*	�ӿ�˵������ʼ�����Ƶ
	*	����˵����
	*	URL					Ԥ����طŵĲ���·������Plat_QueryRealStreamURL��Plat_QueryRecordFile���
	*	hWnd				���Ŵ��ھ��,���Ϊ�գ��򲻲���
	*	iUserHandle			Plat_LoginCMS ���صľ��
	*	fStreamCallback		��Ƶ�������ջص�����ָ�룬����ص�����ΪNULL�򲻸�����
	*		�ص�����������
	*		handle	StartVideoStream���صľ��
	*		data		������Ƶ�������ݻ�����ָ��
	*		size 		������Ƶ���������ֽ���
	*		pUser 	�û�����
	*		iDataType 1-��Ƶͷ 2-��Ƶ����
	*	pUser				�û�����
	*	����ֵ��>=0�ɹ�������ʵʱ��Ƶ�����<0ʧ��	
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_PlayVideo(const char* URL, 
        long hWnd,
        int iUserHandle, 
        void(__stdcall *fStreamCallback)(int handle,const char* data,int size,void *pUser) = NULL, 
        void *pUser = NULL);

	/*******************************************************************************
	*	Plat_StopVideo
	*	�ӿ�˵����ֹͣ��Ƶ���
	*	����˵���� hStream  Plat_PlayVideo���صľ��
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_StopVideo(int  hStream);

	/*******************************************************************************
	*	Plat_ControlCamera
	*	�ӿ�˵�����ƾ�����
	*	����˵����
	*	cameraID		����ͷID
	*	dwPTZCommand 	�������� 
            99:3d�Ŵ� param1ΪxTop,param2ΪyTop,param3ΪxBottom,param4λyBottom��
	*	param1  1����ʼ���� 0��ֹͣ��������dwPTZCommandΪ8,9,39ʱ��param1ΪԤ��λ��ţ����255,���������й�,��dwPTZCommandΪ37,38ʱ��param1ΪѲ���켣���
	*	param2 ��̨�ٶȻ�Ѳ���ٶȻ�Ѳ����ͣ��ʱ��
	*	param3 Ԥ��
	*	param4 Ԥ��
	*	pCruiseInfo	Ѳ���ṹ��
	*	iUserHandle	Plat_LoginCMS ���صľ��
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
    
    3d�Ŵ�
        xTop = ��굱ǰ��ѡ�������ʼ�������ֵ*255/352��
        xBottom = ��굱ǰ��ѡ����Ľ����������ֵ*255/352��
        yTop = ��굱ǰ��ѡ�������ʼ�������ֵ*255/288��
        yBottom = ��굱ǰ��ѡ����Ľ����������ֵ*255/288��
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_ControlCamera(const char* cameraID, 
        int dwPTZCommand, 
        int param1, 
        int param2, 
        int param3, 
        int param4, 
        LPVOID *pCruiseInfo, 
        int iUserHandle);

	/*******************************************************************************
	*	Plat_StartDownLoad
	*	�ӿ�˵������ʼ¼������
	*	����˵����
	*	csURL				¼���ļ���URL
	*	csSaveFilePath		�����ļ�����ȫ·��
	*	lStartTime			����¼��Ŀ�ʼʱ��
	*	lEndtime			����¼��Ľ���ʱ��
	*	lMAXFileSize		�ְ��Ĵ�С
	*	iUserHandle	Plat_LoginCMS ���صľ��
	*	����ֵ��>=0�ɹ��������ļ����ؾ����<0ʧ��
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_StartDownLoad(const char * csURL, 
        const char * csSaveFilePath, 
        const long lStartTime, 
        const long lEndtime, 
        unsigned __int64 lMAXFileSize,
        int iUserHandle);

	/*******************************************************************************
	*	Plat_StopDownLoad
	*	�ӿ�˵����ֹͣ¼������
	*	����˵����
	*	streamhandle  Plat_StartDownLoad���صľ��
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_StopDownLoad (int streamhandle);

	/*******************************************************************************
	*	Plat_StartRecordFile
	*	�ӿ�˵������ʼ����¼��
	*	����˵����
	*	handle  			Plat_PlayVideo���صľ��
	*	Savefilepath 		�����ļ�����ȫ·��
	*	ullMAXFileSize		�ְ��Ĵ�С
	*	����ֵ��>=0�ɹ�������¼������<0ʧ��
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int)  Plat_StartRecordFile(
        int handle,
        const char* savefilepath,
        unsigned __int64 ullMAXFileSize);

	/*******************************************************************************
	*	Plat_StopRecordFile
	*	�ӿ�˵����ֹͣ����¼��
	*	����˵����
	*	handle  Plat_StartRecordFile���صľ��
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_StopRecordFile(int handle);

	/*******************************************************************************
	*	Plat_PlayControl
	*	�ӿ�˵�����طſ��ƽӿ�
	*	����˵����
	*	handle  		Plat_PlayVideo���صľ��
	*	iCommand		���ſ�������
	*	iParam			���Ʋ���
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_PlayControl( int hStream, int iCommand, int iParam);

	/*******************************************************************************
	*	Plat_GetFilePercent
	*	�ӿ�˵������ȡ¼�����ذٷֱ�
	*	����˵����
	*	handle  Plat_StartDownLoad���صľ��
	*	pVal	�ļ����ذٷֱ�ָ�� ��ΧΪ0-100
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetFilePercent(int hStream,long* pVal);

	/*******************************************************************************
	*	Plat_GetPlaybackPercent
	*	�ӿ�˵������ȡ¼��طŰٷֱ�
	*	����˵����
	*	handle  Plat_PlayVideo �طŷ��صľ��
	*	pVal	�ļ����ذٷֱ�ָ�� ��ΧΪ0-100
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetPlaybackPercent(int handle,long* pVal);

    /*******************************************************************************
	*	Plat_GetPlaybackTime
	*	�ӿ�˵������ȡ¼��طž���ʱ��
	*	����˵����
	*	handle  Plat_PlayVideo �طŷ��صľ��
	*	pAbsTime	¼��ط�ʱ�䣬�˻��������û����٣����Ȳ�С��20
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetPlaybackTime(int handle, DWORD& dwAbsTime);

	/*******************************************************************************
	*	Plat_CapPic
	*	�ӿ�˵������Ƶץͼ
	*	����˵����
	*	handle  			Plat_PlayVideo���صľ��
	*	Savefilepath 		�����ļ�����ȫ·��
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1	
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_CapPic(int handle,const char* savefilepath);

	/*******************************************************************************
	*	Plat_SubscribeAlarm
	*	�ӿ�˵�������ı�����Ϣ
	*	����˵����
	*	fAlarmCallback 	�澯���ջص�����ָ��
	*		�ص�����������
	*		csResourceid	    �豸��ԴID
	*		alarmtype			�澯���ͣ�1������Ƶ2������ 3���ڵ���
	*		csAlarmDetail		��������
	*		iAlarmLevel			�����ȼ�
	*		alarmtime			�澯ʱ��
	*		action				1���澯���� 0���澯����
	*		pUser       		�û�����
	*	pUser					�û�����
	*	iHandle					Plat_LoginCMS���صľ��
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_SubscribeAlarm(void(__stdcall *fAlarmCallback)(
        const char* csResourceid,
        int alarmtype,
        const char* csAlarmDetail,
        int iAlarmLevel,
        const char* charalarmtime,
        int action,
        void *pUser),
        void * pUser,
        int iUserHandle);

	/*******************************************************************************
	*	Plat_UnSubscribeAlarm
	*	�ӿ�˵����ȡ�����ı���
	*	����˵����
	*	iHandle		Plat_LoginCMS���صľ��
	*	����ֵ���ɹ�ʱ����0 ����ʱ����-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_UnSubscribeAlarm(int iUserHandle);

	/*******************************************************************************
	*	Plat_GetVersion
	*	�ӿ�˵������ȡSDK�汾
	*	����˵������
	*	����ֵ��  �ɹ�����unsigned int�汾�� 
	*				ǰ6λ000001��ʾ���汾��Ϊ1�����5λ00010��ʾ�Ӱ汾��Ϊ2��
	*				���5λ00000��ʾ�����汾��Ϊ0������ǰ�汾��Ϊ1.2.0��
	*				�ӵ�17λ��ʼ��ʾSVN�İ汾�� 0000001111100111
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(unsigned int) Plat_GetVersion();

	/*******************************************************************************
	*	Plat_GetLastError
	*	�ӿ�˵������ȡ������Ϣ
	*	����˵������
	*	����ֵ���ɹ�ʱ����0 ����ʱ���ش�����
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetLastError();

	/*******************************************************************************
	*	Plat_GetResourceState
	*	�ӿ�˵������ȡ��ص�״̬
	*	����˵����
	*	csResourceID 		�豸��Դ ID
	*	iResourceType		�豸���� 0Ϊ��ص� 1Ϊ�豸
	*	currentState		��ǰ״̬��������������������������豸����ص����Դ 1Ϊ���� 0Ϊ������
	*	iUserHandle			Plat_LoginCMS ���صľ��
	*	����ֵ�� �ɹ�ʱ����0 ����ʱ����-1
	********************************************************************************/
	TMCP_INTERFACE_SDK_API(int) Plat_GetResourceState(
		const char * 	csResourceID,
		int				iResourceType,
		long * 			currentState,
		int				iUserHandle);

	/*******************************************************************************
	*	Plat_QueryPrivilege
	*	�ӿ�˵������ѯ�û���ָ���豸��Ȩ��
	*	����˵����
	*	csResourceID		�豸ID
	*	csPrivilegeCode		Ȩ�޴���
	*	iUserHandle			Plat_LoginCMS ���صľ��
	*	����ֵ��  ��Ȩ�޷���0  ��Ȩ�޷���-1
	*******************************************************************************/
	TMCP_INTERFACE_SDK_API(int) Plat_QueryPrivilege(
		const char* csResourceID,
		const char* csPrivilegeCode,
		int			iUserHandle);

	/*******************************************************************************
	*	Plat_StartTalk
	*	�ӿ�˵������ʼ�Խ�
	*	����˵����
	*	DeviceID	�豸ID
	*	iChannelID	ͨ��ID
	*	iUserHandle	Plat_LoginCMS���صľ��
	*	����ֵ��>=0�ɹ������ضԽ����������ʱ����-1
	********************************************************************************/
	TMCP_INTERFACE_SDK_API(int) Plat_StartTalk(const char* DeviceID, int iChannelID, int iUserHandle);

	/*******************************************************************************
	*	Plat_StopTalk
	*	�ӿ�˵����ֹͣ�Խ�
	*	����˵����
	*	iTalkHandle		Plat_StartTalk	���صľ��
	*	����ֵ��0 �ɹ� ����ʱ����-1
	********************************************************************************/
	TMCP_INTERFACE_SDK_API(int) Plat_StopTalk(int italkHandle);

	/*******************************************************************************
	*	Plat_GetDeviceReport
	*	�ӿ�˵�������ͱ���ƽ̨
	*	����˵����
	*	lStartTime		��ʼʱ�� 
	*	lEndTime		����ʱ��
	*	reportType		�������� 1���豸״̬����ͳ�ƣ���·�������ϴ���������·���ȣ�
	*							2���豸������ϸͳ�ƣ�����������Ƶ���豸���ߡ��ڵ��ȣ�
	*	iUserHandle		Plat_LoginCMS ���صľ��
	*	����ֵ���ַ�������ֵ�����û�з��ؿ��ַ����� �ɹ�����һ��XML��
	********************************************************************************/
	TMCP_INTERFACE_SDK_API(const char*) Plat_GetDeviceReport(
		const long 	lStartTime, 
		const long 	lEndTime,
		int			reportype,
		int			iUserHandle);


}// namespace Platform

