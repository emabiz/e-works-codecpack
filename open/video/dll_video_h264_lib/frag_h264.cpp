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

#include "stdafx.h"

#include "frag_h264.h"
#include "param_h264.h"
#include "frag_helper.h"
#include "video_codec_def.h"
#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_STAP_FRAGMENTS 64

enum {
	// 0 undefined
	// 1-23 NAL unit        // supported (enc, dec)
	FRAG_STAP_A     = 24,   // supported (enc, dec)
	FRAG_STAP_B     = 25,
	FRAG_MTAP16     = 26,
	FRAG_MTAP24     = 27,
	FRAG_FU_A       = 28,   // supported (enc, dec)
	FRAG_FU_B       = 29
	// 30-31 undefined
};

namespace ew {

	bool H264SkipNal(const x264_nal_t *nal)
	{
		_ASSERT(NULL != nal);

		if (NAL_SEI == nal->i_type) // discards codec info
			return true;
		return false;
	}

	int H264EncodeNalAnnexB(unsigned char *dst, int dst_size, const unsigned char *src, int src_size)
	{
		_ASSERT(NULL != dst);
		_ASSERT(NULL != src);
		_ASSERT(src_size > 1); // NAL header at least

		if (dst_size < 4 + 1) {// 0001 + NAL header at least
			_ASSERT(0);
			return 0;
		}

		/* long NAL start code (we always use long ones)*/
		*dst++ = 0x00;
		*dst++ = 0x00;
		*dst++ = 0x00;
		*dst++ = 0x01;

		memcpy(dst, src, src_size);
		return 4+src_size;
	}

	/* modified from x264.c of an old ffmpeg version */
	int H264EncodeNals(unsigned char *dst, int dst_size, const x264_nal_t nal_ary[], int nal_count)
	{ 
		_ASSERT(NULL != dst);
		_ASSERT(dst_size >= 0);
		_ASSERT(NULL != nal_ary);
		_ASSERT(nal_count > 0);

		unsigned char *p = dst;

		for(int i = 0; i < nal_count; ++i) {
			const x264_nal_t *nal = &nal_ary[i];
			if (H264SkipNal(nal))
				continue;
			int s = H264EncodeNalAnnexB(p, dst_size-(p-dst), &nal->p_payload[4], nal->i_payload-4);
			if (0 == s)
      			return 0;
			p += s;
		}

		return p - dst;
	}

	int H264FragmentSingleNalUnit(FragWriter *fragwriter, const x264_nal_t nal_ary[], int nal_count)
	{
		_ASSERT(NULL != fragwriter);
		_ASSERT(nal_ary);
		_ASSERT(nal_count > 0);

		fragwriter->Clear();
		for (int i = 0; i < nal_count; ++i) {
			if (H264SkipNal(&nal_ary[i]))
		        continue;
			_VERIFY(fragwriter->AppendItem(&nal_ary[i].p_payload[4], nal_ary[i].i_payload-4));
		}
		//*fragcount() = nal_count;
		return fragwriter->frag_total_size();
	}

