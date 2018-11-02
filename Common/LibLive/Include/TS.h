#pragma once
#include "LiveInstance.h"

#define TS_PACKET_HEADER               4
#define TS_PACKET_SIZE                 188
#define TS_SYNC_BYTE                   0x47
#define TS_PAT_PID                     0x00
#define TS_PMT_PID                     0xFFF
#define TS_H264_PID                    0x100
#define TS_AAC_PID                     0x101
#define TS_H264_STREAM_ID              0xE0
#define TS_AAC_STREAM_ID               0xC0
#define PMT_STREAM_TYPE_VIDEO          0x1B
#define PMT_STREAM_TYPE_AUDIO          0x0F
#define TS_ONE_FILE_SIZE               1024 * 1024
#define TS_TARGETDURATION              3600 * 25    //1s

#pragma pack(1)
/**
 * TS头结构
 */
typedef struct ts_header{
    unsigned char sync_byte;                          // '0x47',表示后面的是一个TS分组

    unsigned char PID1 : 5;                           // Packet ID号码，唯一的号码对应不同的包
    unsigned char transport_priority : 1;             // 传输优先级标志（0:优先级低；1:优先级高）
    unsigned char payload_unit_start_indicator : 1;   // 有效荷载单元起始指示符（一个视频PES拆分成多个包时，第一个需要设置‘1’，其他为‘0’；PAT,PMT及其他单个包也是‘1’）
    unsigned char transport_error_indicator : 1;      // 传输误码指示符

    unsigned char PID2;                               // Packet ID号码，唯一的号码对应不同的包

    unsigned char continuity_counter : 4;             // 包递增计数器
    unsigned char adaptation_field_control : 2;       // 附加区域控制(00:；10:仅有调整字段；01:仅有有效负载；11:含有调整字段和有效负载)
    unsigned char transport_scrambling_control : 2;   // 加密标志（00：未加密；其他表示已加密）
}ts_header_t; //4B

/**
 * PAT中的节目信息
 */
typedef struct ts_pat_program  
{  
    uint16_t program_number;                   //节目号

    uchar network_id_or_program_map_PID_1 : 5; // 节目号为0x0000时,表示这是NIT，PID=0x001f，即31
    uchar reserved:3;                          // 保留位 '111'

    uchar network_id_or_program_map_PID_2;     // 节目号为0x0001时,表示这是PMT，PID=0x100，即256
}ts_pat_program_t; //4B

/**
 * PAT结构
 */
typedef struct ts_pat{
    uchar table_id;                      //PAT的table_id只能是0x00

    uchar section_length_1 : 4;          // 表示这个字节后面有用的字节数，包括CRC32
    uchar reserved_1 : 2;                // 保留位 '11'
    uchar zero : 1;                      // '0' 
    uchar section_syntax_indicator : 1;  // 段语法标志位，固定为1

    uchar section_length_2;              // 表示这个字节后面有用的字节数，包括CRC32

    uint16_t transport_stream_id;        // 该传输流的ID，区别于一个网络中其它多路复用的流

    uchar current_next_indicator: 1;     // 发送的PAT是当前有效('1')还是下一个PAT有效('0')
    uchar version_number : 5;            // 范围0-31，表示PAT的版本号;第一个是0，一旦PAT有变化，版本号加1
    uchar reserved_2 : 2;                // 保留位 '11'

    uchar section_number;                // 分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段

    uchar last_section_number;           // 最后一个分段的号码

    /** 循环节目信息 ?*4B */
    ts_pat_program pat_program[0];       // 节目信息，根据(section_length-8)/4可以计算出有多少个节目信息

    /** CRC 4B*/
}ts_pat_t;

/**
 * PMT中的节目描述信息
 */
typedef struct ts_pmt_program
{
    uchar stream_type;             // 指示特定PID的节目元素包的类型。该处PID由elementary PID指定

    uchar elementary_PID_1: 5;     // 该域指示TS包的PID值。这些TS包含有相关的节目元素
    uchar reserved_5: 3;           // '111' 0x07

    uchar elementary_PID_2;        // 该域指示TS包的PID值。这些TS包含有相关的节目元素

    uchar ES_info_length_1 : 4;    //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
    uchar reserved_6 : 4;          // '1111' 0x0F

    uchar ES_info_length_2;        //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数

}ts_pmt_program_t; //5B

/**
 * PMT结构
 */
typedef struct ts_pmt
{
    uchar sync_byte;                     // 固定为0x02, 表示PMT表

    uchar section_length_1 : 4;          // 首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC。
    uchar reserved_1 : 2;                // '11' 0x3
    uchar zero: 1;                       // '0'
    uchar section_syntax_indicator : 1;  // '1'

    uchar section_length_2;              // 首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC。

    uint16_t program_number;             // 指出该节目对应于可应用的Program map PID

    uchar current_next_indicator: 1;     // 当该位置1时，当前传送的Program map section可用；当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。
    uchar version_number: 5;             // 指出TS流中Program map section的版本号
    uchar reserved_2: 2;                 // '11' 0x3

    uchar section_number : 8;            // 固定为0x00

    uchar last_section_number: 8;        // 固定为0x00

    uchar PCR_PID_1 : 5;                 // 指明TS包的PID值，该TS包含有PCR域，该PCR值对应于由节目号指定的对应节目。如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。
    uchar reserved_3 : 3;                // '111' 0x7

    uchar PCR_PID_2;                     // 指明TS包的PID值，该TS包含有PCR域，该PCR值对应于由节目号指定的对应节目。如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。

    uchar program_info_length_1 : 4;     // 前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。
    uchar reserved_4 : 4;                // '1111' 0xF

    uchar program_info_length_2;         // 前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。

    /** 循环节目信息 ?*5B */
    ts_pmt_program_t pmt_program[0];     // 节目信息，根据(section_length-program_info_length-13)/5可以计算出有多少个节目信息

    /** CRC 4B*/
}ts_pmt_t;

