#ifndef AVFORMAT_RTSP
#define AVFORMAT_RTSP

#include "cstl.h"

typedef enum _response_code_
{
    Code_100_Continue = 0,
    Code_110_ConnectTimeout,
    Code_200_OK,
    Code_201_Created,
    Code_250_LowOnStorageSpace,
    Code_300_MultipleChoices,
    Code_301_MovedPermanently,
    Code_302_MovedTemporarily,
    Code_303_SeeOther,
    Code_304_NotModified,
    Code_305_UseProxy,
    Code_350_GoingAway,
    Code_351_LoadBalancing,
    Code_400_BadRequest,
    Code_401_Unauthorized,
    Code_402_PaymentRequired,
    Code_403_Forbidden,
    Code_404_NotFound,
    Code_405_MethodNotAllowed,
    Code_406_NotAcceptable,
    Code_407_ProxyAuthenticationRequired,
    Code_408_RequestTimeOut,
    Code_410_Gone,
    Code_411_LengthRequired,
    Code_412_PreconditionFailed,
    Code_413_RequestEntityTooLarge,
    Code_414_RequestUriTooLarge,
    Code_415_UnsupportedMediaType,
    Code_451_ParameterNotUnderstood,
    Code_452_Reserved,
    Code_453_NotEnoughBandwidth,
    Code_454_SessionNotFound,
    Code_455_MethodNotValidInThisState,
    Code_456_HeaderFieldNotValidForResource,
    Code_457_InvalidRange,
    Code_458_ParameterIsReadOnly,
    Code_459_AggregateOperationNotAllowed,
    Code_460_OnlyAggregateOperationAllowed,
    Code_461_UnsupportedTransport,
    Code_462_DestinationUnreachable,
    Code_500_InternalServerError,
    Code_501_NotImplemented,
    Code_502_BadGateway,
    Code_503_ServiceUnavailable,
    Code_504_GatewayTimeOut,
    Code_505_RtspVersionNotSupported,
    Code_551_OptionNotSupported
}response_code;

extern char* response_status[];

typedef enum _rtsp_method_ {
    RTSP_ERROR = 0,
    RTSP_OPTIONS,
    RTSP_DESCRIBE,
    RTSP_SETUP,
    RTSP_PLAY,
    RTSP_PAUSE,
    RTSP_TEARDOWN,
}rtsp_method;

typedef struct _rtsp_ruquest_
{
    rtsp_method     method;
    char            *uri;
    uint32_t        CSeq;
    uint32_t        rtp_port;
    uint32_t        rtcp_port;
    hash_map_t      *headers; //map<string,string>
}rtsp_ruquest_t;

typedef struct _rtsp_ rtsp;
typedef void(*rtsp_callback)(void *user, rtsp_ruquest_t *req);

/**
 * 创建一个rtsp环境句柄
 */
extern rtsp* create_rtsp(void *user, rtsp_callback cb);

/**
 * 清理并销毁一个rtsp环境句柄
 */
extern void destory_rtsp(rtsp *h);

/**
 * 输入服务端socket接收的请求数据
 */
extern void rtsp_handle_request(rtsp *h, char *data, int len);

/**
 * 输入客户端socket接收到的应答数据
 */
extern void rtsp_handle_answer(rtsp *h, char *data, int len);

/**
 * 获取rtsp请求头内容
 */
extern const char* rtsp_request_get_header(rtsp_ruquest_t *req, char *data);

#endif