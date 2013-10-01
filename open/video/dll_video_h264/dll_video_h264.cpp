/*****************************************************************************
 * Dll exported functions
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
#include "dll_vc.h"
#include "video_codec_h264.h"
#include "dllmain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct CodecDescr {
	const char *name;
	int api_version;
	unsigned char payload;
	bool is_frag_supported;
	bool incomplete_supported;
};

const CodecDescr _codec_descr = {
	"h264",
	3,
	INVALID_PAYLOAD,
	true,
	true
};

DLL_VC_API const char *GetName()
{
	return _codec_descr.name;
}

DLL_VC_API int GetApiVersion()
{
	return _codec_descr.api_version;
}

DLL_VC_API unsigned char GetPayload()
{
	return _codec_descr.payload;
}

DLL_VC_API const char *GetParamsDefault()
{
	return g_enc_params_default->c_str();
}

DLL_VC_API int FragSupport()
{
	return _codec_descr.is_frag_supported;
}

DLL_VC_API int IncompleteSupport()
{
	return _codec_descr.incomplete_supported;
}

DLL_VC_API VideoHandle NewCodec(void)
{
	return reinterpret_cast<VideoHandle>(new ew::VideoCodecH264);
}

DLL_VC_API void DeleteCodec(VideoHandle handle)
{
	ew::VideoCodec *p = reinterpret_cast<ew::VideoCodec *>(handle);
	if (NULL != p)
		delete p;
}

DLL_VC_API int EncInit(VideoHandle handle,
          unsigned char **EncoderInput,
          int Bitrate, int Fps, int Width, int Height,
          const char *Params,
          int fragSize, struct FragItem **fragItem, int *fragCount)
{
	ew::VideoCodec *p = reinterpret_cast<ew::VideoCodec *>(handle);
	if (NULL == p)
		return 0;
	return p->UserEncInit(EncoderInput, Bitrate, Fps, Width, Height,
		Params, fragSize, fragItem, fragCount);
}

DLL_VC_API void EncFree(VideoHandle handle)
{
	ew::VideoCodec *p = reinterpret_cast<ew::VideoCodec *>(handle);
	if (NULL == p)
		return;
	p->DestroyEncoder();
}

DLL_VC_API int Encode(VideoHandle handle, int forceKeyFrame, int *isKeyFrame)
{
	ew::VideoCodec *p = reinterpret_cast<ew::VideoCodec *>(handle);
	if (NULL == p)
		return 0;

	return p->UserEncode(forceKeyFrame, isKeyFrame);
}

DLL_VC_API int DecInit(VideoHandle handle, unsigned char **DecoderOutput,
          int PictureW, int PictureH, int IsFragStream)
{
	ew::VideoCodec *p = reinterpret_cast<ew::VideoCodec *>(handle);
	if (NULL == p)
		return 0;

	return p->UserDecInit(DecoderOutput, PictureW, PictureH, IsFragStream);
}

DLL_VC_API void DecFree(VideoHandle handle)
{
	ew::VideoCodec *p = reinterpret_cast<ew::VideoCodec *>(handle);
	if (NULL == p)
		return;
	p->DestroyDecoder();
}

DLL_VC_API int Decode(VideoHandle handle,
          struct FragItem *fragItem, int fragCount, int *w, int *h, int *isKeyFrame)
{
	ew::VideoCodec *p = reinterpret_cast<ew::VideoCodec *>(handle);
	if (NULL == p)
		return 0;

	return p->UserDecode(fragItem, fragCount, w, h, isKeyFrame);
}
