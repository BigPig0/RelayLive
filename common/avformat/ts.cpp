#include "common.h"
#include "Endian.h"
#include "ts.h"
#include "crc.h"
#include "h264.h"

const uint8_t nTsContentLength = TS_PACKET_SIZE - TS_PACKET_HEADER;

CTS::CTS(AV_CALLBACK cb, void* handle)
    : m_pTsBuff(nullptr)
    , m_nTsBuffLen(0)
    , m_nTsDataLen(0)
    , m_nBeginPts(0)
    , m_nNalType(0)
    , m_nVideoPts(0)
    , m_hUser(handle)
    , m_fCB(cb)
{
}


CTS::~CTS(void)
{
}

int CTS::Code(char* pBuf, uint32_t nLen)
{
    //Log::debug("ts begin");
    CHECKPOINT_INT(pBuf, -1);

    uint8_t  nNalType  = m_nNalType;
    uint64_t nVideoPts = m_nVideoPts;
    if (m_nBeginPts == 0)
        m_nBeginPts = nVideoPts;

    //�ж��Ƿ��Ѿ����ڻ�������
    if (m_pTsBuff != nullptr)
    {
        bool bEnough = false;
        if (nVideoPts > m_nBeginPts)
        {
            if (nVideoPts - m_nBeginPts >= TS_TARGETDURATION)
            {
                bEnough = true;
            }
        }
        else if (m_nBeginPts > nVideoPts)
        {
            if ((uint64_t)(0x1FFFFFFFF) - m_nBeginPts + nVideoPts >= TS_TARGETDURATION)
            {
                bEnough = true;
            }
        }
        // �洢���㹻��������
        if (bEnough && (nNalType == idr_Nal || nNalType == sps_Nal || nNalType == pps_Nal))
        {
            Log::debug("ts callback m_nBeginPts:%lld, nVideoPts:%lld", m_nBeginPts, nVideoPts);
            if(m_fCB != nullptr){
                AV_BUFF buff = {TS, m_pTsBuff, m_nTsDataLen};
                m_fCB(buff, m_hUser);
            }
            m_pTsBuff = nullptr;
            m_nTsBuffLen = m_nTsDataLen = 0;
            m_nBeginPts = nVideoPts;
        }
    }

    //�ж��Ƿ���Ҫ���뻺��ռ�
    if (m_pTsBuff == nullptr)
    {
        m_nTsBuffLen = TS_ONE_FILE_SIZE;
        m_pTsBuff = (char*)malloc(TS_ONE_FILE_SIZE);
        CHECKPOINT_INT(m_pTsBuff,-1)
    }

    //��Ƭ���ĵ�һ�����ĸ��س���
    uint8_t FirstPacketLoadLength;
    if(nNalType == b_Nal || nNalType == idr_Nal) //��Ƶ֡
    {
        //ȥ��tsͷ4�ֽڣ�����Ӧ�γ���1�ֽڣ�����Ӧ��tags1�ֽ�, pcr�ֶ�6�ֽ�
        FirstPacketLoadLength = nTsContentLength - 8;
    }
    else //����Ƶ֡
    {
        //ȥ��tsͷ4�ֽڣ�����Ӧ�γ���1�ֽڣ�����Ӧ��tags1�ֽ�
        FirstPacketLoadLength = nTsContentLength - 2;
    }

    // �˴ο����Ļ���λ��
    char* pPos = m_pTsBuff + m_nTsDataLen;

#define MOVE_POS \
    pPos += TS_PACKET_SIZE;\
    m_nTsDataLen += TS_PACKET_SIZE;\
    if (m_nTsDataLen > m_nTsBuffLen)\
    {\
        m_nTsBuffLen += TS_ONE_FILE_SIZE;\
        m_pTsBuff = (char*)realloc(m_pTsBuff, m_nTsBuffLen);\
        pPos = m_pTsBuff + m_nTsDataLen;\
    }\

    if (m_nTsDataLen == 0)
    {
        ts_header_t* pTsPat = CreatPAT(pPos);
        if (nullptr == pTsPat)
        {
            Log::error("CreatPAT Failed");
        }
        MOVE_POS

        ts_header_t* pTsPmt = CreatPMT(pPos);
        if (nullptr == pTsPmt)
        {
            Log::error("CreatPMT failed");
        }
        MOVE_POS
    }

    //����Ҫ��Ƭ�����
    if (nLen < FirstPacketLoadLength)
    {
        ts_header_t* pTs = CreatVideoTS(pPos, 0x01, 0x03); //0x01��Ԫ��ʼ����0x03��������Ӧ�ֶκ���Ч�غ�
        if (nullptr == pTs)
        {
            Log::error("CreatVideoTS failed");
            return false;
        }
        MOVE_POS

        // ts_adaptation_field ������
        uchar* pAdaptationLen = (uchar*)(pTs+1);
        // ����Ӧ�εĴ�СΪ�ܴ�С(�ų���tsͷ4�ֽڣ�pes��С�������ֶ��Լ�1���ֽ�)
        *pAdaptationLen = nTsContentLength - (uint8_t)nLen  - 1;
        *(pAdaptationLen+1) = 0x00; //����tagΪ0

        //PES��������
        uchar* pPes = pAdaptationLen + 1 + *pAdaptationLen;
        memcpy_s(pPes, (rsize_t)nLen, pBuf, (rsize_t)nLen);

        //++m_nNeedPatPmt;
        //Log::debug("ts end");
        return true;
    }

    /******** ��Ҫ��Ƭ����� *************/

    //��¼���ݴ����λ��
    char* NeafBuf = pBuf;
    // ��һ����
    ts_header_t* pTs = CreatVideoTS(pPos, 0x01, 0x03);  //0x01��Ԫ��ʼ����0x03��������Ӧ�ֶκ���Ч�غ�
    if (nullptr == pTs)
    {
        Log::error("CreatVideoTS failed");
        return false;
    }
    MOVE_POS

    // ts_adaptation_field ������
    ts_adaptation_field* pAdaptation = (ts_adaptation_field*)(pTs+1);
    memset(pAdaptation,0,2);
    if(nNalType == b_Nal || nNalType == idr_Nal) //��Ƶ֡
    {
        pAdaptation->adaptation_field_length = 7; //tagһ�ֽڣ�pcr6�ֽ�
        pAdaptation->PCR_flag = 1;

        ts_adaptation_field_pcr* pPcr = (ts_adaptation_field_pcr*)(pAdaptation + 1);
        SetAdaptationPCRBase(pPcr, nVideoPts);
        SetAdaptationPCRExtension(pPcr, 0);

        //PES
        char* pPes = (char*)(pPcr + 1);
        memcpy_s(pPes, FirstPacketLoadLength, NeafBuf, FirstPacketLoadLength);
    }
    else  //����Ƶ֡
    {
        pAdaptation->adaptation_field_length = 1; //tagһ�ֽ�
        //PES
        char* pPes = (char*)(pAdaptation + 1);
        memcpy_s(pPes, FirstPacketLoadLength, NeafBuf, FirstPacketLoadLength);
    }

    NeafBuf += FirstPacketLoadLength;
    nLen -= FirstPacketLoadLength;
    //++m_nNeedPatPmt;

    while (nLen > 0)
    {
        if (nLen >= nTsContentLength)
        {
            ts_header_t* pTs = CreatVideoTS(pPos, 0x00, 0x01); //0x00�ǵ�Ԫ��ʼ����0x01������Ч�غ�
            if (nullptr == pTs)
            {
                Log::error("CreatVideoTS failed");
                continue;
            }
            MOVE_POS

            //PES
            char* pPes = (char*)(pTs + 1);
            memcpy_s(pPes, nTsContentLength, NeafBuf, nTsContentLength);

            NeafBuf += nTsContentLength;
            nLen -= nTsContentLength;
            //++m_nNeedPatPmt;
        }
        else if (nLen == nTsContentLength-1 || nLen == nTsContentLength-2)
        {//С��184���������Ӧ�ֶΣ�������Ӧ�ֶ�����2�ֽڣ�����183���ֽڵ����ݷ���Ҫ�������ts��
            ts_header_t* pTs = CreatVideoTS(pPos, 0x00, 0x03);  //0x00�ǵ�Ԫ��ʼ����0x03��������Ӧ�ֶκ���Ч�غ�
            if (nullptr == pTs)
            {
                Log::error("CreatVideoTS failed");
                continue;
            }
            MOVE_POS

            // ts_adaptation_field ������
            ts_adaptation_field* pAdaptation = (ts_adaptation_field*)(pTs+1);
            memset(pAdaptation,0,2);
            pAdaptation->adaptation_field_length = 1; //tagһ�ֽ�

            //PES
            char* pPes = (char*)pAdaptation;
            pPes = pPes + 1 + pAdaptation->adaptation_field_length;
            memcpy_s(pPes, 182, NeafBuf, 182);

            NeafBuf += 182;
            nLen -= 182;
            //++m_nNeedPatPmt;
        }
        else
        {
            ts_header_t* pTs = CreatVideoTS(pPos, 0x00, 0x03);  //0x00�ǵ�Ԫ��ʼ����0x03��������Ӧ�ֶκ���Ч�غ�
            if (nullptr == pTs)
            {
                Log::error("CreatVideoTS failed");
                continue;
            }
            MOVE_POS

            // ts_adaptation_field ������
            ts_adaptation_field* pAdaptation = (ts_adaptation_field*)(pTs+1);
            memset(pAdaptation,0,2);
            pAdaptation->adaptation_field_length = nTsContentLength - (uint8_t)nLen - 1;

            //PES
            char* pPes = (char*)pAdaptation;
            pPes = pPes + 1 + pAdaptation->adaptation_field_length;
            memcpy_s(pPes, (rsize_t)nLen, NeafBuf, (rsize_t)nLen);

            NeafBuf += nLen;
            nLen = 0;
            //++m_nNeedPatPmt;
        }
    }//while (nLen > 0)
    //Log::debug("ts begin2");
    return true;
}

