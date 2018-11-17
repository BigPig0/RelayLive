#include "stdafx.h"
#include "h264.h"

CH264::CH264(CLiveObj* pObj)
    : m_pObj(pObj)
    , m_pNaluBuff(nullptr)
    , m_nBuffLen(0)
    , m_pDataBuff(nullptr)
    , m_nDataLen(0)
    , m_eNaluType(NalType::unknow)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nFps(0)
    , m_bFirstKey(false)
    , m_bDecode(false)
{
    m_pSPS = new CNetStreamMaker();
    m_pPPS = new CNetStreamMaker();
    m_pFullBuff = new CNetStreamMaker();
}

CH264::~CH264()
{
    SAFE_DELETE(m_pSPS);
    SAFE_DELETE(m_pPPS);
    SAFE_DELETE(m_pFullBuff);
}

int CH264::InputBuffer(char *pBuf, uint32_t nLen)
{
    m_pNaluBuff = pBuf;
    m_nBuffLen = nLen;
    ParseNalu();

    switch (m_eNaluType)
    {
    case b_Nal:
        // 发送非关键帧
        if(!m_bFirstKey)
            break;
        //Log::debug("h264 frame");
        m_pFullBuff->append_data(pBuf, nLen);
        break;
    case idr_Nal:
        // 发送关键帧
        Log::debug("h264 key frame");
        if(m_pFullBuff->size() > 0) {
            m_pObj->H264Cb(m_pFullBuff->get(), m_pFullBuff->size());
            m_pFullBuff->clear();
        }
        m_pFullBuff->append_data(m_pSPS->get(), m_pSPS->size());
        m_pFullBuff->append_data(m_pPPS->get(), m_pPPS->size());
        m_pFullBuff->append_data(pBuf, nLen);
        m_bFirstKey = true;
        break;
    case sei_Nal:
        break;
    case sps_Nal:
        {
            //Log::debug("save sps size:%d",nLen);
            CHECK_POINT_INT(m_pSPS,-1);
            m_pSPS->clear();
            m_pSPS->append_data(pBuf, nLen);
            if(!m_bDecode) {
                m_bDecode = DecodeSps();
                if(m_bDecode)
                    m_pObj->H264SpsCb(m_nWidth, m_nHeight, m_nFps);
            }
        }
        break;
    case pps_Nal:
        {
            //Log::debug("save pps size:%d",nLen);
            CHECK_POINT_INT(m_pPPS,-1);
            m_pPPS->clear();
            m_pPPS->append_data(pBuf, nLen);
        }
        break;
    case other:
    case unknow:
    default:
        Log::warning("h264 nal type: %d", m_eNaluType);
        break;
    }

    return 0;
}

void CH264::ParseNalu()
{
    CHECK_POINT_VOID(m_pNaluBuff)
    m_eNaluType = NalType::unknow;

    // 00 00 00 01开头
    if (m_pNaluBuff[0] == 0 && m_pNaluBuff[1] == 0 && m_pNaluBuff[2] == 0 && m_pNaluBuff[3] == 1)
    {
        m_pDataBuff = m_pNaluBuff + 4;
        m_nDataLen  = m_nBuffLen - 4;
    }
    // 00 00 01开头
    else if (m_pNaluBuff[0] == 0 && m_pNaluBuff[1] == 0 && m_pNaluBuff[2] == 1)
    {
        m_pDataBuff = m_pNaluBuff + 3;
        m_nDataLen  = m_nBuffLen - 3;
    }
    else
    {
        return;
    }

    nal_unit_header* pNalUnit = (nal_unit_header*)m_pDataBuff;
    if ( NalType::sps_Nal != pNalUnit->nal_type
      && NalType::pps_Nal != pNalUnit->nal_type
      && NalType::sei_Nal != pNalUnit->nal_type
      && NalType::idr_Nal != pNalUnit->nal_type
      && NalType::b_Nal   != pNalUnit->nal_type)
    {
        m_eNaluType = NalType::other;
        return;
    }
    m_eNaluType = (NalType)pNalUnit->nal_type;
}

