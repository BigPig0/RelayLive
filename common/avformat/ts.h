#pragma once
#include "avtypes.h"

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
 * TSͷ�ṹ
 */
typedef struct ts_header{
    unsigned char sync_byte;                          // '0x47',��ʾ�������һ��TS����

    unsigned char PID1 : 5;                           // Packet ID���룬Ψһ�ĺ����Ӧ��ͬ�İ�
    unsigned char transport_priority : 1;             // �������ȼ���־��0:���ȼ��ͣ�1:���ȼ��ߣ�
    unsigned char payload_unit_start_indicator : 1;   // ��Ч���ص�Ԫ��ʼָʾ����һ����ƵPES��ֳɶ����ʱ����һ����Ҫ���á�1��������Ϊ��0����PAT,PMT������������Ҳ�ǡ�1����
    unsigned char transport_error_indicator : 1;      // ��������ָʾ��

    unsigned char PID2;                               // Packet ID���룬Ψһ�ĺ����Ӧ��ͬ�İ�

    unsigned char continuity_counter : 4;             // ������������
    unsigned char adaptation_field_control : 2;       // �����������(00:��10:���е����ֶΣ�01:������Ч���أ�11:���е����ֶκ���Ч����)
    unsigned char transport_scrambling_control : 2;   // ���ܱ�־��00��δ���ܣ�������ʾ�Ѽ��ܣ�
}ts_header_t; //4B

/**
 * PAT�еĽ�Ŀ��Ϣ
 */
typedef struct ts_pat_program  
{  
    uint16_t program_number;                   //��Ŀ��

    uchar network_id_or_program_map_PID_1 : 5; // ��Ŀ��Ϊ0x0000ʱ,��ʾ����NIT��PID=0x001f����31
    uchar reserved:3;                          // ����λ '111'

    uchar network_id_or_program_map_PID_2;     // ��Ŀ��Ϊ0x0001ʱ,��ʾ����PMT��PID=0x100����256
}ts_pat_program_t; //4B

/**
 * PAT�ṹ
 */
typedef struct ts_pat{
    uchar table_id;                      //PAT��table_idֻ����0x00

    uchar section_length_1 : 4;          // ��ʾ����ֽں������õ��ֽ���������CRC32
    uchar reserved_1 : 2;                // ����λ '11'
    uchar zero : 1;                      // '0' 
    uchar section_syntax_indicator : 1;  // ���﷨��־λ���̶�Ϊ1

    uchar section_length_2;              // ��ʾ����ֽں������õ��ֽ���������CRC32

    uint16_t transport_stream_id;        // �ô�������ID��������һ��������������·���õ���

    uchar current_next_indicator: 1;     // ���͵�PAT�ǵ�ǰ��Ч('1')������һ��PAT��Ч('0')
    uchar version_number : 5;            // ��Χ0-31����ʾPAT�İ汾��;��һ����0��һ��PAT�б仯���汾�ż�1
    uchar reserved_2 : 2;                // ����λ '11'

    uchar section_number;                // �ֶεĺ��롣PAT���ܷ�Ϊ��δ��䣬��һ��Ϊ00���Ժ�ÿ���ֶμ�1����������256���ֶ�

    uchar last_section_number;           // ���һ���ֶεĺ���

    /** ѭ����Ŀ��Ϣ ?*4B */
    ts_pat_program pat_program[0];       // ��Ŀ��Ϣ������(section_length-8)/4���Լ�����ж��ٸ���Ŀ��Ϣ

    /** CRC 4B*/
}ts_pat_t;

/**
 * PMT�еĽ�Ŀ������Ϣ
 */
typedef struct ts_pmt_program
{
    uchar stream_type;             // ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��

    uchar elementary_PID_1: 5;     // ����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ��
    uchar reserved_5: 3;           // '111' 0x07

    uchar elementary_PID_2;        // ����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ��

    uchar ES_info_length_1 : 4;    //ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��
    uchar reserved_6 : 4;          // '1111' 0x0F

    uchar ES_info_length_2;        //ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��

}ts_pmt_program_t; //5B

/**
 * PMT�ṹ
 */