	int H264FragmentNonInterleavedMode(FragWriter *fragwriter, int fragsize,
		const x264_nal_t nal_ary[], int nal_count)
	{
		_ASSERT(NULL != fragwriter);
		_ASSERT(fragsize >= kFragsizeMin);
		_ASSERT(nal_ary);
		_ASSERT(nal_count > 0);

		fragwriter->Clear();

        const unsigned char *stap_dst_p[MAX_STAP_FRAGMENTS];
		int stap_dst_size[MAX_STAP_FRAGMENTS];
        int stap_dst_total_size = 0;
        int stap_dst_count = 0;

		for (int i = 0; i < nal_count; ++i) {

			const x264_nal_t *nal = &nal_ary[i];
			if (H264SkipNal(nal))
				continue;

			int dstsize = nal->i_payload-4;
			const unsigned char *dst = &nal->p_payload[4];

			if (dstsize > fragsize) {// FU-A
                 // emits previous nals
                 H264PackStapA(fragwriter, stap_dst_p, stap_dst_size, stap_dst_count);
                 stap_dst_total_size = 0;
                 stap_dst_count = 0;
                 H264PackFuA(fragwriter, fragsize, dst, dstsize);
			} else {
				// STAP-A or Single NAL Unit
				// 1=stap header,  2=stap_size
				if (1 + stap_dst_total_size + 2*stap_dst_count + 2 + dstsize <= fragsize) {
                    // aggregates nals
                    stap_dst_p[stap_dst_count] = dst;
                    stap_dst_size[stap_dst_count] = dstsize;
                    stap_dst_total_size += dstsize;
                    stap_dst_count++;
                } else {
                    // emits previous nals
                    H264PackStapA(fragwriter, stap_dst_p, stap_dst_size, stap_dst_count);
                    stap_dst_total_size = 0;
                    stap_dst_count = 0;
					// stap header, stap size
					if (dstsize > fragsize) // FU_A for current
                        H264PackFuA(fragwriter, fragsize, dst, dstsize);
                    else {
                        // aggregates nal
                        stap_dst_p[stap_dst_count] = dst;
                        stap_dst_size[stap_dst_count] = dstsize;
                        stap_dst_total_size += dstsize;
                        stap_dst_count++;
                    }
                }
			}

		} // for

		// emits queued nals (if present)
        H264PackStapA(fragwriter, stap_dst_p, stap_dst_size, stap_dst_count);
		//*fragcount() = frag_writer()->frag_count();
		return fragwriter->frag_total_size();
	}

	void H264PackFuA(FragWriter *fragwriter, int fragsize, const unsigned char *buf, int size)
	{
		_ASSERT(NULL != fragwriter);
		_ASSERT(fragsize >= kFragsizeMin);
		_ASSERT(NULL != buf);
		// stap hdr, stap size
		_ASSERT(size + 1 + 2 > fragsize);

		int nalType = buf[0] & 0x1f;
		const unsigned char *bufCur = &buf[1];
		int sizeLeft = size - 1;
		// adds FU indicator, FU header
		int maxSplitSize = fragsize-2;

		unsigned char *fragBuf = new unsigned char[fragsize];
		for (; sizeLeft > 0; sizeLeft -= maxSplitSize, bufCur += maxSplitSize) {
			int fragSize = min(maxSplitSize, sizeLeft) + 2;
			fragBuf[0] = (buf[0] & 0xe0) | FRAG_FU_A; // FU indicator
			fragBuf[1] = static_cast<unsigned char>(nalType); // FU header (bit s,e,r are set after)
			if (sizeLeft == size-1) // first fragment
				fragBuf[1] |= 0x80; // bit s
			if (sizeLeft <= maxSplitSize) // last fragment
				fragBuf[1] |= 0x40; // bit e
			memcpy(&fragBuf[2], bufCur, fragSize-2);
			_VERIFY(fragwriter->AppendItem(fragBuf, fragSize));
		}
		delete fragBuf;
	}

	void H264PackStapA(FragWriter *fragwriter, const unsigned char *dst_p[], const int dst_size[], int dst_count)
	{
		_ASSERT(NULL != fragwriter);
		_ASSERT(NULL != dst_p);
		_ASSERT(NULL != dst_size);
		_ASSERT(dst_count >= 0);

#ifdef _DEBUG
		for (int i = 0; i < dst_count; ++i) {
			_ASSERT(NULL != dst_p[i]);
			_ASSERT(dst_size[i] > 0);
		}
#endif

		switch (dst_count) {
		case 0:
			break;
		case 1: // Single NAL Unit
			_VERIFY(fragwriter->AppendItem(dst_p[0], dst_size[0]));
			break;
		default: // STAP-A
			{
				int i;
				int size = 0;
				for (i = 0; i < dst_count; i++)
					size += dst_size[i];
				
				unsigned char *buf = new unsigned char[1+2*dst_count+size];
				buf[0] = (dst_p[0][0] & 0xe0) | FRAG_STAP_A;

				unsigned char *buf_p = &buf[1];
				for (i = 0; i < dst_count; i++) {
					buf_p[0] = static_cast<unsigned char>(dst_size[i] >> 8);
					buf_p[1] = dst_size[i] & 0xff;
					memcpy(&buf_p[2], dst_p[i], dst_size[i]);
					buf_p += 2 + dst_size[i];
				}
				_VERIFY(fragwriter->AppendItem(buf, buf_p - buf));
				delete buf;
			}
		}
	}


