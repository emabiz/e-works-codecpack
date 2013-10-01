/*****************************************************************************
 * H263 framentation unit tests
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
//#include "param_h263.h"
#include "frag_helper.h"
#include "bitstreamer.h"
#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int kFragsize = 100;

// 0000 0000 | 0000 0000 | 1000 00	// psc
// 10 | 0101 01				// tr
// 10 | 0000 11				// ptype
// 1						// not i-frame
// 1 | 010					// annex d, e, f ,g
// 1 1001					// pquant
// | 1						// cpm
// 0						// pei
const unsigned char kH263SampleHeader[] = { 0x00, 0x00, 0x82, 0x56, 0x0f, 0x59, 0x80 };

// 0000 0000 | 0000 0000 | 1	// gbsc
// 010 11						// gn = 11
// 00 |							// gsbi
// 00							// gfid
// 10 111						// gquant
const unsigned char kH263SampleGobHeader[] = { 0x00, 0x00, 0xac, 0x2e };
const int kGobNumSplit = 11;

///////////////////////////////////////////////////////////////////////////
// H263DepacketiseRfc2190
///////////////////////////////////////////////////////////////////////////

TEST(H263DepacketiseRfc2190, ModeASingle)
{
	unsigned char frag[] = { 0x00, 0x00, 0x00, 0x00, 0x45 };
	FragItem fragitem = { frag, sizeof(frag) };
	unsigned char buf[sizeof(frag)-4];
	ASSERT_EQ(sizeof(buf), ew::H263DepacketiseRfc2190(&fragitem, 1, buf, sizeof(buf)));
	EXPECT_TRUE(0 == memcmp(&frag[4], buf, sizeof(buf)));
}

TEST(H263DepacketiseRfc2190, ModeBSingle)
{
	unsigned char frag[] = { 0x80, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x45 };
	FragItem fragitem = { frag, sizeof(frag) };
	unsigned char buf[sizeof(frag)-8];
	ASSERT_EQ(sizeof(buf), ew::H263DepacketiseRfc2190(&fragitem, 1, buf, sizeof(buf)));
	EXPECT_TRUE(0 == memcmp(&frag[8], buf, sizeof(buf)));
}

TEST(H263DepacketiseRfc2190, ModeCSingle)
{
	unsigned char frag[] = { 0xC0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45 };
	FragItem fragitem = { frag, sizeof(frag) };
	unsigned char buf[sizeof(frag)-12];
	ASSERT_EQ(sizeof(buf), ew::H263DepacketiseRfc2190(&fragitem, 1, buf, sizeof(buf)));
	EXPECT_TRUE(0 == memcmp(&frag[12], buf, sizeof(buf)));
}

TEST(H263DepacketiseRfc2190, ModeAMultiple)
{
	unsigned char frag0[] = { 0x00, 0x00, 0x00, 0x00, 0x45 };
	unsigned char frag1[] = { 0x00, 0x00, 0x00, 0x00, 0xf4 };
	FragItem fragitem[2] = { { frag0, sizeof(frag0) }, { frag1, sizeof(frag1) } };
	unsigned char buf[sizeof(frag0)-4 + sizeof(frag1)-4];
	ASSERT_EQ(sizeof(buf), ew::H263DepacketiseRfc2190(fragitem, 2, buf, sizeof(buf)));
	EXPECT_EQ(frag0[4], buf[0]);
	EXPECT_EQ(frag1[4], buf[1]);
}

TEST(H263DepacketiseRfc2190, ModeAMultipleEbitSbit)
{
	// 10110|101 = 0xb0 | 0x05 = 0xb5
	unsigned char frag0[] = { 3, 0x00, 0x00, 0x00, 0xb0 };
	unsigned char frag1[] = { 5 << 3, 0x00, 0x00, 0x00, 0x05 };
	FragItem fragitem[2] = { { frag0, sizeof(frag0) }, { frag1, sizeof(frag1) } };
	unsigned char buf;
	ASSERT_EQ(sizeof(buf), ew::H263DepacketiseRfc2190(fragitem, 2, &buf, sizeof(buf)));
	EXPECT_EQ(0xb5, buf);
}


//////////////////////////////////////////////////////////////////////
// H263HeaderDecoder
//////////////////////////////////////////////////////////////////////

TEST(H263HeaderDecoder, Decode)
{
	ew::H263HeaderDecoder hdr;
	EXPECT_FALSE(hdr.is_valid());
	ASSERT_TRUE(hdr.Decode(kH263SampleHeader, sizeof(kH263SampleHeader)));
	EXPECT_EQ(0x95, hdr.tr());
	EXPECT_EQ(3, hdr.frame_size());
	EXPECT_EQ((352/16)*(288/16), hdr.macroblocks_per_pic());
	EXPECT_FALSE(hdr.i_frame());
	EXPECT_TRUE(hdr.annex_d());
	EXPECT_FALSE(hdr.annex_e());
	EXPECT_TRUE(hdr.annex_f());
	EXPECT_FALSE(hdr.annex_g());
	EXPECT_EQ(25, hdr.p_quant());
	EXPECT_TRUE(hdr.cpm());
	EXPECT_TRUE(hdr.is_valid());
}

TEST(H263HeaderDecoder, DecodeWithBitWriterBuffer)
{
	// 0000 0000 | 0000 0000 | 1000 00	// psc
	// 10 | 0101 01				// tr
	// 10 | 0000 11				// ptype
	// 1						// not i-frame
	// 1 | 010					// annex d, e, f ,g
	// 1 1001					// pquant
	// | 1						// cpm
	// 0						// pei

	unsigned char buf[7];
	ew::BitWriter writer(buf, sizeof(buf));
	// psc
	ASSERT_TRUE(writer.PutByteBits(0, 8));
	ASSERT_TRUE(writer.PutByteBits(0, 8));
	ASSERT_TRUE(writer.PutByteBits(1, 1));
	ASSERT_TRUE(writer.PutByteBits(0, 5));
	
	// tr
	ASSERT_TRUE(writer.PutByteBits(0x95, 8));
	
	// ptype
	ASSERT_TRUE(writer.PutByteBits(2, 2)); // start code
	ASSERT_TRUE(writer.PutByteBits(0, 3)); // split_screen, document_camera, picture_freeze
	ASSERT_TRUE(writer.PutByteBits(3, 3)); // CIF
	
	ASSERT_TRUE(writer.PutByteBits(1, 1)); // not i-frame
	ASSERT_TRUE(writer.PutByteBits(10, 4)); // annex d, e, f ,g

	ASSERT_TRUE(writer.PutByteBits(25, 5)); // pquant
	ASSERT_TRUE(writer.PutByteBits(1, 1)); // cpm
	ASSERT_TRUE(writer.PutByteBits(0, 1)); // pei

	ew::H263HeaderDecoder hdr;
	EXPECT_FALSE(hdr.is_valid());
	ASSERT_TRUE(hdr.Decode(buf, sizeof(buf)));
	EXPECT_EQ(0x95, hdr.tr());
	EXPECT_EQ(3, hdr.frame_size());
	EXPECT_EQ((352/16)*(288/16), hdr.macroblocks_per_pic());
	EXPECT_FALSE(hdr.i_frame());
	EXPECT_TRUE(hdr.annex_d());
	EXPECT_FALSE(hdr.annex_e());
	EXPECT_TRUE(hdr.annex_f());
	EXPECT_FALSE(hdr.annex_g());
	EXPECT_EQ(25, hdr.p_quant());
	EXPECT_TRUE(hdr.cpm());
	EXPECT_TRUE(hdr.is_valid());
}

// TODO: fare i casi in cui la decode ritorna false

///////////////////////////////////////////////////////////////////////////
// FragmenterVideoH263_Rfc2190
///////////////////////////////////////////////////////////////////////////

#define CHECK_TRUE_OR_RETURN(__expr) { if (!__expr) return false; }
#define CHECK_EQ_OR_RETURN(__expected, __actual) { if ((__expected) != (__actual)) return false; }

static bool IsHdrModeAValid(const ew::H263HeaderDecoder *hdr, const unsigned char *data, int size)
{
	_ASSERT(NULL != hdr);
	_ASSERT(hdr->is_valid());
	_ASSERT(NULL != data);
	_ASSERT(size >= ew::kHdrModeALen);

	// header Mode A
	ew::BitReader reader(data, size);
	unsigned char bits;
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(0, bits); // f
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(0, bits); // p
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 3));
	CHECK_EQ_OR_RETURN(0, bits); // sbit
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 3));
	CHECK_EQ_OR_RETURN(0, bits); // ebit
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 3));
	CHECK_EQ_OR_RETURN(hdr->frame_size(), bits); // src
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(hdr->i_frame(), bits == 0); // i_frame
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(hdr->annex_d(), bits != 0); // u
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(hdr->annex_e(), bits != 0); // s
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(hdr->annex_f(), bits != 0); // a
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 4));
	CHECK_EQ_OR_RETURN(0, bits); // r
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 2));
	CHECK_EQ_OR_RETURN(0, bits); // dqb
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 3));
	CHECK_EQ_OR_RETURN(0, bits); // trb
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 8));
	CHECK_EQ_OR_RETURN(0, bits); // tr

	return true;
}

TEST(FragmenterVideoH263_Rfc2190, PackModeA)
{
	ew::FragWriter fragwriter(1);
	ew::FragmenterVideoH263_Rfc2190 fragmenter(kFragsize, &fragwriter, false);

	ew::H263HeaderDecoder hdr;
	ASSERT_TRUE(hdr.Decode(kH263SampleHeader, sizeof(kH263SampleHeader)));

	unsigned char payload[kFragsize-ew::kHdrModeALen];
	memcpy(payload, kH263SampleHeader, sizeof(kH263SampleHeader));
	for (int i = 0; i < sizeof(payload)-sizeof(kH263SampleHeader) ; ++i)
		payload[i+sizeof(kH263SampleHeader)] = static_cast<unsigned char>(i);

	int size = sizeof(payload)+ew::kHdrModeALen;
	ASSERT_EQ(size,	fragmenter.PackModeA(&hdr, payload, sizeof(payload)));
	ASSERT_EQ(1, fragwriter.frag_count());
	const FragItem *frag_ary = fragwriter.frag_ary();
	ASSERT_TRUE(NULL != frag_ary);

	// frag0
	const unsigned char *frag0_data = frag_ary[0].data;
	ASSERT_TRUE(NULL != frag0_data);
	ASSERT_EQ(kFragsize, frag_ary[0].size);

	EXPECT_TRUE(IsHdrModeAValid(&hdr, frag0_data, frag_ary[0].size));

	// payload
	EXPECT_TRUE(0 == memcmp(&frag0_data[ew::kHdrModeALen], payload, sizeof(payload)));
}

static bool IsHdrModeBValid(const ew::H263HeaderDecoder *hdr, int gob_n, const unsigned char *data, int size)
{
	_ASSERT(NULL != hdr);
	_ASSERT(hdr->is_valid());
	_ASSERT(0 <= gob_n && gob_n <= 31);
	_ASSERT(NULL != data);
	_ASSERT(size > ew::kHdrModeBLen);

	// header Mode B
	ew::BitReader reader(data, size);
	unsigned char bits;
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(1, bits); // f
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(0, bits); // p
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 3));
	CHECK_EQ_OR_RETURN(0, bits); // sbit
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 3));
	CHECK_EQ_OR_RETURN(0, bits); // ebit
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 3));
	CHECK_EQ_OR_RETURN(hdr->frame_size(), bits); // src
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 5));
	CHECK_EQ_OR_RETURN(0, bits); // quant
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 5));
	CHECK_EQ_OR_RETURN(gob_n, bits); // gobn
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(0, bits); // mba high bit
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 8));
	CHECK_EQ_OR_RETURN(0, bits); // mba low bits
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 2));
	CHECK_EQ_OR_RETURN(0, bits); // r
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(hdr->i_frame(), bits == 0); // i_frame
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(hdr->annex_d(), bits != 0); // u
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(hdr->annex_e(), bits != 0); // s
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 1));
	CHECK_EQ_OR_RETURN(hdr->annex_f(), bits != 0); // a
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 7));
	CHECK_EQ_OR_RETURN(0, bits); // hmv1
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 7));
	CHECK_EQ_OR_RETURN(0, bits); // vmv1
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 7));
	CHECK_EQ_OR_RETURN(0, bits); // hmv2
	CHECK_TRUE_OR_RETURN(reader.GetByteBits(&bits, 7));
	CHECK_EQ_OR_RETURN(0, bits); // vmv2

	return true;
}

TEST(FragmenterVideoH263_Rfc2190, PackModeB)
{
	ew::FragWriter fragwriter(1);
	ew::FragmenterVideoH263_Rfc2190 fragmenter(kFragsize, &fragwriter, false);

	ew::H263HeaderDecoder hdr;
	ASSERT_TRUE(hdr.Decode(kH263SampleHeader, sizeof(kH263SampleHeader)));

	unsigned char payload[kFragsize-ew::kHdrModeBLen];
	memcpy(payload, kH263SampleHeader, sizeof(kH263SampleHeader));
	for (int i = 0; i < sizeof(payload)-sizeof(kH263SampleHeader) ; ++i)
		payload[i+sizeof(kH263SampleHeader)] = static_cast<unsigned char>(i);

	int size = sizeof(payload)+ew::kHdrModeBLen;
	ASSERT_EQ(size,	fragmenter.PackModeB(&hdr, payload, sizeof(payload)));
	ASSERT_EQ(1, fragwriter.frag_count());
	const FragItem *frag_ary = fragwriter.frag_ary();
	ASSERT_TRUE(NULL != frag_ary);

	// frag0
	const unsigned char *frag0_data = frag_ary[0].data;
	ASSERT_TRUE(NULL != frag0_data);
	ASSERT_EQ(kFragsize, frag_ary[0].size);

	EXPECT_TRUE(IsHdrModeBValid(&hdr, 0, frag0_data, frag_ary[0].size));

	// payload
	EXPECT_TRUE(0 == memcmp(&frag0_data[ew::kHdrModeBLen], payload, sizeof(payload)));
}

#if 0
TEST(FragmenterVideoH263_Rfc2190, FragmentModeBMulti)
{
	ew::FragWriter fragwriter(2);
	ew::FragmenterVideoH263_Rfc2190 fragmenter(kFragsize, &fragwriter, false);

	ew::H263HeaderDecoder hdr;
	ASSERT_TRUE(hdr.Decode(kH263SampleHeader, sizeof(kH263SampleHeader)));

	unsigned char payload[kFragsize-ew::kHdrModeALen+1];

	const int kGobStart = 30;
	// picture start
	memcpy(payload, kH263SampleHeader, sizeof(kH263SampleHeader));
	for (int i = 0; i < kGobStart-sizeof(kH263SampleHeader); ++i)
		payload[i+sizeof(kH263SampleHeader)] = static_cast<unsigned char>(i);
	// mb
	memcpy(&payload[kGobStart], kH263SampleGobHeader, sizeof(kH263SampleGobHeader));
	for (int i = 0; i < sizeof(payload)-(kGobStart+sizeof(kH263SampleGobHeader)); ++i)
		payload[kGobStart+sizeof(kH263SampleGobHeader)+i] = static_cast<unsigned char>(i);

	ASSERT_EQ(sizeof(payload)+2*ew::kHdrModeBLen, fragmenter.Fragment(payload, sizeof(payload)));
	ASSERT_EQ(2, fragwriter.frag_count());

	const FragItem *frag_ary = fragwriter.frag_ary();
	ASSERT_TRUE(NULL != frag_ary);

	// frag0
	const unsigned char *frag0_data = frag_ary[0].data;
	ASSERT_TRUE(NULL != frag0_data);
	ASSERT_EQ(ew::kHdrModeBLen + kGobStart, frag_ary[0].size);
	EXPECT_TRUE(IsHdrModeBValid(&hdr, 0, frag0_data, frag_ary[0].size));
	EXPECT_TRUE(0 == memcmp(&frag0_data[ew::kHdrModeBLen], payload, kGobStart-ew::kHdrModeBLen));

	// frag1
	const unsigned char *frag1_data = frag_ary[1].data;
	ASSERT_TRUE(NULL != frag1_data);
	ASSERT_EQ(ew::kHdrModeBLen + sizeof(payload)-kGobStart, frag_ary[1].size);
	EXPECT_TRUE(IsHdrModeBValid(&hdr, kGobNumSplit, frag1_data, frag_ary[1].size));
	EXPECT_TRUE(0 == memcmp(&frag1_data[ew::kHdrModeBLen], &payload[kGobStart],
		sizeof(payload)-kGobStart));
}
#endif
