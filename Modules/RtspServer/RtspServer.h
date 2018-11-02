#pragma once
#include "uv.h"

typedef enum _RTSP_METHOD_
{
    RTSP_ERROR = 0,
    RTSP_OPTIONS,
    RTSP_DESCRIBE,
    RTSP_SETUP,
    RTSP_PLAY,
    RTSP_PAUSE,
    RTSP_TEARDOWN
}rtsp_method;

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

typedef struct _rtsp_ruquest_
{
    rtsp_method     method;
    string          uri;
    bool            parse_status;
    response_code   code;
    uint64_t        CSeq;
    uint32_t        rtp_port;
    uint32_t        rtcp_port;
    map<string,string> headers;
}rtsp_ruquest;

typedef struct _rtsp_response_
{
    response_code   code;
    uint64_t        CSeq;
    string          body;
    unordered_map<string,string> headers;
}rtsp_response;

class CRtspServer;
class CClient
{
public:
    CClient();
    ~CClient();

    int Init(uv_loop_t* uv);
    int Recv();
    rtsp_ruquest parse(char* buff, int len);
    int answer(rtsp_ruquest req);
    rtsp_response make_option_answer(rtsp_ruquest req);
    rtsp_response make_describe_answer(rtsp_ruquest req);
    rtsp_response make_setup_answer(rtsp_ruquest req);
    rtsp_response make_play_answer(rtsp_ruquest req);
    rtsp_response make_pause_answer(rtsp_ruquest req);
    rtsp_response make_teardown_answer(rtsp_ruquest req);

    uv_tcp_t        m_tcp;
    uv_loop_t*      m_ploop;
    CRtspServer*    m_server;
	string          m_strDevCode;
	string          m_strRtpIP;
	int             m_strRtpPort;
};

class CRtspServer
{
public:
    CRtspServer(void);
    ~CRtspServer(void);

    int Init(uv_loop_t* uv);

    uv_tcp_t        m_tcp;
    uv_loop_t*      m_ploop;

    static volatile uint64_t m_nSession;
};

