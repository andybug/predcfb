
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
}
