/*****************************************************************************
 * H264 codec
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

#include "video_codec_h264.h"
#include "frag_h264.h"
#include "param_h264.h"
#include "frag_helper.h"
#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew
{

	//////////////////////////////////////////////////////////////////////
	// VideoEncoderH264
	//////////////////////////////////////////////////////////////////////

	VideoEncoderH264::VideoEncoderH264()
		: VideoEncoder(0, new ParamArchiveH264Enc())
	{
	}

	VideoEncoderH264::~VideoEncoderH264()
	{
		DestroyEncoder();
	}

	bool VideoEncoderH264::InitEncoder(int bitrate, int fps, int width, int height,
		int fragsize, int *fragcount, const char *enc_params)
	{
		if (!VideoEncoder::InitEncoder(bitrate, fps, width, height, fragsize, fragcount, enc_params))
			goto InitEncoder_ErrInitParent;

		// parameters
		_VERIFY(0 == x264_param_default_preset(&x4_param_, "veryfast", "zerolatency"));
		
		x4_param_.b_annexb = 0; // raw nal for rtp; encode as annexb adding 0001 in front of nals
		// log
		//x4_param_.pf_log = LogCallback;
		x4_param_.p_log_private = this;
#ifdef _DEBUG
		x4_param_.i_log_level = X264_LOG_DEBUG;
#else
		x4_param_.i_log_level = X264_LOG_NONE;
#endif
		// threading
		x4_param_.i_threads = X264_THREADS_AUTO;
		x4_param_.b_sliced_threads = 1;
		// dimensions
		x4_param_.i_width = width;
		x4_param_.i_height = height;
		// framerate
		x4_param_.i_fps_num = fps;
		x4_param_.i_fps_den = 1;
		// bitrate
		x4_param_.rc.i_rc_method = X264_RC_CRF;
		x4_param_.rc.i_vbv_max_bitrate = bitrate / 1000;
		x4_param_.rc.i_vbv_buffer_size = bitrate / (1000 * fps); //kbit
		// key frame
		//x4_param_.b_intra_refresh = 1;
		x4_param_.i_keyint_max = GetParamArchiveH264Enc()->GetGopSize();
		x4_param_.b_repeat_headers = 1;
		// fragmentation
		if (fragsize > 0) {
#ifdef FRAG_FILE_DEBUG
			x4_param_.i_slice_max_size = fragsize*4; // testing: i want to obtain large nals to split in fu-a
#else
			x4_param_.i_slice_max_size = fragsize;
#endif
		}
		// only baseline profile
		const ParamArchiveH264Enc *paramH264 = dynamic_cast<const ParamArchiveH264Enc *>(param_video());
		_ASSERT(NULL != paramH264);
		x4_param_.i_level_idc = FindX264Level(static_cast<H264Level>(paramH264->GetLevel()));
		_VERIFY(0 == x264_param_apply_profile(&x4_param_, "baseline"));

		// context
		x4_ctx_ = x264_encoder_open(&x4_param_);
		if (!x4_ctx_)
			goto InitEncoder_ErrInitX264;

		// picture
		_VERIFY(0 == x264_picture_alloc(&x4_pic_, X264_CSP_I420, width, height));
		return true;


InitEncoder_ErrInitX264:
		VideoEncoder::DestroyEncoder();
InitEncoder_ErrInitParent:
		return false;
	}

	void VideoEncoderH264::DestroyEncoder()
	{
		if (bitrate() > 0) {
			x264_encoder_close(x4_ctx_);
			x4_ctx_ = NULL;
			x264_picture_clean(&x4_pic_);
		}
		VideoEncoder::DestroyEncoder();
	}

	unsigned char *VideoEncoderH264::GetInputBuffer() const
	{
		if (bitrate() < 0)
			return NULL;

		_ASSERT(x4_pic_.img.plane[0] + width() * height() == x4_pic_.img.plane[1]);
		_ASSERT(x4_pic_.img.plane[1] + width() * height() / 4 == x4_pic_.img.plane[2]);
		return x4_pic_.img.plane[0];
	}

	int VideoEncoderH264::Encode(int force_key_frame, int *is_key_frame)
	{
		if (bitrate() < 0)
			return 0;

		x4_pic_.i_type = 0 != force_key_frame ? X264_TYPE_IDR : X264_TYPE_AUTO;

		x264_nal_t *nal_ary = NULL;
		int nal_count = 0;
		x264_picture_t pic_out;
		do { // depending on parameters in use, x264 could be preparing some buffers for threads
			if (x264_encoder_encode(x4_ctx_, &nal_ary, &nal_count, &x4_pic_, &pic_out) < 0)
				return 0;
		} while (nal_count == 0);

		if (is_key_frame)
			*is_key_frame = pic_out.i_type == X264_TYPE_IDR || pic_out.i_type == X264_TYPE_I ? 1 : 0;

		int size = 0;

		if (fragsize() > 0) {
#ifdef FRAG_FILE_DEBUG
			{
				static int frame_count_enc = 0;
				FILE *fd = NULL;
				char filename[50];
				sprintf_s(filename, "out_enc_%03d.dbg", frame_count_enc);
				_VERIFY(0 == fopen_s(&fd, filename, "wb"));
				int s = H264EncodeNals(output_buffer(), output_buffer_size(), nal_ary, nal_count);
				fwrite(output_buffer(), s, 1, fd);
				fclose(fd);
				++frame_count_enc;
			}
#endif
			FragmenterVideoH264 fragmenter(GetParamArchiveH264Enc()->GetFragMode(),
				fragsize(), frag_writer());
			size = fragmenter.Fragment(nal_ary, nal_count);
		} else {
			size = H264EncodeNals(output_buffer(), output_buffer_size(), nal_ary, nal_count);
			if (size == 0)
				return 0;
			frag_writer()->Clear();
			_VERIFY(frag_writer()->AppendItem(output_buffer(), size));
		}
		*fragcount() = frag_writer()->frag_count();
		return size;
	}

#if 0
	void VideoEncoderH264::LogCallback(void *p, int level, const char *fmt, va_list args)
	{
		_ASSERT(NULL != p);
		_ASSERT(NULL != fmt);

		ew::FileLogger::Level log_level = ew::FileLogger::kLevelNoLog;

		if (level < 0 || level > X264_LOG_DEBUG)
			return;

		switch (level) {
			case X264_LOG_ERROR:	log_level = ew::FileLogger::kLevelError; break;
			case X264_LOG_WARNING:	log_level = ew::FileLogger::kLevelWarning; break;
			case X264_LOG_INFO:		log_level = ew::FileLogger::kLevelInfo; break;
			case X264_LOG_DEBUG:	log_level = ew::FileLogger::kLevelDebug; break;
		};

		VideoEncoderH264 *encoder = reinterpret_cast<VideoEncoderH264 *>(p);
		_ASSERT(NULL != encoder);
		//_VERIFY(encoder->logger()->Log(log_level, fmt, args));
	}
#endif

	const ParamArchiveH264Enc *VideoEncoderH264::GetParamArchiveH264Enc() const
	{
		const ParamArchiveH264Enc *param = dynamic_cast<const ParamArchiveH264Enc *>(param_video());
		_ASSERT(NULL != param);
		return param;
	}

	//////////////////////////////////////////////////////////////////////
	// VideoDecoderH264
	//////////////////////////////////////////////////////////////////////

	VideoDecoderH264::VideoDecoderH264()
		: VideoDecoderFfmpeg(CODEC_ID_H264, H264DepacketiseRfc3984)
	{
	}

	//////////////////////////////////////////////////////////////////////
	// VideoCodecH264
	//////////////////////////////////////////////////////////////////////

	VideoCodecH264::VideoCodecH264()
		: VideoCodec()
	{
	}

	VideoEncoder *VideoCodecH264::DoCreateEncoder()
	{
		return new VideoEncoderH264();
	}

	VideoDecoder *VideoCodecH264::DoCreateDecoder()
	{
		return new VideoDecoderH264();
	}

} // namespace ew