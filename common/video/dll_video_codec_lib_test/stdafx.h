#pragma once

#include "targetver.h"
#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <inttypes.h>

#include <crtdbg.h>
#ifdef _DEBUG
	#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
#else
	#define DEBUG_NEW
#endif // _DEBUG
