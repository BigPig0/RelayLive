#ifndef AVFORMAT_SDP
#define AVFORMAT_SDP

typedef struct _rtpmap_ {
    uint32_t          number;
    char              name[10];
    uint32_t          clock;
    uint32_t          param;
    struct _rtpmap_  *next;
} rtpmap_t;

typedef struct _sub_session_ {
    //m=<media> <port> <proto> <fmt> ...
    //m=<media> <port>/<number of ports> <proto> <fmt> ...
    char    *media;             //可以是，"audio","video", "text", "application" and "message"
    uint32_t media_port;
    char    *media_protocol;    //可以是，UDP，RTP/AVP和RTP/SAVP
    char    *media_format;
}sub_session_t;

typedef struct _sdp_ {
    // v,o,s,t,m为必须的,其他项为可选。
    //v=
    uint32_t version; //版本
    //o=<用户名><会话id><版本><网络类型><地址类型><地址>
    char    *origin_username;
    uint32_t origin_sess_id;
    uint32_t origin_sess_version;
    char    *origin_nettype;    //IN 表示internet
    uint32_t origin_addrtype;   //4(IP4) 或 6(IP6)
    char    *origin_address;    //源地址
    //s= ISO 10646字符表示的会话名
    char    *session_name;
    //i= 会话信息
    char    *session_information;
    //u= URI描述
    char    *uri;
    //e= 邮件地址
    char    *email;
    //p= 电话号码
    char    *phone;
    //c= <nettype> <addrtype> <connection-address><*/ttl><*/number> 连接信息,如已经包含在所有媒体中则该行不需要
    char    *connect_nettype;    //IN 表示internet
    uint32_t connect_addrtype;   //4(IP4) 或 6(IP6)
    char    *connect_address;    //源地址
    uint32_t connect_ttl;        //仅IP4有ttl
    //b=<bwtype>:<bandwidth> (zero or more bandwidth information lines) 
    char    *bandwidth_type;     //CT方式是设置整个会议的带宽，AS是设置单个会话的带宽
    uint32_t bandwidth;          //带宽 kb/s。
    // One or more time descriptions ("t=" and "r=" lines, see below)
    // 这个可以有行，指定多个不规则时间段，如果是规则的时间段，则r=属性可以使用。start-time和stop- time都遵从NTP(Network Time Protocol),是以秒为单位，自从1900以来的时间。要转换为UNIX时间，减去2208988800。如果stop-time设置为0,则会话没有时间限制。如果start-time也设置为0，则会话被认为是永久的。
    //t=<start-time> <stop-time>
    uint64_t time_start;
    uint64_t time_stop;
    //r=<repeat-interval> <active duration> <offsets from start-time>重复次数在时间表示里面可以如下表示：
    //    d - days (86400 seconds)
    //    h - hours (3600 seconds)
    //    m - minutes (60 seconds)
    //    s - seconds (allowed for completeness)
    uint64_t repeat_interval;
    uint64_t repeat_active_duration;
    uint64_t repeat_offset;
    //z=<adjustment time> <offset> <adjustment time> <offset> ....
    char    *timezone_adjustments;
    //k=<method>
    //k=<method>:<encryption key>
    char    *encryption_method;
    char    *encryption_key;
    //a=<attribute>
    //a=<attribute>:<value>
    //会话级
    char    *attribute_cat;      //a=cat:<类别>//给出点分层次式会话分类号,供接收方筛选会话
    char    *attribute_keywds;   //a=keywds:<关键词>//供接收方筛选会话
    char    *attribute_tool;     //a=tool:<工具名和版本号>//创建会话描述的工具名和版本号
    char    *attribute_sendrecv; // a=recvonly/sendrecv/sendonly//收发模式
    char    *attribute_type;     //a=type:<会议类型>//有:广播,聚会,主席主持,测试,H.323
    char    *attribute_charset;  //a=charset:<字符集>//显示会话名和信息数据的字符集
    char    *attribute_sdplang;  //a=sdplang:<语言标记>//描述所有语言
    char    *attribute_lang;     //a=lang:<语言标记>//会话描述的缺省语言或媒体描述的语言
    uint32_t attribute_framerate;//a=framerate:<帧速率>//单位:帧/秒
    char    *attribute_quality;  //a=quality:<质量>//视频的建议质量(10/5/0)
    char    *attribute_fmtp;     //a=fmtp:<格式>< 格式特定参数>//定义指定格式的附加参数
    char    *attribute_ptime;   //a=ptime:<分组时间>//媒体分组的时长(单位:秒)
    char    *attribute_orient;   //a=orient:<白板方向>//指明白板在屏莫上的方向
    rtpmap_t *attribute_rtpmap;  //a=rtpmap:<净荷类型号><编码名>/<时钟速率>[/<编码参数>]
    //m=<media> <port> <proto> <fmt> ...
    //m=<media> <port>/<number of ports> <proto> <fmt> ...
    char    *media;             //可以是，"audio","video", "text", "application" and "message"
    uint32_t media_port;
    char    *media_protocol;    //可以是，UDP，RTP/AVP和RTP/SAVP
    char    *media_format;
} sdp_t;
extern sdp_t* create_sdp(char const *content = NULL);
extern void destory_sdp(sdp_t* sdp);


#endif