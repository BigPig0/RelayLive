#include "common.h"
#include "mp4.h"
#include "h264.h"


#define MP4_BOX_HEADER \
    uchar       size[4];\
    uchar       type[4];

#define MP4_FULLBOX_HEADER \
    uchar size[4];\
    uchar type[4];\
    uchar version;\
    uchar flags[3];

#pragma pack(1)
struct mp4_ftyp_box {
    MP4_BOX_HEADER;
    uchar       major_brand[4];
    uchar       minor_version[4];
    uchar       compatible_brands[20];
};

struct mp4_mvhd_box {
    MP4_FULLBOX_HEADER;
    uchar creation_time[4];
    uchar modification_time[4];
    uchar timescale[4];
    uchar duration[4];

    uchar rate[4];
    uchar volume[2];
    uchar reserved[10];

    uchar matrix[36];
    uchar pre_defined[24];
    uchar next_track_ID[4];
};

struct mp4_tkhd_box {
    MP4_FULLBOX_HEADER;
    uchar creation_time[4];
    uchar modification_time[4];
    uchar track_ID[4];
    uchar reserved1[4];
    uchar duration[4];
    uchar reserved2[8];

    uchar layer[2];
    uchar alternate_group[2];
    uchar volume[2]; 
    uchar reserved3[2];
    uchar matrix[36];

    uchar width[4];
    uchar height[4];
};

struct mp4_mdhd_box {
    MP4_FULLBOX_HEADER;
    uchar creation_time[4];
    uchar modification_time[4];
    uchar timescale[4];
    uchar duration[4];
    uchar fixed[2];
    uchar pre_defined[2];
};

struct mp4_hdlr_box {
    MP4_FULLBOX_HEADER;
    uchar pre_defined[4];
    uchar handler_type[4];
    uchar reserved[12];
    uchar name[13];
};

struct mp4_vmhd_box {
    MP4_FULLBOX_HEADER;
    uchar graphicsmode[2];
    uchar opcolor[6];
};

struct mp4_dref_box {
    MP4_FULLBOX_HEADER;
    uchar entry_count[4];
    uchar entry_size[4];
    uchar url[4];
    uchar entry_flags[4];
};

struct mp4_dinf_box {
    MP4_BOX_HEADER;
    mp4_dref_box dref;
};

struct VisualSampleEntry {
    MP4_BOX_HEADER;
    uchar reserved[6];
    uchar data_reference_index[2];

    uchar pre_defined1[2];
    uchar reserved1[2];
    uchar pre_defined2[12];
    uchar width[2];
    uchar height[2];
    uchar horizresolution[4];
    uchar vertresolution[4];
    uchar reserved2[4];
    uchar frame_count[2];
    uchar compressorname[32];
    uchar depth[2];
    uchar pre_defined3[2];
    uchar padding[0x2c];
};

struct mp4_stbl_box_inner {
    MP4_FULLBOX_HEADER;
    uchar entry_count[4];
    uchar entry[0];
};

struct mp4_stbl_box {
    MP4_BOX_HEADER;
    uchar body[0];
};


struct mp4_minf_box {
    MP4_BOX_HEADER;
    mp4_vmhd_box vmhd;
    mp4_dinf_box dinf;
    mp4_stbl_box stbl;
};


struct mp4_mdia_box {
    MP4_BOX_HEADER;
    mp4_mdhd_box mdhd;
    mp4_hdlr_box hdlr;
    mp4_minf_box minf;
};

struct mp4_trak_box {
    MP4_BOX_HEADER;
    mp4_tkhd_box tkhd;
    mp4_mdia_box mdia;
};

struct mp4_trex_box {
    MP4_FULLBOX_HEADER;
    uchar track_ID[4];
    uchar default_sample_description_index[4];
    uchar default_sample_duration[4];
    uchar default_sample_size[4];
    uchar default_sample_flags[4];
};

struct mp4_mvex_box {
    MP4_BOX_HEADER;
    mp4_trex_box trex;
};

struct mp4_moov_box {
    MP4_BOX_HEADER;
    mp4_mvhd_box mvhd;
    uchar       trak[0];
};

struct mp4_mfhd_box {
    MP4_FULLBOX_HEADER;
    uchar sequence_number[4];
};