	int H264DepacketiseRfc3984(const FragItem *frag_item_ary, int frag_count,
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

		for (int i = 0; i < frag_count; ++i) {
			const FragItem *frag_item = &frag_item_ary[i];
			if (frag_item->size <= 1) /* at least a byte for 1-23 type NAL Unit */
				return 0;
			int f		= (frag_item->data[0] & 0x80) >> 7;
			int nri		= (frag_item->data[0] & 0x60) >> 5;
			int type	= frag_item->data[0] & 0x1f;

			if (f == 1)
				return 0; // errors in packet

			if (type >= 1 && type <= 23) {

				fu_writer.Clear();
				int size = H264EncodeNalAnnexB(dst, dst_size,
					frag_item_ary[i].data, frag_item_ary[i].size);
				if (0 == size)
					return 0;
				dst += size;
				dst_size -= size;

			} else if (type == FRAG_STAP_A) {
				
				fu_writer.Clear();
				const unsigned char *src = &frag_item_ary[i].data[1];
				int src_size = frag_item_ary[i].size-1;
				while (src_size > 0) {
					int cur_nal_size = (static_cast<int>(src[0]) << 8) + src[1];
					if (cur_nal_size + 2 > src_size)
						return 0;
					int size = H264EncodeNalAnnexB(dst, dst_size, &src[2], cur_nal_size);
					if (0 == size)
						return 0;
					src_size -= cur_nal_size + 2;
					src += cur_nal_size + 2;
					dst += size;
					dst_size -= size;
				}

			} else if (type == FRAG_FU_A) {
				const unsigned char *src = &frag_item_ary[i].data[1];
				int src_size = frag_item_ary[i].size-1;
				int s = (src[0] & 0x80) >> 7;
				int e = (src[0] & 0x40) >> 6;
				//int r = (src[0] & 0x20) >> 6; // reserved
				int naltype = src[0] & 0x1f;

				if (s > 0) {
					// start
					fu_writer.Clear();
					if (e > 0) // there can't be fragments with s and e bit simultaneously set to 1
						return 0;
					unsigned char fu_hdr = 0 // f
						| static_cast<unsigned char>(nri) << 5
						| static_cast<unsigned char>(naltype);
					_VERIFY(fu_writer.AppendItem(&fu_hdr, 1));
				}
				_VERIFY(fu_writer.AppendItem(&src[1], src_size-1));
				if (e > 0) {
					unsigned char *fu = new unsigned char[fu_writer.frag_total_size()];
					int fu_pos = 0;
					const FragItem *frag_item = fu_writer.frag_ary();
					for (int j = 0; j < fu_writer.frag_count(); ++j) {
						memcpy(&fu[fu_pos], frag_item[j].data, frag_item[j].size);
						fu_pos += frag_item[j].size;
					}
					int size = H264EncodeNalAnnexB(dst, dst_size, fu, fu_writer.frag_total_size());
					if (0 == size)
						return 0;
					dst += size;
					dst_size -= size;

					delete fu;
				}
			} else {
				_ASSERT(0);
			}

		}

		return input_buffer_size - dst_size;
	}

	//////////////////////////////////////////////////////////////////////
	// FragmenterVideoH264
	//////////////////////////////////////////////////////////////////////

	FragmenterVideoH264::FragmenterVideoH264(H264FragMode fragmode, int fragsize, FragWriter *fragwriter)
		: fragmode_(fragmode), fragsize_(fragsize), fragwriter_(fragwriter)
	{
		_ASSERT(fragmode >= 0 && fragmode < H264FragMode_Count);
		_ASSERT(fragsize >= kFragsizeMin);
		_ASSERT(NULL != fragwriter);
	}

	int FragmenterVideoH264::Fragment(const x264_nal_t nal_ary[], int nal_count)
	{
		_ASSERT(NULL != nal_ary);
		_ASSERT(nal_count > 0);

		int size = 0;
		switch (fragmode_) {
		case H264FragMode_SingleNalUnit:
			size = H264FragmentSingleNalUnit(fragwriter_, nal_ary, nal_count);
			break;
		case H264FragMode_NonInterleavedMode:
			size = H264FragmentNonInterleavedMode(fragwriter_, fragsize_, nal_ary, nal_count);
			break;
		default:
			_ASSERT(0);
		}

		return size;
	}


} // namespace ew
