/*****************************************************************************
 * H263 framentation
 *****************************************************************************
 * Copyright (C) 2010-2011 Yuri Valentini
 *
 * Authors: Yuri Valentini <yv@opycom.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *****************************************************************************/

#include "stdafx.h"

#include "frag_h263.h"
#include "frag_accumulator.h"
#include "frag_helper.h"
#include "video_codec_def.h"
#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int kPscLen = 3;
const int kGbscLen = 3;

// from rfc2190.cxx of opal 3.8.0
static int kMacroblocksPerPicTable[] = {
    -1,  // forbidden
    -1,  // sub-QCIF
     (144 / 16) *  (176 / 16),  //  QCIF = 99
     (288 / 16) *  (352 / 16),  //  CIF  = 396
     (576 / 32) *  (704 / 32),  //  4CIF = 396
    (1408 / 64) * (1152 / 64),  //  16CIF = 396
    -1,  // Reserved
    -1   // extended
};

static int _FindPsc(const unsigned char *buf, int size)
{
	// Picture Start Code = 0000 0000 0000 0000 1 00000 (22bit)
	_ASSERT(NULL != buf);
	_ASSERT(size > kPscLen);
	
	for (int i = 0; i < size-kPscLen; ++i) {
		if (buf[i] == 0	&& buf[i+1] == 0 && (buf[i+2] & 0xfc) == 0x80)
			return i;
	}

	return -1;
}

static int _FindGbsc(const unsigned char *buf, int size, int start_pos)
{
	// Group of Blocks Start Code = 0000 0000 0000 0000 1 (17 bit)
	_ASSERT(NULL != buf);
	_ASSERT(size > kGbscLen);
	_ASSERT(start_pos < size);
	
	for (int i = start_pos; i < size-kGbscLen; ++i) {
		if (buf[i] == 0	&& buf[i+1] == 0 && (buf[i+2] & 0x80) == 0x80)
			return i;
	}

	return -1;
}

namespace ew {

	int H263DepacketiseRfc2190(const FragItem *frag_item_ary, int frag_count,
		unsigned char *input_buffer, int input_buffer_size)
	{
		_ASSERT(NULL != frag_item_ary);
		_ASSERT(frag_count > 0);

#ifdef _DEBUG
		for (int i = 0; i < frag_count; ++i) {
			_ASSERT(NULL != frag_item_ary[i].data && frag_item_ary[i].size > 0);
		}
#endif

		unsigned char *dst = input_buffer;
		int dst_size = input_buffer_size;

		FragWriter fu_writer(MAX_PACKETS_PER_FRAME);

		int last_ebit = 0;
		for (int i = 0; i < frag_count; ++i) {
			const FragItem *frag_item = &frag_item_ary[i];
			if (frag_item->size < kHdrModeALen) { /* at least 4 bytes for mode A header */
				_ASSERT(0);
				return 0;
			}
			int f = (frag_item->data[0] >> 7) & 1;
			int p = (frag_item->data[0] >> 6) & 1;
			int sbit = (frag_item->data[0] >> 3) & 7;
			int ebit = frag_item->data[0] & 7;

			// if sbit and last_ebit don't complete, then we are in trouble
			if (((sbit + last_ebit) & 7) != 0) {
				_ASSERT(0);
				return 0;
			}

			// ignore the rest
			int skip = 0;
			if (0 == f) {
				skip = kHdrModeALen; // mode A
			} else {
				if (0 == p) {
					skip = kHdrModeBLen; // mode B
				} else {
					skip = kHdrModeCLen; // mode C
				}
			}

			if (static_cast<int>(frag_item->size) < skip+1) { /* at least header bytes and a byte for payload */
				_ASSERT(0);
				return 0;
			}

			if (0 != sbit) {
				_ASSERT(dst > input_buffer);
				unsigned char mask = 0xff >> (8 - sbit);
				dst[-1] |= frag_item->data[skip] & mask;
				++skip;
			}

			if (dst_size < static_cast<int>(frag_item->size) - skip) {
				_ASSERT(0);
				return 0;
			}

			int size = frag_item->size - skip;
			memcpy(dst, &frag_item->data[skip], size);

			last_ebit = ebit;
			dst += size;
			dst_size -= size;
		}

		return input_buffer_size - dst_size;
	}