/**
 * 自适应字段前2个字节 长度和各个标志位
 */
typedef struct ts_adaptation_field
{
    uchar adaptation_field_length;                 // 自适应段长度

    uchar adaptation_field_extension_flag:1;       //调整字段有扩展
    uchar transport_private_data_flag:1;           //私用字节
    uchar splicing_point_flag:1;                   //拼接点标志
    uchar OPCR_flag:1;                             //包含opcr字段
    uchar PCR_flag:1;                              //包含pcr字段
    uchar elementary_stream_priority_indicator:1;  //优先级
    uchar random_access_indicator:1;               //表明下一个有相同PID的PES分组应该含有PTS字段和一个原始流访问点
    uchar discontinuty_indicator:1;                //1表明当前传送流分组的不连续状态为真
}ts_adaptation_field_t;

/**
 * 自适应字段中的PCR或者OPCR，6个字节
 */
typedef struct ts_adaptation_field_pcr
{
    uint32_t program_clock_reference_base_1;       //自适应段中用到的的pcr

    uchar program_clock_reference_base_2 : 1;      //自适应段中用到的的pcr
    uchar Reserved : 6;                            //保留位
    uchar program_clock_reference_extension_1 : 1; //PCR扩展

    uchar program_clock_reference_extension_2;     //PCR扩展
}ts_adaptation_field_pcr;
#pragma pack()


/**
 * TS流处理，用来解析ts流或生成ts流
 */
class CTS : public IAnalyzer
{
public:
    CTS(void);
    ~CTS(void);

    /**
     * 将PES打包成TS流
     * @param pBuf[in] PES数据
     * @param nLen[in] PES数据长度
     */
    int InputBuffer(char* pBuf, long nLen);

    /**
     * 设置回调方法
     */
    void SetCallBack(const function<void(char*,uint32_t)> cb){m_funCallBack = cb;};

    /**
     * 设置下一次输入pes的参数信息
     */
    void SetParam(uint8_t nNalType, uint64_t nVideoPts)
    {
        m_nNalType  = nNalType;
        m_nVideoPts = nVideoPts;
    }

private:
    /** TS Header */
    void SetPID(ts_header_t* pTS, uint16_t uPID);
    uint16_t GetPID(ts_header_t* pTS);

    /** PAT Program */
    void SetPatProgramNum(ts_pat_program_t* pPatProgram, uint16_t program_number);
    uint16_t GetPatProgramNum(ts_pat_program_t* pPatProgram);
    void SetPatPID(ts_pat_program_t* pPatProgram, uint16_t network_id_or_program_map_PID);
    uint16_t GetPatPID(ts_pat_program_t* pPatProgram);

    /** PAT */
    void SetPatSectionLength(ts_pat_t* pPat, uint16_t section_length);
    uint16_t GetPatSectionLength(ts_pat_t* pPat);
    void SetPatTransportStreamID(ts_pat_t* pPat, uint16_t transport_stream_id);
    uint16_t GetPatTransportStreamID(ts_pat_t* pPat);

    /** PMT Program */
    void SetPmtElementaryPID(ts_pmt_program_t* pPmtProgram, uint16_t elementary_PID);
    uint16_t GetPmtElementaryPID(ts_pmt_program_t* pPmtProgram);
    void SetPmtEsInfoLength(ts_pmt_program_t* pPmtProgram, uint16_t ES_info_length);
    uint16_t GetPmtEsInfoLength(ts_pmt_program_t* pPmtProgram);

    /** PMT */
    void SetPmtSectionLength(ts_pmt_t* pPmt, uint16_t section_length);
    uint16_t GetPmtSectionLength(ts_pmt_t* pPmt);
    void SetPmtProgramNum(ts_pmt_t* pPmt, uint16_t program_number);
    uint16_t GetPmtProgramNum(ts_pmt_t* pPmt);
    void SetPmtPCR_PID(ts_pmt_t* pPmt, uint16_t PCR_PID);
    uint16_t GetPmtPCR_PID(ts_pmt_t* pPmt);
    void SetPmtProgramInfoLength(ts_pmt_t* pPmt, uint16_t program_info_length);
    uint16_t GetPmtProgramInfoLength(ts_pmt_t* pPmt);

    /** Adpation field */
    void SetAdaptationPCRBase(ts_adaptation_field_pcr* pPcr, uint64_t pcrb);
    uint64_t GetAdaptationPCRBase(ts_adaptation_field_pcr* pPcr);
    void SetAdaptationPCRExtension(ts_adaptation_field_pcr* pPcr, uint16_t pcre);
    uint16_t GetAdaptationPCRExtension(ts_adaptation_field_pcr* pPcr);

    ts_header_t* CreatPAT(char* pBuff);

    ts_header_t* CreatPMT(char* pBuff);

    ts_header_t* CreatVideoTS(char* pBuff, uchar payload_unit_start_indicator, uchar adaptation_field_control);

private:
    //uint8_t        m_nNeedPatPmt;   // 为0时创建PAT和PMT，设置每40个ts包创建一次PAT和PMT
    char*          m_pTsBuff;       // 缓存的ts内容
    uint32_t       m_nTsBuffLen;    // 缓存的长度
    uint32_t       m_nTsDataLen;    // 实际缓存数据的长度
    uint64_t       m_nBeginPts;     // 起始的帧的pts

    uint8_t        m_nNalType;      // 添加进来的nalu的类型
    uint64_t       m_nVideoPts;     // 添加进来的pes的时间基准
    function<void(char*,uint32_t)>  m_funCallBack;     // 回调方法
};

