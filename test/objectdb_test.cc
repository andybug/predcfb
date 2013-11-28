
#include <string>

#include <gtest/gtest.h>

extern "C" {
#include <predcfb/predcfb.h>
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

	TEST_F(ObjectDBTest, CreateTeam) {
		struct team *team;

		team = objectdb_create_team();
		ASSERT_TRUE(team != NULL);
		ASSERT_EQ(OBJECTDB_ENONE, objectdb_errno);
	}

	TEST_F(ObjectDBTest, CreateTooManyTeams) {
		struct team *team;
		int i;

		for (i = 0; i < TEAM_NUM_MAX; i++) {
			team = objectdb_create_team();
			ASSERT_TRUE(team != NULL);
			ASSERT_EQ(OBJECTDB_ENONE, objectdb_errno);
		}

		team = objectdb_create_team();
		ASSERT_TRUE(team == NULL);
		ASSERT_EQ(OBJECTDB_EMAXTEAMS, objectdb_errno);
	}

	TEST_F(ObjectDBTest, AddTeamAndLookup) {
		struct team *team1, *team2;
		objectid id;
		int err;

		team1 = objectdb_create_team();
		ASSERT_TRUE(team1 != NULL);

		strcpy(team1->name, "Random name");

		err = objectdb_add_team(team1, &id);
		ASSERT_EQ(OBJECTDB_OK, err);

		team2 = objectdb_get_team(&id);
		ASSERT_TRUE(team2 != NULL);
		ASSERT_STREQ(team1->name, team2->name);
	}

	TEST_F(ObjectDBTest, LookupBogusTeam) {
		objectid id;
		struct team *team;

		memset(id.md, 0xaf, sizeof(id.md));

		team = objectdb_get_team(&id);
		ASSERT_TRUE(team == NULL);
		ASSERT_EQ(OBJECTDB_ENOTFOUND, objectdb_errno);
	}

	TEST_F(ObjectDBTest, InsertTeamTwice) {
		struct team *team;
		objectid id;
		int err;

		team = objectdb_create_team();
		ASSERT_TRUE(team != NULL);

		strcpy(team->name, "TeamName");

		err = objectdb_add_team(team, &id);
		ASSERT_EQ(OBJECTDB_OK, err);

		err = objectdb_add_team(team, &id);
		ASSERT_EQ(OBJECTDB_ERROR, err);
		ASSERT_EQ(OBJECTDB_EDUPLICATE, objectdb_errno);
	}
}
