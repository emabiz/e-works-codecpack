/*****************************************************************************
 * Xvid codec
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

#ifndef VIDEO_CODEC_XVID_H_
#define VIDEO_CODEC_XVID_H_

#include "video_codec.h"

#include "xvid.h"

namespace ew
{
	class ParamArchiveXvidEnc;

	//////////////////////////////////////////////////////////////////////
	// VideoEncoderXvid
	//////////////////////////////////////////////////////////////////////

	class VideoEncoderXvid : public VideoEncoder
	{
	public:
		VideoEncoderXvid();
		virtual ~VideoEncoderXvid();

		virtual bool InitEncoder(int bitrate, int fps, int width, int height,
			int fragsize, int *fragcount, const char *enc_params);
		virtual void DestroyEncoder();
		virtual unsigned char *GetInputBuffer() const;
		virtual int Encode(int force_key_frame, int *is_key_frame);

	private:
		const ParamArchiveXvidEnc *GetParamArchiveXvidEnc() const;

	    xvid_enc_create_t enc_create_;
		void *enc_handle_;
		xvid_enc_frame_t enc_frame_;
		unsigned char *input_buffer_;	//< source frame buffer
		int input_buffer_size_;
	};


	//////////////////////////////////////////////////////////////////////
	// VideoDecoderXvid
	//////////////////////////////////////////////////////////////////////

	class VideoDecoderXvid : public VideoDecoder
	{
	public:
		VideoDecoderXvid();
		virtual ~VideoDecoderXvid();

		virtual bool InitDecoder(int max_width, int max_height, int is_frag_stream);
		virtual void DestroyDecoder();
		virtual int Decode(const FragItem *frag_item_ary, int frag_count,
			int *width, int *height, int *is_key_frame);

	private:
   		xvid_dec_create_t dec_create_;
   		void * dec_handle_;
		xvid_dec_frame_t dec_frame_;
		int last_width_;
		int last_height_;
	};


	//////////////////////////////////////////////////////////////////////
	// VideoCodecXvid
	//////////////////////////////////////////////////////////////////////

	class VideoCodecXvid : public VideoCodec
	{
	public:
		VideoCodecXvid();

	protected:
		virtual VideoEncoder *DoCreateEncoder();
		virtual VideoDecoder *DoCreateDecoder();
	};

} // namespace ew

#endif // ! VIDEO_CODEC_XVID_H_
