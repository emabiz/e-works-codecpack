/*****************************************************************************
 * H264 fragmentation unit tests
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
#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum {
	NAL_STAP_A = 24,
	NAL_FU_A = 28
};

const int kFragsize = 100;
const unsigned char kAnnexBHdr[] = { 0, 0, 0, 1 };

///////////////////////////////////////////////////////////////////////////
// H264EncodeNalAnnexB
///////////////////////////////////////////////////////////////////////////

TEST(H264EncodeNalAnnexB, NoEscape)
{
	const unsigned char src[] = { 0x15, 0x23, 0x3F, 0x06 };
	unsigned char dst[sizeof(kAnnexBHdr) + sizeof(src)];

	EXPECT_EQ(sizeof(dst), ew::H264EncodeNalAnnexB(dst, sizeof(dst), src, sizeof(src)));
	EXPECT_TRUE(0 == memcmp(kAnnexBHdr, dst, sizeof(kAnnexBHdr)));
	EXPECT_TRUE(0 == memcmp(src, &dst[sizeof(kAnnexBHdr)], sizeof(src)));
}

TEST(H264EncodeNalAnnexB, AlreadyEscaped)
{
	const unsigned char src[] = { 0x00, 0x00, 0x03, 0x01 };
	unsigned char dst[sizeof(kAnnexBHdr) + sizeof(src)];

	EXPECT_EQ(sizeof(dst), ew::H264EncodeNalAnnexB(dst, sizeof(dst), src, sizeof(src)));
	EXPECT_TRUE(0 == memcmp(kAnnexBHdr, dst, sizeof(kAnnexBHdr)));
	EXPECT_TRUE(0 == memcmp(src, &dst[sizeof(kAnnexBHdr)], sizeof(src)));
}


///////////////////////////////////////////////////////////////////////////
// X264Nal
///////////////////////////////////////////////////////////////////////////

class X264NalAry
{
public:
	X264NalAry() {}

	void Append(int nal_type, const unsigned char *nal_data, int nal_data_size)
	{
		_ASSERT(NULL != nal_data);
		_ASSERT(nal_data_size > 0);
		_ASSERT(nal_type == (nal_data[0] & 0x1f));

		x264_nal_t nal;
		nal.i_ref_idc = 0;
		nal.i_type = nal_type;
		nal.i_payload = 4 + nal_data_size;
		unsigned char *p = new unsigned char[nal.i_payload];
		nal.p_payload = p;
		p[0] = (nal_data_size >> 24) & 0x7f;
		p[1] = (nal_data_size >> 16) & 0xff;
		p[2] = (nal_data_size >>  8) & 0xff;
		p[3] = nal_data_size & 0xff;

		memcpy(&p[4], nal_data, nal_data_size);

		nal_ary_.push_back(nal);
	}

	~X264NalAry() {
		for (std::vector<x264_nal_t>::const_iterator it = nal_ary_.begin();
				it != nal_ary_.end(); ++it)
			delete it->p_payload;
	}

	const x264_nal_t *GetNalAry() const { return &nal_ary_[0]; }
	int GetNalCount() const { return nal_ary_.size(); }

private:
	std::vector<x264_nal_t> nal_ary_;
};

///////////////////////////////////////////////////////////////////////////
// Misc
///////////////////////////////////////////////////////////////////////////

static size_t _StapSize(const unsigned char *data)
{
	_ASSERT(data);
	return (static_cast<size_t>(data[0]) << 8) | static_cast<size_t>(data[1]);
}

///////////////////////////////////////////////////////////////////////////
// H264EncodeNals
///////////////////////////////////////////////////////////////////////////

TEST(H264EncodeNals, SingleNal)
{
	unsigned char nal_type = NAL_SLICE_IDR;
	unsigned char payload[] = { nal_type, 0x10, 0x5C, 0x32 };
	X264NalAry nal_ary;
	nal_ary.Append(nal_type, payload, sizeof(payload));
	unsigned char dst[sizeof(kAnnexBHdr)+sizeof(payload)];

	ASSERT_EQ(sizeof(dst), ew::H264EncodeNals(dst, sizeof(dst), nal_ary.GetNalAry(), nal_ary.GetNalCount()));
	EXPECT_TRUE(0 == memcmp(kAnnexBHdr, dst, sizeof(kAnnexBHdr)));
	EXPECT_TRUE(0 == memcmp(payload, &dst[sizeof(kAnnexBHdr)], sizeof(payload)));
}

TEST(H264EncodeNals, SkipNal)
{
	unsigned char nal_type = NAL_SEI;
	unsigned char payload[] = { nal_type, 0x10, 0x5C, 0x32 };
	X264NalAry nal_ary;
	nal_ary.Append(nal_type, payload, sizeof(payload));
	unsigned char dst[sizeof(kAnnexBHdr)+sizeof(payload)];

	EXPECT_EQ(0, ew::H264EncodeNals(dst, sizeof(dst), nal_ary.GetNalAry(), nal_ary.GetNalCount()));
}

TEST(H264EncodeNals, MultipleNals)
{
	unsigned char nal_type = NAL_SLICE_IDR;
	unsigned char payload[] = { nal_type, 0x10, 0x5C, 0x32 };
	X264NalAry nal_ary;
	nal_ary.Append(nal_type, payload, sizeof(payload));
	nal_ary.Append(nal_type, payload, sizeof(payload));

	unsigned char dst[2 * (sizeof(kAnnexBHdr)+sizeof(payload))];

	ASSERT_EQ(sizeof(dst), ew::H264EncodeNals(dst, sizeof(dst), nal_ary.GetNalAry(), nal_ary.GetNalCount()));
	// nal 1
	EXPECT_TRUE(0 == memcmp(kAnnexBHdr, dst, sizeof(kAnnexBHdr)));
	EXPECT_TRUE(0 == memcmp(payload, &dst[sizeof(kAnnexBHdr)], sizeof(payload)));	
	// nal 2
	const int kOffs = sizeof(kAnnexBHdr)+sizeof(payload);
	EXPECT_TRUE(0 == memcmp(kAnnexBHdr, &dst[kOffs], sizeof(kAnnexBHdr)));
	EXPECT_TRUE(0 == memcmp(payload, &dst[kOffs + sizeof(kAnnexBHdr)], sizeof(payload)));	
}


///////////////////////////////////////////////////////////////////////////
// H264PackFuA
///////////////////////////////////////////////////////////////////////////

TEST(H264PackFuA, FragIn2Pieces)
{
	ew::FragWriter writer(2);
	unsigned char payload[kFragsize+1];
	unsigned char nal_type = NAL_SLICE_IDR;
	payload[0] = nal_type;
	for (int i = 0; i < sizeof(payload)-1; ++i)
		payload[i+1] = static_cast<unsigned char>(i);

	ew::H264PackFuA(&writer, kFragsize, payload, sizeof(payload));
	ASSERT_EQ(2, writer.frag_count());
	const FragItem *frag_ary = writer.frag_ary();
	ASSERT_TRUE(NULL != frag_ary);

	// frammento 1
	const unsigned char *frag0_data = frag_ary[0].data;
	ASSERT_TRUE(NULL != frag0_data);
	ASSERT_EQ(kFragsize, frag_ary[0].size);
	EXPECT_EQ(frag0_data[0], NAL_FU_A);
	EXPECT_EQ(frag0_data[1], 0x80 | nal_type); // s
	EXPECT_TRUE(0 == memcmp(&payload[1], &frag0_data[2], frag_ary[0].size - 2));

	// frammento 2
	const unsigned char *frag1_data = frag_ary[1].data;
	ASSERT_TRUE(NULL != frag1_data);
	ASSERT_EQ(sizeof(payload) - (kFragsize-1) + 2, frag_ary[1].size);
	EXPECT_EQ(frag1_data[0], NAL_FU_A);
	EXPECT_EQ(frag1_data[1], 0x40 | nal_type); // e
	EXPECT_TRUE(0 == memcmp(&payload[frag_ary[0].size-2+1], &frag1_data[2], frag_ary[1].size - 2));
}

TEST(H264PackFuA, FragIn3Pieces)
{
	ew::FragWriter writer(3);
	unsigned char payload[kFragsize*2+1];
	unsigned char nal_type = NAL_SLICE_IDR;
	payload[0] = nal_type;
	for (int i = 0; i < sizeof(payload)-1; ++i)
		payload[i+1] = static_cast<unsigned char>(i);

	ew::H264PackFuA(&writer, kFragsize, payload, sizeof(payload));
	ASSERT_EQ(3, writer.frag_count());
	const FragItem *frag_ary = writer.frag_ary();
	ASSERT_TRUE(NULL != frag_ary);

	// frammento 1
	const unsigned char *frag0_data = frag_ary[0].data;
	ASSERT_TRUE(NULL != frag0_data);
	ASSERT_EQ(kFragsize, frag_ary[0].size);
	EXPECT_EQ(frag0_data[0], NAL_FU_A);
	EXPECT_EQ(frag0_data[1], 0x80 | nal_type); // s
	EXPECT_TRUE(0 == memcmp(&payload[1], &frag0_data[2], frag_ary[0].size - 2));

	// frammento 2
	const unsigned char *frag1_data = frag_ary[1].data;
	ASSERT_TRUE(NULL != frag1_data);
	ASSERT_EQ(kFragsize, frag_ary[1].size);
	EXPECT_EQ(frag1_data[0], NAL_FU_A);
	EXPECT_EQ(frag1_data[1], nal_type); // !s & !e
	EXPECT_TRUE(0 == memcmp(&payload[frag_ary[0].size-2+1], &frag1_data[2], frag_ary[1].size - 2));

	// frammento 3
	const unsigned char *frag2_data = frag_ary[2].data;
	ASSERT_TRUE(NULL != frag2_data);
	ASSERT_EQ(sizeof(payload) - 2*(kFragsize-2) + 1, frag_ary[2].size);
	EXPECT_EQ(frag2_data[0], NAL_FU_A);
	EXPECT_EQ(frag2_data[1], 0x40 | nal_type); // e
	EXPECT_TRUE(0 == memcmp(&payload[2*(frag_ary[0].size-2)+1], &frag2_data[2], frag_ary[2].size - 2));
}


///////////////////////////////////////////////////////////////////////////
// H264PackStapA
///////////////////////////////////////////////////////////////////////////

TEST(H264PackStapA, NoNal)
{
	ew::FragWriter writer(1);
	const unsigned char *nal_p[] = { NULL };
	int nal_s[] = { 0 };
	
	ew::H264PackStapA(&writer, nal_p, nal_s, 0);
	EXPECT_EQ(0, writer.frag_count());
}

TEST(H264PackStapA, SingleNal)
{
	ew::FragWriter writer(1);
	unsigned char nal_type = NAL_SLICE_IDR;
	unsigned char nal_data[] = { nal_type, 0xa4, 0xc3, 0x33, 0xd0 };
	const unsigned char *nal_p[] = { nal_data };
	int nal_s[] = { sizeof(nal_data) };
	
	ew::H264PackStapA(&writer, nal_p, nal_s, 1);
	ASSERT_EQ(1, writer.frag_count());
	const FragItem *frag_ary = writer.frag_ary();
	ASSERT_TRUE(NULL != frag_ary);

	// frammento (single nal unit)
	const unsigned char *frag0_data = frag_ary[0].data;
	ASSERT_TRUE(NULL != frag0_data);
	ASSERT_EQ(sizeof(nal_data), frag_ary[0].size);
	EXPECT_TRUE(0 == memcmp(nal_data, frag0_data, sizeof(nal_data)));
}

TEST(H264PackStapA, MultiNal)
{
	ew::FragWriter writer(1);
	unsigned char nal0_data[] = { NAL_SLICE_IDR, 0xa4, 0xc3, 0x33, 0xd0 };
	unsigned char nal1_data[] = { NAL_SLICE, 0x67, 0x3a, 0x85, 0xab, 0x45 };
	const unsigned char *nal_p[] = { nal0_data, nal1_data };
	int nal_s[] = { sizeof(nal0_data), sizeof(nal1_data) };
	
	ew::H264PackStapA(&writer, nal_p, nal_s, 2);
	ASSERT_EQ(1, writer.frag_count());
	const FragItem *frag_ary = writer.frag_ary();
	ASSERT_TRUE(NULL != frag_ary);

	// frammento1
	const unsigned char *frag0_data = frag_ary[0].data;
	ASSERT_TRUE(NULL != frag0_data);
	EXPECT_EQ(1 + 2 + sizeof(nal0_data) + 2 + sizeof(nal1_data), frag_ary[0].size);
	EXPECT_EQ(NAL_STAP_A, frag0_data[0]);
	// stap0
	ASSERT_EQ(sizeof(nal0_data), _StapSize(&frag0_data[1]));
	EXPECT_TRUE(0 == memcmp(nal0_data, &frag0_data[3], sizeof(nal0_data)));
	// stap1
	int offs = 1 + 2 + sizeof(nal0_data);
	ASSERT_EQ(sizeof(nal1_data), _StapSize(&frag0_data[offs]));
	EXPECT_TRUE(0 == memcmp(nal1_data, &frag0_data[offs + 2], sizeof(nal1_data)));
}


///////////////////////////////////////////////////////////////////////////
// H264FragmentSingleNalUnit
///////////////////////////////////////////////////////////////////////////
// TODO: Test H264FragmentSingleNalUnit

///////////////////////////////////////////////////////////////////////////
// H264FragmentNonInterleavedMode
///////////////////////////////////////////////////////////////////////////

TEST(H264FragmentNonInterleavedMode, SingleNal)
{
	ew::FragWriter writer(1);
	unsigned char nal_type = NAL_SLICE_IDR;
	unsigned char nal_data[] = { nal_type, 0xa4, 0xc3, 0x33, 0xd0 };
	X264NalAry nal_ary;
	nal_ary.Append(nal_type, nal_data, sizeof(nal_data));
	
	EXPECT_EQ(sizeof(nal_data),
		ew::H264FragmentNonInterleavedMode(&writer, 100, nal_ary.GetNalAry(), nal_ary.GetNalCount()));
	ASSERT_EQ(1, writer.frag_count());
	const FragItem *frag_ary = writer.frag_ary();
	ASSERT_TRUE(NULL != frag_ary);
	ASSERT_EQ(sizeof(nal_data), frag_ary[0].size);
	EXPECT_TRUE(0 == memcmp(nal_data, frag_ary[0].data, sizeof(nal_data)));
}

TEST(H264FragmentNonInterleavedMode, StapA)
{
	ew::FragWriter writer(1);
	unsigned char nal0_type = NAL_SLICE_IDR;
	unsigned char nal0_data[] = { nal0_type, 0xa4, 0xc3, 0x33, 0xd0 };
	unsigned char nal1_type = NAL_SLICE;
	unsigned char nal1_data[kFragsize-sizeof(nal0_data)-1-2*2];
	nal1_data[0] = nal1_type;
	for (int i = 0; i < sizeof(nal1_data)-1; ++i)
		nal1_data[i+1] = static_cast<unsigned char>(i);

	X264NalAry nal_ary;
	nal_ary.Append(nal0_type, nal0_data, sizeof(nal0_data));
	nal_ary.Append(nal1_type, nal1_data, sizeof(nal1_data));

	EXPECT_EQ(1 + 2 + sizeof(nal0_data) + 2 + sizeof(nal1_data),
		ew::H264FragmentNonInterleavedMode(&writer, kFragsize, nal_ary.GetNalAry(), nal_ary.GetNalCount()));
	ASSERT_EQ(1, writer.frag_count());
	const FragItem *frag_ary = writer.frag_ary();
	ASSERT_TRUE(NULL != frag_ary);
	ASSERT_EQ(kFragsize, frag_ary[0].size);
	const unsigned char *frag0_data = frag_ary[0].data;
	EXPECT_EQ(NAL_STAP_A, frag0_data[0]);
	ASSERT_EQ(sizeof(nal0_data), _StapSize(&frag0_data[1]));
	EXPECT_TRUE(0 == memcmp(nal0_data, &frag0_data[3], sizeof(nal0_data)));
	size_t offs = 1+2+sizeof(nal0_data);
	ASSERT_EQ(sizeof(nal1_data), _StapSize(&frag0_data[offs]));
	EXPECT_TRUE(0 == memcmp(nal1_data, &frag0_data[offs+2], sizeof(nal1_data)));
}

TEST(H264FragmentNonInterleavedMode, FuA)
{
	ew::FragWriter writer(2);
	unsigned char nal0_type = NAL_SLICE;
	unsigned char nal0_data[kFragsize+1];
	nal0_data[0] = nal0_type;
	for (int i = 0; i < sizeof(nal0_data)-1; ++i)
		nal0_data[i+1] = static_cast<unsigned char>(i);

	X264NalAry nal_ary;
	nal_ary.Append(nal0_type, nal0_data, sizeof(nal0_data));

	EXPECT_EQ(1 + sizeof(nal0_data) + 2,
		ew::H264FragmentNonInterleavedMode(&writer, kFragsize, nal_ary.GetNalAry(), nal_ary.GetNalCount()));
	ASSERT_EQ(2, writer.frag_count());
	const FragItem *frag_ary = writer.frag_ary();
	ASSERT_TRUE(NULL != frag_ary);
	// fragment 0
	ASSERT_EQ(kFragsize, frag_ary[0].size);
	const unsigned char *frag0_data = frag_ary[0].data;
	EXPECT_EQ(NAL_FU_A, frag0_data[0]);
	EXPECT_EQ(nal0_type | 0x80, frag0_data[1]); // s
	EXPECT_TRUE(0 == memcmp(&nal0_data[1], &frag0_data[2], frag_ary[0].size-2));
	// fragment 1
	ASSERT_EQ(1 + sizeof(nal0_data) + 2 - kFragsize, frag_ary[1].size);
	const unsigned char *frag1_data = frag_ary[1].data;
	EXPECT_EQ(NAL_FU_A, frag1_data[0]);
	EXPECT_EQ(nal0_type | 0x40, frag1_data[1]); // e
	int offs = kFragsize-2+1;
	EXPECT_TRUE(0 == memcmp(&nal0_data[offs], &frag1_data[2], frag_ary[1].size-2));
}

// TODO: test H264FragmentNonInterleavedMode
// TEST(H264FragmentNonInterleavedMode, Single_FuA)
// TEST(H264FragmentNonInterleavedMode, FuA_Single)
// TEST(H264FragmentNonInterleavedMode, StapA_FuA)
// TEST(H264FragmentNonInterleavedMode, FuA_StapA)


///////////////////////////////////////////////////////////////////////////
// H264DepacketiseRfc3984
///////////////////////////////////////////////////////////////////////////

TEST(H264DepacketiseRfc3984, StapA)
{
	const int nal0_size = 4;
	const int nal1_size = 5;
	unsigned char stap_data[] = { NAL_STAP_A,
		(nal0_size >> 8) & 0xff, nal0_size & 0xff, NAL_SLICE_IDR, 0x03, 0x46, 0x76,
		(nal1_size >> 8) & 0xff, nal1_size & 0xff, NAL_SLICE, 0xB3, 0xE6, 0x76, 0x8a
	};

	FragItem fragitem = { stap_data, sizeof(stap_data) };
	unsigned char buf[4+nal0_size+4+nal1_size];
	ASSERT_EQ(sizeof(buf), ew::H264DepacketiseRfc3984(&fragitem, 1, buf, sizeof(buf)));
	const unsigned char *stap_p0 = &stap_data[1+2];
	const unsigned char *stap_p1 = &stap_data[1+2+nal0_size+2];
	EXPECT_TRUE(0 == memcmp(kAnnexBHdr, buf, sizeof(kAnnexBHdr)));
	EXPECT_TRUE(0 == memcmp(stap_p0, &buf[sizeof(kAnnexBHdr)], nal0_size));
	EXPECT_TRUE(0 == memcmp(kAnnexBHdr, &buf[sizeof(kAnnexBHdr)+nal0_size], sizeof(kAnnexBHdr)));
	EXPECT_TRUE(0 == memcmp(stap_p1, &buf[2*sizeof(kAnnexBHdr)+nal0_size], nal1_size));
}

TEST(H264DepacketiseRfc3984, FuAIn2)
{
	unsigned char fu0[] = { NAL_FU_A, 0x80 | NAL_SLICE_IDR, 0xB3, 0xE6, 0x76, 0x8a }; // s
	unsigned char fu1[] = { NAL_FU_A, 0x40 | NAL_SLICE_IDR, 0x03, 0x46, 0x76 }; // e

	FragItem fragitem[3] = {
		{ fu0, sizeof(fu0) },
		{ fu1, sizeof(fu1) },
	};

	unsigned char buf[4 + sizeof(fu0)-1 + sizeof(fu1)-2];
	ASSERT_EQ(sizeof(buf),
		ew::H264DepacketiseRfc3984(fragitem, 2, buf, sizeof(buf)));

	int pos = 0;
	EXPECT_TRUE(0 == memcmp(kAnnexBHdr, buf, sizeof(kAnnexBHdr)));
	pos += sizeof(kAnnexBHdr);
	EXPECT_EQ(NAL_SLICE_IDR, buf[pos]);
	pos += 1;
	EXPECT_TRUE(0 == memcmp(&fu0[2], &buf[pos], sizeof(fu0)-2));
	pos += sizeof(fu0)-2;
	EXPECT_TRUE(0 == memcmp(&fu1[2], &buf[pos], sizeof(fu1)-2));
}

TEST(H264DepacketiseRfc3984, FuAIn3)
{
	unsigned char fu0[] = { NAL_FU_A, 0x80 | NAL_SLICE_IDR, 0xB3, 0xE6, 0x76, 0x8a }; // s
	unsigned char fu1[] = { NAL_FU_A, NAL_SLICE_IDR, 0xAA, 0x57, 0x7F, 0x33 };
	unsigned char fu2[] = { NAL_FU_A, 0x40 | NAL_SLICE_IDR, 0x03, 0x46, 0x76 }; // e

	FragItem fragitem[3] = {
		{ fu0, sizeof(fu0) },
		{ fu1, sizeof(fu1) },
		{ fu2, sizeof(fu2) }
	};

	unsigned char buf[4 + sizeof(fu0)-1 + sizeof(fu1)-2 + sizeof(fu2)-2];
	ASSERT_EQ(sizeof(buf),
		ew::H264DepacketiseRfc3984(fragitem, 3, buf, sizeof(buf)));

	int pos = 0;
	EXPECT_TRUE(0 == memcmp(kAnnexBHdr, buf, sizeof(kAnnexBHdr)));
	pos += sizeof(kAnnexBHdr);
	EXPECT_EQ(NAL_SLICE_IDR, buf[pos]);
	pos += 1;
	EXPECT_TRUE(0 == memcmp(&fu0[2], &buf[pos], sizeof(fu0)-2));
	pos += sizeof(fu0)-2;
	EXPECT_TRUE(0 == memcmp(&fu1[2], &buf[pos], sizeof(fu1)-2));
	pos += sizeof(fu1)-2;
	EXPECT_TRUE(0 == memcmp(&fu2[2], &buf[pos], sizeof(fu2)-2));
}

//int H264DepacketiseRfc3984(const FragItem *frag_item_ary, int frag_count,
//	unsigned char *input_buffer, int input_buffer_size);
