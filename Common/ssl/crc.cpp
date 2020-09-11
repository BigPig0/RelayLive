#include "crc.h"

#define BSWAP16C(x) (((x) << 8 & 0xff00)  | ((x) >> 8 & 0x00ff))
#define BSWAP32C(x) (BSWAP16C(x) << 16 | BSWAP16C((x) >> 16))
#define BSWAP64C(x) (BSWAP32C(x) << 32 | BSWAP32C((x) >> 32))

static uint32_t crc_tab[256] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
    0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
    0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
    0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
    0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
    0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
    0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
    0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
    0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
    0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
    0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
    0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
    0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
    0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
    0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
    0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
    0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
    0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
    0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
    0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
    0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
    0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

uint32_t  CRC::calc_crc32 (uint8_t *data, uint32_t datalen)
{
    uint32_t i;
    uint32_t crc = 0xffffffff;

    for (i=0; i<datalen; i++) 
    {
        crc = (crc << 8) ^ crc_tab[((crc >> 24) ^ *data++) & 0xff];
    }
    return crc;
}

uint32_t CRC::Zwg_ntohl(uint32_t s)
{
    union 
    {
        int  i;
        char buf;
    }a;
    a.i = 0x01;
    if(a.buf)
    {
        // 小端
        s = BSWAP32C(s);
    }
    return s;
}

//////////////////////////////////////////////////////////////////////////

/** 
 * Name:    InvertUint8 
 * Note: 	把字节颠倒过来，如0x12变成0x48
			0x12: 0001 0010
			0x48: 0100 1000
 */
static void InvertUint8(uint8_t *dBuf, uint8_t *srcBuf) {
	uint8_t tmp[4]={0};
	for(int i=0;i< 8;i++) {
		if(srcBuf[0]& (1 << i))
		tmp[0]|=1<<(7-i);
	}
	dBuf[0] = tmp[0];
}

static void InvertUint16(uint16_t *dBuf, uint16_t *srcBuf) {
	uint16_t tmp[4]={0};
	for(int i=0;i< 16;i++) {
		if(srcBuf[0]& (1 << i))
		tmp[0]|=1<<(15 - i);
	}
	dBuf[0] = tmp[0];
}

static void InvertUint32(uint32_t *dBuf, uint32_t *srcBuf) {
	uint32_t tmp[4]={0};
	for(int i=0;i< 32;i++) {
		if(srcBuf[0]& (1 << i))
		tmp[0]|=1<<(31 - i);
	}
	dBuf[0] = tmp[0];
}

/** 
 * Name:    CRC-16/CCITT        x16+x12+x5+1 
 * Width:	16
 * Poly:    0x1021 
 * Init:    0x0000 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x0000 
 * Alias:   CRC-CCITT,CRC-16/CCITT-TRUE,CRC-16/KERMIT 
 */
uint16_t CRC::CRC16_CCITT(uint8_t *data, uint32_t datalen) {
    uint16_t wCRCin = 0x0000;
    uint16_t wCPoly = 0x1021;
    //uint8_t wChar = 0;
    
    InvertUint16(&wCPoly,&wCPoly);
    while (datalen--) {
        wCRCin ^= *(data++);
        for(int i = 0;i < 8;i++) {
            if(wCRCin & 0x01)
                wCRCin = (wCRCin >> 1) ^ wCPoly;
            else
                wCRCin = wCRCin >> 1;
        }
    }
    return (wCRCin);
}

/** 
 * Name:    CRC-16/CCITT-FALSE   x16+x12+x5+1 
 * Width:	16 
 * Poly:    0x1021 
 * Init:    0xFFFF 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x0000 
 * Note: 
 */ 
uint16_t CRC::CRC16_CCITT_FALSE(uint8_t *data, uint32_t datalen) {
	uint16_t wCRCin = 0xFFFF;
	uint16_t wCPoly = 0x1021;
	
	while (datalen--) {
		wCRCin ^= *(data++) << 8;
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x8000)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin);
}

/** 
 * Name:    CRC-16/XMODEM       x16+x12+x5+1 
 * Width:	16 
 * Poly:    0x1021 
 * Init:    0x0000 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x0000 
 * Alias:   CRC-16/ZMODEM,CRC-16/ACORN 
 */ 
uint16_t CRC::CRC16_XMODEM(uint8_t *data, uint32_t datalen) {
	uint16_t wCRCin = 0x0000;
	uint16_t wCPoly = 0x1021;
	
	while (datalen--) {
		wCRCin ^= (*(data++) << 8);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x8000)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin);
}

