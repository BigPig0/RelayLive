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
    char    *media;             //�����ǣ�"audio","video", "text", "application" and "message"
    uint32_t media_port;
    char    *media_protocol;    //�����ǣ�UDP��RTP/AVP��RTP/SAVP
    char    *media_format;
}sub_session_t;

typedef struct _sdp_ {
    // v,o,s,t,mΪ�����,������Ϊ��ѡ��
    //v=
    uint32_t version; //�汾
    //o=<�û���><�Ựid><�汾><��������><��ַ����><��ַ>
    char    *origin_username;
    uint32_t origin_sess_id;
    uint32_t origin_sess_version;
    char    *origin_nettype;    //IN ��ʾinternet
    uint32_t origin_addrtype;   //4(IP4) �� 6(IP6)
    char    *origin_address;    //Դ��ַ
    //s= ISO 10646�ַ���ʾ�ĻỰ��
    char    *session_name;
    //i= �Ự��Ϣ
    char    *session_information;
    //u= URI����
    char    *uri;
    //e= �ʼ���ַ
    char    *email;
    //p= �绰����
    char    *phone;
    //c= <nettype> <addrtype> <connection-address><*/ttl><*/number> ������Ϣ,���Ѿ�����������ý��������в���Ҫ
    char    *connect_nettype;    //IN ��ʾinternet
    uint32_t connect_addrtype;   //4(IP4) �� 6(IP6)
    char    *connect_address;    //Դ��ַ
    uint32_t connect_ttl;        //��IP4��ttl
    //b=<bwtype>:<bandwidth> (zero or more bandwidth information lines) 
    char    *bandwidth_type;     //CT��ʽ��������������Ĵ���AS�����õ����Ự�Ĵ���
    uint32_t bandwidth;          //���� kb/s��
    // One or more time descriptions ("t=" and "r=" lines, see below)
    // ����������У�ָ�����������ʱ��Σ�����ǹ����ʱ��Σ���r=���Կ���ʹ�á�start-time��stop- time�����NTP(Network Time Protocol),������Ϊ��λ���Դ�1900������ʱ�䡣Ҫת��ΪUNIXʱ�䣬��ȥ2208988800�����stop-time����Ϊ0,��Ựû��ʱ�����ơ����start-timeҲ����Ϊ0����Ự����Ϊ�����õġ�
    //t=<start-time> <stop-time>
    uint64_t time_start;
    uint64_t time_stop;
    //r=<repeat-interval> <active duration> <offsets from start-time>�ظ�������ʱ���ʾ����������±�ʾ��
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
    //�Ự��
    char    *attribute_cat;      //a=cat:<���>//������ֲ��ʽ�Ự�����,�����շ�ɸѡ�Ự
    char    *attribute_keywds;   //a=keywds:<�ؼ���>//�����շ�ɸѡ�Ự
    char    *attribute_tool;     //a=tool:<�������Ͱ汾��>//�����Ự�����Ĺ������Ͱ汾��
    char    *attribute_sendrecv; // a=recvonly/sendrecv/sendonly//�շ�ģʽ
    char    *attribute_type;     //a=type:<��������>//��:�㲥,�ۻ�,��ϯ����,����,H.323
    char    *attribute_charset;  //a=charset:<�ַ���>//��ʾ�Ự������Ϣ���ݵ��ַ���
    char    *attribute_sdplang;  //a=sdplang:<���Ա��>//������������
    char    *attribute_lang;     //a=lang:<���Ա��>//�Ự������ȱʡ���Ի�ý������������
    uint32_t attribute_framerate;//a=framerate:<֡����>//��λ:֡/��
    char    *attribute_quality;  //a=quality:<����>//��Ƶ�Ľ�������(10/5/0)
    char    *attribute_fmtp;     //a=fmtp:<��ʽ>< ��ʽ�ض�����>//����ָ����ʽ�ĸ��Ӳ���
    char    *attribute_ptime;   //a=ptime:<����ʱ��>//ý������ʱ��(��λ:��)
    char    *attribute_orient;   //a=orient:<�װ巽��>//ָ���װ�����Ī�ϵķ���
    rtpmap_t *attribute_rtpmap;  //a=rtpmap:<�������ͺ�><������>/<ʱ������>[/<�������>]
    //m=<media> <port> <proto> <fmt> ...
    //m=<media> <port>/<number of ports> <proto> <fmt> ...
    char    *media;             //�����ǣ�"audio","video", "text", "application" and "message"
    uint32_t media_port;
    char    *media_protocol;    //�����ǣ�UDP��RTP/AVP��RTP/SAVP
    char    *media_format;
} sdp_t;
extern sdp_t* create_sdp(char const *content = NULL);
extern void destory_sdp(sdp_t* sdp);


#endif