
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

/* StrBufTest implementation */

void StrBufTest::SetUp()
{
	strbuf_clear(&sb);
}

/* StrBufTest tests */

TEST_F(StrBufTest, Clear)
{
	/* sb is cleared during SetUp, so just check the values */
	ASSERT_EQ(0, sb.used);
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
	ASSERT_EQ(STRBUF_SIZE, sb.used);
	ASSERT_STREQ(buf, result);
}

#if 0
namespace {

	class FieldListTest : public ::testing::Test {
		protected:
			FieldListTest() {}
			virtual ~FieldListTest() {}
			virtual void SetUp();
			virtual void TearDown();

			struct fieldlist fieldlist;
			std::vector<const char*> strings;
			std::vector<const char*> numbers;

			void addStrings();
			void addNumbers();
	};

	void FieldListTest::SetUp()
	{
		fieldlist_clear(&fieldlist);

		strings.push_back("test one");
		strings.push_back("test two");
		strings.push_back("test three");

		numbers.push_back("0");
		numbers.push_back("127");
		numbers.push_back("-10");
		numbers.push_back("32767");
		numbers.push_back("32768");
		numbers.push_back("2147483647");
		numbers.push_back("2147483648");
		numbers.push_back("string");
	}

	void FieldListTest::TearDown()
	{
	}

	void FieldListTest::addStrings()
	{
		int i;
		size_t len;
		const char *str;

		for (i = 0; i < 3; i++) {
			str = strings.at(i);
			len = strlen(str);

			fieldlist_add(&fieldlist, str, len);
		}
	}

	void FieldListTest::addNumbers()
	{
		size_t len;
		std::vector<const char*>::iterator it = numbers.begin();

		while (it != numbers.end()) {
			len = strlen(*it);
			fieldlist_add(&fieldlist, *it, len);

			it++;
		}
	}

	/*************************************************/

	TEST_F(FieldListTest, AddOne) {
		const std::string str = "test string";

		fieldlist_add(&fieldlist, str.c_str(), str.length());

		ASSERT_EQ(fieldlist.num_fields, 1);
		ASSERT_EQ(fieldlist.error, 0);
		ASSERT_STREQ(fieldlist.fields[0], str.c_str());
	}

	TEST_F(FieldListTest, AddThree) {
		addStrings();

		ASSERT_EQ(fieldlist.num_fields, 3);
		ASSERT_EQ(fieldlist.error, 0);

		for (int i = 0; i < 3; i++) {
			ASSERT_STREQ(fieldlist.fields[i], strings.at(i));
		}
	}

	TEST_F(FieldListTest, AddNullString) {
		fieldlist_add(&fieldlist, NULL, 0);
		ASSERT_EQ(fieldlist.num_fields, 0);
		ASSERT_EQ(fieldlist.error, FIELDLIST_ENULLSTR);
	}

	TEST_F(FieldListTest, Clear) {
		addStrings();

		/*
		 * go ahead and set some fields to make sure
		 * that they are being cleared right
		 */

		fieldlist.error = FIELDLIST_ESTRBUFSPACE;
		fieldlist.iter = 2;

		fieldlist_clear(&fieldlist);

		ASSERT_EQ(0, fieldlist.num_fields);
		ASSERT_EQ(FIELDLIST_ENONE, fieldlist.error);
		ASSERT_EQ(FIELDLIST_ITER_FIRST, fieldlist.iter);
	}

	TEST_F(FieldListTest, Iterator) {
		addStrings();

		const char *str;
		std::vector<const char*>::iterator it;
		int count = 0;

		fieldlist_iter_begin(&fieldlist);
		it = strings.begin();

		while ((str = fieldlist_iter_next(&fieldlist))) {
			ASSERT_STREQ(str, *it);
			count++;
			it++;
		}

		ASSERT_EQ(count, 3);

		str = fieldlist_iter_next(&fieldlist);
		ASSERT_EQ(NULL, str);
		ASSERT_EQ(FIELDLIST_EITEREND, fieldlist.error);
	}

