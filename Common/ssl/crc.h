#pragma once

#include "ssl_export.h"
#include <stdint.h>

class _SSL_API CRC
{
public:

static uint32_t  calc_crc32 (unsigned char *data, uint32_t datalen);

static uint32_t Zwg_ntohl(uint32_t s);

};