struct mp4_tfhd_box {
    MP4_FULLBOX_HEADER;
    uchar track_ID[4];
    uchar sample_description_index[4];
    uchar default_sample_size[4];
    uchar default_sample_flags[4];
};

struct mp4_tfdt_box {
    MP4_FULLBOX_HEADER;
    uchar baseMediaDecodeTime[8];
};

struct mp4_trun_box {
    MP4_FULLBOX_HEADER;
    uchar sample_count[4];
    uchar data_offset[4]; // moof头到mdat荷载数据之间的偏移，也就是moof头大小+8字节
    uchar first_sample_flags[4];
    uchar sample_composition_time_offset[0]; //uint32 [sample_count]
};

struct mp4_traf_box {
    MP4_BOX_HEADER;
    mp4_tfhd_box tfhd;
    mp4_tfdt_box tfdt;
    mp4_trun_box trun;
};

struct mp4_moof_box {
    MP4_BOX_HEADER;
    mp4_mfhd_box mfhd;
    mp4_traf_box traf;
};

struct mp4_mdat_box {
    MP4_BOX_HEADER;
    uchar   body[0];
};
#pragma pack()


CMP4::CMP4(AV_CALLBACK cb, void* handle)
    : m_pSPS(nullptr)
    , m_pPPS(nullptr)
    , m_pMdat(nullptr)
    , m_nSampleNum(0)
    , m_timestamp(0)
    , m_tick_gap(400)
    , m_nNodelay(0)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nfps(25.0)
    , m_nHorizresolution(0x00480000)
    , m_mVertresolution(0x00480000)
    , m_bMakeHeader(false)
    , m_bFirstKey(false)
    , m_bGotSPS(false)
    , m_bGotPPS(false)
    , m_bRun(true)
    , m_nSeq(1)
    , m_hUser(handle)
    , m_fCB(cb)
{
    m_pSPS       = new CNetStreamMaker();
    m_pPPS       = new CNetStreamMaker();
	m_pKeyFrame  = new CNetStreamMaker();
    m_pMdat      = new CNetStreamMaker();
    m_pSamplePos = new CNetStreamMaker();
    m_pHeader    = new CNetStreamMaker();
    m_pFragment  = new CNetStreamMaker();
}

CMP4::~CMP4()
{
    m_bRun = false;
    SAFE_DELETE(m_pSPS);
    SAFE_DELETE(m_pPPS);
	SAFE_DELETE(m_pKeyFrame);
    SAFE_DELETE(m_pMdat);
    SAFE_DELETE(m_pSamplePos);
    SAFE_DELETE(m_pHeader);
    SAFE_DELETE(m_pFragment);
}

int CMP4::Code(AV_BUFF buff)
{
    if(!m_bRun)
    {
        Log::error("already stop");
        return false;
    }

    char* pBuf;
    uint32_t nLen = 0;
    h264_nalu_data2(buff.pData, buff.nLen, &pBuf, &nLen);
    NalType eType = h264_naltype(pBuf);

    if(eType == sps_Nal && m_nWidth==0){
        double fps;
        h264_sps_info(pBuf, nLen, &m_nWidth, &m_nHeight, &fps);
		m_tick_gap = 90000/(m_nfps>0?m_nfps:25);
        Log::debug("width = %d,height = %d, fps= %lf, tickgap= %d",m_nWidth,m_nHeight,m_nfps,m_tick_gap);
    }

    switch (eType)
    {
    case b_Nal:
		{
			// 发送非关键帧
			if(!m_bFirstKey)
				break;
			//如果存在关键帧没有处理，需要先发送关键帧。pps可能在关键帧后面接收到。
			//sps或pps丢失，可以使用上一次的。
			MakeKeyVideo(); 
			//Log::debug("send frame");
			MakeVideo(pBuf, nLen, false);
		}
        break;
    case idr_Nal: // 关键帧
		{
			m_pKeyFrame->clear();
			m_pKeyFrame->append_data(pBuf, nLen);
			if(m_bGotPPS && m_bGotSPS)
				MakeKeyVideo();
		}
        break;
    case sei_Nal:
        break;
    case sps_Nal:
        {
            //Log::debug("save sps size:%d",nLen);
            CHECKPOINT_INT(m_pSPS,-1);
            m_pSPS->clear();
            m_pSPS->append_data(pBuf, nLen);
            m_bGotSPS = true;
        }
        break;
    case pps_Nal:
        {
            //Log::debug("save pps size:%d",nLen);
            CHECKPOINT_INT(m_pPPS,-1);
            m_pPPS->clear();
            m_pPPS->append_data(pBuf, nLen);
            m_bGotPPS = true;
        }
        break;
    case other:
    case unknow:
    default:
        //Log::warning("h264 nal type: %d", eType);
        break;
    }
    return true;
}

