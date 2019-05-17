// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>   
#include <iostream>
#include <string>
#include <sstream>


// TODO: 在此处引用程序需要的其他头文件  
#include "common.h"
#include "Endian.h"

using namespace std;



enum STREAM_TYPE {
	STREAM_UNKNOW = 0,
	STREAM_PS,
	STREAM_H264
};
extern STREAM_TYPE g_stream_type;