typedef struct ts_pmt
{
    uchar sync_byte;                     // �̶�Ϊ0x02, ��ʾPMT��

    uchar section_length_1 : 4;          // ������λbit��Ϊ00����ָʾ�ε�byte�����ɶγ�����ʼ������CRC��
    uchar reserved_1 : 2;                // '11' 0x3
    uchar zero: 1;                       // '0'
    uchar section_syntax_indicator : 1;  // '1'

    uchar section_length_2;              // ������λbit��Ϊ00����ָʾ�ε�byte�����ɶγ�����ʼ������CRC��

    uint16_t program_number;             // ָ���ý�Ŀ��Ӧ�ڿ�Ӧ�õ�Program map PID

    uchar current_next_indicator: 1;     // ����λ��1ʱ����ǰ���͵�Program map section���ã�����λ��0ʱ��ָʾ��ǰ���͵�Program map section�����ã���һ��TS����Program map section��Ч��
    uchar version_number: 5;             // ָ��TS����Program map section�İ汾��
    uchar reserved_2: 2;                 // '11' 0x3

    uchar section_number : 8;            // �̶�Ϊ0x00

    uchar last_section_number: 8;        // �̶�Ϊ0x00

    uchar PCR_PID_1 : 5;                 // ָ��TS����PIDֵ����TS������PCR�򣬸�PCRֵ��Ӧ���ɽ�Ŀ��ָ���Ķ�Ӧ��Ŀ���������˽���������Ľ�Ŀ������PCR�޹أ�������ֵ��Ϊ0x1FFF��
    uchar reserved_3 : 3;                // '111' 0x7

    uchar PCR_PID_2;                     // ָ��TS����PIDֵ����TS������PCR�򣬸�PCRֵ��Ӧ���ɽ�Ŀ��ָ���Ķ�Ӧ��Ŀ���������˽���������Ľ�Ŀ������PCR�޹أ�������ֵ��Ϊ0x1FFF��

    uchar program_info_length_1 : 4;     // ǰ��λbitΪ00������ָ���������Խ�Ŀ��Ϣ��������byte����
    uchar reserved_4 : 4;                // '1111' 0xF

    uchar program_info_length_2;         // ǰ��λbitΪ00������ָ���������Խ�Ŀ��Ϣ��������byte����

    /** ѭ����Ŀ��Ϣ ?*5B */
    ts_pmt_program_t pmt_program[0];     // ��Ŀ��Ϣ������(section_length-program_info_length-13)/5���Լ�����ж��ٸ���Ŀ��Ϣ

    /** CRC 4B*/
}ts_pmt_t;

/**
 * ����Ӧ�ֶ�ǰ2���ֽ� ���Ⱥ͸�����־λ
 */
typedef struct ts_adaptation_field
{
    uchar adaptation_field_length;                 // ����Ӧ�γ���

    uchar adaptation_field_extension_flag:1;       //�����ֶ�����չ
    uchar transport_private_data_flag:1;           //˽���ֽ�
    uchar splicing_point_flag:1;                   //ƴ�ӵ��־
    uchar OPCR_flag:1;                             //����opcr�ֶ�
    uchar PCR_flag:1;                              //����pcr�ֶ�
    uchar elementary_stream_priority_indicator:1;  //���ȼ�
    uchar random_access_indicator:1;               //������һ������ͬPID��PES����Ӧ�ú���PTS�ֶκ�һ��ԭʼ�����ʵ�
    uchar discontinuty_indicator:1;                //1������ǰ����������Ĳ�����״̬Ϊ��
}ts_adaptation_field_t;

/**
 * ����Ӧ�ֶ��е�PCR����OPCR��6���ֽ�
 */
typedef struct ts_adaptation_field_pcr
{
    uint32_t program_clock_reference_base_1;       //����Ӧ�����õ��ĵ�pcr

    uchar program_clock_reference_base_2 : 1;      //����Ӧ�����õ��ĵ�pcr
    uchar Reserved : 6;                            //����λ
    uchar program_clock_reference_extension_1 : 1; //PCR��չ

    uchar program_clock_reference_extension_2;     //PCR��չ
}ts_adaptation_field_pcr;
#pragma pack()


/**
 * TS��������������ts��������ts��
 */
class CTS
{
public:
    CTS(AV_CALLBACK cb, void* handle=NULL);
    ~CTS(void);

    /**
     * ��PES�����TS��
     * @param pBuf[in] PES����
     * @param nLen[in] PES���ݳ���
     */
    int Code(char* pBuf, uint32_t nLen);

    /**
     * ������һ������pes�Ĳ�����Ϣ
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
    //uint8_t        m_nNeedPatPmt;   // Ϊ0ʱ����PAT��PMT������ÿ40��ts������һ��PAT��PMT
    char*          m_pTsBuff;       // �����ts����
    uint32_t       m_nTsBuffLen;    // ����ĳ���
    uint32_t       m_nTsDataLen;    // ʵ�ʻ������ݵĳ���
    uint64_t       m_nBeginPts;     // ��ʼ��֡��pts

    uint8_t        m_nNalType;      // ��ӽ�����nalu������
    uint64_t       m_nVideoPts;     // ��ӽ�����pes��ʱ���׼
    
    void*              m_hUser;                  // �ص��������
    AV_CALLBACK        m_fCB;
};