	TEST_F(FieldListTest, IteratorInt) {
		addNumbers();

		int err;
		int val;

		fieldlist_iter_begin(&fieldlist);

		err = fieldlist_iter_next_int(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_OK, err);
		ASSERT_EQ(0, val);

		err = fieldlist_iter_next_int(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_OK, err);
		ASSERT_EQ(127, val);

		err = fieldlist_iter_next_int(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_OK, err);
		ASSERT_EQ(-10, val);

		err = fieldlist_iter_next_int(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_OK, err);
		ASSERT_EQ(32767, val);

		err = fieldlist_iter_next_int(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_OK, err);
		ASSERT_EQ(32768, val);

		err = fieldlist_iter_next_int(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_OK, err);
		ASSERT_EQ(2147483647, val);

		err = fieldlist_iter_next_int(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_ERROR, err);
		ASSERT_EQ(FIELDLIST_ERANGE, fieldlist.error);

		err = fieldlist_iter_next_int(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_ERROR, err);
		ASSERT_EQ(FIELDLIST_EWRONGTYPE, fieldlist.error);

		const char *str = fieldlist_iter_next(&fieldlist);
		ASSERT_EQ(NULL, str);
		ASSERT_EQ(FIELDLIST_EITEREND, fieldlist.error);
	}

	TEST_F(FieldListTest, IteratorShort) {
		addNumbers();

		int err;
		short val;

		fieldlist_iter_begin(&fieldlist);

		err = fieldlist_iter_next_short(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_OK, err);
		ASSERT_EQ(0, val);

		err = fieldlist_iter_next_short(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_OK, err);
		ASSERT_EQ(127, val);

		err = fieldlist_iter_next_short(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_OK, err);
		ASSERT_EQ(-10, val);

		err = fieldlist_iter_next_short(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_OK, err);
		ASSERT_EQ(32767, val);

		err = fieldlist_iter_next_short(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_ERROR, err);
		ASSERT_EQ(FIELDLIST_ERANGE, fieldlist.error);

		err = fieldlist_iter_next_short(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_ERROR, err);
		ASSERT_EQ(FIELDLIST_ERANGE, fieldlist.error);

		err = fieldlist_iter_next_short(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_ERROR, err);
		ASSERT_EQ(FIELDLIST_ERANGE, fieldlist.error);

		err = fieldlist_iter_next_short(&fieldlist, &val);
		ASSERT_EQ(FIELDLIST_ERROR, err);
		ASSERT_EQ(FIELDLIST_EWRONGTYPE, fieldlist.error);

		const char *str = fieldlist_iter_next(&fieldlist);
		ASSERT_EQ(NULL, str);
		ASSERT_EQ(FIELDLIST_EITEREND, fieldlist.error);
	}

	TEST_F(FieldListTest, TooBig) {
		char buf[STRBUF_SIZE];

		/*
		 * this should fail because it needs to add the null
		 * byte
		 */

		fieldlist_add(&fieldlist, buf, STRBUF_SIZE);
		ASSERT_EQ(fieldlist.num_fields, 0);
		ASSERT_EQ(fieldlist.error, FIELDLIST_ESTRBUFSPACE);

		/* reset the error */
		fieldlist_clear(&fieldlist);

		/* this should just fit */
		fieldlist_add(&fieldlist, buf, STRBUF_SIZE - 1);
		ASSERT_EQ(fieldlist.num_fields, 1);
		ASSERT_EQ(fieldlist.error, FIELDLIST_ENONE);
	}

	TEST_F(FieldListTest, TooBigTwo) {
		char buf[STRBUF_SIZE];
		EXPECT_EQ(STRBUF_SIZE % 2, 0);

		size_t len = (STRBUF_SIZE / 2) - 1;

		fieldlist_add(&fieldlist, buf, len);
		ASSERT_EQ(fieldlist.num_fields, 1);
		ASSERT_EQ(fieldlist.error, FIELDLIST_ENONE);

		/* this should just fit */
		fieldlist_add(&fieldlist, buf, len);
		ASSERT_EQ(fieldlist.num_fields, 2);
		ASSERT_EQ(fieldlist.error, FIELDLIST_ENONE);

		/* but no more */
		fieldlist_add(&fieldlist, buf, 1);
		ASSERT_EQ(fieldlist.num_fields, 2);
		ASSERT_EQ(fieldlist.error, FIELDLIST_ESTRBUFSPACE);
	}

	TEST_F(FieldListTest, TooMany) {
		char buf[1];
		int i;

		for (i = 0; i < FIELDLIST_MAX_FIELDS; i++) {
			fieldlist_add(&fieldlist, buf, 1);
			ASSERT_EQ(fieldlist.num_fields, i + 1);
			ASSERT_EQ(fieldlist.error, FIELDLIST_ENONE);
		}

		/* this should be one too many */
		fieldlist_add(&fieldlist, buf, 1);
		ASSERT_EQ(fieldlist.num_fields, FIELDLIST_MAX_FIELDS);
		ASSERT_EQ(fieldlist.error, FIELDLIST_EMAXFIELDS);
	}
}

#endif
