
#include <gtest/gtest.h>

extern "C" {
#include <predcfb/csvparse.h>
}

/* StrBufTest class */

class StrBufTest : public ::testing::Test {
protected:
	/* methods */
	StrBufTest() {}
	virtual ~StrBufTest() {}
	virtual void SetUp();
	virtual void TearDown() {}

	/* data */
	struct strbuf sb;
};

class CSVLineTest : public ::testing::Test {
protected:
	/* methods */
	CSVLineTest() {}
	virtual ~CSVLineTest() {}
	virtual void SetUp();
	virtual void TearDown() {}

	int insertTestIntegersGood();
	int insertTestIntegersRange();
	int insertTestIntegersBad();
	int insertTestShorts();

	/* data */
	struct csvline csvl;

	static const char *int_good_strs[];
	static const int int_good_vals[];
	static const int int_good_num;

	static const char *int_range_strs[];
	static const int int_range_num;

	static const char *int_bad_strs[];
	static const int int_bad_num;
};

/* StrBufTest implementation */

void StrBufTest::SetUp()
{
	strbuf_clear(&sb);
}

/* StrBufTest tests */

TEST_F(StrBufTest, Clear)
{
	/* sb is cleared during SetUp, so just check the values */
	ASSERT_EQ((size_t)0, sb.used);
}

TEST_F(StrBufTest, AddOne)
{
	const char *str = "Test String";
	const char *result;
	size_t len = strlen(str);
	size_t space = len + 1;		// need to account for \0

	result = strbuf_add(&sb, str, len);

	ASSERT_EQ(space, sb.used);
	ASSERT_STREQ(str, result);
}

TEST_F(StrBufTest, AddTwo)
{
	const char *str1 = "First String";
	const char *str2 = "Second String";
	const char *result1, *result2;
	size_t len1, len2;
	size_t space1, space2;

	len1 = strlen(str1);
	space1 = len1 + 1;

	len2 = strlen(str2);
	space2 = len2 + 1;

	result1 = strbuf_add(&sb, str1, len1);
	result2 = strbuf_add(&sb, str2, len2);

	ASSERT_EQ(space1 + space2, sb.used);
	ASSERT_STREQ(str1, result1);
	ASSERT_STREQ(str2, result2);
}

TEST_F(StrBufTest, LargeStrings)
{
	char buf[STRBUF_SIZE + 1];
	const char *result;

	memset(buf, 'a', STRBUF_SIZE);
	buf[STRBUF_SIZE] = '\0';

	result = strbuf_add(&sb, buf, STRBUF_SIZE);
	ASSERT_TRUE(result == NULL);

	buf[STRBUF_SIZE - 1] = '\0';
	result = strbuf_add(&sb, buf, STRBUF_SIZE - 1);
	ASSERT_TRUE(result != NULL);
	ASSERT_EQ((size_t)STRBUF_SIZE, sb.used);
	ASSERT_STREQ(buf, result);
}

/* CSVLine methods */

void CSVLineTest::SetUp()
{
	csvline_clear(&csvl);
}

const char *CSVLineTest::int_good_strs[] = {
	"0",
	"127",
	"-10",
	"32767",
	"32768",
	"2147483647"
};

const int CSVLineTest::int_good_vals[] = {
	0,
	127,
	-10,
	32767,
	32768,
	2147483647
};

const int CSVLineTest::int_good_num = 6;

int CSVLineTest::insertTestIntegersGood()
{
	for (int i = 0; i < int_good_num; i++) {
		size_t len = strlen(int_good_strs[i]);
		if (csvline_add(&csvl, int_good_strs[i], len) != CSVP_OK)
			return CSVP_ERROR;
	}

	return CSVP_OK;
}

const char *CSVLineTest::int_range_strs[] = { "2147483648" };
const int CSVLineTest::int_range_num = 1;

int CSVLineTest::insertTestIntegersRange()
{
	for (int i = 0; i < int_range_num; i++) {
		size_t len = strlen(int_range_strs[i]);
		if (csvline_add(&csvl, int_range_strs[i], len) != CSVP_OK)
			return CSVP_ERROR;
	}

	return CSVP_OK;
}

const char *CSVLineTest::int_bad_strs[] = { "string", "127string" };
const int CSVLineTest::int_bad_num = 2;

int CSVLineTest::insertTestIntegersBad()
{
	for (int i = 0; i < int_bad_num; i++) {
		size_t len = strlen(int_bad_strs[i]);
		if (csvline_add(&csvl, int_bad_strs[i], len) != CSVP_OK)
			return CSVP_ERROR;
	}

	return CSVP_OK;
}

int CSVLineTest::insertTestShorts()
{
	return CSVP_OK;
}

/* CSVLineTest tests */

TEST_F(CSVLineTest, Clear)
{
	ASSERT_EQ(0, csvl.num_fields);
	ASSERT_EQ(0, csvl.line);
	ASSERT_EQ((size_t)0, csvl.strbuf.used);
	ASSERT_EQ(CSVLINE_ENONE, csvl.error);
}

TEST_F(CSVLineTest, AddOne)
{
	const char *str = "Test String";
	size_t len = strlen(str);
	int err;

	err = csvline_add(&csvl, str, len);
	ASSERT_EQ(err, CSVP_OK);
	ASSERT_EQ(1, csvl.num_fields);
	ASSERT_EQ(len + 1, csvl.strbuf.used);
	ASSERT_EQ(CSVLINE_ENONE, csvl.error);
	ASSERT_STREQ(csvl.fields[0], str);
}

