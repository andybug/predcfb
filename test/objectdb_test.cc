
#include <time.h>
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

	TEST_F(ObjectDBTest, CreateGame) {
		struct game *game;

		game = objectdb_create_game();
		ASSERT_TRUE(game != NULL);
		ASSERT_EQ(OBJECTDB_ENONE, objectdb_errno);
	}

	TEST_F(ObjectDBTest, CreateTooManyGames) {
		struct game *game;
		int i;

		for (i = 0; i < GAME_NUM_MAX; i++) {
			game = objectdb_create_game();
			ASSERT_TRUE(game != NULL);
			ASSERT_EQ(OBJECTDB_ENONE, objectdb_errno);
		}

		game = objectdb_create_game();
		ASSERT_TRUE(game == NULL);
		ASSERT_EQ(OBJECTDB_EMAXGAMES, objectdb_errno);
	}

	TEST_F(ObjectDBTest, AddGameAndLookup) {
		struct game *game1, *game2;
		objectid id;
		int err;
		bool equal;

		game1 = objectdb_create_game();
		ASSERT_TRUE(game1 != NULL);

		memset(game1->home_oid.md, 1, OBJECTDB_MD_SIZE);
		memset(game1->away_oid.md, 2, OBJECTDB_MD_SIZE);
		game1->date = time(NULL);

		err = objectdb_add_game(game1, &id);
		ASSERT_EQ(OBJECTDB_OK, err);

		game2 = objectdb_get_game(&id);
		ASSERT_TRUE(game2 != NULL);

		equal = objectid_compare(&game1->home_oid, &game2->home_oid);
		ASSERT_TRUE(equal == true);
		equal = objectid_compare(&game1->away_oid, &game2->away_oid);
		ASSERT_TRUE(equal == true);
	}

	TEST_F(ObjectDBTest, LookupBogusGame) {
		objectid id;
		struct game *game;

		memset(id.md, 0xaf, sizeof(id.md));

		game = objectdb_get_game(&id);
		ASSERT_TRUE(game == NULL);
		ASSERT_EQ(OBJECTDB_ENOTFOUND, objectdb_errno);
	}

	TEST_F(ObjectDBTest, InsertGameTwice) {
		struct game *game;
		objectid id;
		int err;

		game = objectdb_create_game();
		ASSERT_TRUE(game != NULL);

		memset(game->home_oid.md, 1, OBJECTDB_MD_SIZE);
		memset(game->away_oid.md, 2, OBJECTDB_MD_SIZE);
		game->date = time(NULL);

		err = objectdb_add_game(game, &id);
		ASSERT_EQ(OBJECTDB_OK, err);

		err = objectdb_add_game(game, &id);
		ASSERT_EQ(OBJECTDB_ERROR, err);
		ASSERT_EQ(OBJECTDB_EDUPLICATE, objectdb_errno);
	}

	TEST_F(ObjectDBTest, LinkObjects) {
		/*
		 * add a conference, two teams in that conference,
		 * and a game between those two teams. Ensure that
		 * all of the pointers have been set after calling
		 * objectdb_link
		 */

		struct conference *conf;
		struct team *team1, *team2;
		struct game *game;
		objectid conf_oid;
		objectid team1_oid, team2_oid;
		objectid game_oid;
		int err;

		/* add conference */
		conf = objectdb_create_conference();
		ASSERT_TRUE(conf != NULL);

		strcpy(conf->name, "Conference");
		conf->subdivision = CONFERENCE_FBS;

		err = objectdb_add_conference(conf, &conf_oid);
		ASSERT_EQ(OBJECTDB_OK, err);


		/* add first team */
		team1 = objectdb_create_team();
		ASSERT_TRUE(team1 != NULL);

		strcpy(team1->name, "Team One");
		team1->conf_oid = conf_oid;

		err = objectdb_add_team(team1, &team1_oid);
		ASSERT_EQ(OBJECTDB_OK, err);


		/* add second team */
		team2 = objectdb_create_team();
		ASSERT_TRUE(team2 != NULL);

		strcpy(team2->name, "Team Two");
		team2->conf_oid = conf_oid;

		err = objectdb_add_team(team2, &team2_oid);
		ASSERT_EQ(OBJECTDB_OK, err);


		/* add game */
		game = objectdb_create_game();
		ASSERT_TRUE(game != NULL);

		game->home_oid = team1_oid;
		game->away_oid = team2_oid;
		memset(&game->date, 0, sizeof(time_t));

		err = objectdb_add_game(game, &game_oid);
		ASSERT_EQ(OBJECTDB_OK, err);


		/* link objects */
		err = objectdb_link();
		ASSERT_EQ(OBJECTDB_OK, err);

		/* check pointers */
		ASSERT_EQ(conf, team1->conf);
		ASSERT_EQ(conf, team2->conf);
		ASSERT_EQ(team1, game->home);
		ASSERT_EQ(team2, game->away);
	}
}
