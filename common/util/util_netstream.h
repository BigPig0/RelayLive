#pragma once
#include "util_public.h"
#include <stdint.h>

namespace util {
/**
 * 本类的作用是生成一个网络数据流，可以向里面添加数据
 * 封装了添加整数和字符串到数据流的接口
 */
class UTIL_API CNetStreamMaker
{
public:
    CNetStreamMaker();
    virtual ~CNetStreamMaker();

    char* get(){return m_pData;}
    uint32_t size(){return m_nCurrent;}
    void clear(){m_nCurrent = 0;}

    /**
     * 向数据流中添加内容
     * @param data[in] 需要添加的数据
     * @param size[in] 数据大小
     * @return 成功true，失败false
     */
    bool append_data(char* data, uint32_t size );

    /**
     * 向数据流中追加一个字符串(以0结尾)
     */
    void append_string(const char *str );

    /**
     * 向数据流中追加数据
     */
    void append_byte(uint8_t  val);
    void append_be16(uint16_t val);
    void append_be24(uint32_t val);
    void append_be32(uint32_t val);
    void append_be64(uint64_t val);
    void append_bytes(uint8_t val, uint32_t num);
    void append_double(double val);

    /**
     * 修改数据流中的数据
     * @param start[in] 需要修改的数据的起始位置
     * @param val[in] 新的值
     */
    void rewrite_data(uint32_t start, char* data, uint32_t size);
    void rewrite_byte(uint32_t start, uint8_t  val);
    void rewrite_be16(uint32_t start, uint16_t val);
    void rewrite_be24(uint32_t start, uint32_t val);
    void rewrite_be32(uint32_t start, uint32_t val);
    void rewrite_be64(uint32_t start, uint64_t val);
    void rewrite_double(uint32_t start, double val);

private:
    uint64_t dbl2int( double value );

private:
    char*    m_pData;      //内存地址
    uint32_t m_nCurrent;   //当前写入数据的位置
    uint32_t m_nMax;       //内存大小
};

class UTIL_API CNetStreamParser
{
public:
    CNetStreamParser(char* buff, uint32_t len);
    ~CNetStreamParser();

    uint8_t read_byte(uint8_t bitCount);
    uint16_t read_be16(uint8_t bitCount);
    uint32_t read_be32(uint8_t bitCount);
    uint64_t read_be64(uint8_t bitCount);
    char* read_buff(uint32_t len);
    void skip(uint32_t bitCount);

private:
    char*    pData;      //内存地址
    uint32_t nCurrent;   //当前读取数据的位置(位)
    uint32_t nLen;       //数据长度
};
};