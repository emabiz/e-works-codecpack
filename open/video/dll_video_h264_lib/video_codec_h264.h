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

#ifndef VIDEO_CODEC_H264_H_
#define VIDEO_CODEC_H264_H_

#include "video_codec.h"
#include "video_decoder_ffmpeg.h"

extern "C" {
// x264
#define _INTTYPES_H_
#include "x264.h"
}

namespace ew
{
	class ParamArchiveH264Enc;

	//////////////////////////////////////////////////////////////////////
	// VideoEncoderH264
	//////////////////////////////////////////////////////////////////////

	class VideoEncoderH264 : public VideoEncoder
	{
	public:
		VideoEncoderH264();
		virtual ~VideoEncoderH264();

		virtual bool InitEncoder(int bitrate, int fps, int width, int height,
			int fragsize, int *fragcount, const char *enc_params);
		virtual void DestroyEncoder();
		virtual unsigned char *GetInputBuffer() const;
		virtual int Encode(int force_key_frame, int *is_key_frame);

	private:
		const ParamArchiveH264Enc *GetParamArchiveH264Enc() const;

		x264_param_t x4_param_;
		x264_picture_t x4_pic_;
		x264_t *x4_ctx_;
	};


	//////////////////////////////////////////////////////////////////////
	// VideoDecoderH264
	//////////////////////////////////////////////////////////////////////

	class VideoDecoderH264 : public VideoDecoderFfmpeg
	{
	public:
		VideoDecoderH264();
		//virtual ~VideoDecoderH264();

		//virtual bool InitDecoder(int max_width, int max_height, bool is_frag_stream);
		//virtual int Decode(const FragItem *frag_item_ary, int frag_count,
		//	int *width, int *height, int *is_key_frame);
	};


	//////////////////////////////////////////////////////////////////////
	// VideoCodecH264
	//////////////////////////////////////////////////////////////////////

	class VideoCodecH264 : public VideoCodec
	{
	public:
		VideoCodecH264();

	protected:
		virtual VideoEncoder *DoCreateEncoder();
		virtual VideoDecoder *DoCreateDecoder();
	};

} // namespace ew

#endif // ! VIDEO_CODEC_H264_H_