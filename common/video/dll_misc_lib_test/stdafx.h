#pragma once

#include "targetver.h"
#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"

#include <crtdbg.h>
#ifdef _DEBUG
	#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
#else
	#define DEBUG_NEW
#endif // _DEBUG