void CTS::SetPID(ts_header_t* pTS, uint16_t uPID)
{
    CHECKPOINT_VOID(pTS)
    pTS->PID1 = (uPID >> 8)&0x1F;
    pTS->PID2 = uPID & 0xFF;
}

uint16_t CTS::GetPID(ts_header_t* pTS)
{
    CHECKPOINT_INT(pTS,(uint16_t)(-1))
    uint16_t uPID = pTS->PID1;
    uPID = uPID << 8;
    uPID |= pTS->PID2;
    return uPID;
}

void CTS::SetPatProgramNum(ts_pat_program_t* pPatProgram, uint16_t program_number)
{
    CHECKPOINT_VOID(pPatProgram)
    pPatProgram->program_number = Util::EndianChange16(program_number);
}

uint16_t CTS::GetPatProgramNum(ts_pat_program_t* pPatProgram)
{
    CHECKPOINT_INT(pPatProgram,(uint16_t)(-1));
    return Util::EndianChange16(pPatProgram->program_number);
}

void CTS::SetPatPID(ts_pat_program_t* pPatProgram, uint16_t network_id_or_program_map_PID)
{
    CHECKPOINT_VOID(pPatProgram)
    pPatProgram->network_id_or_program_map_PID_1 = (network_id_or_program_map_PID >> 8)&0x1F;
    pPatProgram->network_id_or_program_map_PID_2 = network_id_or_program_map_PID & 0xFF;
}

