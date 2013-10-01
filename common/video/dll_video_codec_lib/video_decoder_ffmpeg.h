#ifndef VIDEO_DECODER_FFMPEG_H_
#define VIDEO_DECODER_FFMPEG_H_

#include "video_codec.h"

enum CodecID;

namespace ew
{
	//////////////////////////////////////////////////////////////////////
	// VideoDecoderFfmpeg
	//////////////////////////////////////////////////////////////////////

	typedef int (*DepacketiseFunc)(const FragItem *frag_item_ary, int frag_count,
		unsigned char *input_buffer, int input_buffer_size);

	class VideoDecoderFfmpeg : public VideoDecoder
	{
	public:
		VideoDecoderFfmpeg(CodecID codecId, DepacketiseFunc depacketize);
		virtual ~VideoDecoderFfmpeg();

		virtual bool InitDecoder(int max_width, int max_height, int is_frag_stream);
		virtual void DestroyDecoder();
		virtual int Decode(const FragItem *frag_item_ary, int frag_count,
			int *width, int *height, int *is_key_frame);

	private:
		CodecID	codecId_;
		AVCodec	*codec_;			///< AVCodec descriptor
		AVCodecContext *context_;	///< AVCodec codec context
		AVFrame *frame_;			///< Decoded frame
		AVDictionary *opts_;		///< Decoder options
		DepacketiseFunc depacketise_;
	};

} // namespace ew

#endif // ! VIDEO_DECODER_FFMPEG_H_