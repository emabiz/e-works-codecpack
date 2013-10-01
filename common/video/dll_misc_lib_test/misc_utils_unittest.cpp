#include "stdafx.h"
#include "misc_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Split
//////////////////////////////////////////////////////////////////////

TEST(Split, SimpleCase)
{
	std::vector<std::string> v(ew::Split("a;b", ';'));
	ASSERT_EQ(2, v.size());
	EXPECT_EQ("a", v[0]);
	EXPECT_EQ("b", v[1]);
}

TEST(Split, NoDelim)
{
	std::vector<std::string> v(ew::Split("a", ';'));
	ASSERT_EQ(1, v.size());
	EXPECT_EQ("a", v[0]);	
}

TEST(Split, DelimAtEnd)
{
	std::vector<std::string> v(ew::Split("a;", ';'));
	ASSERT_EQ(1, v.size());
	EXPECT_EQ("a", v[0]);	
}

TEST(Split, DelimAtBeginning)
{
	std::vector<std::string> v(ew::Split(";a", ';'));
	ASSERT_EQ(1, v.size());
	EXPECT_EQ("a", v[0]);	
}

TEST(Split, Empty)
{
	std::vector<std::string> v(ew::Split("", ';'));
	EXPECT_EQ(0, v.size());
}

//////////////////////////////////////////////////////////////////////
// SplitKeyValue
//////////////////////////////////////////////////////////////////////

TEST(SplitKeyValue, SimpleCase)
{
	std::string name;
	std::string value;

	ASSERT_TRUE(ew::SplitKeyValue("a=b", '=', &name, &value));
	EXPECT_EQ("a", name);
	EXPECT_EQ("b", value);
}

TEST(SplitKeyValue, Empty)
{
	std::string name;
	std::string value;

	EXPECT_FALSE(ew::SplitKeyValue("", '=', &name, &value));
}

TEST(SplitKeyValue, EmptyName)
{
	std::string name;
	std::string value;

	EXPECT_FALSE(ew::SplitKeyValue("=b", '=', &name, &value));
}

TEST(SplitKeyValue, EmptyValue)
{
	std::string name;
	std::string value;

	EXPECT_FALSE(ew::SplitKeyValue("a=", '=', &name, &value));
}

//////////////////////////////////////////////////////////////////////
// Trim
//////////////////////////////////////////////////////////////////////

TEST(Trim, NoSpaces)
{
	EXPECT_EQ("a", ew::Trim("a"));
}

TEST(Trim, Empty)
{
	EXPECT_EQ("", ew::Trim(""));
}

TEST(Trim, Space)
{
	EXPECT_EQ("", ew::Trim(" "));
}

TEST(Trim, SpaceLeft)
{
	EXPECT_EQ("a", ew::Trim(" a"));
}

TEST(Trim, SpaceRight)
{
	EXPECT_EQ("a", ew::Trim("a "));
}

TEST(Trim, SpaceBothSides)
{
	EXPECT_EQ("a", ew::Trim(" a "));
}
