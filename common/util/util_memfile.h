/**
 * �ڴ滺��,���Զ�д.
 * �������ڶ�д�ֱ��ж�����ָ��, read() �� write() ����ִ��ʱ,����Ҫʹ�� seek() ���ƶ�.
 * by ������ - Que's C++ Studio
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
     * ���ڲ������ڴ�Ĺ��캯��
     */
    memfile(size_t memInc = 1024, size_t maxSize = SIZE_T_MAX);

    /**
     * ʹ���ⲿ������ڴ�Ĺ��캯��
     */
    memfile(const void* buf, size_t len);

    ~memfile();

    /**
     * ���ڴ��ļ�д��һ������
     * @param buf[in] ��Ҫд�������
     * @param len[in] ���ݳ���
     * @return д����ֽ���
     */
    size_t write(const void *buf, size_t len);

    /**
     * ���ڴ��ļ�д��һ���ַ���������������0
     * @param buf[in] ��Ҫд����ַ�������0��β
     * @return д����ֽ���,����������0
     */
    size_t puts(const char* buf);

    /**
     * ���ڴ��ļ�д��һ���ַ�
     * @param ch[in] ��Ҫд����ַ�
     * @return д����ֽ���
     */
    size_t putc(const char ch);

    /**
     * ��дָ����ת��ָ��λ��
     * @param offset[in] ƫ��ֵ
     * @param origin[in] ��ʼλ�� SEEK_CUR:�ӵ�ǰλ�����ƫ�� SEEK_END:��ĩβ��ǰƫ��
     * @return 0 ��ʾ�ɹ�
     */
    size_t seekp(long offset, int origin);

    /**
     * ����дָ���λ��
     * @param дָ���λ��
     */
    size_t tellp() const;

    /**
     * ��ȡָ�����ȵ�����
     * @param buf[in] ��Ŷ������ݵĻ���
     * @param size[in] ��ȡ��С
     * @return ��ȡ�ֽ���
     */
    size_t read(void *buf, size_t size);

    /**
     * ��ȡһ���ַ�
     * @return ��ȡ���ַ�
     */
    char getc();

    /**
     * ��ȡָ���������ݣ��������н���
     * @param buf[in] ��Ŷ������ݵĻ���
     * @param size[in] ��ȡ��С
     * @return ��ȡ�ֽ���,�������з�
     */
    size_t gets(char *buf, size_t size);

    /**
     * ����ָ����ת��ָ��λ��
     * @param offset[in] ƫ��ֵ
     * @param origin[in] ��ʼλ�� SEEK_CUR:�ӵ�ǰλ�����ƫ�� SEEK_END:��ĩβ��ǰƫ��
     * @return 0 ��ʾ�ɹ�
     */
    size_t seekg(long offset, int origin);

    /**
     * ���ض�ָ���λ��
     * @param ��ָ���λ��
     */
    size_t tellg() const;

    /**
     * ��ȡ�ڴ��ļ���ָ��
     */
    void* buffer();

    /**
     * ��ȡ�ڴ��ļ��Ļ�������С
     */
    inline size_t bufferSize()  const { return _bufLen; }

    /**
     * ��ȡ�ڴ��ļ����ļ���С
     */
    inline size_t fsize() const { return _fileSize; }

    /**
     * ����ļ�
     * @param freeBuf[in] �Ƿ�ͬʱ�ͷŻ�������true:�ͷţ������ڲ�����Ļ���������Ч
     */
    void trunc(bool freeBuf = true);

    /**
     * �Ƿ�����ļ�β
     * @return true�Ѷ����ļ�β
     */
    bool eof() const;

    /**
     * �������û��������С
     * @param r[in] �µĻ���������Ҫ�Ĵ�С����ԭ���������
     * @param buf[out] �µĻ�������ָ��
     * @param len[out] �µĻ��������ʵ�ʴ�С
     */
    bool reserve(size_t r, void **buf, size_t *len);

private:
	char*   _buffer;      //< ��������ָ��
	size_t  _bufLen;      //< ���������С

	size_t  _readPos;     //< ��ָ��λ��
	size_t  _writePos;    //< дָ��λ��
	size_t  _fileSize;    //< �ļ���С

	size_t  _maxSize;     //< ����������С
	size_t  _memInc;      //< ÿ�������ڴ�Ĵ�С
	bool _selfAlloc;      //< �������Ƿ����ڲ�����

    /**
     * ����ʣ��ռ��С
     */
	size_t space();

    /**
     * �������û��������С
     * @param r[in] �µĻ���������Ҫ�Ĵ�С
     */
	size_t reserve(size_t s);

    /**
     * ���ÿ������캯������ֹ����
     */
	memfile(const memfile &other);
	memfile& operator = (const memfile &other);
};
};