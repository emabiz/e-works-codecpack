/*****************************************************************************
 * Dll entry point
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

#include "stdafx.h"
#include "dllmain.h"
#include "param_xvid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::string *g_enc_params_default = NULL;

BOOL APIENTRY DllMain( HMODULE /* hModule */,
                       DWORD  ul_reason_for_call,
                       LPVOID /* lpReserved */
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			ew::ParamArchiveXvidEnc ar;
			g_enc_params_default = new std::string(ar.GetParamsDefault());
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		delete g_enc_params_default;
	}
	return TRUE;
}