TEST_F(CSVLineTest, AddTwo)
{
	const char *str = "Test String";
	size_t len = strlen(str);
	int err;

	err = csvline_add(&csvl, str, len);
	ASSERT_EQ(CSVP_OK, err);

	err = csvline_add(&csvl, str, len);
	ASSERT_EQ(CSVP_OK, err);
	ASSERT_EQ(2, csvl.num_fields);
	ASSERT_EQ((len+1) * 2, csvl.strbuf.used);
	ASSERT_EQ(CSVLINE_ENONE, csvl.error);

	ASSERT_STREQ(csvl.fields[0], str);
	ASSERT_STREQ(csvl.fields[1], str);
}

TEST_F(CSVLineTest, AddNull)
{
	int err;

	err = csvline_add(&csvl, NULL, 0);
	ASSERT_EQ(CSVP_ERROR, err);
	ASSERT_EQ(CSVLINE_ENULLSTR, csvl.error);
}

TEST_F(CSVLineTest, Clear2)
{
	int err;

	err = csvline_add(&csvl, "string", 6);
	ASSERT_EQ(CSVLINE_ENONE, err);

	csvline_clear(&csvl);
	ASSERT_EQ(0, csvl.num_fields);
	ASSERT_EQ(0, csvl.line);
	ASSERT_EQ((size_t)0, csvl.strbuf.used);
	ASSERT_EQ(CSVLINE_ENONE, csvl.error);
}

TEST_F(CSVLineTest, AddTooMany)
{
	int err;

	for (int i = 0; i < CSVLINE_MAX_FIELDS; i++) {
		err = csvline_add(&csvl, "string", 6);
		ASSERT_EQ(CSVP_OK, err);
		ASSERT_EQ(CSVLINE_ENONE, csvl.error);
	}

	err = csvline_add(&csvl, "string", 6);
	ASSERT_EQ(CSVP_ERROR, err);
	ASSERT_EQ(CSVLINE_EMAXFIELDS, csvl.error);
}

TEST_F(CSVLineTest, AddTooLarge)
{
	char buf[STRBUF_SIZE + 1];
	int err;

	memset(buf, 'a', STRBUF_SIZE);
	buf[STRBUF_SIZE - 1] = '\0';

	err = csvline_add(&csvl, buf, STRBUF_SIZE - 1);
	ASSERT_EQ(CSVP_OK, err);
	ASSERT_EQ(CSVLINE_ENONE, csvl.error);

	csvline_clear(&csvl);
	buf[STRBUF_SIZE - 1] = 'a';
	buf[STRBUF_SIZE] = '\0';

	err = csvline_add(&csvl, buf, STRBUF_SIZE);
	ASSERT_EQ(CSVP_ERROR, err);
	ASSERT_EQ(CSVLINE_ESTRBUFSPACE, csvl.error);
}

TEST_F(CSVLineTest, InvalidIndex)
{
	int err;
	const char *val;

	err = csvline_str_at(&csvl, 0, &val);
	ASSERT_EQ(CSVP_ERROR, err);
	ASSERT_EQ(CSVLINE_EINDEX, csvl.error);
}

TEST_F(CSVLineTest, StringAt)
{
	int err;
	const char *str_in = "Test String";
	const char *str_out;
	size_t len = strlen(str_in);
	int i;

	for (i = 0; i < 3; i++) {
		err = csvline_add(&csvl, str_in, len);
		ASSERT_EQ(CSVP_OK, err);
	}

	for (i = 0; i < 3; i++) {
		err = csvline_str_at(&csvl, i, &str_out);
		ASSERT_EQ(CSVP_OK, err);
		ASSERT_EQ(CSVLINE_ENONE, csvl.error);
		ASSERT_STREQ(str_in, str_out);
	}
}

TEST_F(CSVLineTest, IntAt)
{
	int err;

	err = insertTestIntegersGood();
	ASSERT_EQ(CSVP_OK, err);

	for (int i = 0; i < csvl.num_fields; i++) {
		int val;
		err = csvline_int_at(&csvl, i, &val);
		ASSERT_EQ(CSVP_OK, err);
		ASSERT_EQ(CSVLINE_ENONE, csvl.error);
		ASSERT_TRUE(i < int_good_num);
		ASSERT_EQ(int_good_vals[i], val);
	}

	csvline_clear(&csvl);

	err = insertTestIntegersRange();
	ASSERT_EQ(CSVP_OK, err);

	for (int i = 0; i < csvl.num_fields; i++) {
		int val;
		err = csvline_int_at(&csvl, i, &val);
		ASSERT_EQ(CSVP_ERROR, err);
		ASSERT_EQ(CSVLINE_ERANGE, csvl.error);
		csvl.error = CSVLINE_ENONE;
	}

	csvline_clear(&csvl);

	err = insertTestIntegersBad();
	ASSERT_EQ(CSVP_OK, err);

	for (int i = 0; i < csvl.num_fields; i++) {
		int val;
		err = csvline_int_at(&csvl, i, &val);
		ASSERT_EQ(CSVP_ERROR, err);
		ASSERT_EQ(CSVLINE_EWRONGTYPE, csvl.error);
		csvl.error = CSVLINE_ENONE;
	}
}

TEST_F(CSVLineTest, ShortAt)
{
}
