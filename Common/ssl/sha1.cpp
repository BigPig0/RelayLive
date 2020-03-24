#include "sha1.h"
#include <memory.h>

#define SHA1_SIZE_BYTE 20

class SHA1_Context
{
public:
    void Init();

    void Update(const uint8_t* in, uint32_t len);

    void Finalize();

    uint8_t* Result(uint8_t* out);

private:
    void Transform();
    void Clear();
    int GetMsgBits();
    void PadMessage();

    unsigned long h[SHA1_SIZE_BYTE / 4]; /* 存放摘要结果(32*5=160 bits)*/
    unsigned long Nl;
    unsigned long Nh;       /*存放信息总位数，Nh：高32位，Nl：低32位*/
    unsigned long data[16]; /*数据从第0个的高8位开始依次放置*/
    int FlagInWord;     /*标识一个data元素中占用的字节数（从高->低），取值0,1,2,3*/
    int msgIndex;       /*当前已填充满的data数组元素数。*/
    int isTooMang;      /*正常为0，当处理的信息超过2^64 bits时为1；*/
    uint8_t md[SHA1_SIZE_BYTE + 1];        // 计算结果
};

#define INIT_DATA_h0 0x67452301U
#define INIT_DATA_h1 0xEFCDAB89U
#define INIT_DATA_h2 0x98BADCFEU
#define INIT_DATA_h3 0x10325476U
#define INIT_DATA_h4 0xC3D2E1F0U

#define SHA1CircularShift(bits, word) (((word) << (bits)) | ((word) >> (32 - (bits))))

const unsigned long SHA1_Kt[] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};

/*定义四个函数*/
static unsigned long SHA1_ft0_19(unsigned long b, unsigned long c, unsigned long d)
{
    return (b&c) | ((~b)&d);
}

static unsigned long SHA1_ft20_39(unsigned long b, unsigned long c, unsigned long d)
{
    return b ^ c ^ d;
}

static unsigned long SHA1_ft40_59(unsigned long b, unsigned long c, unsigned long d)
{
    return (b&c) | (b&d) | (c&d);
}

static unsigned long SHA1_ft60_79(unsigned long b, unsigned long c, unsigned long d)
{
    return b ^ c ^ d;
}

typedef unsigned long(*SHA1_pFun)(unsigned long, unsigned long, unsigned long);
SHA1_pFun ft[] = {SHA1_ft0_19, SHA1_ft20_39, SHA1_ft40_59, SHA1_ft60_79};

void SHA1_Context::Init()
{
    h[0] = INIT_DATA_h0;
    h[1] = INIT_DATA_h1;
    h[2] = INIT_DATA_h2;
    h[3] = INIT_DATA_h3;
    h[4] = INIT_DATA_h4;
    Nl = 0;
    Nh = 0;
    FlagInWord = 0;
    msgIndex = 0;
    isTooMang = 0;
    memset(data, 0, 64);
    memset(md, 0, SHA1_SIZE_BYTE + 1);
}

int SHA1_Context::GetMsgBits()
{
    int a = 0;

    if (isTooMang)
    {
        return -1;
    }

    a = sizeof(unsigned long) * 8 * msgIndex + 8 * FlagInWord;

    return a;
}

void SHA1_Context::Clear()
{
    memset(data, 0, 64);
    msgIndex = 0;
    FlagInWord = 0;
}

void SHA1_Context::Transform()
{
    unsigned long AE[5];
    unsigned long w[80];
    unsigned long temp = 0;
    int t = 0;

    if (0 != isTooMang)
    {
        return;
    }

    for (t = 0; t < 16; ++t)
    {
        w[t] = data[t];
    }

    for (t = 16; t < 80; ++t)
    {
        w[t] = SHA1CircularShift(1, w[t-3] ^ w[t-8] ^ w[t-14] ^ w[t-16]);
    }

    for (t = 0; t < 5; ++t)
    {
        AE[t] = h[t];
    }

    for (t = 0; t <= 79; ++t)
    {
        temp = SHA1CircularShift(5, AE[0]) + (*ft[t/20])(AE[1], AE[2], AE[3]) + AE[4] + w[t] + SHA1_Kt[t/20];
        AE[4] = AE[3];
        AE[3] = AE[2];
        AE[2] = SHA1CircularShift(30, AE[1]);
        AE[1] = AE[0];
        AE[0] = temp;
    }

    for (t = 0; t < 5; ++t)
    {
        h[t] += AE[t];
    }

    Clear();
}

void SHA1_Context::PadMessage()
{
    int msgBits = -1;

    if (0 != isTooMang)
    {
        return;
    }

    msgBits = GetMsgBits();

    if (440 < msgBits)
    {
        data[msgIndex++] |= (1 << (8 * (4 - FlagInWord) - 1));
        FlagInWord = 0;

        while (msgIndex < 16)
        {
            data[msgIndex++] = 0;
        }

        Transform();

        while (msgIndex < 14)
        {
            data[msgIndex++] = 0;
        }

    }
    else
    {
        data[msgIndex++] |= (1 << (8 * (4 - FlagInWord) - 1));
        FlagInWord = 0;

        while (msgIndex < 14)
        {
            data[msgIndex++] = 0;
        }
    }

    while (msgIndex < 14)
    {
        data[msgIndex++] = 0;
    }

    data[msgIndex++] = Nh;

    data[msgIndex++] = Nl;
}

