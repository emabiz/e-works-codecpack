#include "stdafx.h"

#include "video_encoder_ffmpeg.h"
#include "video_codec_def.h"
#include "frag_helper.h"
#include "fragmenter.h"
#include "param_video.h"
#include "misc_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#define _aligned_malloc(s,a) DEBUG_ALIGNED_MALLOC(s,a)
#define _aligned_free(p) DEBUG_ALIGNED_FREE(p)
#endif

#define MIN_QUANT 2

static double round(double r)
{
    return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// VideoEncoderFfmpeg
	//////////////////////////////////////////////////////////////////////

	VideoEncoderFfmpeg::VideoEncoderFfmpeg(ParamArchiveVideo *param_video)
		: VideoEncoder(0, param_video),
		codec_(NULL),
		context_(NULL),
		frame_(NULL),
		opts_(NULL),
		input_buffer_(NULL),
		input_buffer_size_(0)
	{
	}

	// initialization taken from plugin h263-1998 in opal-2.8.0
	bool VideoEncoderFfmpeg::InitEncoder(int bitrate, int fps, int width, int height,
			int fragsize, int *fragcount, const char *enc_params)
	{
		if (!VideoEncoder::InitEncoder(bitrate, fps, width, height, fragsize, fragcount, enc_params))
			goto InitEncoder_ErrInitParent;

		avcodec_init();
		avcodec_register_all();

		// codec
		codec_ = avcodec_find_encoder(GetCodecId());
		if (NULL == codec_)
			goto InitEncoder_ErrFindEncoder;

		// frame
		input_buffer_size_ = width * height * 3 / 2;
		input_buffer_ = static_cast<unsigned char *>(
			_aligned_malloc(input_buffer_size_, kMemAlign));
		frame_ = avcodec_alloc_frame();
		if (NULL == frame_)
			goto InitEncoder_ErrAllocFrame;
		frame_->data[0] = input_buffer_;
		frame_->data[1] = frame_->data[0] + width * height;
		frame_->data[2] = frame_->data[1] + width * height / 4;
		frame_->linesize[0] = width;
		frame_->linesize[1] =
		frame_->linesize[2] = width / 2;

		// context
		context_ = avcodec_alloc_context3(codec_);
		if (NULL == context_)
			goto InitEncoder_ErrAllocContext;
		context_->pix_fmt = PIX_FMT_YUV420P;
		context_->width = width;
		context_->height = height;
		context_->time_base.num = 1;
		context_->time_base.den = fps;
		context_->gop_size = param_video()->GetGopSize();

		context_->flags = CODEC_FLAG_INPUT_PRESERVED
			| CODEC_FLAG_EMU_EDGE
			| CODEC_FLAG_PASS1
			| GetFlags();
		context_->mb_decision = FF_MB_DECISION_SIMPLE;
		context_->me_method = ME_EPZS;
		context_->max_b_frames = 0;
		
		// target bitrate
		context_->bit_rate = bitrate * 3 / 4;
		context_->bit_rate_tolerance = bitrate / 2;
		context_->rc_min_rate = 0;
		context_->rc_max_rate = bitrate;
		context_->rc_buffer_size = bitrate / 1000;

		/* ratecontrol qmin qmax limiting method
		 0-> clipping, 1-> use a nice continous function to limit qscale wthin qmin/qmax.
		*/  
		context_->rc_qsquish = 0;            // limit q by clipping 
		context_->rc_eq = (char*) "1";       // rate control equation
		context_->rc_buffer_size = bitrate * 64;

		// temporal spatial trade off
		context_->max_qdiff = 10;  // was 3      // max q difference between frames
		context_->qcompress = 0.5;               // qscale factor between easy & hard scenes (0.0-1.0)
		context_->i_quant_factor = (float)-0.6;  // qscale factor between p and i frames
		context_->i_quant_offset = (float)0.0;   // qscale offset between p and i frames
		context_->me_subpel_quality = 8;

		context_->qmin = MIN_QUANT;
		context_->qmax = static_cast<int>(round ( (31.0 - MIN_QUANT) / 31.0 * GetTsto() + MIN_QUANT));
		context_->qmax = min(context_->qmax, 31);

		// TODO: vedere come mapparli in ffmpeg 0.10.3
		//context_->mb_qmin = context_->qmin;
		//context_->mb_qmax = context_->qmax;

		// Lagrange multipliers - this is how the context defaults do it:
		context_->lmin = context_->qmin * FF_QP2LAMBDA;
		context_->lmax = context_->qmax * FF_QP2LAMBDA; 

		context_->debug = FF_DEBUG_RC | FF_DEBUG_PICT_INFO | FF_DEBUG_MV;

		// frammentazione
		if (fragsize > 0) {
			context_->opaque = GetOpaque();
			context_->rtp_payload_size = 1; // if I leave 0, ffmpeg doesn't split at gobs, if I use a large value
				// ffmpeg aggregates gobs without inserting gob headers
		}

		if (0 != avcodec_open2(context_, codec_, &opts_)) {
			goto InitEncoder_ErrOpenCodec;
		}
		return true;

InitEncoder_ErrOpenCodec:
		avcodec_close(context_);
		av_free(context_);
		context_ = NULL;
InitEncoder_ErrAllocContext:
		av_free(frame_);	
		frame_ = NULL;
InitEncoder_ErrAllocFrame:
		codec_ = NULL;
		_aligned_free(input_buffer_);
		input_buffer_ = NULL;
		input_buffer_size_ = 0;
InitEncoder_ErrFindEncoder:
		VideoEncoder::DestroyEncoder();
InitEncoder_ErrInitParent:
		return false;
	}

	VideoEncoderFfmpeg::~VideoEncoderFfmpeg()
	{
		DestroyEncoder();
	}

	void VideoEncoderFfmpeg::DestroyEncoder()
	{
		if (NULL != codec_) {
			avcodec_close(context_);
			av_free(context_);
			av_free(frame_);
			av_dict_free(&opts_);
			frame_ = NULL;
			_aligned_free(input_buffer_);
			input_buffer_ = NULL;
			input_buffer_size_ = 0;
			context_ = NULL;
			codec_ = NULL;
			VideoEncoder::DestroyEncoder();
		}
	}

	unsigned char *VideoEncoderFfmpeg::GetInputBuffer() const
	{
		if (bitrate() < 0)
			return NULL;

		return frame_->data[0];
	}

	int VideoEncoderFfmpeg::Encode(int force_key_frame, int *is_key_frame)
	{
		if (bitrate() < 0)
			return 0;
		
		frag_writer()->Clear();
		frame_->pict_type = AV_PICTURE_TYPE_NONE != force_key_frame ? AV_PICTURE_TYPE_I : AV_PICTURE_TYPE_NONE;
		int enc_size = avcodec_encode_video(context_,
			output_buffer(), output_buffer_size(), frame_);
		if (0 == enc_size)
			return 0;

		if (NULL != is_key_frame)
			*is_key_frame = frame_->key_frame;

		int size = 0;

		Fragmenter *fragmenter = GetFragmenter();
		if (fragsize() > 0 && NULL != fragmenter) {
#ifdef FRAG_FILE_DEBUG
			{
				static int frame_count_enc = 0;
				FILE *fd = NULL;
				char filename[50];
				sprintf_s(filename, "out_enc_%03d.dbg", frame_count_enc);
				_VERIFY(0 == fopen_s(&fd, filename, "wb"));
				fwrite(output_buffer(), enc_size, 1, fd);
				fclose(fd);
				++frame_count_enc;
			}
#endif
			size = fragmenter->Fragment(output_buffer(), enc_size);
		} else {
			size = enc_size;
			_VERIFY(frag_writer()->AppendItem(output_buffer(), enc_size));
		}
		*fragcount() = frag_writer()->frag_count();
		return size;
	}

	void *VideoEncoderFfmpeg::GetOpaque()
	{
		return NULL;
	}

	VideoEncoderFfmpeg::RtpCallback VideoEncoderFfmpeg::GetRtpCallback() const
	{
		return NULL;
	}

	Fragmenter *VideoEncoderFfmpeg::GetFragmenter()
	{
		return NULL;
	}

} // namespace ew