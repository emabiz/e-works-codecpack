#ifndef BITSTREAMER_H_
#define BITSTREAMER_H_

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// BitWriter
	//////////////////////////////////////////////////////////////////////

	class BitWriter
	{
	public:
		BitWriter(unsigned char *buffer, int buffer_byte_size);

		int buffer_byte_size() const { return buffer_byte_size_; }
		const unsigned char *buffer() const { return buffer_; }
		int next_bit_in_byte() const { return next_bit_in_byte_; }
		int byte_pos() const { return byte_pos_; }

		bool PutByteBits(unsigned char bits, int n_bits);
		//bool PutLongBits(unsigned long bits, int n_bits);

	private:
		int buffer_byte_size_;
		unsigned char *buffer_;
		int next_bit_in_byte_;	// first available bit position (0-7)
		int byte_pos_;			// first available byte position (0-buffer_byte_size_)
	};


	//////////////////////////////////////////////////////////////////////
	// BitReader
	//////////////////////////////////////////////////////////////////////

	class BitReader
	{
	public:
		BitReader(const unsigned char *buffer, int buffer_byte_size);
		int buffer_byte_size() const { return buffer_byte_size_; }
		const unsigned char *buffer() const { return buffer_; }
		int next_bit_in_byte() const { return next_bit_in_byte_; }
		int byte_pos() const { return byte_pos_; }

		bool GetByteBits(unsigned char *bits, int n_bits);
		//bool GetLongBits(unsigned long *bits, int n_bits);

	private:
		int buffer_byte_size_;
		const unsigned char *buffer_;
		int next_bit_in_byte_;	// next bit to be read position (0-7)
		int byte_pos_;			// next byte to be read position (0-buffer_byte_size_)
	};

} // namespace ew

#endif // ! BITSTREAMER_H_