#pragma once
#ifdef EXTEND_CHANNELS
#include "avtypes.h"
#include "ring_buff.h"
#include "uv.h"
#include "Netstreammaker.h"

enum NalType;
namespace LiveClient
{
    class IDecoder
    {
    public:
        IDecoder(){};
        virtual ~IDecoder(){};

        static IDecoder* Create(AV_CALLBACK cb, void* handle=NULL);

        virtual int Decode(AV_BUFF buff) = 0;
    };

    class IEncoder
    {
    public:
        IEncoder(){};
        virtual ~IEncoder(){};
        static IEncoder* Create(AV_CALLBACK cb, void* handle=NULL);
        virtual int Code(AV_BUFF buff) = 0;
        virtual void SetDecoder(IDecoder* dec) = 0;
		
        uint32_t         m_width;     //缩放大小
        uint32_t         m_height;    //缩放大小
    };
};
#endif