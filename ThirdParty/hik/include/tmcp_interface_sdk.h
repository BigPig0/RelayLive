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

// 兼容5000接口
namespace Platform
{
	/*******************************************************************************
	*	Plat_Init
	*	接口说明：初始化接口
	*	参数说明：无
	*	返回值：成功返回0，错误时返回-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_Init();

	/*******************************************************************************
	*   Plat_Free
	*	接口说明：资源释放接口，关闭所有的连接
	*	参数说明：无
	*	返回值：成功返回0，错误时返回-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_Free();

	/*******************************************************************************
	*	Plat_LoginCMS
	*	接口说明：登录到指定平台
	*	参数说明：
	*	cscmsURL		中心管理服务器地址
	*	csUserName		用户名
	*	csPSW			用户密码
	*	csPort			端口
	*	iLoginType		登录类型 预留参数，默认为0
	*	返回值：>0 成功，返回一个会话句柄，错误时返回-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_LoginCMS(const char * cscmsURL, 
        const char * csUserName, 
        const char * csPSW, 
        const char * csPort,
        int iLoginType = 0);

	/*******************************************************************************
	*	Plat_LogoutCMS
	*	接口说明：退出登录
	*	参数说明：
	*	iUserhandle	Plat_LoginCMS 返回的句柄
	*	返回值：成功时返回0 错误时返回-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_LogoutCMS(int iUserhandle);

    	/*******************************************************************************
	*	Plat_LoginCMS
	*	接口说明：登录到指定平台
	*	参数说明：
	*	cscmsURL		中心管理服务器地址
	*	csUserName		用户名
	*	csPSW			用户密码
	*	csPort			端口
	*	iLoginType		登录类型 预留参数，默认为0
	*	返回值：>0 成功，返回一个会话句柄，错误时返回-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetResource(int iUserHandle);

	/*******************************************************************************
	*	Plat_QueryResource
	*	接口说明：查询用户有权限的所有指定类型的设备信息
	*	参数说明：
	*	iResourceType 设备类型 
	*	iUserHandle	Plat_LoginCMS 返回的句柄
	*	返回值：  成功时返回0 错误时返回-1，查询成功后使用 Plat_MoveNext()， 
	*				Plat_GetValueStr()， Plat_GetValueInt()逐条获取设备信息
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_QueryResource(int iResourceType, int iUserHandle, int iExtraParam = 0);

	/*******************************************************************************
	*	Plat_MoveNext
	*	接口说明：向后移动数据访问游标
	*	参数说明：
	*	iUserHandle			Plat_LoginCMS 返回的句柄
	*	返回值： 0 数据未读取完， -1 数据读取完成，
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_MoveNext(int iUserHandle);

	/*******************************************************************************
	*	Plat_GetValueStr
	*	接口说明：获取查询信息的字符串属性
	*	参数说明：
	*	propertyName 属性名称 
	*	iUserHandle	 Plat_LoginCMS 返回的句柄
	*	返回值：字符串属性值，如果没有返回””
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(char*) Plat_GetValueStr(const char* propertyName, int iUserHandle,bool * bCascad=NULL);

	/*******************************************************************************
	*	Plat_GetValueInt
	*	接口说明：获取查询片段信息的int属性
	*	参数说明：
	*	propertyName 属性名称 
	*	iUserHandle	 Plat_LoginCMS 返回的句柄
	*	返回值：int类型属性，获取属性失败返回INT_MIN
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetValueInt(const char* propertyName, int iUserHandle);

	/*******************************************************************************
	*	Plat_QueryRecordFile
	*	接口说明：查询指定摄像头某一时间段内的录像文件信息
	*	参数说明：
	*	csCameraID			摄像头ID
	*	lStartTime			查询录像的开始时间
	*	lEndtime			查询录像的结束时间
	*	csQueryCondition	查询条件 0为IPSAN  1为设备  2为PCNVR
	*	iUserHandle			Plat_LoginCMS 返回的句柄
	*	返回值：成功时返回0 查询成功 错误时返回-1，查询成功后使用 MoveNext()， GetValueStr()， 
	*		GetValueInt()逐条获取录像信息(录像片段的 开始时间 结束时间 录像URL) 
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_QueryRecordFile(const char* csCameraID, 
        const long lStartTime, 
        const long lEndtime, 
        const char* csQueryCondition, 
        int iUserHandle);

	/*******************************************************************************
	*	Plat_GetPlayBackUrl
	*	接口说明：获取回放URL
	*	参数说明：
	*	iUserHandle			Plat_LoginCMS 返回的句柄
	*	返回值：录像的地址
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(const char*) Plat_GetPlayBackUrl(int iUserHandle);

	/*******************************************************************************
	*	Plat_QueryRealStreamURL
	*	接口说明：查询实时流视频的URL 
	*	参数说明：
    *	csCameraID			摄像头ID
    *	iUserHandle			Plat_LoginCMS 返回的句柄
    *   streamType           子码流/主码流
                                        NONE： 按照平台的设置
                                        MAIN： 主码流
                                        SUB   ： 子码流
    *   streamLink             TCP/UDP
	*	返回值：成功时返回URL字符串 错误时返回空字符串
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(const char *) Plat_QueryRealStreamURL(const char* csCameraID, int iUserHandle\
        , TMCP_STREAM_TYPE streamType = DEFAULT, TMCP_STREAM_LINK streamLink = TCP);

	/*******************************************************************************
	*	Plat_PlayVideo
	*	接口说明：开始浏览视频
	*	参数说明：
	*	URL					预览或回放的播放路径，由Plat_QueryRealStreamURL或Plat_QueryRecordFile获得
	*	hWnd				播放窗口句柄,如果为空，则不播放
	*	iUserHandle			Plat_LoginCMS 返回的句柄
	*	fStreamCallback		视频码流接收回调函数指针，如果回调函数为NULL则不给码流
	*		回调函数参数：
	*		handle	StartVideoStream返回的句柄
	*		data		接收视频码流数据缓冲区指针
	*		size 		接收视频码流数据字节数
	*		pUser 	用户数据
	*		iDataType 1-视频头 2-视频数据
	*	pUser				用户数据
	*	返回值：>=0成功，返回实时视频句柄，<0失败	
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_PlayVideo(const char* URL, 
        long hWnd,
        int iUserHandle, 
        void(__stdcall *fStreamCallback)(int handle,const char* data,int size,void *pUser) = NULL, 
        void *pUser = NULL);

	/*******************************************************************************
	*	Plat_StopVideo
	*	接口说明：停止视频浏览
	*	参数说明： hStream  Plat_PlayVideo返回的句柄
	*	返回值：成功时返回0 错误时返回-1
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_StopVideo(int  hStream);

	/*******************************************************************************
	*	Plat_ControlCamera
	*	接口说明：云镜控制
	*	参数说明：
	*	cameraID		摄像头ID
	*	dwPTZCommand 	控制命令 
            99:3d放大， param1为xTop,param2为yTop,param3为xBottom,param4位yBottom；
	*	param1  1：开始动作 0：停止动作，当dwPTZCommand为8,9,39时，param1为预置位序号，最多255,具体和球机有关,当dwPTZCommand为37,38时，param1为巡航轨迹编号
	*	param2 云台速度或巡航速度或巡航点停顿时间
	*	param3 预留
	*	param4 预留
	*	pCruiseInfo	巡航结构体
	*	iUserHandle	Plat_LoginCMS 返回的句柄
	*	返回值：成功时返回0 错误时返回-1
    
    3d放大：
        xTop = 鼠标当前所选区域的起始点坐标的值*255/352；
        xBottom = 鼠标当前所选区域的结束点坐标的值*255/352；
        yTop = 鼠标当前所选区域的起始点坐标的值*255/288；
        yBottom = 鼠标当前所选区域的结束点坐标的值*255/288；
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
	*	接口说明：开始录像下载
	*	参数说明：
	*	csURL				录像文件的URL
	*	csSaveFilePath		本地文件保存全路径
	*	lStartTime			下载录像的开始时间
	*	lEndtime			下载录像的结束时间
	*	lMAXFileSize		分包的大小
	*	iUserHandle	Plat_LoginCMS 返回的句柄
	*	返回值：>=0成功，返回文件下载句柄，<0失败
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_StartDownLoad(const char * csURL, 
        const char * csSaveFilePath, 
        const long lStartTime, 
        const long lEndtime, 
        unsigned __int64 lMAXFileSize,
        int iUserHandle);

	/*******************************************************************************
	*	Plat_StopDownLoad
	*	接口说明：停止录像下载
	*	参数说明：
	*	streamhandle  Plat_StartDownLoad返回的句柄
	*	返回值：成功时返回0 错误时返回-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_StopDownLoad (int streamhandle);

	/*******************************************************************************
	*	Plat_StartRecordFile
	*	接口说明：开始本地录像
	*	参数说明：
	*	handle  			Plat_PlayVideo返回的句柄
	*	Savefilepath 		本地文件保存全路径
	*	ullMAXFileSize		分包的大小
	*	返回值：>=0成功，返回录像句柄，<0失败
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int)  Plat_StartRecordFile(
        int handle,
        const char* savefilepath,
        unsigned __int64 ullMAXFileSize);

	/*******************************************************************************
	*	Plat_StopRecordFile
	*	接口说明：停止本地录像
	*	参数说明：
	*	handle  Plat_StartRecordFile返回的句柄
	*	返回值：成功时返回0 错误时返回-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_StopRecordFile(int handle);

	/*******************************************************************************
	*	Plat_PlayControl
	*	接口说明：回放控制接口
	*	参数说明：
	*	handle  		Plat_PlayVideo返回的句柄
	*	iCommand		播放控制命令
	*	iParam			控制参数
	*	返回值：成功时返回0 错误时返回-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_PlayControl( int hStream, int iCommand, int iParam);

	/*******************************************************************************
	*	Plat_GetFilePercent
	*	接口说明：获取录像下载百分比
	*	参数说明：
	*	handle  Plat_StartDownLoad返回的句柄
	*	pVal	文件下载百分比指针 范围为0-100
	*	返回值：成功时返回0 错误时返回-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetFilePercent(int hStream,long* pVal);

	/*******************************************************************************
	*	Plat_GetPlaybackPercent
	*	接口说明：获取录像回放百分比
	*	参数说明：
	*	handle  Plat_PlayVideo 回放返回的句柄
	*	pVal	文件下载百分比指针 范围为0-100
	*	返回值：成功时返回0 错误时返回-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetPlaybackPercent(int handle,long* pVal);

    /*******************************************************************************
	*	Plat_GetPlaybackTime
	*	接口说明：获取录像回放绝对时间
	*	参数说明：
	*	handle  Plat_PlayVideo 回放返回的句柄
	*	pAbsTime	录像回放时间，此缓冲区由用户开辟，长度不小于20
	*	返回值：成功时返回0 错误时返回-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetPlaybackTime(int handle, DWORD& dwAbsTime);

	/*******************************************************************************
	*	Plat_CapPic
	*	接口说明：视频抓图
	*	参数说明：
	*	handle  			Plat_PlayVideo返回的句柄
	*	Savefilepath 		本地文件保存全路径
	*	返回值：成功时返回0 错误时返回-1	
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_CapPic(int handle,const char* savefilepath);

	/*******************************************************************************
	*	Plat_SubscribeAlarm
	*	接口说明：订阅报警信息
	*	参数说明：
	*	fAlarmCallback 	告警接收回调函数指针
	*		回调函数参数：
	*		csResourceid	    设备资源ID
	*		alarmtype			告警类型（1：无视频2：断线 3：遮挡）
	*		csAlarmDetail		报警描述
	*		iAlarmLevel			报警等级
	*		alarmtime			告警时间
	*		action				1：告警产生 0：告警消除
	*		pUser       		用户数据
	*	pUser					用户数据
	*	iHandle					Plat_LoginCMS返回的句柄
	*	返回值：成功时返回0 错误时返回-1
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
	*	接口说明：取消订阅报警
	*	参数说明：
	*	iHandle		Plat_LoginCMS返回的句柄
	*	返回值：成功时返回0 错误时返回-1
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_UnSubscribeAlarm(int iUserHandle);

	/*******************************************************************************
	*	Plat_GetVersion
	*	接口说明：获取SDK版本
	*	参数说明：无
	*	返回值：  成功返回unsigned int版本号 
	*				前6位000001表示主版本号为1，其后5位00010表示子版本号为2，
	*				其后5位00000表示修正版本号为0。即当前版本号为1.2.0。
	*				从第17位开始表示SVN的版本号 0000001111100111
	********************************************************************************/
    TMCP_INTERFACE_SDK_API(unsigned int) Plat_GetVersion();