	//////////////////////////////////////////////////////////////////////
	// H263HeaderDecoder
	//////////////////////////////////////////////////////////////////////

	H263HeaderDecoder::H263HeaderDecoder()
		: tr_(0), frame_size_(0), i_frame_(0),
		annex_d_(0), annex_e_(0), annex_f_(0), annex_g_(0),
		p_quant_(0), cpm_(0), macroblocks_per_pic_(0),
		is_valid_(false)
	{
	}

	// from rfc2190.cxx of opal 3.8.0
	bool H263HeaderDecoder::Decode(unsigned const char *src, int src_size)
	{
		_ASSERT(NULL != src);
		_ASSERT(src_size >= 7);

		is_valid_ = false;

		// frame strarts with PSC
		if (0 != _FindPsc(src, src_size))
			return false;

		// get TR
		//     2         3    
		// .... ..XX XXXX XX..
		tr_ = ((src[2] << 6) & 0xfc) | (src[3] >> 2);

		// make sure mandatory part of PTYPE is present
		//     3    
		// .... ..10
		if ((src[3] & 0x03) != 2)
			return false;

		// we don't do split screen, document indicator, full picture freeze
		//     4    
		// XXX. ....
		if ((src[4] & 0xe0) != 0)
			return false;

		// get image size
		//     4
		// ...X XX..
		frame_size_ = (src[4] >> 2) & 0x7;
		macroblocks_per_pic_ = kMacroblocksPerPicTable[frame_size_];
		if (macroblocks_per_pic_ == -1)
			return false;

		// get I-frame flag
		//     4
		// .... ..X.
		i_frame_= (src[4] & 2) == 0;

		// get annex bits:
		//   Annex D - unrestricted motion vector mode
		//   Annex E - syntax-based arithmetic coding mode
		//   Annex F - advanced prediction mode
		//   Annex G - PB-frames mode
		//
		//     4         5
		// .... ...X XXX. ....
		annex_d_ = 0 != (src[4] & 0x01);
		annex_e_ = 0 != (src[5] & 0x80);
		annex_f_ = 0 != (src[5] & 0x40);
		annex_g_ = 0 != (src[5] & 0x20);

		// annex G not supported 
		if (annex_g_)
			return false;

		// get PQUANT
		//     5
		// ...X XXXX
		p_quant_ = src[5] & 0x1f;

		// get CPM
		//     6
		// X... ....
		cpm_ = (src[6] & 0x80) != 0;

		// ensure PEI is always 0
		//     6
		// .X.. ....
		if ((src[6] & 0x40) != 0)
			return false;

		is_valid_ = true;
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// FragmenterVideoH263_Rfc2190
	//////////////////////////////////////////////////////////////////////

	FragmenterVideoH263_Rfc2190::FragmenterVideoH263_Rfc2190(int fragsize,
		FragWriter *fragwriter, bool split_oversize)
		: Fragmenter(fragsize, fragwriter), split_oversize_(split_oversize)
	{
	}

	int FragmenterVideoH263_Rfc2190::Fragment(const unsigned char *src, int src_size)
	{
		H263HeaderDecoder hdr;
		if (!hdr.Decode(src, src_size)) {
			_ASSERT(0);
			return 0;
		}

		// fragmentation
		if (src_size + kHdrModeALen <= fragsize())
			return PackModeA(&hdr, src, src_size);

		// Mode B
		// TODO: accumulate gobs until fragment size and determine when to fragment in mode C

		FragAccumulator gob_acc(fragsize()-kHdrModeBLen);

		fragwriter()->Clear();
		int size = 0;
		int gob_pos = 0;
		for (int next_gob_pos = _FindGbsc(src, src_size, kPscLen);
			gob_pos >= 0; next_gob_pos = _FindGbsc(src, src_size, gob_pos+kGbscLen))
		{
			int gob_size = next_gob_pos > 0 ? next_gob_pos-gob_pos : src_size-gob_pos;
			int gob_n = (src[gob_pos+2] >> 2) & 0x1f;
			if (!gob_acc.Accumulate(gob_pos, gob_size, gob_n)) {
				// output fragments
				if (gob_acc.GetTotalSize() == 0) {
					// single gob with size exceeding fragsize
					// TODO: report anomaly
					if (split_oversize_)
						size += PackModeB(&hdr, &src[gob_pos], gob_size);
					else
						size += PackModeA(&hdr, &src[gob_pos], gob_size);
				} else {
					// output fragments
					size += PackModeA(&hdr, &src[gob_acc.GetStartPos()], gob_acc.GetTotalSize());
					gob_acc.Clear();
					if (!gob_acc.Accumulate(gob_pos, gob_size, gob_n)) {
						// singole gob with size exceeding fragsize
						// TODO: report anomaly
						if (split_oversize_)
							size += PackModeB(&hdr, &src[gob_pos], gob_size);
						else
							size += PackModeA(&hdr, &src[gob_pos], gob_size);
					}
				}
			}
			gob_pos = next_gob_pos;
		}

		if (gob_acc.GetTotalSize() > 0) {
			// output remaining fragments
			size += PackModeA(&hdr, &src[gob_acc.GetStartPos()], gob_acc.GetTotalSize());
		}
		return size;
	}

	// modified from rfc2190.cxx RFC2190Packetizer::Open of opal 3.8.0
	int FragmenterVideoH263_Rfc2190::PackModeA(const H263HeaderDecoder *hdr,
		const unsigned char *src, int src_size)
	{
		_ASSERT(NULL != hdr);
		_ASSERT(hdr->is_valid());
		_ASSERT(NULL != src);
		_ASSERT(src_size > 0);
		int size = kHdrModeALen + src_size;
		_ASSERT(size <= fragsize()); // sometimes the encoder is not able to create gob under a certain size
		unsigned char *buf = new unsigned char[size];
		memset(buf, 0, kHdrModeALen);
	    buf[1] = static_cast<unsigned char>((hdr->frame_size() << 5) | (hdr->i_frame() ? 0 : 0x10)
			| (hdr->annex_d() ? 0x08 : 0) | (hdr->annex_e() ? 0x04 : 0) | (hdr->annex_f() ? 0x02 : 0));
		memcpy(&buf[kHdrModeALen], src, src_size);
		_VERIFY(fragwriter()->AppendItem(buf, size));
		delete buf;
		return size;
	}

	int FragmenterVideoH263_Rfc2190::PackModeB(const H263HeaderDecoder *hdr,
		const unsigned char *src, int src_size)
	{
		_ASSERT(NULL != hdr);
		_ASSERT(hdr->is_valid());
		_ASSERT(NULL != src);
		_ASSERT(src_size > 3);
		int size = kHdrModeBLen + src_size;
		_ASSERT(size <= fragsize() || (size > fragsize() && !split_oversize_));
		unsigned char *buf = new unsigned char[size];
		memset(buf, 0, kHdrModeBLen);
		int mba = 0; // split at gob boundary
		int gob_n = 0;
		if (_FindPsc(src, size) < 0) // first gob has only psc, but not gbsc
			gob_n = (src[3] >> 2) & 0x1f;
			
		buf[0] = 0x80; // f=1 p=1
		buf[1] = static_cast<unsigned char>(hdr->frame_size() << 5);
		buf[2] = static_cast<unsigned char>(((gob_n << 3) & 0xf8) | ((mba >> 6) & 0x7));
		buf[3] = (unsigned char)((mba << 2) & 0xfc);
		buf[4] = (hdr->i_frame() ? 0 : 0x80) | (hdr->annex_d() ? 0x40 : 0)
			| (hdr->annex_e() ? 0x20 : 0) | (hdr->annex_f() ? 0x010: 0);
		memcpy(&buf[kHdrModeBLen], src, src_size);
		_VERIFY(fragwriter()->AppendItem(buf, size));
		delete buf;
		return size;
	}

} // namespace ew
