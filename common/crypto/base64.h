#pragma once

#include "crypto_export.h"
#include <string>
#include <stdint.h>

class _SSL_API Base64
{
public:
    /*
     * 编码
     * @param Data[in] 输入原始数据
     * @param DataByte[in] 输入的数据长度,以字节为单位
     * @return 编码后的数据
     */
    static std::string Encode(const uint8_t* Data, uint32_t DataByte);

    /*
     * 解码
     * @param Data[in] 输入原始数据
     * @param DataByte[in] 输入的数据长度,以字节为单位
     * @param OutByte[out] 输出的数据长度,以字节为单位,请不要通过返回值计算
     * @return 解码后的数据  
     */
    static std::string Decode(const char* Data, uint32_t DataByte, uint32_t& OutByte);
};


