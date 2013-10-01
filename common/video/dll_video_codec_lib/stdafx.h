#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Escludere gli elementi utilizzati di rado dalle intestazioni di Windows
#include <windows.h>

#include <sstream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

extern "C" {
// avcodec
#ifdef _MSC_VER
	#define inline __inline
	#define _MSVC 1
#endif

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#pragma warning(push,2)
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#pragma warning(pop)
}

#include <crtdbg.h>
#ifdef _DEBUG
	#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
	#define DEBUG_ALIGNED_MALLOC(s,a) _aligned_malloc_dbg(s, a, __FILE__, __LINE__)
	#define DEBUG_ALIGNED_FREE(p) _aligned_free_dbg(p)
#else
	#define DEBUG_NEW
	#define DEBUG_ALIGNED_MALLOC(s,a)
	#define DEBUG_ALIGNED_FREE(p)
#endif // _DEBUG

#ifdef _DEBUG
#define _VERIFY(expr) _ASSERT(expr)
#else
#define _VERIFY(expr) (expr)
#endif