uint16_t CTS::GetPatPID(ts_pat_program_t* pPatProgram)
{
    CHECKPOINT_INT(pPatProgram,(uint16_t)(-1));
    uint16_t uPID = pPatProgram->network_id_or_program_map_PID_1;
    uPID = uPID << 8;
    uPID |= pPatProgram->network_id_or_program_map_PID_2;
    return uPID;
}

void CTS::SetPatSectionLength(ts_pat_t* pPat, uint16_t section_length)
{
    CHECKPOINT_VOID(pPat)
    pPat->section_length_1 = (section_length >> 8)&0x0F;
    pPat->section_length_2 = section_length & 0xFF;
}

uint16_t CTS::GetPatSectionLength(ts_pat_t* pPat)
{
    CHECKPOINT_INT(pPat,(uint16_t)(-1));
    uint16_t uSectionLen = pPat->section_length_1;
    uSectionLen = uSectionLen << 8;
    uSectionLen |= pPat->section_length_2;
    return uSectionLen;
}

void CTS::SetPatTransportStreamID(ts_pat_t* pPat, uint16_t transport_stream_id)
{
    CHECKPOINT_VOID(pPat)
    pPat->transport_stream_id = Util::EndianChange16(transport_stream_id);
}

uint16_t CTS::GetPatTransportStreamID(ts_pat_t* pPat)
{
    CHECKPOINT_INT(pPat,(uint16_t)(-1));
    return Util::EndianChange16(pPat->transport_stream_id);
}

