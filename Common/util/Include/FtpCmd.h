#pragma once

////////////////////////////////////////////////////////////////////////////////
class FtpCmd
{
public:
    char *port(char *buf, int size, const char *endpoint);
    const char *pasv();

    char *user(char *buf, int size, const char *user);
    char *pass(char *buf, int size, const char *password);
    char *type(char *buf, int size, char t);

    char *cd(char *buf, int size, const char *path);
    const char *cdup();
    char *mkdir(char *buf, int size, const char *path);
    char *list(char *buf, int size, const char *path);

    char *get(char *buf, int size, const char *file);
    char *put(char *buf, int size, const char *file);

    char *rm(char *buf, int size, const char *file);
    char *rmdir(char *buf, int size, const char *dir);

    int statusCode(const char *buf);
    bool pasvAddr(char *buf, char *endpoint, int size);
};

////////////////////////////////////////////////////////////////////////////////
inline
char *FtpCmd::port(char *buf, int size, const char *endpoint)
{
    char tmp[32];
    char *p = tmp, *q = NULL;
    strcpy_s(tmp, 32, endpoint);
    while ((p = strchr(p, '.')) != NULL)
    {
        *p++ = ',';
        q = p;
    }
    p = strchr(q, ':');
    if (!p)
        return NULL;
    int port = atoi(p + 1);
    *p = '\0';
    sprintf_s(buf, size, "PORT %s,%d,%d\r\n", tmp, port / 256, port % 256);
    return buf;
}

inline
const char *FtpCmd::pasv()
{
    return "PASV \r\n";
}

inline
char *FtpCmd::user(char *buf, int size, const char *user)
{
    sprintf_s(buf, size, "USER %s\r\n", user);
    return buf;
}

inline
char *FtpCmd::pass(char *buf, int size, const char *password)
{
    sprintf_s(buf, size, "PASS %s\r\n", password);
    return buf;
}

inline
char *FtpCmd::type(char *buf, int size, char t)
{
    sprintf_s(buf, size, "TYPE %c\r\n", t);
    return buf;
}

inline
char *FtpCmd::cd(char *buf, int size, const char *path)
{
    sprintf_s(buf, size, "CWD %s\r\n", path);
    return buf;
}

inline
const char *FtpCmd::cdup()
{
    return "CDUP\r\n";
}

inline
char *FtpCmd::mkdir(char *buf, int size, const char *path)
{
    sprintf_s(buf, size, "MKD %s\r\n", path);
    return buf;
}

inline
char *FtpCmd::list(char *buf, int size, const char *path)
{
    sprintf_s(buf, size, "LIST %s\r\n", path);
    return buf;
}

inline
char *FtpCmd::get(char *buf, int size, const char *file)
{
    sprintf_s(buf, size, "RETR %s\r\n", file);
    return buf;
}

inline
char *FtpCmd::put(char *buf, int size, const char *file)
{
    sprintf_s(buf, size, "STOR %s\r\n", file);
    return buf;
}

inline
char *FtpCmd::rm(char *buf, int size, const char *file)
{
    sprintf_s(buf, size, "DELE %s\r\n", file);
    return buf;
}

inline
char *FtpCmd::rmdir(char *buf, int size, const char *dir)
{
    sprintf_s(buf, size, "RMD %s\r\n", dir);
    return buf;
}

inline
int FtpCmd::statusCode(const char *buf)
{
    return atoi(buf);
}

inline
bool FtpCmd::pasvAddr(char *buf, char *endpoint, int size)
{
    char *p = strchr(buf, '(');
    if (!p)
        return false;

    int a[6];
    sscanf_s(p + 1, "%d,%d,%d,%d,%d,%d", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]);
    sprintf_s(endpoint, size, "%d.%d.%d.%d:%d", a[0], a[1], a[2], a[3], a[4] * 256 + a[5]);
    return true;
}
