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
)

TARGET_LINK_LIBRARIES(
	test_predcfb
	# --- libraries ---
	gtest
	gtest_main
	pthread
)


# --- add the test target ---

ADD_TEST(
	NAME test_predcfb
	COMMAND test_predcfb
)
