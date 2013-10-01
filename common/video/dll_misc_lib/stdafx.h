#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <string>
#include <sstream>
#include <vector>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <crtdbg.h>
#ifdef _DEBUG
	#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
#else
	#define DEBUG_NEW
#endif // _DEBUG

#ifdef _DEBUG
#define _VERIFY(expr) _ASSERT(expr)
#else
#define _VERIFY(expr) (expr)
#endif