void CMP4::SetSps(uint32_t nWidth, uint32_t nHeight, double fFps) 
{
    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_nfps = fFps;
    m_tick_gap = 90000/(m_nfps>0?m_nfps:25);
    Log::debug("width = %d,height = %d, fps= %lf, tickgap= %d",m_nWidth,m_nHeight,m_nfps,m_tick_gap);
}

bool CMP4::MakeHeader()
{
    m_pHeader->clear();
    char* pPos = m_pHeader->get();

    //ftyp
    mp4_ftyp_box *ftyp = (mp4_ftyp_box*)pPos;
    m_pHeader->append_be32(sizeof(mp4_ftyp_box));       //ftyp->size
    m_pHeader->append_string("ftypisom\0");             //ftyp->type && ftyp->major_brand
    m_pHeader->append_be32(0x0200);                     //ftyp->minor_version
    m_pHeader->append_string("isomiso2avc1iso6mp41\0"); //ftyp->compatible_brands
    //ftyp end

    // moov
    mp4_moov_box *moov = (mp4_moov_box*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_moov_box));       //moov->size 需要重新填写
    m_pHeader->append_string("moov\0");                 //moov->type

    // moov=>mvhd
    mp4_mvhd_box *mvhd = (mp4_mvhd_box*)(pPos + m_pHeader->size()); // moov->mvhd
    m_pHeader->append_be32(sizeof(mp4_mvhd_box));       //mvhd->size
    m_pHeader->append_string("mvhd\0");                 //mvhd->type
    m_pHeader->append_bytes(0, 12);                     //mvhd->version mvhd->flags mvhd->creation_time mvhd->modification_time
    m_pHeader->append_be32(1000);                       //mvhd->timescale
    m_pHeader->append_be32(0);                          //mvhd->duration
    m_pHeader->append_be32(0x10000);                    //mvhd->rate
    m_pHeader->append_be16(0x100);                      //mvhd->volume
    m_pHeader->append_bytes(0, 10);                     //mvhd->reserved
    m_pHeader->append_be32(0x10000);                    //mvhd->matrix1
    m_pHeader->append_bytes(0, 12);                     //mvhd->matrix2、3、4
    m_pHeader->append_be32(0x10000);                    //mvhd->matrix5
    m_pHeader->append_bytes(0, 12);                     //mvhd->matrix6、7、8
    m_pHeader->append_be32(0x40000000);                 //mvhd->matrix9
    m_pHeader->append_bytes(0, 24);                     //mvhd->pre_defined
    m_pHeader->append_be32(0xffffffff);                 //mvhd->next_track_ID
    // moov=>mvhd end

    // moov=>trak
    mp4_trak_box* trak = (mp4_trak_box*)(pPos + m_pHeader->size()); // moov->trak
    m_pHeader->append_be32(sizeof(mp4_trak_box));       //trak->size 需要重新填写
    m_pHeader->append_string("trak\0");                 //trak->type

    // moov=>trak=>tkhd
    mp4_tkhd_box* tkhd = (mp4_tkhd_box*)(pPos + m_pHeader->size()); //moov->trak->tkhd
    m_pHeader->append_be32(sizeof(mp4_tkhd_box));       //tkhd->size
    m_pHeader->append_string("tkhd\0");                 //tkhd->type
    m_pHeader->append_bytes(0, 12);                     //tkhd->version tkhd->flags tkhd->creation_time tkhd->modification_time
    m_pHeader->append_be32(1);                          //tkhd->track_ID
    m_pHeader->append_bytes(0, 24);                     //tkhd->reserved1 tkhd->duration tkhd->reserved2 tkhd->layer tkhd->alternate_group tkhd->volume  tkhd->reserved3
    m_pHeader->append_be32(0x10000);                    //tkhd->matrix1
    m_pHeader->append_bytes(0, 12);                     //tkhd->matrix2、3、4
    m_pHeader->append_be32(0x10000);                    //tkhd->matrix5
    m_pHeader->append_bytes(0, 12);                     //tkhd->matrix6、7、8
    m_pHeader->append_be32(0x40000000);                 //tkhd->matrix9
    m_pHeader->append_be16(m_nWidth);                   //tkhd->width
	m_pHeader->append_be16(0);
    m_pHeader->append_be16(m_nHeight);                  //tkhd->height
	m_pHeader->append_be16(0);
    // moov=>trak=>tkhd end

    // moov=>trak=>mdia
    mp4_mdia_box* mdia = (mp4_mdia_box*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_mdia_box));       //mdia->size 需要重新填写
    m_pHeader->append_string("mdia\0");                 //mdia->type

    // moov=>trak=>mdia=>mdhd
    mp4_mdhd_box* mdhd = (mp4_mdhd_box*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_mdhd_box));       //mdhd->size
    m_pHeader->append_string("mdhd\0");                 //mdhd->type
    m_pHeader->append_bytes(0, 12);                     //mdhd->version mdhd->flags tkhd->creation_time mdhd->modification_time
    m_pHeader->append_be32(90000);                      //mdhd->timescale
    m_pHeader->append_be32(0);                          //mdhd->duration
    m_pHeader->append_be16(0x55c4);                     //mdhd->fixed
    m_pHeader->append_be16(0);                          //mdhd->pre_defined
    // moov=>trak=>mdia=>mdhd end

    // moov=>trak=>mdia=>hdlr
    mp4_hdlr_box* hdlr = (mp4_hdlr_box*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_hdlr_box));       //hdlr->size
    m_pHeader->append_string("hdlr\0");                 //hdlr->type
    m_pHeader->append_bytes(0, 8);                      //hdlr->version hdlr->flags tkhd->pre_defined
    m_pHeader->append_string("vide\0");                 //hdlr->handler_type
    m_pHeader->append_bytes(0, 12);                     //hdlr->reserved
    m_pHeader->append_string("videoHandler\0");         //hdlr->name
    m_pHeader->append_byte(0);                          //hdlr->name
    // moov=>trak=>mdia=>hdlr end

    // moov=>trak=>mdia=>minf
    mp4_minf_box* minf = (mp4_minf_box*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_minf_box));       //minf->size 需要重新填写
    m_pHeader->append_string("minf\0");                 //minf->type

    // moov=>trak=>mdia=>minf=>vmhd
    mp4_vmhd_box* vmhd = (mp4_vmhd_box*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_vmhd_box));       //vmhd->size
    m_pHeader->append_string("vmhd\0");                 //vmhd->type
    m_pHeader->append_bytes(0, 12);                     //vmhd->version vmhd->flags vmhd->graphicsmode vmhd->opcolor
    // moov=>trak=>mdia=>minf=>vmhd end

    // moov=>trak=>mdia=>minf=>dinf
    mp4_dinf_box* dinf = (mp4_dinf_box*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_dinf_box));       //dinf->size
    m_pHeader->append_string("dinf\0");                 //dinf->type

    // moov=>trak=>mdia=>minf=>dinf=>dref
    mp4_dref_box* dref = (mp4_dref_box*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_dref_box));       //dref->size
    m_pHeader->append_string("dref\0");                 //dref->type
    m_pHeader->append_be32(0);                          //dref->version  dref->flags
    m_pHeader->append_be32(1);                          //dref->entry_count
    m_pHeader->append_be32(12);                         //dref->entry_size
    m_pHeader->append_string("url \0");                 //dref->url
    m_pHeader->append_be32(1);                          //dref->entry_flags
    // moov=>trak=>mdia=>minf=>dinf=>dref end
    // moov=>trak=>mdia=>minf=>dinf end

    // moov=>trak=>mdia=>minf=>stbl
    mp4_stbl_box* stbl = (mp4_stbl_box*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_stbl_box));       //stbl->size 需要重新填写
    m_pHeader->append_string("stbl\0");                 //stbl->type

    // moov=>trak=>mdia=>minf=>stbl=>stsd
    mp4_stbl_box_inner* stsd = (mp4_stbl_box_inner*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_stbl_box));       //stsd->size 需要重新填写
    m_pHeader->append_string("stsd\0");                 //stsd->type
    m_pHeader->append_be32(0);                          //stsd->version  stsd->flags
    m_pHeader->append_be32(1);                          //stsd->entry_count

    VisualSampleEntry* vsen = (VisualSampleEntry*)(pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(VisualSampleEntry));  //vsen->size
    m_pHeader->append_string("avc1\0");                 //vsen->type
    m_pHeader->append_bytes(0, 6);                      //vsen->reserved
    m_pHeader->append_be16(1);                          //vsen->data_reference_index
    m_pHeader->append_bytes(0, 16);                     //vsen->pre_defined1 vsen->reserved1 vsen->pre_defined2
    m_pHeader->append_be16(m_nWidth);                   //vsen->width
    m_pHeader->append_be16(m_nHeight);                  //vsen->height
    m_pHeader->append_be32(m_nHorizresolution);         //vsen->horizresolution
    m_pHeader->append_be32(m_mVertresolution);          //vsen->vertresolution
    m_pHeader->append_be32(0);                          //vsen->reserved2
    m_pHeader->append_be16(1);                          //vsen->frame_count
    m_pHeader->append_bytes(0, 32);                     //vsen->compressorname
    m_pHeader->append_be16(0x0018);                     //vsen->depth
    m_pHeader->append_be16(-1);                         //vsen->pre_defined3
    m_pHeader->append_data(
        "\x00\x00\x00\x2C\x61\x76\x63\x43\x01\x42\x80\x28\xFF\xE1\x00\x14"
        "\x67\x42\x80\x28\x8B\x95\x00\xC8\x04\xBD\x08\x00\x00\x38\x40\x00"
        "\x0A\xFC\x84\x20\x01\x00\x05\x68\xDE\x38\x80\x00", 0x2c);  // vsen->padding

    m_pHeader->rewrite_be32((char*)stsd - pPos, m_pHeader->size() - ((char*)stsd - pPos));
    // moov=>trak=>mdia=>minf=>stbl=>stsd end

    // moov=>trak=>mdia=>minf=>stbl=>stts
    mp4_stbl_box_inner *stts = (mp4_stbl_box_inner*)((char*)pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_stbl_box_inner)); //stts->size
    m_pHeader->append_string("stts\0");                 //stts->type
    m_pHeader->append_be32(0);                          //stsd->version  stsd->flags
    m_pHeader->append_be32(0);                          //stts->entry_count
    // moov=>trak=>mdia=>minf=>stbl=>stts end

    // moov=>trak=>mdia=>minf=>stbl=>stsc
    mp4_stbl_box_inner *stsc = (mp4_stbl_box_inner*)((char*)pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_stbl_box_inner)); //stsc->size
    m_pHeader->append_string("stsc\0");                 //stsc->type
    m_pHeader->append_be32(0);                          //stsd->version  stsd->flags
    m_pHeader->append_be32(0);                          //stsc->entry_count
    // moov=>trak=>mdia=>minf=>stbl=>stsc end

    // moov=>trak=>mdia=>minf=>stbl=>stsz
    mp4_stbl_box_inner *stsz = (mp4_stbl_box_inner*)((char*)pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_stbl_box_inner)+4); //stsz->size
    m_pHeader->append_string("stsz\0");                 //stsz->type
    m_pHeader->append_be32(0);                          //stsd->version  stsd->flags
    m_pHeader->append_be32(0);                          //stsz->sample_size
	m_pHeader->append_be32(0);                          //stsz->sample_count
    // moov=>trak=>mdia=>minf=>stbl=>stsz end

    // moov=>trak=>mdia=>minf=>stbl=>stco
    mp4_stbl_box_inner *stco = (mp4_stbl_box_inner*)((char*)pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_stbl_box_inner)); //stco->size
    m_pHeader->append_string("stco\0");                 //stco->type
    m_pHeader->append_be32(0);                          //stsd->version  stsd->flags
    m_pHeader->append_be32(0);                          //stco->entry_count
    // moov=>trak=>mdia=>minf=>stbl=>stco end

    m_pHeader->rewrite_be32((char*)stbl - pPos, m_pHeader->size() - ((char*)stbl - pPos));
    // moov=>trak=>mdia=>minf=>stbl end
    m_pHeader->rewrite_be32((char*)minf - pPos, m_pHeader->size() - ((char*)minf - pPos));
    // moov=>trak=>mdia=>minf end
    m_pHeader->rewrite_be32((char*)mdia - pPos, m_pHeader->size() - ((char*)mdia - pPos));
    // moov=>trak=>mdia end
    m_pHeader->rewrite_be32((char*)trak - pPos, m_pHeader->size() - ((char*)trak - pPos));
    // moov=>trak end

    // moov=>mvex
    mp4_mvex_box* mvex = (mp4_mvex_box*)((char*)pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_mvex_box));       //mvex->size
    m_pHeader->append_string("mvex\0");                 //mvex->type

    // moov=>mvex=>trex
    mp4_trex_box* trex = (mp4_trex_box*)((char*)pPos + m_pHeader->size());
    m_pHeader->append_be32(sizeof(mp4_trex_box));       //trex->size
    m_pHeader->append_string("trex\0");                 //trex->type
    m_pHeader->append_be32(0);                          //trex->version trex->tag
    m_pHeader->append_be32(1);                          //trex->track_ID
    m_pHeader->append_be32(1);                          //trex->default_sample_description_index
    m_pHeader->append_be32(0);                          //trex->default_sample_duration
    m_pHeader->append_be32(0);                          //trex->default_sample_size
    m_pHeader->append_be32(0);                          //trex->default_sample_flags
    // moov=>mvex=>trex end
    // moov=>mvex end

    m_pHeader->rewrite_be32((char*)moov - pPos, m_pHeader->size() - ((char*)moov - pPos));
    // moov end

    if(m_fCB != nullptr){
        AV_BUFF buff= {MP4_HEAD, m_pHeader->get(), m_pHeader->size()};
        m_fCB(buff, m_hUser);
    }
    return true;
}

