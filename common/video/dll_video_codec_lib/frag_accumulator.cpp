#include "stdafx.h"
#include "frag_accumulator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// FragAccumulator
	//////////////////////////////////////////////////////////////////////

	FragAccumulator::FragAccumulator(int maxsize)
		: maxsize_(maxsize) 
	{
		_ASSERT(maxsize > 0);
		Clear();
	}

	bool FragAccumulator::Accumulate(int pos, int size, int gob_n)
	{
		_ASSERT(pos >= 0);
		_ASSERT(size > 0);

		if (GetTotalSize() + size > maxsize_)
			return false;

		if (start_pos_ < 0) {
			start_pos_ = pos;
			start_gob_n_ = gob_n;
		} else {
			if (next_pos_ != pos)
				return false;
			if (start_gob_n_ > gob_n)
				return false;
		}

		next_pos_ = pos + size;
		return true;
	}

	void FragAccumulator::Clear()
	{
		start_pos_ = -1;
		next_pos_ = -1;
		start_gob_n_ = -1;
	}

} // namespace ew
