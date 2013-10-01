#ifndef FRAG_ACCUMULATOR_H_
#define FRAG_ACCUMULATOR_H_

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// FragAccumulator
	//////////////////////////////////////////////////////////////////////

	class FragAccumulator
	{
	public:
		explicit FragAccumulator(int maxsize);
		bool Accumulate(int pos, int size, int gob_n); // true position is correct and there is enough space
		void Clear();
		int GetStartPos() const { return start_pos_; }
		int GetStartGobN() const { return start_gob_n_; }
		int GetTotalSize() const { return next_pos_ - start_pos_; }

	private:
		int maxsize_;
		int start_pos_;
		int next_pos_;
		int start_gob_n_; // used in h.263
	};

} // namespace ew

#endif // ! FRAG_ACCUMULATOR_H_
