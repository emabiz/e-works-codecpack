/*****************************************************************************
 * H263 codec
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

#include "video_codec_h263.h"
//#include "frag_null.h"
#include "frag_h263.h"
#include "param_h263.h"
#include "frag_helper.h"

//#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ENABLE_CALLBACK_LOG 0

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// VideoEncoderH263
	//////////////////////////////////////////////////////////////////////

	VideoEncoderH263::VideoEncoderH263()
		: VideoEncoderFfmpeg(new ParamArchiveH263Enc()), fragmenter_(NULL)
	{
#ifdef ENABLE_CALLBACK_LOG
		_VERIFY(0 == fopen_s(&fd_callback_log_, "av_callback.txt", "wt"));
#else
		fd_callback_log_ = NULL;
#endif
	}

	VideoEncoderH263::~VideoEncoderH263()
	{
		if (NULL != fd_callback_log_)
			fclose(fd_callback_log_);
		DestroyEncoder();
	}

	bool VideoEncoderH263::InitEncoder(int bitrate, int fps, int width, int height,
		int fragsize, int *fragcount, const char *enc_params)
	{
		if (!VideoEncoderFfmpeg::InitEncoder(bitrate, fps, width, height, fragsize, fragcount, enc_params))
			return false;
		
		_ASSERT(NULL == fragmenter_);
		if (fragsize > 0)
			fragmenter_ = new FragmenterVideoH263_Rfc2190(fragsize, frag_writer(), false);
		return true;
	}

	void VideoEncoderH263::DestroyEncoder()
	{
		if (NULL != fragmenter_) {
			delete fragmenter_;
			fragmenter_ = NULL;
		}
		VideoEncoderFfmpeg::DestroyEncoder();
	}

	CodecID VideoEncoderH263::GetCodecId() const
	{
		return GetParamArchiveH263Enc()->GetPlus() ? CODEC_ID_H263P : CODEC_ID_H263;
	}

	int VideoEncoderH263::GetTsto() const 
	{
		return GetParamArchiveH263Enc()->GetTsto();
	}

	int VideoEncoderH263::GetFlags() const
	{
		const ew::ParamArchiveH263Enc *p = GetParamArchiveH263Enc();
		if (0 == p->GetPlus())
			return 0;

		int flags = 0;
		if (p->GetAnnexD())
			flags |= CODEC_FLAG_H263P_UMV;
		if (p->GetAnnexI())
			flags |= CODEC_FLAG_AC_PRED;
		if (p->GetAnnexI())
			flags |= CODEC_FLAG_LOOP_FILTER;
		return flags;
	}

	void *VideoEncoderH263::GetOpaque()
	{
		return this;
	}

	VideoEncoderH263::RtpCallback VideoEncoderH263::GetRtpCallback() const
	{
		//return H263RtpCallback;
		return NULL;
	}

	Fragmenter *VideoEncoderH263::GetFragmenter()
	{
		return fragmenter_;
	}

	void VideoEncoderH263::H263RtpCallback(struct AVCodecContext *avctx, void *data, int size, int mb_nb)
	{
		_ASSERT(NULL != avctx);
		_ASSERT(NULL != avctx->opaque);
		_ASSERT(NULL != data);
		_ASSERT(size > 0);
		_ASSERT(mb_nb > 0);
		
		//if (mb_nb <= 22)
		//	printf("********callback size=%d mb_nb=%d\n", size, mb_nb);
		VideoEncoderH263 *instance = reinterpret_cast<VideoEncoderH263 *>(avctx->opaque);
		if (NULL != instance->fd_callback_log_) {
			if (size > instance->fragsize())
				_VERIFY(0 < fprintf_s(instance->fd_callback_log_,
					"H263RtpCallback size=%5d mb_nb=%5d\n", size, mb_nb));
		}
		_VERIFY(instance->frag_writer()->AppendItem(reinterpret_cast<unsigned char*>(data), size));
	}

	const ParamArchiveH263Enc *VideoEncoderH263::GetParamArchiveH263Enc() const
	{
		const ParamArchiveH263Enc *param = dynamic_cast<const ParamArchiveH263Enc *>(param_video());
		_ASSERT(NULL != param);
		return param;
	}

	//////////////////////////////////////////////////////////////////////
	// VideoDecoderH263
	//////////////////////////////////////////////////////////////////////

	VideoDecoderH263::VideoDecoderH263()
		: VideoDecoderFfmpeg(CODEC_ID_H263, H263DepacketiseRfc2190)
	{
	}


	//////////////////////////////////////////////////////////////////////
	// VideoCodecH263
	//////////////////////////////////////////////////////////////////////

	VideoCodecH263::VideoCodecH263()
		: VideoCodec()
	{
	}

	VideoEncoder *VideoCodecH263::DoCreateEncoder()
	{
		return new VideoEncoderH263();
	}

	VideoDecoder *VideoCodecH263::DoCreateDecoder()
	{
		return new VideoDecoderH263();
	}

} // namespace ew