void SHA1_Context::Update(const uint8_t* in, uint32_t len)
{
    unsigned int lastBytes = 0;
    unsigned int temp      = 0;
    unsigned int i         = 0;
    unsigned int tempBits  = 0;

    if (!in || !len || isTooMang)
    {
        return;
    }

    if (FlagInWord > 0)
    {
        temp = (unsigned int)(4 - FlagInWord) < len ? (unsigned int)(4 - FlagInWord) : len;

        for (i = temp; i > 0; --i)
        {
            data[msgIndex] |= ((unsigned long)in[temp-i]) << (3 - FlagInWord++) * 8;
        }

        tempBits = Nl;

        Nl += 8 * temp;

        if (tempBits > Nl)
        {
            ++Nh;

            if (Nh == 0)
            {
                isTooMang = 1;
            }
        }

        if ((FlagInWord) / 4 > 0)
        {
            ++msgIndex;
        }

        FlagInWord = FlagInWord % 4;

        if (msgIndex == 16)
        {
            Transform();
        }
    }

    in += temp;

    len -= temp;

    if (len >= 4)
    {
        for (i = 0; i <= len - 4; i += 4)
        {
            data[msgIndex] |= ((unsigned long)in[i]) << 24;
            data[msgIndex] |= ((unsigned long)in[i+1]) << 16;
            data[msgIndex] |= ((unsigned long)in[i+2]) << 8;
            data[msgIndex] |= ((unsigned long)in[i+3]);
            ++msgIndex;

            tempBits = Nl;
            Nl += 32;

            if (tempBits > Nl)
            {
                Nh++;

                if (Nh == 0)
                {
                    isTooMang = 1;
                }
            }

            if (msgIndex == 16)
            {
                Transform();
            }
        }
    }

    if (len > 0 && len % 4 != 0)
    {
        lastBytes = len - i;

        switch (lastBytes)
        {

        case 3:
            data[msgIndex] |= ((unsigned long)in[i+2]) << 8;

        case 2:
            data[msgIndex] |= ((unsigned long)in[i+1]) << 16;

        case 1:
            data[msgIndex] |= ((unsigned long)in[i]) << 24;
        }

        FlagInWord = lastBytes;

        tempBits = Nl;
        Nl += 8 * lastBytes;

        if (tempBits > Nl)
        {
            ++Nh;

            if (0 == Nh)
            {
                isTooMang = 1;
            }
        }

        if (16 == msgIndex)
        {
            Transform();
        }
    }

}

void SHA1_Context::Finalize()
{
    int i = 0;

    if (isTooMang)
    {
        return;
    }

    PadMessage();

    Transform();

    for (i = 0; i < 5; ++i)
    {
        md[4 * i] = (unsigned char)((h[i] & 0xff000000) >> 24);
        md[4 * i + 1] = (unsigned char)((h[i] & 0x00ff0000) >> 16);
        md[4 * i + 2] = (unsigned char)((h[i] & 0x0000ff00) >> 8);
        md[4 * i + 3] = (unsigned char)(h[i] & 0x000000ff);
    }

    return;
}

uint8_t* SHA1_Context::Result(uint8_t* out)
{
    if(NULL == out)
        return md;

    memcpy(out, md, SHA1_SIZE_BYTE);
    return out;
}

SHA1::SHA1()
    : handle(new SHA1_Context())
{
    Init();
}

SHA1::~SHA1()
{
    delete (SHA1_Context*)handle;
}

void SHA1::Init(){
    ((SHA1_Context*)handle)->Init();
}

void SHA1::Update(const uint8_t* in, const uint32_t len) {
    ((SHA1_Context*)handle)->Update(in, len);
}

void SHA1::Finalize()
{
    ((SHA1_Context*)handle)->Finalize();
}


uint8_t* SHA1::Result(uint8_t* out) 
{
    return ((SHA1_Context*)handle)->Result(out);
}

uint8_t* SHA1::Comput(const uint8_t* in, const uint32_t len, uint8_t* out)
{
    Init();
    Update(in, len);
    Finalize();
    return Result(out);
}

uint8_t* SHA1::File(const uint8_t* path, uint8_t* out){
    FILE *file = fopen((const char*)path, "rb");
    if (NULL == file) {
        return NULL;
    }

    Init();
    uint32_t len = 0;
    uint8_t buffer[0x0400] = {0};
    while (len = fread(buffer, 1, 1024, file))
    {
        Update(buffer, len);
    }
    fclose(file);
    Finalize();
    return Result(out);
}