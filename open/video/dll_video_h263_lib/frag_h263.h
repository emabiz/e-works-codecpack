/*****************************************************************************
 * H263 fragmentation
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

#ifndef FRAG_H263_H_
#define FRAG_H263_H_

#include "fragmenter.h"

struct FragItem;

namespace ew {

	class FragWriter;

	enum {
		kHdrModeALen = 4,
		kHdrModeBLen = 8,
		kHdrModeCLen = 12,
	};

	int H263DepacketiseRfc2190(const FragItem *frag_item_ary, int frag_count,
		unsigned char *input_buffer, int input_buffer_size);

	//////////////////////////////////////////////////////////////////////
	// H263HeaderDecoder
	//////////////////////////////////////////////////////////////////////

	class H263HeaderDecoder
	{
	public:
		H263HeaderDecoder();

		bool Decode(unsigned const char *src, int src_size);

		unsigned int tr() const { return tr_; }
		unsigned int frame_size() const { return frame_size_; }
		bool i_frame() const { return i_frame_; }
		bool annex_d() const { return annex_d_; }
		bool annex_e() const { return annex_e_; }
		bool annex_f() const { return annex_f_; }
		bool annex_g() const { return annex_g_; }
		int p_quant() const { return p_quant_; }
		bool cpm() const { return cpm_; }
		int macroblocks_per_pic() const { return macroblocks_per_pic_; }
		bool is_valid() const { return is_valid_; }

	private:
		unsigned int tr_;
		unsigned int frame_size_;
		bool i_frame_;
		bool annex_d_, annex_e_, annex_f_, annex_g_;
		int p_quant_;
		bool cpm_;
		int macroblocks_per_pic_;
		bool is_valid_;
	};



	//////////////////////////////////////////////////////////////////////
	// FragmenterVideoH263_Rfc2190
	//////////////////////////////////////////////////////////////////////

	class FragmenterVideoH263_Rfc2190 : public Fragmenter
	{
	public:
		FragmenterVideoH263_Rfc2190(int fragsize, FragWriter *fragwriter, bool split_oversize);

		bool split_oversize() const { return split_oversize_; }

		virtual int Fragment(const unsigned char *src, int src_size);
		int PackModeA(const H263HeaderDecoder *hdr, const unsigned char *src, int src_size);
		int PackModeB(const H263HeaderDecoder *hdr, const unsigned char *src, int src_size);
		
	private:
		bool split_oversize_;

		const char *curr_src_;				// buffer encodato corrente
		int curr_src_size_;					// dimensione buffer encodato corrente

		void AccumulateGob(const unsigned char *gob_src, int gob_size);
	};

} // namespace ew

#endif // ! FRAG_H263_H_
