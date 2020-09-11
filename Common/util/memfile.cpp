#include "memfile.h"
#include <string.h>

memfile::memfile(size_t memInc, size_t maxSize)
	: _buffer(NULL)
    , _bufLen(0)
    , _readPos(0)
    , _writePos(0)
    , _fileSize(0)
    , _maxSize(maxSize)
    , _memInc(memInc)
    , _selfAlloc(true)
{
	if( _memInc > _maxSize ) 
        _memInc = _maxSize;
}

memfile::memfile(const void* buf, size_t len)
	: _buffer((char*)buf)
    , _bufLen(len)
    , _readPos(0)
    , _writePos(0)
    , _fileSize(len)
    , _maxSize(len)
    , _memInc(0)
	, _selfAlloc(false)
{
}

memfile::~memfile()
{
	trunc(true);
}

void memfile::trunc(bool freeBuf /* = true */)
{
	if( freeBuf && _selfAlloc )
	{
		if(_buffer != NULL) delete[]_buffer;
		_buffer = NULL;
		_bufLen = 0;
	}

	_readPos = 0;
	_writePos = 0;
	_fileSize = 0;
}

size_t memfile::putc(const char ch)
{
	return write(&ch, 1);
}

size_t memfile::puts(const char *buf)
{
	return write(buf, strlen(buf));
}

size_t memfile::space()
{
	return _bufLen - _writePos;
}

size_t memfile::reserve(size_t s)
{
	// 已经拥有足够的空间
	if( space() >= s ) return s;

    // 外部内存不能重新申请
	if( !_selfAlloc ) return space();

	// 空间不足,分配新的空间

	// 1. 一次最少分配 _memInc 大小的内存.
	size_t incSize = s - space();
	if( incSize < _memInc ) incSize = _memInc;

	// 2. 最多只能有 _maxSize 大小的内存.
	size_t newSize = _bufLen + incSize;
	if( newSize > _maxSize ) newSize = _maxSize;

	if( newSize <= _bufLen )
	{
	}
	else
	{
		char *tmp = new char[newSize];
		if(tmp)
		{
			memset(tmp, 0, newSize);
			if(_buffer)
			{
				memcpy(tmp, _buffer, _bufLen);
				delete []_buffer;
			}
			_buffer = tmp;
			_bufLen = newSize;

			return space() >= s ? s : space();
		}
	}
	return space();
}

bool memfile::reserve(size_t r, void **buf, size_t *len)
{
	size_t sz = reserve(r);
	if(len) *len = sz;
	if(buf) *buf = reinterpret_cast<void*>(_buffer + _writePos);
	return sz > 0;
}

size_t memfile::write(const void *buf, size_t len)
{
	// 计算最多可以写入多少个字节
	size_t writeSize = reserve(len);

	// 写入数据
	if(writeSize > 0)
	{
		if(buf)
		{
			memcpy( _buffer + _writePos, buf, writeSize );
		}
		_writePos += writeSize;

		// 最大到达过的位置为文件大小.
		if(_writePos > _fileSize)
		{
			_fileSize = _writePos;
		}
	}

	return writeSize;
}

size_t memfile::tellp() const
{
	return _writePos;
}

char memfile::getc()
{
	char ch = 0;
	read(&ch, 1);
	return ch;
}

size_t memfile::gets(char *buf, size_t size)
{
	size_t readed = 0;
	char ch = 0;
	while(readed < size)
	{
		ch = getc();
		buf[readed++] = ch;
		if( '\n' == ch ) break;
	}
	return readed;
}

size_t memfile::read(void *buf, size_t size)
{
	size_t realRead = _fileSize - _readPos;
	if( realRead > size ) realRead = size;
	if(buf == NULL || realRead <= 0) return 0;

	memcpy(buf, _buffer + _readPos, realRead);
	_readPos += realRead;
	return realRead;
}

size_t memfile::seekg(long offset, int origin)
{
	if( SEEK_CUR == origin )
	{
		_readPos += offset;
	}
	else if( SEEK_END == origin )
	{
		_readPos = _fileSize - offset;
	}
	else // SEEK_SET
	{
		_readPos = offset;
	}

	if( _readPos > _fileSize)
	{
		_readPos = _fileSize;
	}
	return _readPos;
}

size_t memfile::seekp(long offset, int origin)
{
	if( SEEK_CUR == origin )
	{
		_writePos += offset;
	}
	else if( SEEK_END == origin )
	{
		_writePos = _fileSize - offset;
	}
	else // SEEK_SET
	{
		_writePos = offset;
	}

	if( _writePos > _fileSize )
	{
		_writePos = _fileSize;
	}

	if( _writePos > _bufLen )
	{
		_writePos = _bufLen;
		_fileSize = _bufLen;
	}
	return _writePos;
}

size_t memfile::tellg() const
{
	return _readPos;
}

void* memfile::buffer()
{
	return _buffer;
}

bool memfile::eof() const
{
	return _readPos >= _fileSize;
}