bool CH264::DecodeSps()
{
    if(m_eNaluType != sps_Nal) return false;
    CHECK_POINT(m_pDataBuff);
    m_nFps    = 0;
    m_nWidth  = 0;
    m_nHeight = 0;

    // 拷贝一份临时数据
    uint32_t nLen = m_nDataLen;
    uchar* buf = (uchar*)malloc(nLen);
    CHECK_POINT(buf)
    memcpy_s(buf, nLen, m_pDataBuff, m_nDataLen);

    // 将数据中的003转成00(组包时001和0001转成了0031和00301)
    de_emulation_prevention(buf,&nLen);

    uint32_t StartBit = 1*8; //跳过nal头一个字节，在ParseNalu已经解析过了
    int profile_idc=u(8,buf,StartBit);
    int constraint_set0_flag=u(1,buf,StartBit);//(buf[1] & 0x80)>>7;
    int constraint_set1_flag=u(1,buf,StartBit);//(buf[1] & 0x40)>>6;
    int constraint_set2_flag=u(1,buf,StartBit);//(buf[1] & 0x20)>>5;
    int constraint_set3_flag=u(1,buf,StartBit);//(buf[1] & 0x10)>>4;
    int reserved_zero_4bits=u(4,buf,StartBit);
    int level_idc=u(8,buf,StartBit);

    int seq_parameter_set_id=Ue(buf,nLen,StartBit);
    //Log::debug("h264_decode_sps seq_parameter_set_id:%d",seq_parameter_set_id);
    if( profile_idc == 100 || profile_idc == 110 ||
        profile_idc == 122 || profile_idc == 144 )
    {
        int chroma_format_idc=Ue(buf,nLen,StartBit);
        if( chroma_format_idc == 3 )
            int residual_colour_transform_flag=u(1,buf,StartBit);
        int bit_depth_luma_minus8=Ue(buf,nLen,StartBit);
        int bit_depth_chroma_minus8=Ue(buf,nLen,StartBit);
        int qpprime_y_zero_transform_bypass_flag=u(1,buf,StartBit);
        int seq_scaling_matrix_present_flag=u(1,buf,StartBit);
        if( seq_scaling_matrix_present_flag )
        {
            int seq_scaling_list_present_flag[8];
            int* ScalingList4x4[6];
            for (int i=0; i<6; i++)
            {
                ScalingList4x4[i] = new int[16];
                memset(ScalingList4x4[i],0,sizeof(int)*16);
            }
            int UseDefaultScalingMatrix4x4Flag[6];
            int* ScalingList8x8[2];
            for (int i=0; i<2; i++)
            {
                ScalingList8x8[i] = new int[64];
                memset(ScalingList8x8[i],0,sizeof(int)*64);
            }
            int UseDefaultScalingMatrix8x8Flag[2];

            for( int i = 0; i < 8; i++ ) 
            {
                seq_scaling_list_present_flag[i]=u(1,buf,StartBit);
                if( seq_scaling_list_present_flag[ i ] )
                {
                    if( i < 6 )
                    {
                        int lastScale = 8;
                        int nextScale = 8;
                        for(int j = 0; j < 16; j++ )
                        {
                            if( nextScale != 0 )
                            {
                                int delta_scale = Se(buf,nLen,StartBit);
                                nextScale = ( lastScale + delta_scale + 256 ) % 256;
                                UseDefaultScalingMatrix4x4Flag[ i ] = ( j == 0 && nextScale == 0 );
                            }
                            ScalingList4x4[i][j] = ( nextScale == 0 ) ? lastScale : nextScale;
                            lastScale = ScalingList4x4[i][j];
                        }
                    }
                    else
                    {
                        int lastScale = 8;
                        int nextScale = 8;
                        for(int j = 0; j < 64; j++ )
                        {
                            if( nextScale != 0 )
                            {
                                int delta_scale = Se(buf,nLen,StartBit);
                                nextScale = ( lastScale + delta_scale + 256 ) % 256;
                                UseDefaultScalingMatrix8x8Flag[i-6] = ( j == 0 && nextScale == 0 );
                            }
                            ScalingList8x8[i-6][j] = ( nextScale == 0 ) ? lastScale : nextScale;
                            lastScale = ScalingList8x8[i-6][j];
                        }
                    }
                }
            }

            for (int i=0; i<6; i++)
            {
                SAFE_DELETE_ARRAY(ScalingList4x4[i]);
            }
            for (int i=0; i<2; i++)
            {
                SAFE_DELETE_ARRAY(ScalingList8x8[i]);
            }
        }
    }
    int log2_max_frame_num_minus4=Ue(buf,nLen,StartBit);
    int pic_order_cnt_type=Ue(buf,nLen,StartBit);
    //Log::debug("h264_decode_sps pic_order_cnt_type:%d",pic_order_cnt_type);
    if( pic_order_cnt_type == 0 )
        int log2_max_pic_order_cnt_lsb_minus4=Ue(buf,nLen,StartBit);
    else if( pic_order_cnt_type == 1 )
    {
        int delta_pic_order_always_zero_flag=u(1,buf,StartBit);
        int offset_for_non_ref_pic=Se(buf,nLen,StartBit);
        int offset_for_top_to_bottom_field=Se(buf,nLen,StartBit);
        int num_ref_frames_in_pic_order_cnt_cycle=Ue(buf,nLen,StartBit);

        int *offset_for_ref_frame=new int[num_ref_frames_in_pic_order_cnt_cycle];
        for( int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
            offset_for_ref_frame[i]=Se(buf,nLen,StartBit);
        delete [] offset_for_ref_frame;
    }
    int num_ref_frames=Ue(buf,nLen,StartBit);
    int gaps_in_frame_num_value_allowed_flag=u(1,buf,StartBit);
    int pic_width_in_mbs_minus1=Ue(buf,nLen,StartBit);
    int pic_height_in_map_units_minus1=Ue(buf,nLen,StartBit);
    //Log::debug("h264_decode_sps pic_width_in_mbs_minus1:%d pic_width_in_mbs_minus1:%d",pic_width_in_mbs_minus1,pic_width_in_mbs_minus1);
    m_nWidth = (pic_width_in_mbs_minus1+1)*16;
    m_nHeight = (pic_height_in_map_units_minus1+1)*16;

    int frame_mbs_only_flag=u(1,buf,StartBit);
    if(!frame_mbs_only_flag)
        int mb_adaptive_frame_field_flag=u(1,buf,StartBit);

    int direct_8x8_inference_flag=u(1,buf,StartBit);
    int frame_cropping_flag=u(1,buf,StartBit);
    if(frame_cropping_flag)
    {
        int frame_crop_left_offset=Ue(buf,nLen,StartBit);
        int frame_crop_right_offset=Ue(buf,nLen,StartBit);
        int frame_crop_top_offset=Ue(buf,nLen,StartBit);
        int frame_crop_bottom_offset=Ue(buf,nLen,StartBit);
    }
    int vui_parameter_present_flag=u(1,buf,StartBit);
    if(vui_parameter_present_flag)
    {
        int aspect_ratio_info_present_flag=u(1,buf,StartBit);              
        if(aspect_ratio_info_present_flag)
        {
            int aspect_ratio_idc=u(8,buf,StartBit);   
            if(aspect_ratio_idc==255)
            {
                int sar_width=u(16,buf,StartBit);                                  
                int sar_height=u(16,buf,StartBit);                                      
            }
        }
        int overscan_info_present_flag=u(1,buf,StartBit); 
        if(overscan_info_present_flag)
            int overscan_appropriate_flagu=u(1,buf,StartBit);                   
        int video_signal_type_present_flag=u(1,buf,StartBit); 
        if(video_signal_type_present_flag)
        {
            int video_format=u(3,buf,StartBit);                         
            int video_full_range_flag=u(1,buf,StartBit);                       
            int colour_description_present_flag=u(1,buf,StartBit);
            if(colour_description_present_flag)
            {
                int colour_primaries=u(8,buf,StartBit);              
                int transfer_characteristics=u(8,buf,StartBit);                     
                int matrix_coefficients=u(8,buf,StartBit);                  		
            }
        }
        int chroma_loc_info_present_flag=u(1,buf,StartBit);  
        if(chroma_loc_info_present_flag)
        {
            int chroma_sample_loc_type_top_field=Ue(buf,nLen,StartBit);             
            int chroma_sample_loc_type_bottom_field=Ue(buf,nLen,StartBit);       
        }
        int timing_info_present_flag=u(1,buf,StartBit);    
        //Log::debug("h264_decode_sps timing_info_present_flag:%d",timing_info_present_flag);
        if(timing_info_present_flag)
        {
            int num_units_in_tick=u(32,buf,StartBit);   
            int time_scale=u(32,buf,StartBit);    
            //Log::debug("h264_decode_sps num_units_in_tick:%d, time_scale:%d",num_units_in_tick,time_scale);
            m_nFps = (double)time_scale/(2*num_units_in_tick);
        }
    }

    free(buf);
    return true;
}

uint32_t CH264::Ue(uchar *pBuff, uint32_t nLen, uint32_t &nStartBit)
{
    //计算0bit的个数
    UINT nZeroNum = 0;
    while (nStartBit < nLen * 8)
    {
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) //&:按位与，%取余
        {
            break;
        }
        nZeroNum++;
        nStartBit++;
    }
    nStartBit ++;


    //计算结果
    DWORD dwRet = 0;
    for (UINT i=0; i<nZeroNum; i++)
    {
        dwRet <<= 1;
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        {
            dwRet += 1;
        }
        nStartBit++;
    }
    return (1 << nZeroNum) - 1 + dwRet;
}

