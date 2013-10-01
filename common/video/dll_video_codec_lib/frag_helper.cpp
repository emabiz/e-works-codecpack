#include "stdafx.h"

#include "frag_helper.h"
#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// FragReader
	//////////////////////////////////////////////////////////////////////

	FragReader::FragReader(const FragItem *frag_ary, int frag_count)
		: frag_ary_(frag_ary),
		frag_count_(frag_count)
	{
		_ASSERT(frag_ary);
		_ASSERT(frag_count > 0);
	}

	int FragReader::get_item_size(int n) const
	{
		_ASSERT(0 <= n && n < frag_count_);
		return frag_ary_[n].size;
	}

	const unsigned char *FragReader::get_item_data(int n) const
	{
		_ASSERT(0 <= n && n < frag_count_);
		return frag_ary_[n].data;
	}


	//////////////////////////////////////////////////////////////////////
	// FragWriter
	//////////////////////////////////////////////////////////////////////

	FragWriter::FragWriter(int frag_ary_max_size)
		: frag_ary_max_size_(frag_ary_max_size), frag_count_(0), frag_total_size_(0)
	{
		_ASSERT(frag_ary_max_size > 0);
		frag_ary_ = new FragItem[frag_ary_max_size];
	}

	FragWriter::~FragWriter()
	{
		Clear();
		delete [] frag_ary_;
	}

	bool FragWriter::AppendItem(const unsigned char *data, int size)
	{
		_ASSERT(data);
		_ASSERT(size > 0);

		if (frag_count_ == frag_ary_max_size_)
			return false;

		unsigned char *p = new unsigned char[size];
		memcpy(p, data, size);
		frag_ary_[frag_count_].data = p;
		frag_ary_[frag_count_].size = size;
		++frag_count_;
		frag_total_size_ += size;
		return true;
	}

	void FragWriter::Clear()
	{
		for (int i = frag_count_ - 1; i >= 0; --i)
			delete [] frag_ary_[i].data;
		frag_count_ = 0;
		frag_total_size_ = 0;
	}

} // namespace ew
