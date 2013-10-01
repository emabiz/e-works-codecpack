/*****************************************************************************
 * H264 fragmentation
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

#ifndef FRAG_H264_H_
#define FRAG_H264_H_

extern "C" {
// x264
#define _INTTYPES_H_
#include "x264.h"
}

struct FragItem;

namespace ew {
	enum H264FragMode;
	class FragWriter;

	//! Decides whether to skip a NAL
	/*!
	  \param [in] nal NAL to examine
	  \return \a true if NAL has to be skipped
	*/
	bool H264SkipNal(const x264_nal_t *nal);

	//! Encodes a NAL into annex B format
	/*!
	  \param[out] dst destination buffer
	  \param[in] dst_size destination buffer size
	  \param[in] src source NAL buffer (src[0] is NAL header)
	  \param[in] src_size NAL size
	  \return number of bytes used in destination buffer or \a 0 if error 
	*/	
	int H264EncodeNalAnnexB(unsigned char *dst, int dst_size, const unsigned char *src, int src_size);

	//! Encodes an array of NAL in Annex B format
	/*!
	  \param[out] dst destination buffer
	  \param[in] dst_size destination buffer size
	  \param[in] nal_ary NAL array (can be modified)
	  \param[in] nal_count array size
	  \return number of bytes used in destination buffer or \a 0 if error
	*/
	int H264EncodeNals(unsigned char *dst, int dst_size, const x264_nal_t nal_ary[], int nal_count);

	int H264FragmentSingleNalUnit(FragWriter *fragwriter, const x264_nal_t nal_ary[], int nal_count);
	int H264FragmentNonInterleavedMode(FragWriter *fragwriter, int fragsize,
		const x264_nal_t nal_ary[], int nal_count);
	void H264PackFuA(FragWriter *fragwriter, int fragsize,
		const unsigned char *buf, int size);
	void H264PackStapA(FragWriter *fragwriter,
		const unsigned char *dst_p[], const int dst_size[], int dst_count);

	int H264DepacketiseRfc3984(const FragItem *frag_item_ary, int frag_count,
		unsigned char *input_buffer, int input_buffer_size);


	//////////////////////////////////////////////////////////////////////
	// FragmenterVideoH264
	//////////////////////////////////////////////////////////////////////

	class FragmenterVideoH264
	{
	public:
		FragmenterVideoH264(H264FragMode fragmode, int fragsize, FragWriter *fragwriter);

		H264FragMode fragmode() const { return fragmode_; }
		int fragsize() const { return fragsize_; }
		FragWriter *fragwriter() const { return fragwriter_; }

		int Fragment(const x264_nal_t nal_ary[], int nal_count);
		
	private:
		H264FragMode fragmode_;
		int fragsize_;
		FragWriter *fragwriter_;
	};

} // namespace ew

#endif // ! FRAG_H264_H_