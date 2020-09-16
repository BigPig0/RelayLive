#include "common.h"
#include "h264.h"


static uint32_t Ue(uchar *pBuff, uint32_t nLen, uint32_t &nStartBit)
{
    //����0bit�ĸ���
    UINT nZeroNum = 0;
    while (nStartBit < nLen * 8)
    {
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) //&:��λ�룬%ȡ��
        {
            break;
        }
        nZeroNum++;
        nStartBit++;
    }
    nStartBit ++;


    //������
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

static int Se(uchar *pBuff, uint32_t nLen, uint32_t &nStartBit)
{
    int UeVal=Ue(pBuff,nLen,nStartBit);
    double k=UeVal;
    int nValue=ceil(k/2);//ceil������ceil��������������С�ڸ���ʵ������С������ceil(2)=ceil(1.2)=cei(1.5)=2.00
    if (UeVal % 2==0)
        nValue=-nValue;
    return nValue;
}

/**
 * �����ֽ����д�nStartBitλ��ʼ��BitCountλ��ֵ
 * @param buf �����ֽ�����ʼλ��
 * @param nStartBit ������ʼλ������������λ��
 * @param BitCount ����λ��
 * @return ָ��λ����ֵ
 */
static uint32_t u(uint32_t BitCount,uchar *buf,uint32_t &nStartBit)
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

/** �������nalu�����е�003ת��00(���ʱ001��0001ת����0031��00301) */
static void de_emulation_prevention(uchar* buf,uint32_t* buf_size)
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

bool h264_sps_info(char *buff, uint32_t len, uint32_t *width, uint32_t *height, double *fps) {
    char *nalu;
    uint32_t nalue_len;
    h264_nalu_data2(buff, len, &nalu, &nalue_len);

    NalType tpye = h264_naltype(nalu);
    if(tpye != sps_Nal) 
        return false;

    *fps    = 0;
    *width  = 0;
    *height = 0;

    // ����һ����ʱ����
    uint32_t nLen = nalue_len;
    uchar* buf = (uchar*)malloc(nLen);
    CHECKPOINT_BOOL(buf);
    memcpy_s(buf, nLen, nalu, nalue_len);

    // �������е�003ת��00(���ʱ001��0001ת����0031��00301)
    de_emulation_prevention(buf,&nLen);

    uint32_t StartBit = 1*8; //����nalͷһ���ֽڣ���ParseNalu�Ѿ���������
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
    *width = (pic_width_in_mbs_minus1+1)*16;
    *height = (pic_height_in_map_units_minus1+1)*16;

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
            *fps = (double)time_scale/(2*num_units_in_tick);
        }
    }

    free(buf);
    return true;
}

CH264::CH264(AV_CALLBACK cb, void* handle)
    : m_bFirstKey(false)
    , m_hUser(handle)
    , m_fCB(cb)
{
    m_pSPS = new CNetStreamMaker();
    m_pPPS = new CNetStreamMaker();
    m_pKeyFrame = new CNetStreamMaker();
    m_pData = new CNetStreamMaker();
}

CH264::~CH264()
{
    SAFE_DELETE(m_pSPS);
    SAFE_DELETE(m_pPPS);
    SAFE_DELETE(m_pKeyFrame);
    SAFE_DELETE(m_pData);
}

int CH264::Code(AV_BUFF buff)
{
    char *pBuf = buff.pData;
    uint32_t nLen = buff.nLen;

    char* pDataBuff;    //< ȥ����001��0001�������
    h264_nalu_data(pBuf, &pDataBuff);
    NalType eType = h264_naltype(pDataBuff);

    switch (eType)
    {
    case b_Nal:
        // ���ͷǹؼ�֡
        if(!m_bFirstKey)
            break;
        //������ڹؼ�֡û�д�����Ҫ�ȷ��͹ؼ�֡��pps�����ڹؼ�֡������յ���
        //sps��pps��ʧ������ʹ����һ�εġ�
        MakeKeyVideo(); 
        //Log::debug("send frame");
        MakeVideo(pBuf,nLen,0);
        break;
    case idr_Nal:
        // ���͹ؼ�֡
        m_pKeyFrame->clear();
        m_pKeyFrame->append_data(pBuf, nLen);
        //һ��sps��pps���ڹؼ�֡ǰ�棬����ʱ���ڹؼ�֡���档
        if(m_bGotPPS && m_bGotSPS)
            MakeKeyVideo();
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
        //Log::warning("h264 nal type: %d", m_eNaluType);
        break;
    }

    return 0;
}

bool CH264::MakeVideo(char *data,int size,int bIsKeyFrame)
{
    CHECKPOINT_BOOL(m_pData);

    // ��ʱ����ģʽ������һ�����ݣ�ÿ���յ��ؼ�֡����֮ǰ���������һ������
    if(m_nNodelay == 0) {
        if(bIsKeyFrame && m_pData->size() > 0){
            if(m_fCB != nullptr){
                AV_BUFF buff = {H264_IDR, m_pData->get(), m_pData->size()};
                m_fCB(buff, m_hUser);
            }
            m_pData->clear();
        }
        if (bIsKeyFrame) {
            m_pData->append_data(m_pSPS->get(), m_pSPS->size());
            m_pData->append_data(m_pPPS->get(), m_pPPS->size());
        }
        m_pData->append_data(data, size);
    } else {
        // ��������ģʽ��ÿ֡���ݶ���������
        if (bIsKeyFrame) {
            m_pData->append_data(m_pSPS->get(), m_pSPS->size());
            m_pData->append_data(m_pPPS->get(), m_pPPS->size());
            m_pData->append_data(data, size);
            if(m_fCB != nullptr){
                AV_BUFF buff = {H264_IDR, m_pData->get(), m_pData->size()};
                m_fCB(buff, m_hUser);
            }
            m_pData->clear();
        } else {
            if(m_fCB != nullptr){
                AV_BUFF buff = {H264_NDR, data, size};
                m_fCB(buff, m_hUser);
            }
        }
    }
    return true;
}

bool CH264::MakeKeyVideo()
{
    if(m_pSPS->size() && m_pPPS->size() && m_pKeyFrame->size()) {
        // ���͹ؼ�֡
        static uint64_t num = 0;
        Log::debug("send key frame %lld", num++);
        MakeVideo(m_pKeyFrame->get(),m_pKeyFrame->size(),1);

        m_pKeyFrame->clear();
        m_bGotSPS = false;
        m_bGotPPS = false;

        m_bFirstKey = true;

        return true;
    }
    return false;
}