	/*******************************************************************************
	*	Plat_GetLastError
	*	接口说明：获取错误信息
	*	参数说明：无
	*	返回值：成功时返回0 错误时返回错误码
	*******************************************************************************/
    TMCP_INTERFACE_SDK_API(int) Plat_GetLastError();

	/*******************************************************************************
	*	Plat_GetResourceState
	*	接口说明：获取监控点状态
	*	参数说明：
	*	csResourceID 		设备资源 ID
	*	iResourceType		设备类型 0为监控点 1为设备
	*	currentState		当前状态（输出参数），包括服务器、设备、监控点等资源 1为在线 0为不在线
	*	iUserHandle			Plat_LoginCMS 返回的句柄
	*	返回值： 成功时返回0 错误时返回-1
	********************************************************************************/
	TMCP_INTERFACE_SDK_API(int) Plat_GetResourceState(
		const char * 	csResourceID,
		int				iResourceType,
		long * 			currentState,
		int				iUserHandle);

	/*******************************************************************************
	*	Plat_QueryPrivilege
	*	接口说明：查询用户对指定设备的权限
	*	参数说明：
	*	csResourceID		设备ID
	*	csPrivilegeCode		权限代码
	*	iUserHandle			Plat_LoginCMS 返回的句柄
	*	返回值：  有权限返回0  无权限返回-1
	*******************************************************************************/
	TMCP_INTERFACE_SDK_API(int) Plat_QueryPrivilege(
		const char* csResourceID,
		const char* csPrivilegeCode,
		int			iUserHandle);

