#ifndef DLL_DEF_H_
#define DLL_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Max number of fragments to subdivide a coded frame.
	For many codecs (such as h264) there aren't limits, but
	a fixed maximum simplifies implementation
*/
#define MAX_PACKETS_PER_FRAME 512

/** Fragment descriptor for coded frame.
	Used for both fragmented streams and not-fragmented streams */
struct FragItem {
	unsigned char *data; ///< pointer to encoded fragment
	unsigned int size;   ///< encoded fragment size
};

#ifdef __cplusplus
}
#endif

#endif // DLL_DEF_H_
