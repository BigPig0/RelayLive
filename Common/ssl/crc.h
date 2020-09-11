#pragma once

/**
 * CRC16几种模式
     模式          |        多项式          | 初始值 |      数据位序      |   结果处理
 CRC16_CCITT       | x16+x12+x5+1（0x1021） | 0x0000 | 低位在前，高位在后 | 与0x0000异或
 CRC16_CCITT_FALSE | x16+x12+x5+1（0x1021） | 0xFFFF | 低位在后，高位在前 | 与0x0000异或
 CRC16_XMODEM      | x16+x12+x5+1（0x1021） | 0x0000 | 低位在后，高位在前 | 与0x0000异或
 CRC16_X25         | x16+x12+x5+1（0x1021） | 0x0000 | 低位在后，高位在前 | 与0xFFFF异或
 CRC16_MODBUS      | x16+x15+x2+1（0x8005） | 0xFFFF | 低位在前，高位在后 | 与0x0000异或
 CRC16_IBM         | x16+x15+x2+1（0x8005） | 0x0000 | 低位在前，高位在后 | 与0x0000异或
 CRC16_MAXIM       | x16+x15+x2+1（0x8005） | 0x0000 | 低位在前，高位在后 | 与0xFFFF异或
 CRC16_USB         | x16+x15+x2+1（0x8005） | 0xFFFF | 低位在前，高位在后 | 与0xFFFF异或
 */

#include "ssl_export.h"
#include <stdint.h>

class _SSL_API CRC
{
public:

static uint16_t CRC16_CCITT(uint8_t *data, uint32_t datalen);
static uint16_t CRC16_CCITT_FALSE(uint8_t *data, uint32_t datalen);
static uint16_t CRC16_XMODEM(uint8_t *data, uint32_t datalen);
static uint16_t CRC16_X25(uint8_t *data, uint32_t datalen);
static uint16_t CRC16_MODBUS(uint8_t *data, uint32_t datalen);
static uint16_t CRC16_IBM(uint8_t *data, uint32_t datalen);
static uint16_t CRC16_MAXIM(uint8_t *data, uint32_t datalen);
static uint16_t CRC16_USB(uint8_t *data, uint32_t datalen);
static uint16_t CRC16_DNP(uint8_t *data, uint32_t datalen);

static uint32_t CRC32_ADCCP(uint8_t *data, uint32_t datalen);
static uint32_t CRC32_MPEG(uint8_t *data, uint32_t datalen);

static uint8_t CRC4_ITU(uint8_t *data, uint32_t datalen);

static uint8_t CRC5_EPC(uint8_t *data, uint32_t datalen);
static uint8_t CRC5_USB(uint8_t *data, uint32_t datalen);
static uint8_t CRC5_ITU(uint8_t *data, uint32_t datalen);

static uint8_t CRC6_ITU(uint8_t *data, uint32_t datalen);

static uint8_t CRC7_MMC(uint8_t *data, uint32_t datalen);

static uint8_t CRC8(uint8_t *data, uint32_t datalen);
static uint8_t CRC8_ITU(uint8_t *data, uint32_t datalen);
static uint8_t CRC8_ROHC(uint8_t *data, uint32_t datalen);
static uint8_t CRC8_MAXIM(uint8_t *data, uint32_t datalen);

static uint32_t  calc_crc32 (unsigned char *data, uint32_t datalen);

static uint32_t Zwg_ntohl(uint32_t s);

};