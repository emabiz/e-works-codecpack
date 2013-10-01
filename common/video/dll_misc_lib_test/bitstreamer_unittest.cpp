#include "stdafx.h"
#include "bitstreamer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// BitWriter
//////////////////////////////////////////////////////////////////////

TEST(BitWriter, PutByteBitsTwoBitsFromEmpty)
{
	unsigned char buffer;
	ew::BitWriter writer(&buffer, sizeof(buffer));
	ASSERT_TRUE(writer.PutByteBits(3, 2)); // 11
	EXPECT_EQ(3 << 6, buffer);
	EXPECT_EQ(2, writer.next_bit_in_byte());
	EXPECT_EQ(0, writer.byte_pos());
}

TEST(BitWriter, PutByteBitsThreeBitsFromAlreadyInserted)
{
	unsigned char buffer;
	ew::BitWriter writer(&buffer, sizeof(buffer));
	ASSERT_TRUE(writer.PutByteBits(2, 2)); // 10
	ASSERT_TRUE(writer.PutByteBits(5, 3)); // 101
	EXPECT_EQ(0x15 << 3, buffer);
	EXPECT_EQ(5, writer.next_bit_in_byte());
	EXPECT_EQ(0, writer.byte_pos());
}

TEST(BitWriter, PutByteBitsFillByte)
{
	unsigned char buffer;
	ew::BitWriter writer(&buffer, sizeof(buffer));
	ASSERT_TRUE(writer.PutByteBits(0xf4, 8));
	EXPECT_EQ(0xf4, buffer);
	EXPECT_EQ(0, writer.next_bit_in_byte());
	EXPECT_EQ(1, writer.byte_pos());
}

TEST(BitWriter, PutByteBitsSpanByteFromEmpty)
{
	unsigned char buffer[2];
	ew::BitWriter writer(buffer, sizeof(buffer));
	ASSERT_TRUE(writer.PutByteBits(2, 2)); // 10
	ASSERT_TRUE(writer.PutByteBits(0x5D, 7)); // 10 1110 1
	EXPECT_EQ(0xAE, buffer[0]);
	EXPECT_EQ(0x80, buffer[1]);
	EXPECT_EQ(1, writer.next_bit_in_byte());
	EXPECT_EQ(1, writer.byte_pos());
}

TEST(BitWriter, PutByteBitsFailTooManyBits)
{
	unsigned char buffer;
	ew::BitWriter writer(&buffer, sizeof(buffer));
	EXPECT_FALSE(writer.PutByteBits(2, 1));
}

TEST(BitWriter, PutByteBitsFailNoBufferLeft)
{
	unsigned char buffer;
	ew::BitWriter writer(&buffer, sizeof(buffer));
	ASSERT_TRUE(writer.PutByteBits(2, 2)); // 10
	EXPECT_FALSE(writer.PutByteBits(0x5D, 7)); // 10 1110 1
}


//////////////////////////////////////////////////////////////////////
// BitReader
//////////////////////////////////////////////////////////////////////

TEST(BitReader, GetByteBitsTwoBits)
{
	unsigned char buffer = 3 << 6;
	ew::BitReader reader(&buffer, sizeof(buffer));
	unsigned char bits;
	ASSERT_TRUE(reader.GetByteBits(&bits, 2));
	EXPECT_EQ(3, bits);
	EXPECT_EQ(2, reader.next_bit_in_byte());
	EXPECT_EQ(0, reader.byte_pos());
}

TEST(BitReader, GetByteBitsThreeBitsFromAlreadyRead)
{
	unsigned char buffer = 0xa8;
	ew::BitReader reader(&buffer, sizeof(buffer));
	unsigned char bits;
	ASSERT_TRUE(reader.GetByteBits(&bits, 2)); // 10
	ASSERT_TRUE(reader.GetByteBits(&bits, 3)); // 101
	EXPECT_EQ(5, bits);
	EXPECT_EQ(5, reader.next_bit_in_byte());
	EXPECT_EQ(0, reader.byte_pos());
}

TEST(BitReader, GetByteBitsFullByte)
{
	unsigned char buffer = 0xf4;
	ew::BitReader reader(&buffer, sizeof(buffer));
	unsigned char bits;
	ASSERT_TRUE(reader.GetByteBits(&bits, 8));
	EXPECT_EQ(0xf4, bits);
	EXPECT_EQ(0, reader.next_bit_in_byte());
	EXPECT_EQ(1, reader.byte_pos());
}

TEST(BitReader, GetByteBitsSpanByteFromEmpty)
{
	unsigned char buffer[] = { 0xae, 0x80 } ;
	ew::BitReader reader(buffer, sizeof(buffer));
	unsigned char bits;
	ASSERT_TRUE(reader.GetByteBits(&bits, 2)); // 10
	ASSERT_TRUE(reader.GetByteBits(&bits, 7)); // 10 1110 1
	EXPECT_EQ(0x5d, bits);
	EXPECT_EQ(1, reader.next_bit_in_byte());
	EXPECT_EQ(1, reader.byte_pos());
}

TEST(BitReader, GetByteBitsFailNoBufferLeft)
{
	unsigned char buffer = 0x5d;
	ew::BitReader reader(&buffer, sizeof(buffer));
	unsigned char bits;
	ASSERT_TRUE(reader.GetByteBits(&bits, 2)); // 10
	EXPECT_FALSE(reader.GetByteBits(&bits, 7)); // 10 1110 1
}
