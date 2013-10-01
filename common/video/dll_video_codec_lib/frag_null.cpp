#include "stdafx.h"

#include "frag_null.h"
#include "frag_helper.h"
#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	int DepacketiseNull(const FragItem *frag_item_ary, int frag_count,
		unsigned char *input_buffer, int input_buffer_size)
	{
		int size = 0;

		for (int i = 0; i < frag_count; ++i) {
			const FragItem *frag_item = &frag_item_ary[i];
			if (static_cast<int>(frag_item->size) >= input_buffer_size - size)
				return 0;
			memcpy(&input_buffer[size], frag_item->data, frag_item->size);
			size += frag_item->size;
		}

		return size;
	}

} // namespace ew