bool CMP4::MakeMP4Frag(bool bIsKeyFrame)
{
    Log::debug("make MP4 fragment");
    m_pFragment->clear();
    char* pPos = m_pFragment->get();
    uint32_t samples_size = 4*m_nSampleNum;

    // moof
    mp4_moof_box* moof = (mp4_moof_box*)pPos;
    m_pFragment->append_be32(sizeof(mp4_moof_box) + samples_size); //moof->size
    m_pFragment->append_string("moof\0");                 //moof->type

    // moof=>mfhd
    mp4_mfhd_box* mfhd = (mp4_mfhd_box*)((char*)pPos + m_pFragment->size());
    m_pFragment->append_be32(sizeof(mp4_mfhd_box));       //mfhd->size
    m_pFragment->append_string("mfhd\0");                 //mfhd->type
    m_pFragment->append_be32(0);                          //mfhd->version mfhd->flag
    m_pFragment->append_be32(m_nSeq++);                   //mfhd->sequence_number
    // moof=>mfhd end

    // moof=>traf
    mp4_traf_box* traf = (mp4_traf_box*)((char*)pPos + m_pFragment->size());
    m_pFragment->append_be32(sizeof(mp4_traf_box) + samples_size); //traf->size
    m_pFragment->append_string("traf\0");                 //traf->type

    // moof=>traf=>tfhd
    mp4_tfhd_box* tfhd = (mp4_tfhd_box*)((char*)pPos + m_pFragment->size());
    m_pFragment->append_be32(sizeof(mp4_tfhd_box));       //tfhd->size
    m_pFragment->append_string("tfhd\0");                 //tfhd->type
    m_pFragment->append_byte(0);                          //tfhd->vesion
    m_pFragment->append_be24(0x000038);                   //tfhd->flag
    m_pFragment->append_be32(1);                          //tfhd->track_ID
    m_pFragment->append_be32(0x0e10);                     //tfhd->sample_description_index
    m_pFragment->append_be32(0);                          //tfhd->default_sample_size
    m_pFragment->append_be32(0x01010000);                 //tfhd->default_sample_flags
    // moof=>traf=>tfhd end

    // moof=>traf=>tfdt
    mp4_tfdt_box* tfdt = (mp4_tfdt_box*)((char*)pPos + m_pFragment->size());
    m_pFragment->append_be32(sizeof(mp4_tfdt_box));       //tfdt->size
    m_pFragment->append_string("tfdt\0");                 //tfdt->type
    m_pFragment->append_byte(1);                          //tfdt->vesion
    m_pFragment->append_bytes(0, 3);                      //tfdt->flag
    m_pFragment->append_be64(m_timestamp);                //tfdt->baseMediaDecodeTime
    m_timestamp += m_tick_gap*m_nSampleNum;
    // moof=>traf=>tfdt end

    // moof=>traf=>trun
    mp4_trun_box* trun = (mp4_trun_box*)((char*)pPos + m_pFragment->size());
    m_pFragment->append_be32(sizeof(mp4_trun_box) + samples_size); //trun->size
    m_pFragment->append_string("trun\0");                 //trun->type
    m_pFragment->append_byte(0);                          //trun->vesion
    m_pFragment->append_be24(0x000205);                   //trun->flag
    m_pFragment->append_be32(m_nSampleNum);                //trun->sample_count
    m_pFragment->append_be32(sizeof(mp4_moof_box) + samples_size  + 8); //trun->data_offset
    m_pFragment->append_be32(0x02000000);                 //trun->first_sample_flags
    m_pFragment->append_data(m_pSamplePos->get(), m_pSamplePos->size()); //trun->sample_composition_time_offset
    // moof=>traf=>trun end
    // moof=>traf end
    // moof end

    // mdat
    mp4_mdat_box *mdat = (mp4_mdat_box*)((char*)pPos + m_pFragment->size());
    m_pFragment->append_be32(sizeof(mp4_mdat_box) + m_pMdat->size()); //mdat->size
    m_pFragment->append_string("mdat\0");                             //mdat->type
    m_pFragment->append_data(m_pMdat->get(), m_pMdat->size());        //mdat->data
    // mdat end

    if(m_fCB != nullptr){
        AV_BUFF buff ={bIsKeyFrame?MP4_FRAG_KEY:MP4_FRAG, m_pFragment->get(), m_pFragment->size()};
        m_fCB(buff, m_hUser);
    }

    m_nSampleNum = 0;
    m_pSamplePos->clear();
    m_pMdat->clear();
    return true;
}

