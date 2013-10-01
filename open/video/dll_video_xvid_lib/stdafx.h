/*****************************************************************************
 * Precompiled headers
 *****************************************************************************
 * Copyright (C) 2010-2011 Yuri Valentini
 *
 * Authors: Yuri Valentini <yv@opycom.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *****************************************************************************/

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <vector>

#include <stdlib.h>
#include <stdio.h>

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
