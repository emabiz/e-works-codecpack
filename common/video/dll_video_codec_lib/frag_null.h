#ifndef FRAG_NULL_H_
#define FRAG_NULL_H_

struct FragItem;

namespace ew {

	class FragWriter;

	int DepacketiseNull(const FragItem *frag_item_ary, int frag_count,
		unsigned char *input_buffer, int input_buffer_size);

} // namespace ew

#endif // ! FRAG_NULL_H_
