#ifndef VIDEO_ENCODER_FFMPEG_H_
#define VIDEO_ENCODER_FFMPEG_H_

#include "video_codec.h"

enum CodecID;

namespace ew
{
	class ParamArchiveVideo;
	class Fragmenter;

	//////////////////////////////////////////////////////////////////////
	// VideoEncoderFfmpeg
	//////////////////////////////////////////////////////////////////////

	class VideoEncoderFfmpeg : public VideoEncoder
	{
	public:
		typedef void (*RtpCallback)(struct AVCodecContext *avctx, void *data, int size, int mb_nb);

		explicit VideoEncoderFfmpeg(ParamArchiveVideo *param_video);
		virtual ~VideoEncoderFfmpeg();

		virtual bool InitEncoder(int bitrate, int fps, int width, int height,
			int fragsize, int *fragcount, const char *enc_params);
		virtual void DestroyEncoder();
		virtual unsigned char *GetInputBuffer() const;
		virtual int Encode(int force_key_frame, int *is_key_frame);

		virtual CodecID GetCodecId() const = 0;
		virtual int GetTsto() const = 0;
		virtual int GetFlags() const = 0;
		virtual void *GetOpaque(); // returns NULL
		virtual RtpCallback GetRtpCallback() const; // returns NULL
		virtual Fragmenter *GetFragmenter(); // returns NULL

	private:
		Fragmenter *fragmenter_;
		AVCodec	*codec_;			///< AVCodec descriptor
		AVCodecContext *context_;	///< AVCodec codec context
		AVFrame *frame_;			///< Encoded frame
		AVDictionary *opts_;		///< Encoder options
		unsigned char *input_buffer_; ///< Source frame buffer
		int input_buffer_size_;		///< Source frame buffer size
	};

} // namespace ew

#endif // ! VIDEO_ENCODER_FFMPEG_H_
