#include "stdafx.h"
#include "pes.h"

//#define SELF_DTS

static PESType pes_type(pes_header_t* pes)
{
    if (pes->pes_start_code_prefix[0] == 0 && pes->pes_start_code_prefix[1] == 0 && pes->pes_start_code_prefix[2] == 1)
    {
        if (pes->stream_id == 0xC0)
        {
            return pes_audio;
        }
        else if (pes->stream_id == 0xE0)
        {
            return pes_video;
        }
        else if ((pes->stream_id == 0xBC) || (pes->stream_id == 0xBD) || (pes->stream_id == 0xBE) || (pes->stream_id == 0xBF))
        {
            return pes_jump;
        }
        else if ((pes->stream_id == 0xF0) || (pes->stream_id == 0xF1) || (pes->stream_id == 0xF2) || (pes->stream_id == 0xF8))
        {
            return pes_jump;
        }
    }
    return pes_unkown;
}

#ifndef AV_RB16
#define AV_RB16(x)                           \
    ((((const unsigned char*)(x))[0] << 8) | \
    ((const unsigned char*)(x))[1])
#endif

static uint64_t ff_parse_pes_pts(const unsigned char* buf) {
    return (uint64_t)(*buf & 0x0e) << 29 |
        (AV_RB16(buf + 1) >> 1) << 15 |
        AV_RB16(buf + 3) >> 1;
}

#ifdef SELF_DTS
static uint64_t _dts = 0, _pts = 0;
static uint64_t get_pts(optional_pes_header* option)
{
	return _pts+= 3690;
}
static uint64_t get_dts(optional_pes_header* option)
{
	return _dts+= 3690;
}
#else
static uint64_t get_pts(optional_pes_header* option)
{
    if (option->PTS_DTS_flags != 2 && option->PTS_DTS_flags != 3 && option->PTS_DTS_flags != 0)
    {
        return 0;
    }
    if ((option->PTS_DTS_flags & 2) == 2)
    {
        unsigned char* pts = (unsigned char*)option + sizeof(optional_pes_header);
        return ff_parse_pes_pts(pts);
    }
    return 0;
}

static uint64_t get_dts(optional_pes_header* option)
{
    if (option->PTS_DTS_flags != 2 && option->PTS_DTS_flags != 3 && option->PTS_DTS_flags != 0)
    {
        return 0;
    }
    if ((option->PTS_DTS_flags & 3) == 3)
    {
        unsigned char* dts = (unsigned char*)option + sizeof(optional_pes_header)+5;
        return ff_parse_pes_pts(dts);
    }
    return 0;
}
#endif


CPes::CPes(PES_CALLBACK cb, void* handle)
    : m_hUser(handle)
    , m_fCB(cb)
{
}


CPes::~CPes(void)
{
}

int CPes::Decode(AV_BUFF buff)
{
    pes_header_t* pes = (pes_header_t*)buff.pData;
    if (!is_pes_header(pes))
    {
        Log::error("CPesAnalyzer::InsertPacket this is not a pes packet");
        return -1;
    }

    // 只需要处理视频
    PESType pesType = pes_type(pes);
    if (pesType != pes_video)
    {
        //Log::error("CPesAnalyzer::InsertPacket this pes is not video %d", pesType);
        return -1;
    }

    optional_pes_header* option = (optional_pes_header*)((char*)pes + sizeof(pes_header_t));
    if (option->PTS_DTS_flags != 2 && option->PTS_DTS_flags != 3 && option->PTS_DTS_flags != 0)
    {
        Log::error("PTS_DTS_flags is 01 which is invalid PTS_DTS_flags");
        return -1;
    }

    uint64_t pts = get_pts(option);
    uint64_t dts = get_dts(option);
    //unsigned char stream_id = pes->stream_id;
    int32_t pesLen =  pes->PES_packet_length[0];
    pesLen <<= 8;
    pesLen += pes->PES_packet_length[1];

    char* pESBuffer = ((char*)option + sizeof(optional_pes_header) + option->PES_header_data_length);
    int nESLength = pesLen - (sizeof(optional_pes_header) + option->PES_header_data_length);

    // 回调处理ES包
    if (m_fCB != nullptr)
    {
        AV_BUFF es = {AV_TYPE::ES, pESBuffer, nESLength};
        m_fCB(es, m_hUser, pts, dts);
    }

    return 0;
}