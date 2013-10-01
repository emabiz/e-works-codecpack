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

#include "stdafx.h"

#include "video_codec_xvid.h"
#include "video_codec_def.h"
#include "param_xvid.h"
#include "param_video.h"
#include "frag_helper.h"
#include "misc_utils.h"
#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#define _aligned_malloc(s,a) DEBUG_ALIGNED_MALLOC(s,a)
#define _aligned_free(p) DEBUG_ALIGNED_FREE(p)
#endif

static void _InitXvid()
{
    xvid_gbl_init_t xvid_gbl_init;
    memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init));
    xvid_gbl_init.version = XVID_VERSION;
	_VERIFY(0 == xvid_global(NULL, XVID_GBL_INIT, &xvid_gbl_init, NULL));
}

namespace ew
{

	//////////////////////////////////////////////////////////////////////
	// VideoEncoderXvid
	//////////////////////////////////////////////////////////////////////

	VideoEncoderXvid::VideoEncoderXvid()
		: VideoEncoder(0, new ParamArchiveXvidEnc()),
		enc_handle_(NULL),
		input_buffer_(NULL),
		input_buffer_size_(0)
	{
	}

	VideoEncoderXvid::~VideoEncoderXvid()
	{
		DestroyEncoder();
	}

	bool VideoEncoderXvid::InitEncoder(int bitrate, int fps, int width, int height,
		int fragsize, int *fragcount, const char *enc_params)
	{
	    xvid_plugin_single_t single;
		xvid_enc_plugin_t plugins[1];
		xvid_gbl_info_t xinfo;

		_InitXvid();

		if (fragsize > 0)
			goto InitEncoder_ErrFragsize;

		if (!VideoEncoder::InitEncoder(bitrate, fps, width, height, fragsize, fragcount, enc_params))
			goto InitEncoder_ErrInitParent;
		
		// number of threads
		memset(&xinfo, 0, sizeof(xinfo));
		xinfo.version = XVID_VERSION;
		_VERIFY(0 == xvid_global(NULL, XVID_GBL_INFO, &xinfo, NULL));

	    memset(&enc_create_, 0, sizeof(enc_create_));
		enc_create_.version = XVID_VERSION;
		enc_create_.num_threads = xinfo.num_threads;
		enc_create_.width = width;
		enc_create_.height = height;
		enc_create_.profile = XVID_PROFILE_AS_L3; //up to 352x288 30fps, 768kbps
	
		// frame
		input_buffer_size_ = width * height * 3 / 2;
		input_buffer_ = static_cast<unsigned char *>(
			_aligned_malloc(input_buffer_size_, kMemAlign));

		memset(&enc_frame_, 0, sizeof(enc_frame_));
		enc_frame_.version = XVID_VERSION;
		enc_frame_.bitstream = output_buffer();
		enc_frame_.length = output_buffer_size();
		enc_frame_.input.csp = XVID_CSP_PLANAR; /* YUV420P */
		enc_frame_.input.plane[0] = input_buffer_;
		enc_frame_.input.plane[1] = reinterpret_cast<unsigned char *>(enc_frame_.input.plane[0])
			+ width * height;
		enc_frame_.input.plane[2] = reinterpret_cast<unsigned char *>(enc_frame_.input.plane[1])
			+ width * height / 4;
		enc_frame_.input.stride[0] = width;
		enc_frame_.input.stride[1] = 
		enc_frame_.input.stride[2] = width / 2;
		enc_frame_.type = XVID_TYPE_AUTO;

		const ew::ParamArchiveXvidEnc *paramsXvid = GetParamArchiveXvidEnc();
		
		// qualità (livelli della vecchia dll)
		switch (paramsXvid->GetQuality()) {
			case 0: /* xvid quality 0 */
				enc_frame_.vop_flags = 0;
				enc_frame_.motion = 0;
				break;
			case 1: /* xvid quality 2 */
				enc_frame_.vop_flags =
            		XVID_VOP_HALFPEL;
				enc_frame_.motion =
					XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16;
				break;
			case 2: /* xvid quality 3 */ /* default */
				enc_frame_.vop_flags =
            		XVID_VOP_HALFPEL | XVID_VOP_INTER4V;
				enc_frame_.motion =
            		XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 |
            		XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8;
				break;
			case 3: /* xvid quality 5 */
				enc_frame_.vop_flags =
            		XVID_VOP_HALFPEL | XVID_VOP_INTER4V |
            		XVID_VOP_TRELLISQUANT;
				enc_frame_.motion =
            		XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 |
            		XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8 |
            		XVID_ME_CHROMA_PVOP | XVID_ME_CHROMA_BVOP;
				break;
			case 4: /* xvid quality 6 */
				enc_frame_.vop_flags =
            		XVID_VOP_HALFPEL | XVID_VOP_INTER4V |
            		XVID_VOP_TRELLISQUANT | XVID_VOP_HQACPRED;
				enc_frame_.motion =
            		XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 | XVID_ME_EXTSEARCH16 |
            		XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8 | XVID_ME_EXTSEARCH8 |
            		XVID_ME_CHROMA_PVOP | XVID_ME_CHROMA_BVOP;
				break;
		}

		/* plugin single pass */
		memset(&single, 0, sizeof(xvid_plugin_single_t));
		single.version = XVID_VERSION;
		single.bitrate = bitrate;
		plugins[0].func = xvid_plugin_single;
		plugins[0].param = &single;
		enc_create_.plugins = plugins;
		enc_create_.num_plugins = 1;

		enc_create_.fincr = 1;
		enc_create_.fbase = fps;
		enc_create_.max_key_interval = paramsXvid->GetGopSize();
	    
		for (int i = 0; i < 3; i++) {
			enc_create_.min_quant[i] = paramsXvid->GetQMin();
			enc_create_.max_quant[i] = paramsXvid->GetQMax();
		}
		// qdiff=?

		if (xvid_encore(NULL, XVID_ENC_CREATE, &enc_create_, NULL) < 0)
			goto InitEncoder_ErrOpen;
		enc_handle_ = enc_create_.handle;

		// TODO: logging?
		return true;

InitEncoder_ErrOpen:
		_aligned_free(input_buffer_);
		input_buffer_ = 0;
		VideoEncoder::DestroyEncoder();
InitEncoder_ErrInitParent:
InitEncoder_ErrFragsize:
		return false;
	}

