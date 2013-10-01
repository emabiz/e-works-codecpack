#include "stdafx.h"

#include "video_codec.h"
#include "frag_helper.h"
#include "param_video.h"
#include "video_codec_def.h"
#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#define _aligned_malloc(s,a) DEBUG_ALIGNED_MALLOC(s,a)
#define _aligned_free(p) DEBUG_ALIGNED_FREE(p)
#endif

static int _GetI420Size(int width, int height)
{
	_ASSERT(width > 0);
	_ASSERT(height > 0);

	return width * height * 3 / 2;
}

namespace ew
{

	//////////////////////////////////////////////////////////////////////
	// VideoEncoder
	//////////////////////////////////////////////////////////////////////

	VideoEncoder::VideoEncoder(int output_buffer_min_size, ParamArchiveVideo *param_video)
		: output_buffer_min_size_ (output_buffer_min_size), param_video_(param_video)
	{
		_ASSERT(output_buffer_min_size >= 0);
		_ASSERT(NULL != param_video);
		SetDefaults();
		frag_writer_ = new FragWriter(MAX_PACKETS_PER_FRAME);
	}

	VideoEncoder::~VideoEncoder()
	{
		DestroyEncoder();
		delete frag_writer_;
		delete param_video_;
	}

	bool VideoEncoder::InitEncoder(int bitrate, int fps, int width, int height,
		int fragsize, int *fragcount, const char *enc_params)
	{
		if (bitrate_ > 0)
			return false; // already initialized

		if (bitrate < kBitrateMin)
			return false;
		if (fps < kFpsMin)
			return false;
		if (width < kWidthMin)
			return false;
		if (0 != width % kWidthStep)
			return false;
		if (height < kHeightMin)
			return false;
		if (0 != height % kHeightStep)
			return false;
		if (0 != fragsize && fragsize < kFragsizeMin)
			return false;
		if (NULL == fragcount)
			return false;

		bitrate_ = bitrate;
		fps_ = fps;
		width_ = width;
		height_ = height;
		fragsize_ = fragsize;
		fragcount_ = fragcount;
		*fragcount_ = 0;
		output_buffer_size_ = max(output_buffer_min_size_, _GetI420Size(width, height));
		output_buffer_ = reinterpret_cast<unsigned char *> (_aligned_malloc(output_buffer_size_, kMemAlign));
		
		if (NULL != enc_params) {
			if (!param_video_->Parse(enc_params)) {
				VideoEncoder::DestroyEncoder();
				return false;
			}
		}

		return true;
	}

	void VideoEncoder::DestroyEncoder()
	{
		if (bitrate() > 0) {// if initialized
			_aligned_free(output_buffer_);
			SetDefaults();
		}
	}

	void VideoEncoder::SetDefaults()
	{
		bitrate_ = -1;
		fps_ = -1;
		width_ = -1;
		height_ = -1;
		fragsize_ = -1;
		output_buffer_ = NULL;
		output_buffer_size_ = -1;
		param_video_->SetDefaults();
	}

	//////////////////////////////////////////////////////////////////////
	// VideoDecoder
	//////////////////////////////////////////////////////////////////////

	VideoDecoder::VideoDecoder(int input_buffer_padding)
		: input_buffer_padding_(input_buffer_padding)
	{
		_ASSERT(input_buffer_padding >= 0);
		SetDefaults();
	}

	VideoDecoder::~VideoDecoder()
	{
		DestroyDecoder();
	}

	bool VideoDecoder::InitDecoder(int max_width, int max_height, int is_frag_stream)
	{
		if (max_width_ > 0)
			return false; // already initialized

		if (max_width < kWidthMin)
			return false;
		if (max_height < kHeightMin)
			return false;
		
		max_width_ = max_width;
		max_height_ = max_height;
		is_frag_stream_ = 0 != is_frag_stream;
		input_buffer_size_ = _GetI420Size(max_width, max_height);
		input_buffer_ = reinterpret_cast<unsigned char *> (
			_aligned_malloc(input_buffer_size_ + input_buffer_padding_, kMemAlign));
		output_buffer_ = reinterpret_cast<unsigned char *> (
			_aligned_malloc(input_buffer_size_, kMemAlign));

		return true;
	}

