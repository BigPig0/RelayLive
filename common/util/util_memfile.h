/**
 * 内存缓冲,可以读写.
 * 区别在于读写分别有独立的指针, read() 和 write() 交叉执行时,不需要使用 seek() 来移动.
 * by 阙荣文 - Que's C++ Studio
 */
#pragma once
#include "util_public.h"

namespace util {

#ifndef SIZE_T_MAX
#define SIZE_T_MAX 0xffffffff 
#endif // !SIZE_T_MAX

class UTIL_API memfile
{
public:
    /**
     * 由内部申请内存的构造函数
     */
    memfile(size_t memInc = 1024, size_t maxSize = SIZE_T_MAX);

    /**
     * 使用外部申请的内存的构造函数
     */
    memfile(const void* buf, size_t len);

    ~memfile();

    /**
     * 向内存文件写入一段数据
     * @param buf[in] 需要写入的数据
     * @param len[in] 数据长度
     * @return 写入的字节数
     */
    size_t write(const void *buf, size_t len);

    /**
     * 向内存文件写入一段字符串，不含结束符0
     * @param buf[in] 需要写入的字符串，以0结尾
     * @return 写入的字节数,不含结束符0
     */
    size_t puts(const char* buf);

    /**
     * 向内存文件写入一个字符
     * @param ch[in] 需要写入的字符
     * @return 写入的字节数
     */
    size_t putc(const char ch);

    /**
     * 将写指针跳转到指定位置
     * @param offset[in] 偏移值
     * @param origin[in] 起始位置 SEEK_CUR:从当前位置向后偏移 SEEK_END:从末尾向前偏移
     * @return 0 表示成功
     */
    size_t seekp(long offset, int origin);

    /**
     * 返回写指针的位置
     * @param 写指针的位置
     */
    size_t tellp() const;

    /**
     * 读取指定长度的数据
     * @param buf[in] 存放读出数据的缓存
     * @param size[in] 读取大小
     * @return 读取字节数
     */
    size_t read(void *buf, size_t size);

    /**
     * 读取一个字符
     * @return 读取的字符
     */
    char getc();

    /**
     * 读取指定长度数据，遇到换行结束
     * @param buf[in] 存放读出数据的缓存
     * @param size[in] 读取大小
     * @return 读取字节数,包含换行符
     */
    size_t gets(char *buf, size_t size);

    /**
     * 将读指针跳转到指定位置
     * @param offset[in] 偏移值
     * @param origin[in] 起始位置 SEEK_CUR:从当前位置向后偏移 SEEK_END:从末尾向前偏移
     * @return 0 表示成功
     */
    size_t seekg(long offset, int origin);

    /**
     * 返回读指针的位置
     * @param 读指针的位置
     */
    size_t tellg() const;

    /**
     * 获取内存文件的指针
     */
    void* buffer();

    /**
     * 获取内存文件的缓冲区大小
     */
    inline size_t bufferSize()  const { return _bufLen; }

    /**
     * 获取内存文件的文件大小
     */
    inline size_t fsize() const { return _fileSize; }

    /**
     * 清空文件
     * @param freeBuf[in] 是否同时释放缓存区域。true:释放，仅对内部分配的缓存区域有效
     */
    void trunc(bool freeBuf = true);

    /**
     * 是否读到文件尾
     * @return true已读到文件尾
     */
    bool eof() const;

    /**
     * 重新设置缓存区域大小
     * @param r[in] 新的缓存区域需要的大小，比原来大才有用
     * @param buf[out] 新的缓存区域指针
     * @param len[out] 新的缓存区域的实际大小
     */
    bool reserve(size_t r, void **buf, size_t *len);

private:
	char*   _buffer;      //< 缓存区域指针
	size_t  _bufLen;      //< 缓存区域大小

	size_t  _readPos;     //< 读指针位置
	size_t  _writePos;    //< 写指针位置
	size_t  _fileSize;    //< 文件大小

	size_t  _maxSize;     //< 缓存区最大大小
	size_t  _memInc;      //< 每次申请内存的大小
	bool _selfAlloc;      //< 缓存区是否是内部申请

    /**
     * 返回剩余空间大小
     */
	size_t space();

    /**
     * 重新设置缓存区域大小
     * @param r[in] 新的缓存区域需要的大小
     */
	size_t reserve(size_t s);

    /**
     * 禁用拷贝构造函数，禁止拷贝
     */
	memfile(const memfile &other);
	memfile& operator = (const memfile &other);
};
};