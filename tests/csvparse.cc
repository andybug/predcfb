
#include <vector>
#include <string>

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

	int insertStrings(const std::vector<std::string> &strs);
	void setupShorts();
	void setupIntegers();

	/* data */
	struct csvline csvl;

	static std::vector<std::string> int_good_strs;
	static std::vector<int> int_good_vals;
	static std::vector<std::string> int_range_strs;
	static std::vector<std::string> int_bad_strs;

	static std::vector<std::string> short_good_strs;
	static std::vector<short> short_good_vals;
	static std::vector<std::string> short_range_strs;
	static std::vector<std::string> short_bad_strs;
};

/* TODO: implement CSVParseTest class and unit tests */

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

int CSVLineTest::insertStrings(const std::vector<std::string> &strs)
{
	std::vector<std::string>::const_iterator it;

	for (it = strs.begin(); it != strs.end(); it++) {
		size_t len = (*it).length();
		if (csvline_add(&csvl, (*it).c_str(), len) != CSVP_OK)
			return CSVP_ERROR;
	}

	return CSVP_OK;
}

std::vector<std::string> CSVLineTest::short_good_strs;
std::vector<short> CSVLineTest::short_good_vals;
std::vector<std::string> CSVLineTest::short_range_strs;
std::vector<std::string> CSVLineTest::short_bad_strs;

void CSVLineTest::setupShorts()
{
	short_good_strs.push_back("0");
	short_good_strs.push_back("127");
	short_good_strs.push_back("-10");
	short_good_strs.push_back("32767");

	short_good_vals.push_back(0);
	short_good_vals.push_back(127);
	short_good_vals.push_back(-10);
	short_good_vals.push_back(32767);

	short_range_strs.push_back("32768");
	short_range_strs.push_back("2147483647");

	short_bad_strs.push_back("string");
	short_bad_strs.push_back("127string");
}

std::vector<std::string> CSVLineTest::int_good_strs;
std::vector<int> CSVLineTest::int_good_vals;
std::vector<std::string> CSVLineTest::int_range_strs;
std::vector<std::string> CSVLineTest::int_bad_strs;

void CSVLineTest::setupIntegers()
{
	int_good_strs.push_back("0");
	int_good_strs.push_back("127");
	int_good_strs.push_back("-10");
	int_good_strs.push_back("32767");
	int_good_strs.push_back("32768");
	int_good_strs.push_back("2147483647");

	int_good_vals.push_back(0);
	int_good_vals.push_back(127);
	int_good_vals.push_back(-10);
	int_good_vals.push_back(32767);
	int_good_vals.push_back(32768);
	int_good_vals.push_back(2147483647);

	int_range_strs.push_back("2147483648");

	int_bad_strs.push_back("string");
	int_bad_strs.push_back("127string");
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

	setupIntegers();

	err = insertStrings(int_good_strs);
	ASSERT_EQ(CSVP_OK, err);

	for (int i = 0; i < csvl.num_fields; i++) {
		int val;
		err = csvline_int_at(&csvl, i, &val);
		ASSERT_EQ(CSVP_OK, err);
		ASSERT_EQ(CSVLINE_ENONE, csvl.error);
		ASSERT_EQ(int_good_vals.at(i), val);
	}

	csvline_clear(&csvl);
	err = insertStrings(int_range_strs);
	ASSERT_EQ(CSVP_OK, err);

	for (int i = 0; i < csvl.num_fields; i++) {
		int val;
		err = csvline_int_at(&csvl, i, &val);
		ASSERT_EQ(CSVP_ERROR, err);
		ASSERT_EQ(CSVLINE_ERANGE, csvl.error);
		csvl.error = CSVLINE_ENONE;
	}

	csvline_clear(&csvl);

	err = insertStrings(int_bad_strs);
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
	int err;

	setupShorts();

	err = insertStrings(short_good_strs);
	ASSERT_EQ(CSVP_OK, err);

	for (int i = 0; i < csvl.num_fields; i++) {
		short val;
		err = csvline_short_at(&csvl, i, &val);
		ASSERT_EQ(CSVP_OK, err);
		ASSERT_EQ(CSVLINE_ENONE, csvl.error);
		ASSERT_EQ(short_good_vals.at(i), val);
	}

	csvline_clear(&csvl);
	err = insertStrings(short_range_strs);
	ASSERT_EQ(CSVP_OK, err);

	for (int i = 0; i < csvl.num_fields; i++) {
		short val;
		err = csvline_short_at(&csvl, i, &val);
		ASSERT_EQ(CSVP_ERROR, err);
		ASSERT_EQ(CSVLINE_ERANGE, csvl.error);
		csvl.error = CSVLINE_ENONE;
	}

	csvline_clear(&csvl);
	err = insertStrings(short_bad_strs);
	ASSERT_EQ(CSVP_OK, err);

	for (int i = 0; i < csvl.num_fields; i++) {
		short val;
		err = csvline_short_at(&csvl, i, &val);
		ASSERT_EQ(CSVP_ERROR, err);
		ASSERT_EQ(CSVLINE_EWRONGTYPE, csvl.error);
		csvl.error = CSVLINE_ENONE;
	}
}
