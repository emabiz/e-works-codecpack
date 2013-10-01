#ifndef FRAGMENTER_H_
#define FRAGMENTER_H_

namespace ew {

	class FragWriter;
	
	class Fragmenter
	{
	public:
		Fragmenter(int fragsize, FragWriter *fragwriter);

		int fragsize() const { return fragsize_; }
		FragWriter *fragwriter() const { return fragwriter_; }

		virtual int Fragment(const unsigned char *src, int src_size) = 0;

	private:
		int fragsize_;
		FragWriter *fragwriter_;
	};

} // namespace ew

#endif // ! FRAGMENTER_H_