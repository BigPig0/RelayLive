#ifndef _UTIL_NETSTREAM_H_
#define _UTIL_NETSTREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "utilc_export.h"
#include "utilc_api.h"
#include "utilc_def.h"
#include <stdint.h>

typedef struct _net_stream_maker_ net_stream_maker_t;
typedef struct _net_stream_parser_ net_stream_parser_t;

net_stream_maker_t* create_net_stream_maker();
void destory_net_stream_maker(net_stream_maker_t* h);
char* get_net_stream_data(net_stream_maker_t* h);
uint32_t get_net_stream_len(net_stream_maker_t* h);
void clear_net_stream(net_stream_maker_t* h);
bool net_stream_append_data(net_stream_maker_t* h, char* data, uint32_t size );
void net_stream_append_byte(net_stream_maker_t* h, uint8_t b);
void net_stream_append_string(net_stream_maker_t* h, const char *str);
void net_stream_append_be16(net_stream_maker_t* h, uint16_t val);
void net_stream_append_be24(net_stream_maker_t* h, uint32_t val);
void net_stream_append_be32(net_stream_maker_t* h, uint32_t val);
void net_stream_append_be64(net_stream_maker_t* h, uint64_t val);
void net_stream_append_bytes(net_stream_maker_t* h, uint8_t val, uint32_t num);
void net_stream_append_double(net_stream_maker_t* h, double val);
void rewrite_byte(net_stream_maker_t* h, uint32_t start, uint8_t  val);
void rewrite_be16(net_stream_maker_t* h, uint32_t start, uint16_t val);
void rewrite_be24(net_stream_maker_t* h, uint32_t start, uint32_t val);
void rewrite_be32(net_stream_maker_t* h, uint32_t start, uint32_t val);
void rewrite_be64(net_stream_maker_t* h, uint32_t start, uint64_t val);
void rewrite_double(net_stream_maker_t* h, uint32_t start, double val);

//////////////////////////////////////////////////////////////////////////

net_stream_parser_t* create_net_stream_parser(char* buff, uint32_t len);
void destory_net_stream_parser(net_stream_parser_t* h);
uint8_t net_stream_read_byte(net_stream_parser_t* h, uint8_t bitCount);
uint16_t net_stream_read_be16(net_stream_parser_t* h, uint8_t bitCount);
uint32_t net_stream_read_be32(net_stream_parser_t* h, uint8_t bitCount);
uint64_t net_stream_read_be64(net_stream_parser_t* h, uint8_t bitCount);
char* net_stream_read_buff(net_stream_parser_t* h, uint32_t len);
void net_stream_skip(net_stream_parser_t* h, uint32_t bitCount);

#ifdef __cplusplus
}
#endif
#endif