/** 
 * Name:    CRC-16/X25          x16+x12+x5+1 
 * Width:	16 
 * Poly:    0x1021 
 * Init:    0xFFFF 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0XFFFF 
 * Note: 
 */
uint16_t CRC::CRC16_X25(uint8_t *data, uint32_t datalen) {
    uint16_t wCRCin = 0xFFFF;
    uint16_t wCPoly = 0x1021;

    InvertUint16(&wCPoly,&wCPoly);
    while (datalen--) {
        wCRCin ^= *(data++);
        for(int i = 0;i < 8;i++) {
            if(wCRCin & 0x01)
                wCRCin = (wCRCin >> 1) ^ wCPoly;
            else
                wCRCin = wCRCin >> 1;
        }
    }
    return (wCRCin^0xFFFF);
}

/** 
 * Name:    CRC-16/MODBUS       x16+x15+x2+1 
 * Width:	16 
 * Poly:    0x8005 
 * Init:    0xFFFF 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x0000 
 * Note: 
 */
uint16_t CRC::CRC16_MODBUS(uint8_t *data, uint32_t datalen) {
    uint16_t wCRCin = 0xFFFF;
    uint16_t wCPoly = 0x8005;

    InvertUint16(&wCPoly,&wCPoly);
    while (datalen--) {
        wCRCin ^= *(data++);
        for(int i = 0;i < 8;i++) {
            if(wCRCin & 0x01)
                wCRCin = (wCRCin >> 1) ^ wCPoly;
            else
                wCRCin = wCRCin >> 1;
        }
    }
    return (wCRCin);
}

/** 
 * Name:    CRC-16/IBM          x16+x15+x2+1 
 * Width:	16 
 * Poly:    0x8005 
 * Init:    0x0000 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x0000 
 * Alias:   CRC-16,CRC-16/ARC,CRC-16/LHA 
 */
uint16_t CRC::CRC16_IBM(uint8_t *data, uint32_t datalen) {
    uint16_t wCRCin = 0x0000;
    uint16_t wCPoly = 0x8005;

    InvertUint16(&wCPoly,&wCPoly);
    while (datalen--) {
        wCRCin ^= *(data++);
        for(int i = 0;i < 8;i++) {
            if(wCRCin & 0x01)
                wCRCin = (wCRCin >> 1) ^ wCPoly;
            else
                wCRCin = wCRCin >> 1;
        }
    }
    return (wCRCin);
}

/** 
 * Name:    CRC-16/MAXIM        x16+x15+x2+1 
 * Width:	16 
 * Poly:    0x8005 
 * Init:    0x0000 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0xFFFF 
 * Note: 
 */
uint16_t CRC::CRC16_MAXIM(uint8_t *data, uint32_t datalen) {
    uint16_t wCRCin = 0x0000;
    uint16_t wCPoly = 0x8005;

    InvertUint16(&wCPoly,&wCPoly);
    while (datalen--) {
        wCRCin ^= *(data++);
        for(int i = 0;i < 8;i++) {
            if(wCRCin & 0x01)
                wCRCin = (wCRCin >> 1) ^ wCPoly;
            else
                wCRCin = wCRCin >> 1;
        }
    }
    return (wCRCin^0xFFFF);
}

/** 
 * Name:    CRC-16/USB          x16+x15+x2+1 
 * Width:	16 
 * Poly:    0x8005 
 * Init:    0xFFFF 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0xFFFF 
 * Note: 
 */
uint16_t CRC::CRC16_USB(uint8_t *data, uint32_t datalen) {
    uint16_t wCRCin = 0xFFFF;
    uint16_t wCPoly = 0x8005;

    InvertUint16(&wCPoly,&wCPoly);
    while (datalen--) {
        wCRCin ^= *(data++);
        for(int i = 0;i < 8;i++) {
            if(wCRCin & 0x01)
                wCRCin = (wCRCin >> 1) ^ wCPoly;
            else
                wCRCin = wCRCin >> 1;
        }
    }
    return (wCRCin^0xFFFF);
}

/** 
 * Name:    CRC-16/DNP          x16+x13+x12+x11+x10+x8+x6+x5+x2+1 
 * Width:	16 
 * Poly:    0x3D65 
 * Init:    0x0000 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0xFFFF 
 * Use:     M-Bus,ect. 
 */