void CTS::SetPmtElementaryPID(ts_pmt_program_t* pPmtProgram, uint16_t elementary_PID)
{
    CHECKPOINT_VOID(pPmtProgram)
    pPmtProgram->elementary_PID_1 = (elementary_PID >> 8)&0x1F;
    pPmtProgram->elementary_PID_2 = elementary_PID & 0xFF;
}

uint16_t CTS::GetPmtElementaryPID(ts_pmt_program_t* pPmtProgram)
{
    CHECKPOINT_INT(pPmtProgram,(uint16_t)(-1));
    uint16_t uElementary_PID = pPmtProgram->elementary_PID_1&0x1F;
    uElementary_PID = uElementary_PID << 8;
    uElementary_PID |= pPmtProgram->elementary_PID_2;
    return uElementary_PID;
}

void CTS::SetPmtEsInfoLength(ts_pmt_program_t* pPmtProgram, uint16_t ES_info_length)
{
    CHECKPOINT_VOID(pPmtProgram)
    pPmtProgram->ES_info_length_1 = (ES_info_length >> 8)&0x0F;
    pPmtProgram->ES_info_length_2 = ES_info_length & 0xFF;
}

uint16_t CTS::GetPmtEsInfoLength(ts_pmt_program_t* pPmtProgram)
{
    CHECKPOINT_INT(pPmtProgram,(uint16_t)(-1));
    uint16_t ES_info_length = pPmtProgram->ES_info_length_1&0x0F;
    ES_info_length = ES_info_length << 8;
    ES_info_length |= pPmtProgram->ES_info_length_2;
    return ES_info_length;
}

void CTS::SetPmtSectionLength(ts_pmt_t* pPmt, uint16_t section_length)
{
    CHECKPOINT_VOID(pPmt)
    pPmt->section_length_1 = (section_length >> 8)&0x0F;
    pPmt->section_length_2 = section_length & 0xFF;
}

uint16_t CTS::GetPmtSectionLength(ts_pmt_t* pPmt)
{
    CHECKPOINT_INT(pPmt,(uint16_t)(-1));
    uint16_t uSectionLen = pPmt->section_length_1&0x0F;
    uSectionLen = uSectionLen << 8;
    uSectionLen |= pPmt->section_length_2;
    return uSectionLen;
}

void CTS::SetPmtProgramNum(ts_pmt_t* pPmt, uint16_t program_number)
{
    pPmt->program_number = Util::EndianChange16(program_number);
}

uint16_t CTS::GetPmtProgramNum(ts_pmt_t* pPmt)
{
    return Util::EndianChange16(pPmt->program_number);
}

void CTS::SetPmtPCR_PID(ts_pmt_t* pPmt, uint16_t PCR_PID)
{
    CHECKPOINT_VOID(pPmt)
    pPmt->PCR_PID_1 = (PCR_PID >> 8)&0x1F;
    pPmt->PCR_PID_2 = PCR_PID & 0xFF;
}

