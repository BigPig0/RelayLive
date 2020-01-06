/* Copyright (C) 2011 阙荣文
 *
 * 这是一个开源免费软件,您可以自由的修改和发布.
 * 禁止用作商业用途.
 *
 * 联系原作者: querw@sina.com 
*/

#pragma once
#include "util_public.h"

/*
* 由于系统对于使用C标准库函数 fopen() 同时打开的文件数有限制(一般是512, 可用 _setmaxstdio() 增大到2048)
* 对于 HTTP 服务器来说是远远不够的,所以 class WINFile 直接调用 Windows File API 来实现和 C标准库函数同样的功能.
* WINFile 同时打开的文件数可以达到 16384,并且可以通过 Windows 的启动参数 --max-open-files=N 来修改.
*
*/

class UTIL_API WINFile
{
private:
	HANDLE _h;

public:
	WINFile();
	~WINFile();

	/*
	* 打开模式
	*/
	static const int r = 0x01;
	static const int w = 0x02;
	static const int rw = 0x04;

	static bool exist(const TCHAR *fileName);
	static bool remove(const TCHAR *fileName);

	bool open(const TCHAR *fileName, unsigned int mode, bool tmp = false);
	bool close();
	bool trunc();
	bool isopen();
	bool eof();

	unsigned long read(void *buf, unsigned long len);
	unsigned long write(const void *buf, unsigned long len);
	__int64 seek(__int64 off, DWORD mode);
	__int64 tell();
	__int64 size();
};

