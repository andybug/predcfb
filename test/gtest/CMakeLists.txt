PROJECT(gtest)

INCLUDE(./include)

--- build libtest ---

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