uint16_t CTS::GetPmtPCR_PID(ts_pmt_t* pPmt)
{
    CHECKPOINT_INT(pPmt,(uint16_t)(-1));
    uint16_t uRet = pPmt->section_length_1&0x1F;
    uRet = uRet << 8;
    uRet |= pPmt->section_length_2;
    return uRet;
}

void CTS::SetPmtProgramInfoLength(ts_pmt_t* pPmt, uint16_t program_info_length)
{
    CHECKPOINT_VOID(pPmt)
    pPmt->program_info_length_1 = (program_info_length >> 8)&0x0F;
    pPmt->program_info_length_2 = program_info_length & 0xFF;
}

uint16_t CTS::GetPmtProgramInfoLength(ts_pmt_t* pPmt)
{
    CHECKPOINT_INT(pPmt,(uint16_t)(-1));
    uint16_t uRet = pPmt->program_info_length_1&0x0F;
    uRet = uRet << 8;
    uRet |= pPmt->program_info_length_2;
    return uRet;
}

void CTS::SetAdaptationPCRBase(ts_adaptation_field_pcr* pPcr, uint64_t pcrb)
{
    CHECKPOINT_VOID(pPcr)
    pPcr->program_clock_reference_base_1 = Util::EndianChange32((uint32_t)(pcrb >> 1));  //�ӵ�2λ����33λ
    pPcr->program_clock_reference_base_2 = pcrb&0x01; //��1λ
}

uint64_t CTS::GetAdaptationPCRBase(ts_adaptation_field_pcr* pPcr)
{
    CHECKPOINT_INT(pPcr,(uint16_t)(-1))
    uint64_t uRet = Util::EndianChange32(pPcr->program_clock_reference_base_1);
    uRet = uRet << 1;
    uRet |= pPcr->program_clock_reference_base_2;
    return uRet;
}

void CTS::SetAdaptationPCRExtension(ts_adaptation_field_pcr* pPcr, uint16_t pcre)
{
    CHECKPOINT_VOID(pPcr)
    pPcr->program_clock_reference_extension_1 = pcre >> 8;  //��9λ
    pPcr->program_clock_reference_extension_2 = pcre&0xFF;  //��1λ����8λ
}

uint16_t CTS::GetAdaptationPCRExtension(ts_adaptation_field_pcr* pPcr)
{
    CHECKPOINT_INT(pPcr,(uint16_t)(-1))
    uint16_t uRet = pPcr->program_clock_reference_extension_1;
    uRet = uRet << 8;
    uRet |= pPcr->program_clock_reference_extension_2;
    return uRet;
}

ts_header_t* CTS::CreatPAT(char* pBuff)
{
    static uchar continuity_counter = 0;

    ts_header_t* pTs = (ts_header_t*)pBuff;
    CHECKPOINT_NULLPTR(pTs);
    memset(pTs, 0xFF, TS_PACKET_SIZE);

    pTs->sync_byte = TS_SYNC_BYTE;
    pTs->transport_error_indicator = 0;
    pTs->payload_unit_start_indicator = 1;
    pTs->transport_priority = 0;
    SetPID(pTs, TS_PAT_PID);
    pTs->transport_scrambling_control = 0;
    pTs->adaptation_field_control = 1; // ��������Ч�غ�
    pTs->continuity_counter = (continuity_counter++)%16;

    // ����Ӧ�εĳ���Ϊ0
    char* pAdaptation = (char*)(pTs+1);
    memset(pAdaptation, 0, 1);

    //PAT
    ts_pat_t* pPat = (ts_pat_t*)(pAdaptation+1);
    pPat->table_id = 0x00;
    pPat->section_syntax_indicator = 1;
    pPat->zero = 0;
    pPat->reserved_1 = 3; //'11'
    SetPatSectionLength(pPat, 0x0d); //��transport_stream_id��crc֮��ĳ��ȣ�ֻ��һ��ts_pat_program
    SetPatTransportStreamID(pPat, 1);
    pPat->reserved_2 = 3; //'11'
    pPat->version_number = 0; //ֻ��һ����Ŀ��Զ�����б仯
    pPat->current_next_indicator = 1;
    pPat->section_number = 0;
    pPat->last_section_number = 0;
    // ֻ��һ��program
    SetPatProgramNum(pPat->pat_program,1);
    pPat->pat_program[0].reserved = 7; //'111'
    SetPatPID(pPat->pat_program, TS_PMT_PID);
    //CRC
    uint32_t* pCrc = (uint32_t*)(pPat->pat_program+1);
    *pCrc = Util::EndianChange32(Util::calc_crc32((unsigned char*)pPat, 12));

    return pTs;
}

