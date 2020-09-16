#pragma once
#include "util_public.h"
#include <stdint.h>
#include <string>

namespace util {
namespace Guid {

/**
 * 可以保证全局唯一, 包含: 时间+服务器号+序列号, 多台服务器唯一需设置不同的服务器号
 * 参考twitter snowflake, 生成53bit有序自增id, 41bit timestamp + 6bit serverId + 6bit sequence, javascript数值型最大支持53bit
 * 41bit timestamp 服务器时间, 精确到毫秒
 * 6bit serverid 服务器号, 最多64台, 多台服务器不能重复
 * 6bit sequence 序列号, 1台服务器上1毫秒最多64个id, 1秒64000个id
 * svrid服务号，实际是进程号，一台服务器开启多个进程，需要不同编号
 */
uint64_t getId(uint8_t svrid = 0);

/**
 * 仅在当前进程中唯一, 服务重启后重新从1开始
 */
uint64_t getIntId();

/**
 * getId的值转成16进制字符串
 * 每个进程每秒钟不能超过64个
 */
std::string uuid(uint8_t svrid = 0);

/**
 * 生成guid字符串
 */
std::string guid();

};
};
