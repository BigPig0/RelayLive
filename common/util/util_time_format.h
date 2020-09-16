/**
 * time_t���ַ�������ת����ֻ��ȷ����
 * %y �������͵�ʮ������ݣ�ֵ��0��99��
 * %Y �����Ͳ��ֵ�ʮ�����
 * %m ʮ���Ʊ�ʾ���·�
 * %d ʮ���Ʊ�ʾ��ÿ�µĵڼ���
 * %H 24Сʱ�Ƶ�Сʱ
 * %I 12Сʱ�Ƶ�Сʱ
 * %M ʮʱ�Ʊ�ʾ�ķ�����
 * %S ʮ���Ƶ�����
 */

#pragma once
#include "util_public.h"

namespace util {

class UTIL_API CTimeFormat
{
public:

    /**
     * ��һ����ʽΪyyyyMMddhhmmss��ʽ���ַ���ת��ʱ������
     */
    static time_t scanTime(const char buf[32]);

    /**
     * ��ӡ��ǰʱ��
     * @param fmt ����ĸ�ʽ
     * @param buf ������ݵ��ڴ�
     */
    static char *printNow(const char *fmt, char buf[32]);

    static string printNow(const char *fmt);

    /**
     * ��ӡָ��ʱ��
     * @param time ָ����ʱ��
     * @param fmt ����ĸ�ʽ
     * @param buf ������ݵ��ڴ�
     */
    static char *printTime(time_t time, const char *fmt, char buf[32]);

    static string printTime(time_t time, const char *fmt);

    /**
     * ��һ��ʱ��תΪ�ṹ
     */
    static struct tm getTimeInfo(time_t time);

};
};