int CH264::Se(uchar *pBuff, uint32_t nLen, uint32_t &nStartBit)
{
    int UeVal=Ue(pBuff,nLen,nStartBit);
    double k=UeVal;
    int nValue=ceil(k/2);//ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00
    if (UeVal % 2==0)
        nValue=-nValue;
    return nValue;
}

uint32_t CH264::u(uint32_t BitCount,uchar * buf,uint32_t &nStartBit)
{
    uint32_t dwRet = 0;
    for (uint32_t i=0; i<BitCount; i++)
    {
        dwRet <<= 1;
        if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        {
            dwRet += 1;
        }
        nStartBit++;
    }
    return dwRet;
}

void CH264::de_emulation_prevention(uchar* buf,uint32_t* buf_size)
{
    int i=0,j=0;
    BYTE* tmp_ptr=NULL;
    unsigned int tmp_buf_size=0;
    int val=0;

    tmp_ptr=buf;
    tmp_buf_size=*buf_size;
    for(i=0;i<(tmp_buf_size-2);i++)
    {
        //check for 0x000003
        val=(tmp_ptr[i]^0x00) +(tmp_ptr[i+1]^0x00)+(tmp_ptr[i+2]^0x03);
        if(val==0)
        {
            //kick out 0x03
            for(j=i+2;j<tmp_buf_size-1;j++)
                tmp_ptr[j]=tmp_ptr[j+1];

            //and so we should devrease bufsize
            (*buf_size)--;
        }
    }

    return;
}