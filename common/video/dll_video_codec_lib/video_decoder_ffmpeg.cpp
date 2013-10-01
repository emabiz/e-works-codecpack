#include "stdafx.h"

#include "video_decoder_ffmpeg.h"
#include "frag_helper.h"
#include "dll_def.h"
#include "misc_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#define _aligned_malloc(s,a) DEBUG_ALIGNED_MALLOC(s,a)
#define _aligned_free(p) DEBUG_ALIGNED_FREE(p)
#endif

static int _Yuv420Size(int w, int h)
{
	return w * h * 3 / 2;
}

static void _YuvAvPictureInit(AVPicture *pic, unsigned char *buf, int w, int h)
{
    _ASSERT(pic);
	_ASSERT(w > 0 && h > 0);

    memset(pic, 0, sizeof(AVPicture));
    pic->data[0] = buf;
    pic->data[1] = pic->data[0] + w * h;
    pic->data[2] = pic->data[1] + ((w+1)/2) * ((h+1)/2);
    pic->linesize[0] = w;
    pic->linesize[1] =
    pic->linesize[2] = (w+1) / 2;
}

namespace ew
{
	//////////////////////////////////////////////////////////////////////
	// VideoDecoderFfmpeg
	//////////////////////////////////////////////////////////////////////

	VideoDecoderFfmpeg::VideoDecoderFfmpeg(CodecID codecId, DepacketiseFunc depacketise)
		: VideoDecoder(FF_INPUT_BUFFER_PADDING_SIZE),
		codecId_(codecId),
		codec_(NULL),
		context_(NULL),
		frame_(NULL),
		opts_(NULL),
		depacketise_(depacketise)
	{
	}

	bool VideoDecoderFfmpeg::InitDecoder(int max_width, int max_height, int is_frag_stream)
	{
		if (!VideoDecoder::InitDecoder(max_width, max_height, is_frag_stream))
			goto InitDecoder_ErrInitParent;

		avcodec_init();
		avcodec_register_all();

		// codec
		codec_ = avcodec_find_decoder(codecId_);
		if (!codec_)
			goto InitDecoder_ErrFindDecoder;

		// frame
		frame_ = avcodec_alloc_frame();
		if (!frame_)
			goto InitDecoder_ErrAllocFrame;

		// context
		context_ = avcodec_alloc_context3(codec_);
		if (!context_)
			goto InitDecoder_ErrAllocContext;
		context_->pix_fmt = PIX_FMT_YUV420P;
		context_->width = max_width;
		context_->height = max_height;
        context_->workaround_bugs = 0; // no workaround for buggy H.263 implementations
        context_->error_concealment = FF_EC_GUESS_MVS | FF_EC_DEBLOCK;
        //context_->error_resilience = FF_ER_CAREFUL; // 0.5.1

		if (0 != avcodec_open2(context_, codec_, &opts_)) {
			goto InitDecoder_ErrOpenCodec;
		}
		return true;

InitDecoder_ErrOpenCodec:
		avcodec_close(context_);
		av_free(context_);
		context_ = NULL;
InitDecoder_ErrAllocContext:
		av_free(frame_);	
		frame_ = NULL;
InitDecoder_ErrAllocFrame:
		codec_ = NULL;
InitDecoder_ErrFindDecoder:
		VideoDecoder::DestroyDecoder();
InitDecoder_ErrInitParent:
		return false;
	}

	VideoDecoderFfmpeg::~VideoDecoderFfmpeg()
	{
		DestroyDecoder();
	}

	void VideoDecoderFfmpeg::DestroyDecoder()
	{
		if (NULL != codec_) {
			av_free(frame_);
			frame_ = NULL;
			avcodec_close(context_);
			av_free(context_);
			av_dict_free(&opts_);
			context_= NULL;
			codec_ = NULL;
			VideoDecoder::DestroyDecoder();
		}
	}

	int VideoDecoderFfmpeg::Decode(const FragItem *frag_item_ary, int frag_count,
			int *width, int *height, int *is_key_frame)
	{
		if (!VideoDecoder::Decode(frag_item_ary, frag_count, width, height, is_key_frame))
			return 0;
		
		int got_picture = 0;
		int size = 0;
		unsigned char *buf = input_buffer();

		if (is_frag_stream()) {
			if (!depacketise_)
				return 0;
			size = depacketise_(frag_item_ary, frag_count, input_buffer(), input_buffer_size());
#ifdef FRAG_FILE_DEBUG
			{
				static int frame_count_dec = 0;
				FILE *fd = NULL;
				char filename[50];
				sprintf_s(filename, "out_dec_%03d.dbg", frame_count_dec);
				_VERIFY(0 == fopen_s(&fd, filename, "wb"));
				fwrite(input_buffer(), size, 1, fd);
				fclose(fd);
				++frame_count_dec;
			}
#endif
		} else {
			size = frag_item_ary[0].size;
			if (input_buffer_size() < size)
				return 0;
			memcpy(buf, frag_item_ary[0].data, size);
		}
		// The end of the input buffer buf should be set to 0 to ensure that
		// no overreading happens for damaged MPEG streams.
		memset(&buf[size], 0, input_buffer_padding());
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = input_buffer();
		pkt.size = size;
		int ret = avcodec_decode_video2(context_, frame_, &got_picture, &pkt);
		av_free_packet(&pkt);
		if (ret < 0 || 0 == got_picture)
			return 0;

		int w = context_->width;
		int h = context_->height;

		AVPicture dst;
		_YuvAvPictureInit(&dst, output_buffer(), w, h);
		AVPicture src;
		for (int i = 0; i < 3; ++i) {
			src.data[i] = frame_->data[i];
			src.linesize[i] = frame_->linesize[i];
		}
		av_picture_copy(&dst, &src, PIX_FMT_YUV420P, w, h); 

		if (width)
			*width = w;

		if (height)
			*height = h;

		if (is_key_frame)
			*is_key_frame = (FF_I_TYPE == frame_->pict_type ? 1 : 0);

		return _Yuv420Size(w, h);
	}

} // namespace ew
