
#include <gtest/gtest.h>
#include <predcfb/fieldlist.h>

namespace {

	class FieldListTest : public ::testing::Test {
		protected:
			FieldListTest() {
			}

			virtual ~FieldListTest() {
			}

			virtual void SetUp() {
			}

			virtual void TearDown() {
			}
	};

	TEST_F(FieldListTest, TestZero) {
		EXPECT_EQ(0, 0);
	}

	TEST_F(FieldListTest, TestOne) {
		EXPECT_EQ(1, 1);
	}
}
