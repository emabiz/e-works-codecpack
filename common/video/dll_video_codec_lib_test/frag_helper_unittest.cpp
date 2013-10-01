#include "stdafx.h"
#include "frag_helper.h"

#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// FragReader
//////////////////////////////////////////////////////////////////////

TEST(FragReader, ConstructorSimple)
{
	FragItem frag_item;
	frag_item.data = NULL;
	frag_item.size = 0;
	
	ew::FragReader frag_reader(&frag_item, 1);
	ASSERT_EQ(1, frag_reader.frag_count());
	EXPECT_TRUE(NULL == frag_reader.get_item_data(0));
	EXPECT_EQ(0, frag_reader.get_item_size(0));
}


//////////////////////////////////////////////////////////////////////
// FragWriter
//////////////////////////////////////////////////////////////////////

TEST(FragWriter, ConstructorSimple)
{
	ew::FragWriter frag_writer(1);
	EXPECT_EQ(1, frag_writer.frag_ary_max_size());
	EXPECT_EQ(0, frag_writer.frag_count());
	EXPECT_TRUE(NULL != frag_writer.frag_ary());
}

class FragWriterAppendSingle : public testing::Test
{
protected:
	virtual void SetUp() {
		frag_writer = new ew::FragWriter(1);
		data = 23;
		frag_writer->AppendItem(&data, sizeof(data));
	}

	virtual void TearDown() {
		delete frag_writer;
	}

	ew::FragWriter *frag_writer;
	unsigned char data;
};

TEST_F(FragWriterAppendSingle, AppendItemSingle)
{
	ASSERT_EQ(1, frag_writer->frag_count());
	const FragItem *frag_ary = frag_writer->frag_ary();
	ASSERT_TRUE(NULL != frag_ary);
	EXPECT_EQ(sizeof(data), frag_ary[0].size);
	ASSERT_TRUE(NULL != frag_ary[0].data);
	EXPECT_EQ(data, *(frag_ary[0].data));
}

TEST_F(FragWriterAppendSingle, ClearSingle)
{
	frag_writer->Clear();
	ASSERT_EQ(0, frag_writer->frag_count());
}