	/*******************************************************************************
	*	Plat_StartTalk
	*	接口说明：开始对讲
	*	参数说明：
	*	DeviceID	设备ID
	*	iChannelID	通道ID
	*	iUserHandle	Plat_LoginCMS返回的句柄
	*	返回值：>=0成功，返回对讲句柄，错误时返回-1
	********************************************************************************/
	TMCP_INTERFACE_SDK_API(int) Plat_StartTalk(const char* DeviceID, int iChannelID, int iUserHandle);

	/*******************************************************************************
	*	Plat_StopTalk
	*	接口说明：停止对讲
	*	参数说明：
	*	iTalkHandle		Plat_StartTalk	返回的句柄
	*	返回值：0 成功 错误时返回-1
	********************************************************************************/
	TMCP_INTERFACE_SDK_API(int) Plat_StopTalk(int italkHandle);

	/*******************************************************************************
	*	Plat_GetDeviceReport
	*	接口说明：发送报表到平台
	*	参数说明：
	*	lStartTime		开始时间 
	*	lEndTime		结束时间
	*	reportType		报表类型 1—设备状态总体统计（总路数、故障次数、故障路数等）
	*							2—设备故障详细统计（包括：无视频、设备断线、遮挡等）
	*	iUserHandle		Plat_LoginCMS 返回的句柄
	*	返回值：字符串属性值，如果没有返回空字符串。 成功返回一个XML。
	********************************************************************************/
	TMCP_INTERFACE_SDK_API(const char*) Plat_GetDeviceReport(
		const long 	lStartTime, 
		const long 	lEndTime,
		int			reportype,
		int			iUserHandle);


}// namespace Platform

