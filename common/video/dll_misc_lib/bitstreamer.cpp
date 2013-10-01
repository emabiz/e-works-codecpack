#include "stdafx.h"

#include "bitstreamer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// BitWriter
	//////////////////////////////////////////////////////////////////////

	BitWriter::BitWriter(unsigned char *buffer, int buffer_byte_size)
		: buffer_(buffer),
		buffer_byte_size_(buffer_byte_size),
		next_bit_in_byte_(0),
		byte_pos_(0)
	{
		_ASSERT(NULL != buffer);
		_ASSERT(buffer_byte_size > 0);
		memset(buffer_, 0, buffer_byte_size_);
	}

	bool BitWriter::PutByteBits(unsigned char bits, int n_bits)
	{
		_ASSERT(1 <= n_bits && n_bits <= 8);
		
		// numeric value must be inside specified bit number range
		if (bits >= (1 << n_bits))
			return false;

		int bit_end = next_bit_in_byte_ + n_bits; // first available position for next step
		if (bit_end > 8) {
			bit_end -= 8;
			if (byte_pos_ + 1 >= buffer_byte_size_)
				return false;
			unsigned char l_mask = 0xff << (8 - next_bit_in_byte_);
			unsigned char r_mask = 0xff >> bit_end;
			unsigned char l_value = bits >> (next_bit_in_byte_ - (8 - n_bits));
			unsigned char r_value = bits << (8 - bit_end);
			
			buffer_[byte_pos_] = (buffer_[byte_pos_] & l_mask) | l_value;
			++byte_pos_;
			buffer_[byte_pos_] = (buffer_[byte_pos_] & r_mask) | r_value;
			next_bit_in_byte_ = bit_end;
		} else {
			unsigned char mask = 0xff << (8 - next_bit_in_byte_);
			unsigned char value = bits << (8 - bit_end);
			buffer_[byte_pos_] =  (buffer_[byte_pos_] & mask) | value;
			next_bit_in_byte_ += n_bits;
			if (8 == next_bit_in_byte_) {
				next_bit_in_byte_  = 0;
				++byte_pos_;
			}
		}

		return true;
	}


	//////////////////////////////////////////////////////////////////////
	// BitReader
	//////////////////////////////////////////////////////////////////////

	BitReader::BitReader(const unsigned char *buffer, int buffer_byte_size)
		: buffer_(buffer),
		buffer_byte_size_(buffer_byte_size),
		next_bit_in_byte_(0),
		byte_pos_(0)
	{
		_ASSERT(NULL != buffer);
		_ASSERT(buffer_byte_size > 0);
	}

	bool BitReader::GetByteBits(unsigned char *bits, int n_bits)
	{
		_ASSERT(NULL != bits);
		_ASSERT(1 <= n_bits && n_bits <= 8);
	
		int bit_end = next_bit_in_byte_ + n_bits; // next position to be read on next step
		if (bit_end > 8) {
			bit_end -= 8;
			if (byte_pos_ + 1 >= buffer_byte_size_)
				return false;
			unsigned char l_mask = 0xff << (8 - next_bit_in_byte_);
			unsigned char r_mask = 0xff >> bit_end;
			unsigned char l_value = buffer_[byte_pos_] & ~l_mask;
			++byte_pos_;
			unsigned char r_value = buffer_[byte_pos_] & ~r_mask;

			*bits = (l_value << (next_bit_in_byte_ - (8 - n_bits)))
				| (r_value >> (8 - bit_end));
			next_bit_in_byte_ = bit_end;
		} else {
			unsigned char mask = 0xff << (8 - next_bit_in_byte_);
			unsigned char value = buffer_[byte_pos_] & ~mask;
			*bits = value >> (8 - bit_end);
			next_bit_in_byte_ += n_bits;
			if (8 == next_bit_in_byte_) {
				next_bit_in_byte_  = 0;
				++byte_pos_;
			}
		}

		return true;
	}

} // namespace ew
