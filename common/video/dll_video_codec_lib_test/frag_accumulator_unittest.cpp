#include "stdafx.h"
#include "frag_accumulator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////
// FragAccumulator
///////////////////////////////////////////////////////////////////////////

TEST(FragAccumulator, AccumulateSingle)
{
	ew::FragAccumulator acc(10);
	EXPECT_TRUE(acc.Accumulate(5, 10, 1));
	EXPECT_EQ(5, acc.GetStartPos());
	EXPECT_EQ(1, acc.GetStartGobN());
	EXPECT_EQ(10, acc.GetTotalSize());
}

TEST(FragAccumulator, AccumulateSingleNoFit)
{
	ew::FragAccumulator acc(10);
	EXPECT_FALSE(acc.Accumulate(2, 11, 3));
	EXPECT_TRUE(acc.GetStartPos() < 0);
	EXPECT_TRUE(acc.GetStartGobN() < 0);
	EXPECT_EQ(0, acc.GetTotalSize());
}

TEST(FragAccumulator, AccumulateDouble)
{
	ew::FragAccumulator acc(10);
	EXPECT_TRUE(acc.Accumulate(5, 6, 0));
	EXPECT_TRUE(acc.Accumulate(11, 4, 1));
	EXPECT_EQ(5, acc.GetStartPos());
	EXPECT_EQ(0, acc.GetStartGobN());
	EXPECT_EQ(10, acc.GetTotalSize());
}

TEST(FragAccumulator, AccumulateDoubleNoFit)
{
	ew::FragAccumulator acc(10);
	EXPECT_TRUE(acc.Accumulate(5, 6, 0));
	EXPECT_FALSE(acc.Accumulate(11, 5, 1));
	EXPECT_EQ(5, acc.GetStartPos());
	EXPECT_EQ(0, acc.GetStartGobN());
	EXPECT_EQ(6, acc.GetTotalSize());
}

TEST(FragAccumulator, AccumulateDoubleBadStart)
{
	ew::FragAccumulator acc(10);
	EXPECT_TRUE(acc.Accumulate(5, 6, 0));
	EXPECT_FALSE(acc.Accumulate(10, 4, 1));
	EXPECT_EQ(5, acc.GetStartPos());
	EXPECT_EQ(0, acc.GetStartGobN());
	EXPECT_EQ(6, acc.GetTotalSize());
}

TEST(FragAccumulator, AccumulateDoubleBadGobN)
{
	ew::FragAccumulator acc(10);
	EXPECT_TRUE(acc.Accumulate(5, 6, 2));
	EXPECT_FALSE(acc.Accumulate(11, 4, 1));
	EXPECT_EQ(5, acc.GetStartPos());
	EXPECT_EQ(2, acc.GetStartGobN());
	EXPECT_EQ(6, acc.GetTotalSize());
}
