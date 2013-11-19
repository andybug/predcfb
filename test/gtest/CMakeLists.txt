PROJECT(gtest)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

# --- build libgtest ---

ADD_LIBRARY(
	gtest
	# --- sources ---
	src/gtest.cc
	src/gtest-death-test.cc
	src/gtest-filepath.cc
	src/gtest-port.cc
	src/gtest-printers.cc
	src/gtest-test-part.cc
	src/gtest-typed-test.cc
)


# --- build libgtest_main ---

ADD_LIBRARY(
	gtest_main
	# --- sources ---
	src/gtest_main.cc
)
