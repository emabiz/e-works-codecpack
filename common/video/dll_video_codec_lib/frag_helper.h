#ifndef FRAG_HELPER_H_
#define FRAG_HELPER_H_

struct FragItem;

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// FragReader
	//////////////////////////////////////////////////////////////////////

	class FragReader
	{
	public:
		FragReader(const FragItem *frag_ary, int frag_count);

		int frag_count() const { return frag_count_; }
		int get_item_size(int n) const;
		const unsigned char *get_item_data(int n) const;

	private:
		const FragItem *frag_ary_;
		int frag_count_;
	};


	//////////////////////////////////////////////////////////////////////
	// FragWriter
	//////////////////////////////////////////////////////////////////////

	class FragWriter
	{
	public:
		explicit FragWriter(int frag_ary_max_size);
		~FragWriter();

		int frag_ary_max_size() const { return frag_ary_max_size_; }
		int frag_count() const { return frag_count_; }
		int frag_total_size() const { return frag_total_size_; }
		FragItem *frag_ary() const { return frag_ary_; }
		bool AppendItem(const unsigned char *data, int size);
		void Clear();

	private:
		int frag_ary_max_size_;
		int frag_count_;
		int frag_total_size_;
		FragItem *frag_ary_;
	};

} // namespace ew

#endif // ! FRAG_HELPER_H_

