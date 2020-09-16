#pragma once

#include "ssl_export.h"
#include <string>
#include <stdint.h>

class _SSL_API Base64
{
public:
    /*
     * ����
     * @param Data[in] ����ԭʼ����
     * @param DataByte[in] ��������ݳ���,���ֽ�Ϊ��λ
     * @return ����������
     */
    static std::string Encode(const uint8_t* Data, uint32_t DataByte);

    /*
     * ����
     * @param Data[in] ����ԭʼ����
     * @param DataByte[in] ��������ݳ���,���ֽ�Ϊ��λ
     * @param OutByte[out] ��������ݳ���,���ֽ�Ϊ��λ,�벻Ҫͨ������ֵ����
     * @return ����������  
     */
    static std::string Decode(const char* Data, uint32_t DataByte, uint32_t& OutByte);
};


