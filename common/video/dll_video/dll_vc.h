#ifndef DLL_VC_H_
#define DLL_VC_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DLL_VC_EXPORTS
#define DLL_VC_API __declspec(dllexport)
#else
#define DLL_VC_API __declspec(dllimport)
#endif

typedef struct _VideoCodec *VideoHandle;

/**@name Video codec info */
//@{
/** Returns name of video codec
	\return video codec name string */
DLL_VC_API const char *GetName(void);

/** Returns API version implemented by dll 
	\return API version (at present is \a 2) */
DLL_VC_API int GetApiVersion(void);

#define INVALID_PAYLOAD 127
/** Returns RFC payload type value to be used inside rtp packets
	\return payload type value or #INVALID_PAYLOAD for dynamic payload type or payload type not specified in RFC */
DLL_VC_API unsigned char GetPayload(void);

/** Returns string of default values of encoder parameters; the string format is
	\a key=value with \a ";" delimiter. The parameters configure the encoder in function #EncInit
	\return default values string */
DLL_VC_API const char *GetParamsDefault(void);

/** Checks availabily of fragmented stream creation support for the encoder
	\return \a 0 if encoder doesn't support creation of fragmented streams */
DLL_VC_API int FragSupport(void);

/** Checks decoder capability of dealing with missing rtp fragments
	\return \a 0 if decoder doesn't support missing rtp fragments (all the fragments should be dropped) */
DLL_VC_API int IncompleteSupport(void);
//@}

/**@name Video codec init and destroy */
//@{
/** Initializes data structures for video streams encoding end decoding.
	This function returns a handle needed for encoding and decoding functions
    \return handle to video codec context descriptor or 0 if error */
DLL_VC_API VideoHandle NewCodec(void);

/** Destroys video codec and releases memory for the context descriptor.
    After calling this function, the handle is no longer valid
    \param[in] handle handle to context descriptor */
DLL_VC_API void DeleteCodec(VideoHandle handle);
//@}

/**@name Video encoding and decoding */
//@{

/** Allocates and configures data structures for encoding video streams;
    also fragmented video streams can be created (if supported by codec implementation).
    After calling this #Encode function can be used to encode frames
    \param[in] handle handle to context descriptor
    \param[out] EncoderInput buffer pointer where to store a frame to encode in yuv420p format.
		This function allocates the buffer.
    \param[in] Bitrate target bitrate in bit/s for the encoded stream (for bandwidth rate control)
    \param[in] Fps number of frames per second that are pushed inside the encoder (for bandwidth rate control)
    \param[in] Width frame width in pixel
    \param[in] Height frame height in pixel
    \param[in] Params video encoder parameters (\a NULL for default values). See #GetParamsDefault for string format
    \param[in] fragSize maximum fragment size in bytes encoder should generate (\a 0 for non fragmented streams)
    \param[out] fragItem array pointer to contain fragments structures (allocated by this function)
    \param[in] fragCount pointer to the variable to contain number of fragments (allocated by the caller)
    \return \a 0 on error */
DLL_VC_API int EncInit(VideoHandle handle,
          unsigned char **EncoderInput,
          int Bitrate, int Fps, int Width, int Height,
          const char *Params,
          int fragSize, struct FragItem **fragItem, int *fragCount);

/** Destroys encoder and releases encoder memory
    \param[in] handle handle to context descriptor */
DLL_VC_API void EncFree(VideoHandle handle);

/** Encodes frame pointed to #EncInit allocated buffer.
	Encoded frame is stored into #EncInit fragments array; for non fragmented streams,
	encoder uses only the first element of array
	\param[in] handle handle to context descriptor
    \param[in] forceKeyFrame not 0 if a key frame is to be encoded
    \param[out] isKeyFrame pointer to the variable to contain \a 1 if encoded frame is a key frame or else \a 0 (\a NULL if info is not needed)
    \return encoded frame size in bytes (sum of fragments size), \a 0 on error */
DLL_VC_API int Encode(VideoHandle handle, int forceKeyFrame, int *isKeyFrame);

/** Allocates and configures data structures for video stream decoding.
    After calling this, #Decode function can be used to decode frames.
    \param[in] handle handle to context descriptor
    \param[out] DecoderOutput buffer pointer where decoded frame in yuv420p format is stored.
		This function allocates the buffer
    \param[in] PictureW maximum width in pixel of frames to decode (for output buffer size allocation)
    \param[in] PictureH maximum height in pixel of frames to decode (for output buffer size allocation)
	\param[in] IsFragStream \a 0 for non fragmented streams
    \return \a 0 on error */
DLL_VC_API int DecInit(VideoHandle handle,
          unsigned char **DecoderOutput,
          int PictureW, int PictureH, int IsFragStream);

/** Destroys decoder and releases decoder memory.
    \param[in] handle handle to context descriptor */         
DLL_VC_API void DecFree(VideoHandle handle);

/** Decodes a frame storing it in #DecInit allocated buffer.
    \param[in] handle handle to context descriptor
    \param[in] fragItem pointer to first element of fragment descriptor array (for non fragmented stream there is a single item)
    \param[in] fragCount number of items in fragment array (\a 1 for non fragmented streams)
    \param[out] w pointer to variable to contain width of decoded frame (\a NULL if info is not needed)
    \param[out] h pointer to variable to contain height of decoded frame (\a NULL if info is not needed)
    \param[out] isKeyFrame to variable to contain \a 1 if decoded frame is a key frame or else \a 0 (\a NULL if info is not needed)
    \return size in bytes of decoded frame, or \a 0 on error */
DLL_VC_API int Decode(VideoHandle handle,
          struct FragItem *fragItem, int fragCount, int *w, int *h, int *isKeyFrame);
//@}

#ifdef __cplusplus
}
#endif

#endif // DLL_vc_H_