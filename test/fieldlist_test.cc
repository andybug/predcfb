
#include <string>
#include <vector>

#include <gtest/gtest.h>

extern "C" {
#include <predcfb/fieldlist.h>
}

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
