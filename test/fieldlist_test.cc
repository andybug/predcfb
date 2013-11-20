
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

			void addThreeStrings();
	};

	void FieldListTest::SetUp()
	{
		memset(&fieldlist, 0, sizeof(struct fieldlist));

		strings.push_back("test one");
		strings.push_back("test two");
		strings.push_back("test three");
	}

	void FieldListTest::TearDown()
	{
	}

	void FieldListTest::addThreeStrings()
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

	/*************************************************/

	TEST_F(FieldListTest, AddOne) {
		const std::string str = "test string";

		fieldlist_add(&fieldlist, str.c_str(), str.length());

		ASSERT_EQ(fieldlist.num_fields, 1);
		ASSERT_EQ(fieldlist.error, 0);
		ASSERT_STREQ(fieldlist.fields[0], str.c_str());
	}

	TEST_F(FieldListTest, AddThree) {
		addThreeStrings();

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
		addThreeStrings();

		/*
		 * go ahead and set some fields to make sure
		 * that they are being cleared right
		 */

		fieldlist.error = FIELDLIST_ESTRBUFSPACE;
		fieldlist.iter = 2;

		fieldlist_clear(&fieldlist);

		ASSERT_EQ(fieldlist.num_fields, 0);
		ASSERT_EQ(fieldlist.error, 0);
		ASSERT_EQ(fieldlist.iter, 0);
	}

	TEST_F(FieldListTest, Iterator) {
		addThreeStrings();

		const char *str;
		std::vector<const char*>::iterator it;
		int count = 0;

		str = fieldlist_iter_begin(&fieldlist);
		it = strings.begin();

		while (str) {
			ASSERT_STREQ(str, *it);
			count++;

			str = fieldlist_iter_next(&fieldlist);
			it++;
		}

		ASSERT_EQ(count, 3);
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