ts_header_t* CTS::CreatPMT(char* pBuff)
{
    static uchar continuity_counter = 0;

    ts_header_t* pTs = (ts_header_t*)pBuff;
    CHECKPOINT_NULLPTR(pTs);
    memset(pTs, 0xFF, TS_PACKET_SIZE);

    pTs->sync_byte = TS_SYNC_BYTE;
    pTs->transport_error_indicator = 0;
    pTs->payload_unit_start_indicator = 1;
    pTs->transport_priority = 0;
    SetPID(pTs, TS_PMT_PID);
    pTs->transport_scrambling_control = 0;
    pTs->adaptation_field_control = 1; // ��������Ч�غ�
    pTs->continuity_counter = (continuity_counter++)%16;

    // ����Ӧ�εĳ���Ϊ0
    char* pAdaptation = (char*)(pTs+1);
    memset(pAdaptation, 0, 1);

    //PMT
    ts_pmt_t* pPmt = (ts_pmt_t*)(pAdaptation+1);
    pPmt->sync_byte = 0x02;
    pPmt->section_syntax_indicator = 1;
    pPmt->zero = 0;
    pPmt->reserved_1 = 3; //'11'
    SetPmtSectionLength(pPmt, 18); //��program_number��ʼ��CRC�ĳ��ȣ�ֻ��һ��ts_pmt_program_t
    SetPmtProgramNum(pPmt,1); //pat�е�programInfoһ��
    pPmt->reserved_2 = 3; //'11'
    pPmt->version_number = 0; //��Ŀ��Ϣ�����б仯
    pPmt->current_next_indicator = 1;
    pPmt->section_number = 0;
    pPmt->last_section_number = 0;
    pPmt->reserved_3 = 7; //'111'
    SetPmtPCR_PID(pPmt, TS_H264_PID);
    pPmt->reserved_4 = 0xF; //'1111'
    SetPmtProgramInfoLength(pPmt, 0);
    //ֻ��һ��program
    pPmt->pmt_program->stream_type = PMT_STREAM_TYPE_VIDEO;
    pPmt->pmt_program->reserved_5 = 7; //'111'
    SetPmtElementaryPID(pPmt->pmt_program, TS_H264_PID);
    pPmt->pmt_program->reserved_6 = 0xF; //'1111'
    SetPmtEsInfoLength(pPmt->pmt_program, 0);
    //CRC
    uint32_t* pCrc = (uint32_t*)(pPmt->pmt_program+1);
    *pCrc = Util::EndianChange32(Util::calc_crc32((unsigned char*)pPmt, 17));

    return pTs;
}

ts_header_t* CTS::CreatVideoTS(char* pBuff, uchar payload_unit_start_indicator, uchar adaptation_field_control)
{
    static uchar continuity_counter = 0;

    ts_header_t* pTs = (ts_header_t*)pBuff;
    CHECKPOINT_NULLPTR(pTs);
    memset(pTs, 0xFF, TS_PACKET_SIZE);

    pTs->sync_byte = TS_SYNC_BYTE;
    pTs->transport_error_indicator = 0;
    pTs->payload_unit_start_indicator = payload_unit_start_indicator;
    pTs->transport_priority = 0;
    SetPID(pTs, TS_H264_PID);
    pTs->transport_scrambling_control = 0;
    pTs->adaptation_field_control = adaptation_field_control; // ����Ӧ��
    pTs->continuity_counter = (continuity_counter++)%16;

    return pTs;
}