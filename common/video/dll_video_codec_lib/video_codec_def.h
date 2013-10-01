#ifndef VIDEO_CODEC_DEF_H_
#define VIDEO_CODEC_DEF_H_

enum {
	kMemAlign = 16,
	kBitrateMin = 1000, // 1kbit/s
	kFpsMin = 1,
	kWidthMin = 16,
	kHeightMin = 16,
	kFragsizeMin = 100, // byte
	kWidthStep = 16,
	kHeightStep = 8
};

#endif // ! VIDEO_CODEC_DEF_H_