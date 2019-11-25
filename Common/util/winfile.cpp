#include <windows.h>
#include "winfile.h"

WINFile::WINFile()
	: _h(INVALID_HANDLE_VALUE)
{
}


WINFile::~WINFile()
{
	close();
}

bool WINFile::exist(const TCHAR *fileName)
{
	return INVALID_FILE_ATTRIBUTES != GetFileAttributes(fileName);
	// && ERROR_FILE_NOT_FOUND == GetLastError())
}

bool WINFile::remove(const TCHAR *fileName)
{
	return DeleteFile(fileName) == TRUE;
}

bool WINFile::open(const TCHAR *fileName, unsigned int mode, bool tmp)
{
	if(INVALID_HANDLE_VALUE != _h) return false;
	DWORD dwCreateFlag = 0;
	DWORD dwAccess = 0;
	DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	if(mode == r)
	{
		/* 只读 */
		dwCreateFlag = OPEN_EXISTING;
		dwAccess = GENERIC_READ;
	}
	else if(mode == w)
	{
		/* 只写 */
		dwCreateFlag = CREATE_ALWAYS;
		dwAccess = GENERIC_WRITE;
	}
	else
	{
		/* 读写 */
		dwCreateFlag = OPEN_ALWAYS;
		dwAccess = GENERIC_READ | GENERIC_WRITE;
	}

	if(tmp)
	{
		/* 临时文件,自动删除 */
		dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE;
	}

	_h = CreateFile(fileName, dwAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, dwCreateFlag, dwFlagsAndAttributes, NULL);
	return _h != INVALID_HANDLE_VALUE;
}

bool WINFile::close()
{
	if(INVALID_HANDLE_VALUE == _h) return false;

	BOOL ret = CloseHandle(_h);
	if( ret )
	{
		_h = INVALID_HANDLE_VALUE;
	}
	else
	{
		//assert(0);
	}

	return ret == TRUE;
}

bool WINFile::trunc()
{
	if(INVALID_HANDLE_VALUE == _h) return false;

	seek(0, FILE_BEGIN);
	return SetEndOfFile(_h) == TRUE;
}

bool WINFile::isopen() 
{ 
    return _h != INVALID_HANDLE_VALUE; 
}

unsigned long WINFile::read(void *buf, unsigned long len)
{
	if(INVALID_HANDLE_VALUE == _h) return 0;

	unsigned long bytesRd = 0;
	if(!ReadFile(_h, buf, len, &bytesRd, NULL))
	{
		//assert(0);
	}

	return bytesRd;
}

unsigned long WINFile::write(const void *buf, unsigned long len)
{
	if(INVALID_HANDLE_VALUE == _h) return 0;

	unsigned long bytesWr = 0;
	if(!WriteFile(_h, buf, len, &bytesWr, NULL))
	{
		//assert(0);
	}

	return bytesWr;
}

__int64 WINFile::tell()
{
	if(INVALID_HANDLE_VALUE == _h) return -1;

	LARGE_INTEGER pos;
	LARGE_INTEGER dis;
	dis.QuadPart = 0;
	pos.QuadPart = 0;
	if(!SetFilePointerEx(_h, dis, &pos, FILE_CURRENT))
	{
		//assert(0);
	}

	return pos.QuadPart;
}

__int64 WINFile::seek(__int64 off, DWORD mode)
{
	if(INVALID_HANDLE_VALUE == _h) return -1;

	LARGE_INTEGER pos;
	LARGE_INTEGER dis;
	dis.QuadPart = off;
	pos.QuadPart = 0;
	if(!SetFilePointerEx(_h, dis, &pos, mode))
	{
		//assert(0);
	}

	return pos.QuadPart;
}

__int64 WINFile::size()
{
	if(INVALID_HANDLE_VALUE == _h) return -1;

	LARGE_INTEGER sz;
	sz.QuadPart = 0;

	if(!GetFileSizeEx(_h, &sz))
	{
		//assert(0);
	}

	return sz.QuadPart;
}

bool WINFile::eof()
{
	if(INVALID_HANDLE_VALUE == _h) return true;

	return tell() >= size();
}