uint16_t CRC16_DNP(uint8_t *data, uint32_t datalen) {
    uint16_t wCRCin = 0x0000;
    uint16_t wCPoly = 0x3D65;

    InvertUint16(&wCPoly,&wCPoly);
    while (datalen--) {
        wCRCin ^= *(data++);
        for(int i = 0;i < 8;i++) {
            if(wCRCin & 0x01)
                wCRCin = (wCRCin >> 1) ^ wCPoly;
            else
                wCRCin = (wCRCin >> 1);
        }
    }
    return (wCRCin^0xFFFF);
}

/** 
 * Name:    CRC-32  x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 
 * Width:	32 
 * Poly:    0x4C11DB7 
 * Init:    0xFFFFFFF 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0xFFFFFFF 
 * Alias:   CRC_32/ADCCP 
 * Use:     WinRAR,ect. 
 */
uint32_t CRC::CRC32_ADCCP(uint8_t *data, uint32_t datalen) {

    uint32_t wCRCin = 0xFFFFFFFF;
    uint32_t wCPoly = 0x04C11DB7;

    InvertUint32(&wCPoly,&wCPoly);
    while (datalen--) {
        wCRCin ^= *(data++);
        for(int i = 0;i < 8;i++) {
            if(wCRCin & 0x01)
                wCRCin = (wCRCin >> 1) ^ wCPoly;
            else
                wCRCin = wCRCin >> 1;
        }
    }
    return (wCRCin ^ 0xFFFFFFFF) ;
}

/** 
 * Name:    CRC-32/MPEG-2  x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 
 * Width:	32 
 * Poly:    0x4C11DB7 
 * Init:    0xFFFFFFF 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x0000000 
 * Note: 
 */
uint32_t CRC::CRC32_MPEG(uint8_t *data, uint32_t datalen) {
    uint32_t wCRCin = 0xFFFFFFFF;
    uint32_t wCPoly = 0x04C11DB7;
    uint32_t wChar = 0;
    while (datalen--) {
        wChar = *(data++);
        wCRCin ^= (wChar << 24);
        for(int i = 0;i < 8;i++) {
            if(wCRCin & 0x80000000)
                wCRCin = (wCRCin << 1) ^ wCPoly;
            else
                wCRCin = wCRCin << 1;
        }
    }
    return (wCRCin) ;
}


/** 
 * Name:    CRC-4/ITU	x4+x+1 
 * Width:	4
 * Poly:    0x03 
 * Init:    0x00 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x00 
 * Note: 
 */
uint8_t CRC::CRC4_ITU(uint8_t *data, uint32_t datalen) {
	uint8_t wCRCin = 0x00;
	uint8_t wCPoly = 0x03;
	uint8_t wChar = 0;
	
	while (datalen--) {
		wChar = *(data++);
		InvertUint8(&wChar,&wChar);
		wCRCin ^= (wChar);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x80)
				wCRCin = (wCRCin << 1) ^ (wCPoly << 4);
			else
				wCRCin = wCRCin << 1;
		}
	}
	InvertUint8(&wCRCin,&wCRCin);
	return (wCRCin);
}

/** 
 * Name:    CRC-5/EPC	x5+x3+1 
 * Width:	5
 * Poly:    0x09 
 * Init:    0x09 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x00 
 * Note: 
 */
uint8_t CRC::CRC5_EPC(uint8_t *data, uint32_t datalen) {
	uint8_t wCRCin = 0x09<<3;
	uint8_t wCPoly = 0x09<<3;
	
	while (datalen--) {
		wCRCin ^= *(data++);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x80)
				wCRCin = (wCRCin << 1) ^ (wCPoly);
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin >> 3);
}

/** 
 * Name:    CRC-5/USB	x5+x2+1 
 * Width:	5
 * Poly:    0x05 
 * Init:    0x1F 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x1F 
 * Note: 
 */
uint8_t CRC::CRC5_USB(uint8_t *data, uint32_t datalen)  
{  
	uint8_t wCRCin = 0x1F;
	uint8_t wCPoly = 0x05;
	
	InvertUint8(&wCPoly,&wCPoly);
	while (datalen--) {
		wCRCin ^= *(data++);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ (wCPoly >> 3);
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin^0x1F); 
}

/** 
 * Name:    CRC-5/ITU	x5+x4+x2+1  
 * Width:	5
 * Poly:    0x15 
 * Init:    0x00 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x00 
 * Note: 
 */
