
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
	}

	void ObjectDBTest::TearDown()
	{
	}

	/*************************************************/

	TEST_F(ObjectDBTest, AddConferenceAndLookup) {
		struct conference conf;
		struct conference *pconf;
		objectid id;
		int err;

		strcpy(conf.name, "Southeastern");
		conf.div = CONFERENCE_FBS;

		err = objectdb_add_conference(&conf, &id);
		ASSERT_EQ(err, OBJECTDB_OK);

		pconf = objectdb_get_conference(&id);
		ASSERT_NE(pconf, (struct conference*)NULL);
		ASSERT_STREQ(pconf->name, conf.name);
		ASSERT_EQ(pconf->div, conf.div);
	}

	TEST_F(ObjectDBTest, LookupBogusConference) {
		objectid id;
		struct conference *conf;

		memset(id.md, 0xaf, sizeof(id.md));

		conf = objectdb_get_conference(&id);
		ASSERT_EQ(conf, (struct conference*)NULL);
		ASSERT_EQ(objectdb_errno, OBJECTDB_ENOTFOUND);
	}

	TEST_F(ObjectDBTest, InsertConferenceTwice) {
		struct conference conf;
		objectid id;
		int err;

		strcpy(conf.name, "Southeastern");
		conf.div = CONFERENCE_FBS;

		err = objectdb_add_conference(&conf, &id);
		ASSERT_EQ(err, OBJECTDB_OK);

		err = objectdb_add_conference(&conf, &id);
		ASSERT_EQ(err, OBJECTDB_ERROR);
		ASSERT_EQ(objectdb_errno, OBJECTDB_EDUPLICATE);
	}
}
