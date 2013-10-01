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

#ifndef VIDEO_CODEC_H263_H_
#define VIDEO_CODEC_H263_H_

#include "video_codec.h"
#include "video_encoder_ffmpeg.h"
#include "video_decoder_ffmpeg.h"

namespace ew
{
	class ParamArchiveH263Enc;
	class Fragmenter;
	class FragmenterVideoH263_Rfc2190;

	//////////////////////////////////////////////////////////////////////
	// VideoEncoderH263
	//////////////////////////////////////////////////////////////////////

	class VideoEncoderH263 : public VideoEncoderFfmpeg
	{
	public:
		VideoEncoderH263();
		virtual ~VideoEncoderH263();

		virtual bool InitEncoder(int bitrate, int fps, int width, int height,
			int fragsize, int *fragcount, const char *enc_params);
		virtual void DestroyEncoder();
		//virtual unsigned char *GetInputBuffer() const;
		//virtual int Encode(int force_key_frame, int *is_key_frame);

		virtual CodecID GetCodecId() const;
		virtual int GetTsto() const;
		virtual int GetFlags() const;
		virtual void *GetOpaque();
		virtual VideoEncoderFfmpeg::RtpCallback GetRtpCallback() const;
		virtual Fragmenter *GetFragmenter();

	private:
		FILE *fd_callback_log_;
		FragmenterVideoH263_Rfc2190 *fragmenter_;

		static void H263RtpCallback(struct AVCodecContext *avctx, void *data, int size, int mb_nb);
		const ParamArchiveH263Enc *GetParamArchiveH263Enc() const;
	};


	//////////////////////////////////////////////////////////////////////
	// VideoDecoderH263
	//////////////////////////////////////////////////////////////////////

	class VideoDecoderH263 : public VideoDecoderFfmpeg
	{
	public:
		VideoDecoderH263();
		//virtual ~VideoDecoderH264();

		//virtual bool InitDecoder(int max_width, int max_height, bool is_frag_stream);
		//virtual int Decode(const FragItem *frag_item_ary, int frag_count,
		//	int *width, int *height, int *is_key_frame);
	};


	//////////////////////////////////////////////////////////////////////
	// VideoCodecH263
	//////////////////////////////////////////////////////////////////////

	class VideoCodecH263 : public VideoCodec
	{
	public:
		VideoCodecH263();

	protected:
		virtual VideoEncoder *DoCreateEncoder();
		virtual VideoDecoder *DoCreateDecoder();
	};

} // namespace ew

#endif // ! VIDEO_CODEC_H263_H_