	void VideoEncoderXvid::DestroyEncoder()
	{
		if (bitrate() > 0) {
			xvid_encore(enc_handle_, XVID_ENC_DESTROY, NULL, NULL);
			enc_handle_ = NULL;
			_aligned_free(input_buffer_);
			input_buffer_ = NULL;
		}
		VideoEncoder::DestroyEncoder();
	}

	unsigned char *VideoEncoderXvid::GetInputBuffer() const
	{
		if (bitrate() < 0)
			return NULL;

		return input_buffer_;
	}

	int VideoEncoderXvid::Encode(int force_key_frame, int *is_key_frame)
	{
		if (bitrate() < 0)
			return 0;

		int size = 0;
		xvid_enc_stats_t enc_stats;
   		memset(&enc_stats, 0, sizeof(xvid_enc_stats_t));
		enc_stats.version = XVID_VERSION;

		enc_frame_.type = 0 != force_key_frame ? XVID_TYPE_IVOP : XVID_TYPE_AUTO;
		int xerr = xvid_encore(enc_handle_, XVID_ENC_ENCODE, &enc_frame_, &enc_stats);
		if (NULL != is_key_frame)
			*is_key_frame = 0 != (enc_frame_.out_flags & XVID_KEYFRAME) ? 1 : 0;

		if (xerr >= 0) {
			frag_writer()->Clear();
			_VERIFY(frag_writer()->AppendItem(output_buffer(), enc_stats.length));
			size = enc_stats.length;
			*fragcount() = frag_writer()->frag_count();
		}
		return size;
	}

	const ParamArchiveXvidEnc *VideoEncoderXvid::GetParamArchiveXvidEnc() const
	{
		const ParamArchiveXvidEnc *param = dynamic_cast<const ParamArchiveXvidEnc *>(param_video());
		_ASSERT(NULL != param);
		return param;
	}

	//////////////////////////////////////////////////////////////////////
	// VideoDecoderXvid
	//////////////////////////////////////////////////////////////////////

	VideoDecoderXvid::VideoDecoderXvid()
		: VideoDecoder(0),
		dec_handle_(NULL),
		last_width_(0),
		last_height_(0)
	{
	}

	VideoDecoderXvid::~VideoDecoderXvid()
	{
		DestroyDecoder();
	}

