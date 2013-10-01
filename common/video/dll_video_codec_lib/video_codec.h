#ifndef VIDEO_CODEC_H_
#define VIDEO_CODEC_H_

struct FragItem;

namespace ew
{
	class FragWriter;
	class ParamArchiveVideo;

	//////////////////////////////////////////////////////////////////////
	// VideoEncoder
	//////////////////////////////////////////////////////////////////////

	class VideoEncoder
	{
	public:
		virtual ~VideoEncoder();

		int bitrate() const { return bitrate_; }
		int fps() const { return fps_; }
		int width() const { return width_; }
		int height() const { return height_; }
		int fragsize() const { return fragsize_; }
		int *fragcount() const { return fragcount_;}
		int output_buffer_min_size() const { return output_buffer_min_size_; }
		const ParamArchiveVideo *param_video() const { return param_video_; }
		FragWriter *frag_writer() const { return frag_writer_; }

		virtual bool InitEncoder(int bitrate, int fps, int width, int height,
			int fragsize, int *fragcount, const char *enc_params);
		virtual void DestroyEncoder();
		virtual unsigned char *GetInputBuffer() const = 0;
		virtual int Encode(int force_key_frame, int *is_key_frame) = 0;

	protected:
		VideoEncoder(int output_buffer_min_size, ParamArchiveVideo *param_video); // takes pointer ownership
		unsigned char *output_buffer() const { return output_buffer_; }
		int output_buffer_size() const { return output_buffer_size_; }

	private:
		void SetDefaults();

		int output_buffer_min_size_;
		ParamArchiveVideo *param_video_;
		int bitrate_; // -1 if obj not initialized
		int fps_;
		int width_;
		int height_;
		int fragsize_;
		int *fragcount_;
		unsigned char *output_buffer_;
		int output_buffer_size_;
		FragWriter *frag_writer_;
	};


	//////////////////////////////////////////////////////////////////////
	// VideoDecoder
	//////////////////////////////////////////////////////////////////////

	class VideoDecoder
	{
	public:
		virtual ~VideoDecoder();

		int max_width() const { return max_width_; }
		int max_height() const { return max_height_; }
		bool is_frag_stream() const { return is_frag_stream_; }
		int input_buffer_padding() const { return input_buffer_padding_; }
		unsigned char *input_buffer() const { return input_buffer_; }
		int input_buffer_size() const { return input_buffer_size_; }
		unsigned char *output_buffer() const { return output_buffer_; }

		virtual bool InitDecoder(int max_width, int max_height, int is_frag_stream);
		virtual void DestroyDecoder();
		virtual int Decode(const FragItem *frag_item_ary, int frag_count,
			int *width, int *height, int *is_key_frame);

	protected:
		explicit VideoDecoder(int input_buffer_padding);

	private:
		void SetDefaults();

		int input_buffer_padding_;
		int max_width_; // -1 if obj not initialized
		int max_height_;
		bool is_frag_stream_;
		unsigned char *input_buffer_;
		int input_buffer_size_;
		unsigned char *output_buffer_;
	};

	//////////////////////////////////////////////////////////////////////
	// VideoCodec
	//////////////////////////////////////////////////////////////////////

	class VideoCodec
	{
	public:
		virtual ~VideoCodec();
		VideoEncoder *encoder() const { return encoder_; }
		VideoDecoder *decoder() const { return decoder_; }

		void DestroyEncoder();
		void DestroyDecoder();

		// user dll methods
		int UserEncInit(unsigned char **EncoderInput,
          int Bitrate, int Fps, int Width, int Height,
          const char *Params,
          int fragSize, struct FragItem **fragItem, int *fragCount);
		int UserEncode(int forceKeyFrame, int *isKeyFrame);

		int UserDecInit(unsigned char **DecoderOutput,
          int PictureW, int PictureH, int IsFragStream);
		int UserDecode(struct FragItem *fragItem, int fragCount,
			int *w, int *h, int *isKeyFrame);

	protected:
		VideoCodec();
		virtual VideoEncoder *DoCreateEncoder() = 0;
		virtual VideoDecoder *DoCreateDecoder() = 0;

	private:
		VideoEncoder *encoder_;
		VideoDecoder *decoder_;

		bool InitEncoder(int bitrate, int fps, int width, int height,
			int fragsize, int *fragcount, const char *enc_params);
		bool InitDecoder(int max_width, int max_height, int is_frag_stream);
	};

} // namespace ew

#endif // !VIDEO_CODEC_H_