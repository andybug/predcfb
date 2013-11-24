
#include <gtest/gtest.h>

extern "C" {
#include <predcfb/zipfile.h>
}

namespace {

	class ZipFileTest : public ::testing::Test {
		protected:
			ZipFileTest() {}
			virtual ~ZipFileTest() {}
			virtual void SetUp();
			virtual void TearDown();
			zf_readctx *zf;
	};

	void ZipFileTest::SetUp()
	{
		zf = zipfile_open_archive("test/data/good.zip");
		EXPECT_NE((zf_readctx*) NULL, zf);

		enum zipfile_err err = zipfile_get_error(zf);
		ASSERT_EQ(ZIPFILE_ENONE, err);
	}

	void ZipFileTest::TearDown()
	{
		if (zipfile_close_archive(zf) != ZIPFILE_OK) {
			enum zipfile_err err = zipfile_get_error(zf);
			EXPECT_EQ(ZIPFILE_ESTILLOPEN, err);

			/* i dunno... */
			zipfile_close_file(zf);
			zipfile_close_archive(zf);
		}
	}

	/*************************************************/

	TEST(ZipFileTestNoFixture, OpenNonExistantArchive) {
		zf_readctx *zf;

		zf = zipfile_open_archive("/a/bogus/file.zip");
		EXPECT_NE((zf_readctx*) NULL, zf);

		enum zipfile_err err = zipfile_get_error(zf);
		ASSERT_EQ(ZIPFILE_EPATH, err);
	}

	TEST(ZipFileTestNoFixture, OpenArchive) {
		zf_readctx *zf;

		zf = zipfile_open_archive("test/data/good.zip");
		EXPECT_NE((zf_readctx*) NULL, zf);

		enum zipfile_err err = zipfile_get_error(zf);
		ASSERT_EQ(ZIPFILE_ENONE, err);
	}

	TEST(ZipFileTestNoFixture, OpenBadArchive) {
		zf_readctx *zf;

		zf = zipfile_open_archive("test/data/notazipfile.zip");
		EXPECT_NE((zf_readctx*) NULL, zf);

		enum zipfile_err err = zipfile_get_error(zf);
		ASSERT_EQ(ZIPFILE_EFILEBAD, err);
	}

	TEST_F(ZipFileTest, OpenNonExistantFile) {
		zipfile_open_file(zf, "thisfileisnotlegit.txt");
		enum zipfile_err err = zipfile_get_error(zf);
		ASSERT_EQ(ZIPFILE_ENOENT, err);
	}

	TEST_F(ZipFileTest, OpenFile) {
		int err = zipfile_open_file(zf, "fileone.txt");
		ASSERT_EQ(ZIPFILE_OK, err);
	}

	TEST_F(ZipFileTest, OpenTwoFiles) {
		int err;

		err = zipfile_open_file(zf, "fileone.txt");
		ASSERT_EQ(ZIPFILE_OK, err);

		err = zipfile_open_file(zf, "filetwo.txt");
		ASSERT_EQ(ZIPFILE_ERROR, err);

		enum zipfile_err errorcode = zipfile_get_error(zf);
		ASSERT_EQ(ZIPFILE_ESTILLOPEN, errorcode);
	}

	TEST_F(ZipFileTest, ReadFile) {
		static const int BUF_SIZE = 128;
		char buf[BUF_SIZE];
		int err;
		ssize_t bytes;

		err = zipfile_open_file(zf, "fileone.txt");
		ASSERT_EQ(ZIPFILE_OK, err);

		bytes = zipfile_read_file(zf, buf, BUF_SIZE);
		ASSERT_NE(ZIPFILE_ERROR, bytes);
		ASSERT_EQ(bytes, 8);
		buf[bytes] = '\0';
		ASSERT_STREQ("test123\n", buf);
	}

	TEST_F(ZipFileTest, CloseFile) {
		int err;
		enum zipfile_err errorcode;

		err = zipfile_open_file(zf, "fileone.txt");
		ASSERT_EQ(ZIPFILE_OK, err);

		err = zipfile_close_file(zf);
		ASSERT_EQ(ZIPFILE_OK, err);

		err = zipfile_close_file(zf);
		ASSERT_EQ(ZIPFILE_ERROR, err);
		errorcode = zipfile_get_error(zf);
		ASSERT_EQ(ZIPFILE_ENOTOPEN, errorcode);

		err = zipfile_open_file(zf, "filetwo.txt");
		ASSERT_EQ(ZIPFILE_OK, err);
	}
}