uint8_t CRC::CRC5_ITU(uint8_t *data, uint32_t datalen) {
	uint8_t wCRCin = 0x00;
	uint8_t wCPoly = 0x15;
	
	InvertUint8(&wCPoly,&wCPoly);
	while (datalen--) {
		wCRCin ^= *(data++);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ (wCPoly >> 3);
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin); 
} 

/** 
 * Name:    CRC-6/ITU	x6+x+1 
 * Width:	6
 * Poly:    0x03 
 * Init:    0x00 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x00 
 * Note: 
 */
uint8_t CRC::CRC6_ITU(uint8_t *data, uint32_t datalen) {
	uint8_t wCRCin = 0x00;
	uint8_t wCPoly = 0x03;
	uint8_t wChar = 0;
	
	while (datalen--) {
		wChar = *(data++);
		InvertUint8(&wChar,&wChar);
		wCRCin ^= (wChar);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x80)
				wCRCin = (wCRCin << 1) ^ (wCPoly << 2);
			else
				wCRCin = wCRCin << 1;
		}
	}
	InvertUint8(&wCRCin,&wCRCin);
	return (wCRCin);
}

/** 
 * Name:    CRC-7/MMC           x7+x3+1  
 * Width:	7
 * Poly:    0x09 
 * Init:    0x00 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x00 
 * Use:     MultiMediaCard,SD,ect. 
 */
uint8_t CRC::CRC7_MMC(uint8_t *data, uint32_t datalen) {
	uint8_t wCRCin = 0x00;
	uint8_t wCPoly = 0x09;
	
	while (datalen--) {
		wCRCin ^= *(data++);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x80)
				wCRCin = (wCRCin << 1) ^ (wCPoly<<1);
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin>>1);
}

/** 
 * Name:    CRC-8               x8+x2+x+1 
 * Width:	8 
 * Poly:    0x07 
 * Init:    0x00 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x00 
 * Note: 
 */
uint8_t CRC::CRC8(uint8_t *data, uint32_t datalen) {
	uint8_t wCRCin = 0x00;
	uint8_t wCPoly = 0x07;
	
	while (datalen--) {
		wCRCin ^= *(data++);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x80)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin);
}

/** 
 * Name:    CRC-8/ITU           x8+x2+x+1 
 * Width:	8 
 * Poly:    0x07 
 * Init:    0x00 
 * Refin:   False 
 * Refout:  False 
 * Xorout:  0x55 
 * Alias:   CRC-8/ATM 
 */
uint8_t CRC::CRC8_ITU(uint8_t *data, uint32_t datalen) {
	uint8_t wCRCin = 0x00;
	uint8_t wCPoly = 0x07;
	
	while (datalen--) {
		wCRCin ^= *(data++);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x80)
				wCRCin = (wCRCin << 1) ^ wCPoly;
			else
				wCRCin = wCRCin << 1;
		}
	}
	return (wCRCin^0x55);
}

/** 
 * Name:    CRC-8/ROHC          x8+x2+x+1 
 * Width:	8 
 * Poly:    0x07 
 * Init:    0xFF 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x00 
 * Note: 
 */
uint8_t CRC::CRC8_ROHC(uint8_t *data, uint32_t datalen) {
	uint8_t wCRCin = 0xFF;
	uint8_t wCPoly = 0x07;
	
	InvertUint8(&wCPoly,&wCPoly);
	while (datalen--) {
		wCRCin ^= *(data++);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin);
}

/** 
 * Name:    CRC-8/MAXIM         x8+x5+x4+1 
 * Width:	8 
 * Poly:    0x31 
 * Init:    0x00 
 * Refin:   True 
 * Refout:  True 
 * Xorout:  0x00 
 * Alias:   DOW-CRC,CRC-8/IBUTTON 
 * Use:     Maxim(Dallas)'s some devices,e.g. DS18B20 
 */ 
uint8_t CRC::CRC8_MAXIM(uint8_t *data, uint32_t datalen) {
	uint8_t wCRCin = 0x00;
	uint8_t wCPoly = 0x31;
	
	InvertUint8(&wCPoly,&wCPoly);
	while (datalen--) {
		wCRCin ^= *(data++);
		for(int i = 0;i < 8;i++) {
			if(wCRCin & 0x01)
				wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
				wCRCin = wCRCin >> 1;
		}
	}
	return (wCRCin);
}