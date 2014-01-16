PROJECT(predcfb_test)

ENABLE_TESTING()


# --- build gtest library ---

INCLUDE_DIRECTORIES(gtest/include)
ADD_SUBDIRECTORY(gtest)


# --- build test_predcfb ---

ADD_EXECUTABLE(
	predcfb_test
	# --- sources ---
	csvparse.cc
	objectdb.cc
	objectid.cc
	zipfile.cc
	# --- predcfb objects ---
	$<TARGET_OBJECTS:libpredcfb>
)

TARGET_LINK_LIBRARIES(
	predcfb_test
	# --- static libraries ---
	libcsv
	miniunz
	polarssl
	openbsd
	gtest
	# --- shared libraries ---
	pthread
	yaml
	z
)


# --- add the test target ---

ADD_TEST(
	NAME predcfb_test
	COMMAND predcfb_test
)
