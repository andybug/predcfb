
#include <time.h>
#include <iostream>

#include <gtest/gtest.h>

extern "C" {
#include <polarssl/sha1.h>
#include <predcfb/predcfb.h>
#include <predcfb/objectid.h>
}

namespace {

	class ObjectIDTest : public ::testing::Test {
		protected:
			ObjectIDTest() {}
			virtual ~ObjectIDTest() {}
			virtual void SetUp();
			virtual void TearDown();

			struct objectid oid1;
			struct objectid oid2;
	};

	void ObjectIDTest::SetUp()
	{
		memset(oid1.md, 0xff, OBJECTID_MD_SIZE);
		memset(oid2.md, 0xaf, OBJECTID_MD_SIZE);
	}

	void ObjectIDTest::TearDown()
	{
	}

	/*************************************************/

	TEST_F(ObjectIDTest, Compare)
	{
		bool same;

		same = objectid_compare(&oid1, &oid2);
		ASSERT_TRUE(same == false);

		same = objectid_compare(&oid1, &oid1);
		ASSERT_TRUE(same == true);
		ASSERT_EQ(true, same);
	}

	TEST_F(ObjectIDTest, Stringify)
	{
		char buf[OBJECTID_MD_STR_SIZE];

		objectid_string(&oid1, buf);

		/* need to subtract one because the last byte is '\0' */
		for (int i = 0; i < (OBJECTID_MD_STR_SIZE - 1); i++)
			ASSERT_EQ('f', buf[i]);
		ASSERT_EQ('\0', buf[OBJECTID_MD_STR_SIZE - 1]);

		objectid_string(&oid2, buf);

		for (int i = 0; i < (OBJECTID_MD_STR_SIZE - 1); i += 2) {
			ASSERT_EQ('a', buf[i]);
			ASSERT_EQ('f', buf[i+1]);
		}
		ASSERT_EQ('\0', buf[OBJECTID_MD_STR_SIZE - 1]);
	}

	TEST_F(ObjectIDTest, Conference)
	{
		static const char *conf_name = "Southeastern Conference";
		static const char *conf_sha1 =
			"795abde05cab42bfc37f11de82f86ec1f9275c24";
		struct conference conf;
		struct objectid oid;
		char buf[OBJECTID_MD_STR_SIZE];
		int diff;

		strncpy(conf.name, conf_name, CONFERENCE_NAME_MAX);
		conf.subdivision = CONFERENCE_FBS;

		objectid_from_conference(&conf, &oid);
		objectid_string(&oid, buf);

		diff = strcmp(buf, conf_sha1);
		ASSERT_EQ(0, diff);
	}

	TEST_F(ObjectIDTest, Team)
	{
		static const char *team_name = "Texas A&M";
		static const char *team_sha1 =
			"f4bfdc649e8390f5c3529d7e25940f67fee3a61f";
		struct team team;
		struct objectid oid;
		char buf[OBJECTID_MD_STR_SIZE];
		int diff;

		strncpy(team.name, team_name, TEAM_NAME_MAX);

		objectid_from_team(&team, &oid);
		objectid_string(&oid, buf);

		diff = strcmp(buf, team_sha1);
		ASSERT_EQ(0, diff);
	}

	TEST_F(ObjectIDTest, Game)
	{
		const char *team1_name = "Alabama";
		const char *team2_name = "LSU";
		const char *game_date = "2013-12-01 00:00:00 +0000";
		const char *game_sha1 = "a1bbb417067153367d1443d82bc4b65904d8b13a";

		struct team team1;
		struct team team2;
		struct game game;
		struct objectid oid1;
		struct objectid oid2;
		struct objectid oid;
		char buf[OBJECTID_MD_STR_SIZE];
		struct tm tm;
		int diff;

		strncpy(team1.name, team1_name, TEAM_NAME_MAX);
		strncpy(team2.name, team2_name, TEAM_NAME_MAX);

		game.home = &team1;
		game.away = &team2;

		// times are always GMT
		strptime(game_date, "%Y-%m-%d %H:%M:%S %z", &tm);
		game.date = mktime(&tm);

		objectid_from_game(&game, &oid);
		objectid_string(&oid, buf);

		diff = strcmp(buf, game_sha1);
		ASSERT_EQ(0, diff);
	}
}
