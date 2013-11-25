
#include <string>

#include <gtest/gtest.h>

extern "C" {
#include <predcfb/objectdb.h>
}

namespace {

	class ObjectDBTest : public ::testing::Test {
		protected:
			ObjectDBTest() {}
			virtual ~ObjectDBTest() {}
			virtual void SetUp();
			virtual void TearDown();
	};

	void ObjectDBTest::SetUp()
	{
		objectdb_clear();
		objectdb_errno = OBJECTDB_ENONE;
	}

	void ObjectDBTest::TearDown()
	{
	}

	/*************************************************/

	TEST_F(ObjectDBTest, CreateConference) {
		struct conference *c;

		c = objectdb_create_conference();
		ASSERT_TRUE(c != NULL);
		ASSERT_EQ(OBJECTDB_ENONE, objectdb_errno);
	}

	TEST_F(ObjectDBTest, CreateTooManyConferences) {
		struct conference *c;
		int i;

		for (i = 0; i < CONFERENCE_NUM_MAX; i++) {
			c = objectdb_create_conference();
			ASSERT_TRUE(c != NULL);
			ASSERT_EQ(OBJECTDB_ENONE, objectdb_errno);
		}

		c = objectdb_create_conference();
		ASSERT_TRUE(c == NULL);
		ASSERT_EQ(OBJECTDB_EMAXCONFS, objectdb_errno);
	}

	TEST_F(ObjectDBTest, AddConferenceAndLookup) {
		struct conference *c, *c2;
		objectid id;
		int err;

		c = objectdb_create_conference();
		ASSERT_TRUE(c != NULL);

		strcpy(c->name, "Southeastern");
		c->subdivision = CONFERENCE_FBS;

		err = objectdb_add_conference(c, &id);
		ASSERT_EQ(OBJECTDB_OK, err);

		c2 = objectdb_get_conference(&id);
		ASSERT_TRUE(c2 != NULL);
		ASSERT_STREQ(c->name, c2->name);
		ASSERT_EQ(c->subdivision, c2->subdivision);
	}

	TEST_F(ObjectDBTest, LookupBogusConference) {
		objectid id;
		struct conference *conf;

		memset(id.md, 0xaf, sizeof(id.md));

		conf = objectdb_get_conference(&id);
		ASSERT_TRUE(conf == NULL);
		ASSERT_EQ(OBJECTDB_ENOTFOUND, objectdb_errno);
	}

	TEST_F(ObjectDBTest, InsertConferenceTwice) {
		struct conference *c;
		objectid id;
		int err;

		c = objectdb_create_conference();
		ASSERT_TRUE(c != NULL);

		strcpy(c->name, "Southeastern");
		c->subdivision = CONFERENCE_FBS;

		err = objectdb_add_conference(c, &id);
		ASSERT_EQ(OBJECTDB_OK, err);

		err = objectdb_add_conference(c, &id);
		ASSERT_EQ(OBJECTDB_ERROR, err);
		ASSERT_EQ(OBJECTDB_EDUPLICATE, objectdb_errno);
	}
}
