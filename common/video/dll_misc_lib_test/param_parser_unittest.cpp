#include "stdafx.h"
#include "param_parser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// ParamDescrInt
//////////////////////////////////////////////////////////////////////

TEST(ParamDescrInt, Create)
{
	ew::ParamDescr *descr = ew::ParamDescr::CreateParamDescrInt("Int", 1, 0, 5);
	EXPECT_TRUE(NULL != descr);
	delete descr;
}

class ParamDescrIntFixture : public testing::Test
{
protected:
	virtual void SetUp() {
		descr = ew::ParamDescr::CreateParamDescrInt("Int", 1, 0, 5);
	}

	virtual void TearDown() {
		delete descr;
	}

	ew::ParamDescr *descr;
};

TEST_F(ParamDescrIntFixture, Name)
{
	EXPECT_EQ("INT", descr->name());
}

TEST_F(ParamDescrIntFixture, FormatDefault)
{
	EXPECT_EQ("INT=1", descr->FormatDefault());
}

TEST_F(ParamDescrIntFixture, GetValueDefaultString)
{
	EXPECT_EQ("1", descr->GetValueDefaultString());
}

TEST_F(ParamDescrIntFixture, ConvertIfAllowedTrue)
{
	ew::ParamValue v;
	ASSERT_TRUE(descr->ConvertIfAllowed("0", &v));
	EXPECT_EQ(0, v.i);
}

TEST_F(ParamDescrIntFixture, ConvertIfAllowedFalse)
{
	EXPECT_FALSE(descr->ConvertIfAllowed("6", NULL));
}

TEST_F(ParamDescrIntFixture, ConvertAllowedBadType)
{
	EXPECT_FALSE(descr->ConvertIfAllowed("2?", NULL));
}

//////////////////////////////////////////////////////////////////////
// ParamDescrIntList
//////////////////////////////////////////////////////////////////////

TEST(ParamDescrIntList, Create)
{
	int ary[] = { 10, 15, -5, 7, 20 };
	ew::ParamDescr *descr = ew::ParamDescr::CreateParamDescrIntList("IntList",
		15, ary, sizeof(ary)/sizeof(ary[0]));
	EXPECT_TRUE(NULL != descr);
	delete descr;
}

class ParamDescrIntListFixture : public testing::Test
{
protected:
	virtual void SetUp() {
		int ary[] = { 10, 15, -5, 7, 20 };
		descr = ew::ParamDescr::CreateParamDescrIntList("IntList",
			15, ary, sizeof(ary)/sizeof(ary[0]));
	}

	virtual void TearDown() {
		delete descr;
	}

	ew::ParamDescr *descr;
};

TEST_F(ParamDescrIntListFixture, Name)
{
	EXPECT_EQ("INTLIST", descr->name());
}

TEST_F(ParamDescrIntListFixture, FormatDefault)
{
	EXPECT_EQ("INTLIST=15", descr->FormatDefault());
}

TEST_F(ParamDescrIntListFixture, GetValueDefaultString)
{
	EXPECT_EQ("15", descr->GetValueDefaultString());
}

TEST_F(ParamDescrIntListFixture, ConvertIfAllowedTrue)
{
	ew::ParamValue v;
	ASSERT_TRUE(descr->ConvertIfAllowed("10", &v));
	EXPECT_EQ(10, v.i);
}

TEST_F(ParamDescrIntListFixture, ConvertIfAllowedFalse)
{
	EXPECT_FALSE(descr->ConvertIfAllowed("6", NULL));
}

TEST_F(ParamDescrIntListFixture, ConvertIfAllowedBadType)
{
	EXPECT_FALSE(descr->ConvertIfAllowed("2?", NULL));
}


//////////////////////////////////////////////////////////////////////
// ParamArchive
//////////////////////////////////////////////////////////////////////

class ParamArchiveSingleFixture : public testing::Test
{
protected:
	virtual void SetUp() {
		descr = ew::ParamDescr::CreateParamDescrInt("Int", 1, 0, 5);
		ar.AppendDescr(descr);
	}

	virtual void TearDown() {
		delete descr;
	}

	ew::ParamArchive ar;
	ew::ParamDescr *descr;
};

TEST_F(ParamArchiveSingleFixture, GetValueIntDefault)
{
	EXPECT_EQ(1, ar.GetValueInt(descr));
}

TEST_F(ParamArchiveSingleFixture, GetParamsDefaultSingle)
{
	EXPECT_EQ("INT=1", ar.GetParamsDefault());
}

TEST_F(ParamArchiveSingleFixture, ParseEmpty)
{
	ASSERT_TRUE(ar.Parse(""));
	EXPECT_EQ(1, ar.GetValueInt(descr));
}

TEST_F(ParamArchiveSingleFixture, ParseNotFound)
{
	EXPECT_FALSE(ar.Parse("i=3"));
}

TEST_F(ParamArchiveSingleFixture, ParseBadSyntax1)
{
	EXPECT_FALSE(ar.Parse("int="));
}

TEST_F(ParamArchiveSingleFixture, ParseBadSyntax2)
{
	EXPECT_FALSE(ar.Parse("int=a"));
}

TEST_F(ParamArchiveSingleFixture, ParseValue)
{
	ASSERT_TRUE(ar.Parse("int=3"));
	EXPECT_EQ(3, ar.GetValueInt(descr));
}

class ParamArchiveMultipleFixture : public testing::Test
{
protected:
	virtual void SetUp() {
		d1 = ew::ParamDescr::CreateParamDescrInt("Int1", 1, 0, 5);
		ar.AppendDescr(d1);
		d2 = ew::ParamDescr::CreateParamDescrInt("Int2", 2, 0, 5);
		ar.AppendDescr(d2);
	}

	virtual void TearDown() {
		delete d1;
		delete d2;
	}

	ew::ParamArchive ar;
	ew::ParamDescr *d1;
	ew::ParamDescr *d2;
};

TEST_F(ParamArchiveMultipleFixture, GetParamsDefaultMultiple)
{
	EXPECT_EQ("INT1=1;INT2=2", ar.GetParamsDefault());
}

TEST_F(ParamArchiveMultipleFixture, ParseValues)
{
	ASSERT_TRUE(ar.Parse("int1=3;int2=4"));
	EXPECT_EQ(3, ar.GetValueInt(d1));
	EXPECT_EQ(4, ar.GetValueInt(d2));
}

TEST_F(ParamArchiveMultipleFixture, ParseValuesSemicolonInMiddle)
{
	ASSERT_TRUE(ar.Parse("int1=3;;int2=4"));
	EXPECT_EQ(3, ar.GetValueInt(d1));
	EXPECT_EQ(4, ar.GetValueInt(d2));
}

TEST_F(ParamArchiveMultipleFixture, BadSyntax)
{
	EXPECT_FALSE(ar.Parse("int1=3;pippo;int2=4"));
}

TEST_F(ParamArchiveMultipleFixture, NameNotAllowed)
{
	EXPECT_FALSE(ar.Parse("int1=3;int3=5;int2=4"));
}