	void VideoDecoder::DestroyDecoder()
	{
		if (max_width_ > 0 ) {
			_aligned_free(output_buffer_);
			_aligned_free(input_buffer_);
			SetDefaults();
		}
	}

	int VideoDecoder::Decode(const FragItem *frag_item_ary, int frag_count,
		int * /*width*/, int * /*height*/, int * /*is_key_frame*/)
	{
		if (NULL == frag_item_ary)
			return 0;
		
		if (frag_count <= 0)
			return 0;

		if (!is_frag_stream_ && frag_count != 1)
			return 0;

		for (int i = 0; i < frag_count; ++i) {
			if (NULL == frag_item_ary[i].data || 0 == frag_item_ary[i].size)
				return 0;
		}

		return 1;
	}

	void VideoDecoder::SetDefaults()
	{
		max_width_ = -1;
		max_height_ = -1;
		is_frag_stream_ = false;
		input_buffer_ = NULL;
		output_buffer_ = NULL;
	}

	//////////////////////////////////////////////////////////////////////
	// VideoCodec
	//////////////////////////////////////////////////////////////////////

	VideoCodec::VideoCodec()
		: encoder_(NULL),
		decoder_(NULL)
	{
	}

	VideoCodec::~VideoCodec()
	{
		if (NULL != decoder_)
			delete decoder_;
		if (NULL != encoder_)
			delete encoder_;
	}

	bool VideoCodec::InitEncoder(int bitrate, int fps, int width, int height,
		int fragsize, int *fragCount, const char *enc_params)
	{
		if (NULL != encoder_)
			return false;
		
		encoder_ = DoCreateEncoder();
		_ASSERT(NULL != encoder_);
		if (!encoder_->InitEncoder(bitrate, fps, width, height, fragsize, fragCount, enc_params)) {
			delete encoder_;
			encoder_ = NULL;
			return false;
		}

		return true;
	}

	bool VideoCodec::InitDecoder(int max_width, int max_height, int is_frag_stream)
	{
		if (NULL != decoder_)
			return false;
		if (max_width < kWidthMin)
			return false;
		if (max_height < kHeightMin)
			return false;

		decoder_ = DoCreateDecoder();
		_ASSERT(NULL != decoder_);
		if (!decoder_->InitDecoder(max_width, max_height, is_frag_stream)) {
			delete decoder_;
			decoder_ = NULL;
			return false;
		}
		return true;
	}

	void VideoCodec::DestroyEncoder()
	{
		if (NULL == encoder_)
			return;

		delete encoder_;
		encoder_ = NULL;
	}

	void VideoCodec::DestroyDecoder()
	{
		if (NULL == decoder_)
			return;

		delete decoder_;
		decoder_ = NULL;
	}

	int VideoCodec::UserEncInit(unsigned char **EncoderInput,
		int Bitrate, int Fps, int Width, int Height,
		const char *Params,
		int fragSize, struct FragItem **fragItem, int *fragCount)
	{
		if (NULL == EncoderInput)
			return 0;
		if (NULL == fragItem)
			return 0;

		if (!InitEncoder(Bitrate, Fps, Width, Height, fragSize, fragCount, Params))
			return 0;

		_ASSERT(NULL != encoder_);

		*EncoderInput = encoder_->GetInputBuffer();
		const ew::FragWriter *frag_writer = encoder_->frag_writer();
		*fragItem = frag_writer->frag_ary();
		
		return 1;
	}

	int VideoCodec::UserEncode(int forceKeyFrame, int *isKeyFrame)
	{
		if (NULL == encoder_)
			return 0;

		return encoder_->Encode(forceKeyFrame, isKeyFrame); 
	}

	int VideoCodec::UserDecInit(unsigned char **DecoderOutput,
      int PictureW, int PictureH, int IsFragStream)
	{
		if (NULL == DecoderOutput)
			return 0;

		if (!InitDecoder(PictureW, PictureH, IsFragStream))
			return 0;

		_ASSERT(NULL != decoder_);

		*DecoderOutput = decoder_->output_buffer();
		return 1;
	}

	int VideoCodec::UserDecode(struct FragItem *fragItem, int fragCount,
		int *w, int *h, int *isKeyFrame)
	{
		if (NULL == decoder_)
			return 0;

		return decoder_->Decode(fragItem, fragCount, w, h, isKeyFrame);
	}

} // namespace ew