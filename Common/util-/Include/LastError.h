#pragma once

class LastError
{
public:
    explicit LastError() : m_errCode(0) {}
    virtual ~LastError() {}

    int errorCode()
    {
        return m_errCode;
    }

protected:
    int m_errCode;
};

#define GOTO(label)         { m_errCode = GetLastError(); goto label;       }
#define XGOTO(x, label)     { m_errCode = (x); goto label;                  }
#define IO_FAILED(x)        (!(x) && (m_errCode = GetLastError()) != ERROR_IO_PENDING)