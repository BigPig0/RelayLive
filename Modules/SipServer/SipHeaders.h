#pragma once

//SIP From/To头部
class CSipFromToHeader
{
public:
    CSipFromToHeader()
        : _hasValue(false)
    {
    }
    ~CSipFromToHeader()
    {
    }
    void SetHeader(const char* szCode, const char* szIP, const char* szPort)
    {
        addrCode = nullptr!=szCode?szCode:"";
        addrIp   = nullptr!=szIP?szIP:"";
        addrPort = nullptr!=szPort?szPort:"";
        _hasValue = true;
    }
    string GetFormatHeader()
    {
        std::stringstream stream;
        stream << "<sip:" << addrCode << "@" << addrIp;
        if(!addrPort.empty())
            stream << ":" << addrPort;
        stream << ">";
        return stream.str();
    }
    string GetAddrCode()
    {
        return addrCode;
    }
    //主机名称
    string GetRealName()
    {
        std::stringstream stream;
        stream << addrIp;
        return stream.str();
    }
    string GetPort()
    {
        return addrPort;
    }
    bool hasValue(){return _hasValue;}
private:
    string addrCode;
    string addrIp;
    string addrPort;
    bool   _hasValue;
};

//SIP Contract头部
class CContractHeader: public CSipFromToHeader
{
public:
    CContractHeader()
    {
    }
    ~CContractHeader()
    {
    }
    void SetContractHeader(const char* szCode, const char* szIP, const char* szPort,int expire)
    {
        SetHeader(szCode, szIP, szPort);
        expires = expire;
    }
    string GetContractFormatHeader(bool bExpires)
    {
        if (!bExpires)
        {
            return GetFormatHeader();
        }
        else
        {
            string sTmp = GetFormatHeader();
            std::stringstream stream;
            stream << ";" << "expires=" << expires;
            sTmp += stream.str();
            return sTmp;
        }

    }
private:
    int expires;
};

class CSubjectHeader
{
public:
    CSubjectHeader(){};
    ~CSubjectHeader(){};
    void SetHeader(const char* szSendCode, const char* szReciveCode, bool bHistory)
    {
        senderCode = nullptr!=szSendCode?szSendCode:"";
        reciverCode = nullptr!=szReciveCode?szReciveCode:"";
        isHistory = bHistory;
    }
    string GetFormatHeader()
    {
        static unsigned int sid=0,rid=0;
        std::stringstream stream;
        stream << senderCode << ":";
        if(isHistory)
            stream << "1";
        else
            stream << "0";
        stream << sid++ << "," << reciverCode << ":" << rid++;

        return stream.str();
    }
private:
    string senderCode;
    string senderMediaSN;
    string reciverCode;
    string reciverMeadiaSN;
    bool   isHistory;
};