#pragma once
#include "util_public.h"
#include <stdint.h>
#include <string>

namespace util {
namespace Guid {

/**
 * ���Ա�֤ȫ��Ψһ, ����: ʱ��+��������+���к�, ��̨������Ψһ�����ò�ͬ�ķ�������
 * �ο�twitter snowflake, ����53bit��������id, 41bit timestamp + 6bit serverId + 6bit sequence, javascript��ֵ�����֧��53bit
 * 41bit timestamp ������ʱ��, ��ȷ������
 * 6bit serverid ��������, ���64̨, ��̨�����������ظ�
 * 6bit sequence ���к�, 1̨��������1�������64��id, 1��64000��id
 * svrid����ţ�ʵ���ǽ��̺ţ�һ̨����������������̣���Ҫ��ͬ���
 */
uint64_t getId(uint8_t svrid = 0);

/**
 * ���ڵ�ǰ������Ψһ, �������������´�1��ʼ
 */
uint64_t getIntId();

/**
 * getId��ֵת��16�����ַ���
 * ÿ������ÿ���Ӳ��ܳ���64��
 */
std::string uuid(uint8_t svrid = 0);

/**
 * ����guid�ַ���
 */
std::string guid();

};
};
