#include "util_guid.h"
#include <sstream>
#include <sys/timeb.h>
#include <time.h>
#include <cmath>

#ifdef LINUX_IMPL
#define _abs64 abs
#endif

namespace util {
namespace Guid {

/**
 * ���кų���6bit
 */
static int sequenceBits = 6;
/**
 * �������ų���6bit
 */
static int serverIdBits = 6;
/**
 * ������������6bit
 */
static int serverIdShift = sequenceBits;
/**
 * ������ʱ������12bit
 */
static int timestampShift = serverIdShift + serverIdBits;
/**
 * ��׼ʱ��, 2010-01-01
 */
static uint64_t baseTimestamp = 1262275200000UL;
/**
 * javascript��ֵ�����֧��53bit, 2^53=9007199254740992
 */
static uint64_t jsNumberMax = 9007199254740992UL;
/**
 * ������ʱ��
 */
static volatile uint64_t lastTimestamp = (uint64_t)-1UL;
/**
 * ���̺ų���6bit, ���64
 */
static int workerMax = 64;
/**
 * ���к�
 */
static volatile int sequence = 0;
/**
 * ���кų���6bit, ���64
 */
static int sequenceMax = 64;

/**
 * int id, ���ڵ�ǰ������Ψһ, �������������´�1��ʼ
 */
static volatile int intId = 1;

static uint64_t currentTimeMillis(){
    struct timeb tb;
    ftime(&tb);
    return tb.time * 1000 + tb.millitm;
}
/**
 * ���̺�, ����ʱ�����ȡ
 */
static int worker = currentTimeMillis() % workerMax;

static uint64_t getNextMillis(uint64_t lastTimestamp) {
    // ����һ����
    uint64_t timestamp = currentTimeMillis();
    while (timestamp <= lastTimestamp) {
        timestamp = currentTimeMillis();
    }
    return timestamp;
}

uint64_t getId(uint8_t svrid) {
    uint64_t timestamp = currentTimeMillis();
    if (lastTimestamp == timestamp) {
        sequence = (sequence + 1) % sequenceMax;
        // ���кŵ���64, �ȴ���һ����
        if (sequence == 0) {
            timestamp = getNextMillis(lastTimestamp);
        }
    } else {
        // ÿ�������к�����
        sequence = 0;
    }
    lastTimestamp = timestamp;
    int64_t result = ((timestamp - baseTimestamp) << timestampShift) | ((svrid>0?svrid:worker) << serverIdShift) | sequence;
    //ȡ����ֵ������
    return _abs64(result) % jsNumberMax;
}

uint64_t getIntId(){
    return intId++;
}

std::string uuid(uint8_t svrid)
{
    union
    {
        uint64_t Data;
        struct
        {
            uint32_t Data1;
            uint16_t Data2;
            uint16_t Data3;
        } p1;
        struct
        {
            uint8_t Data4[8];
        }p2;
    } guid;
    guid.Data = getId(svrid);

    char buf[33]={0};
    sprintf(buf, "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x" /*"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"*/
        , guid.p1.Data1, guid.p1.Data2, guid.p1.Data3
        , guid.p2.Data4[0], guid.p2.Data4[1], guid.p2.Data4[2], guid.p2.Data4[3]
    , guid.p2.Data4[4], guid.p2.Data4[5], guid.p2.Data4[6], guid.p2.Data4[7]);
    return buf;
}

#if 0
static std::string getIdInfo(uint64_t id) {
    uint64_t timestamp = ((0x1FFFFFFFFFF000L & id) >> timestampShift) + baseTimestamp;
    int srv = (0xE00 & id) >> serverIdShift;
    int seq = 0x1FF & id;

    struct tm tm;
    time_t timeSecend = timestamp/1000;
#if defined(WINDOWS_IMPL)
    localtime_s(&tm, &timeSecend);
#else
    localtime_r(&timeSecend, &tm);
#endif
    int timeMilli = timestamp%1000;

    char stamp[32];
    strftime(stamp, 32, "%Y-%m-%d %H:%M:%S", &tm);

    std::stringstream ss;
    ss << "id=" << id
        << ", timestamp=" << stamp << "." << timeMilli
        << ", server=" << srv
        << ", sequence=" <<seq;
    return ss.str();
}
#endif


//////////////////////////////////////////////////////////////////////////

#if defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64) || defined(WINDOWS) || defined(_WINDOWS)

#include <windows.h>
#pragma comment(lib, "Ole32.lib")

/**
 * ͨ��ϵͳapi��ȡguid����֤ȫ��Ψһ
 */
std::string guid() {
    char buf[33] = {0};
    GUID _guid;
    if (CoCreateGuid(&_guid) != S_OK)
        return guid();

    sprintf_s(buf, 33, "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x"
        /*"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"*/
        , _guid.Data1, _guid.Data2, _guid.Data3
        , _guid.Data4[0], _guid.Data4[1], _guid.Data4[2], _guid.Data4[3]
    , _guid.Data4[4], _guid.Data4[5], _guid.Data4[6], _guid.Data4[7]);
    return buf;
}
#elif defined(linux) || defined(_UNIX)
#include <uuid/uuid.h>
string guid() {
    char buf[33] = {0};
    uuid_t uu;
    uuid_generate(uu);
    sprintf(buf, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        uu[0],uu[1],uu[2],uu[3],uu[4],uu[5],uu[6],uu[7],uu[8],
        uu[9],uu[10],uu[11],uu[12],uu[13],uu[14],uu[15]);
    return buf;
}
#else
/**
 * ͨ�����������guid�ַ���
 * ͬһ�������б�֤Ψһ�������������ʱ�䲻һ����֤Ψһ��
 * �����������ʱ��һ����������ͬ
 */
string guid(){
    static bool seed = true;
    if(seed){
        srand(time(NULL));
        seed = false;
    }
    static char buf[64] = {0};
    sprintf(buf , "%08X%04X%04X%04X%04X%04X%04X", 
        rand()&0xffffffff,
        rand()&0xffff, 
        rand()&0xffff, 
        rand()&0xffff, 
        rand()&0xffff, 
        rand()&0xffff,
        rand()&0xffff
        );
    return buf;
}

#endif
}
}