bool CMP4::MakeVideo(char *data, uint32_t size, bool bIsKeyFrame)
{
    CHECKPOINT_BOOL(m_pMdat);

    // 延时发送模式，缓存一段数据，每次收到关键帧，将之前缓存的数据一起上抛
    if(m_nNodelay == 0 && bIsKeyFrame && m_pMdat->size() > 0) {
        MakeMP4Frag(true);
    }

    // 关键帧
    if(bIsKeyFrame) {
        // sps和pps放在关键帧的前面
        m_pMdat->append_be32(m_pSPS->size());
        m_pMdat->append_data(m_pSPS->get(), m_pSPS->size());
        m_pMdat->append_be32(m_pPPS->size());
        m_pMdat->append_data(m_pPPS->get(), m_pPPS->size());
        m_pMdat->append_be32(size);
        m_pMdat->append_data(data, size);
        m_pSamplePos->append_be32(m_pSPS->size() + m_pPPS->size() + size + 12); //12是这3个长度区域
        m_nSampleNum++;
    } else {
        m_pMdat->append_be32(size);
        m_pMdat->append_data(data, size);
        m_pSamplePos->append_be32(size+4);
        m_nSampleNum++;
    }

    //立即发送模式，每收到一帧数据就立即上抛发送
    if(m_nNodelay != 0 && m_pMdat->size() > 0) {
        MakeMP4Frag(bIsKeyFrame);
    }

    return true;
}

bool CMP4::MakeKeyVideo()
{
    if(m_pSPS->size() && m_pPPS->size() && m_pKeyFrame->size()) {
        if(!m_bMakeHeader)
        {    
            if(!MakeHeader())
                return false;

            m_bMakeHeader = true;
        }

        // 发送关键帧
        static uint64_t num = 0;
        Log::debug("send key frame %lld", num++);
        MakeVideo(m_pKeyFrame->get(),m_pKeyFrame->size(), true);

        m_pKeyFrame->clear();
        m_bGotSPS = false;
        m_bGotPPS = false;

        m_bFirstKey = true;

        return true;
    }
    return false;
}