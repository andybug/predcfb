PROJECT(test_predcfb)

ENABLE_TESTING()


# --- build gtest library ---

INCLUDE_DIRECTORIES(gtest/include)
ADD_SUBDIRECTORY(gtest)


# --- build test_predcfb ---

ADD_EXECUTABLE(
	test_predcfb
	# --- sources ---
	fieldlist_test.cc
	objectdb_test.cc
	objectid_test.cc
	zipfile_test.cc
	# --- predcfb objects ---
	$<TARGET_OBJECTS:libpredcfb>
)

TARGET_LINK_LIBRARIES(
	test_predcfb
	# --- static libraries ---
	libcsv
	miniunz
	polarssl
	libb64
	gtest
	# --- shared libraries ---
	pthread
	z
)


# --- add the test target ---

ADD_TEST(
	NAME test_predcfb
	COMMAND test_predcfb
)