	bool VideoDecoderXvid::InitDecoder(int max_width, int max_height, int is_frag_stream)
	{
		if (0 != is_frag_stream)
			goto InitDecoder_ErrArgs;

		if (!VideoDecoder::InitDecoder(max_width, max_height, is_frag_stream))
			goto InitDecoder_ErrInitParent;

		_InitXvid();

		memset(&dec_create_, 0, sizeof(dec_create_));
		dec_create_.version = XVID_VERSION;

		/* frame */
		memset(&dec_frame_, 0, sizeof(dec_frame_));
		dec_frame_.version = XVID_VERSION;
		// initilize in decode, because a frame can be decoded in multiple steps
		//dec_frame_.bitstream = input_buffer();
		//dec_frame_.length = input_buffer_size();
		dec_frame_.output.csp = XVID_CSP_PLANAR; /* YUV420P */

		dec_frame_.output.plane[0] = output_buffer();
		dec_frame_.output.stride[0] = max_width;
		dec_frame_.output.plane[1] = reinterpret_cast<unsigned char *>(dec_frame_.output.plane[0])
			+ max_width * max_height;
		dec_frame_.output.stride[1] = max_width / 2;
		dec_frame_.output.plane[2] = reinterpret_cast<unsigned char *>(dec_frame_.output.plane[1])
			+ max_width * max_height / 4;
		dec_frame_.output.stride[2] = max_width / 2;

		if (xvid_decore(NULL, XVID_DEC_CREATE, &dec_create_, NULL) < 0)
			goto InitDecoder_ErrOpen;
		dec_handle_ = dec_create_.handle;

		return true;

InitDecoder_ErrOpen:
		VideoDecoder::DestroyDecoder();
InitDecoder_ErrInitParent:
InitDecoder_ErrArgs:
		return false;
	}

	void VideoDecoderXvid::DestroyDecoder()
	{
		if (max_width() > 0) {
			xvid_decore(dec_handle_, XVID_DEC_DESTROY, NULL, NULL);
			VideoDecoder::DestroyDecoder();
		}
	}

	int VideoDecoderXvid::Decode(const FragItem *frag_item_ary, int frag_count,
		int *width, int *height, int *is_key_frame)
	{
		if (!VideoDecoder::Decode(frag_item_ary, frag_count, width, height, is_key_frame))
			return 0;

		int size = frag_item_ary[0].size;
		unsigned char *buf = input_buffer();
		if (input_buffer_size() < size)
			return 0;
		memcpy(buf, frag_item_ary[0].data, size);

		xvid_dec_stats_t dec_stats;

		memset(&dec_stats, 0, sizeof(xvid_dec_stats_t));
		dec_stats.version = XVID_VERSION;
		dec_frame_.bitstream = buf;
		dec_frame_.length = size;

		do {
			int used = xvid_decore(dec_handle_, XVID_DEC_DECODE, &dec_frame_,
				&dec_stats);

			if (used > 0) {
				if(dec_stats.type == XVID_TYPE_VOL) {
					last_width_ = dec_stats.data.vol.width;
					last_height_ = dec_stats.data.vol.height;
				}
			} else {
				return 0;
			}

			dec_frame_.bitstream = reinterpret_cast<unsigned char *>(dec_frame_.bitstream) + used;
			dec_frame_.length -= used;
		} while (dec_stats.type <= 0 && dec_frame_.length > 0);

		if (dec_stats.type > 0) {
			if (NULL != width)
				*width = last_width_;
			if (NULL != height)
				*height = last_height_;
			if (NULL != is_key_frame)
				*is_key_frame = (dec_stats.type == XVID_TYPE_IVOP)? 1 : 0;

			unsigned char *dst_cur = output_buffer();
			for (int plane = 0; plane < 3; plane++) {
				unsigned char *buf = reinterpret_cast<unsigned char*>(dec_frame_.output.plane[plane]);
				int stride = dec_frame_.output.stride[plane];
				int line_size = last_width_ >> (plane?1:0);
				for (int y = 0; y < last_height_ >> (plane?1:0); y++) {
					memcpy(dst_cur, buf, line_size);
					dst_cur += line_size;
					buf += stride;
				}
			}
			return dst_cur - output_buffer();
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	// VideoCodecXvid
	//////////////////////////////////////////////////////////////////////

	VideoCodecXvid::VideoCodecXvid()
		: VideoCodec()
	{
	}

	VideoEncoder *VideoCodecXvid::DoCreateEncoder()
	{
		return new VideoEncoderXvid();
	}

	VideoDecoder *VideoCodecXvid::DoCreateDecoder()
	{
		return new VideoDecoderXvid();
	}

} // namespace ew
