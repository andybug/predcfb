CMAKE_MINIMUM_REQUIRED(VERSION 2.8.8)
PROJECT(predcfb C)

# --- platform checks ---

INCLUDE(cmake/platform_checks.cmake)


# --- project settings ---

SET(PREDCFB_VERSION_MAJOR 0)
SET(PREDCFB_VERSION_MINOR 1)
SET(PREDCFB_VERSION_PATCH 0)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# --- config.h generation ---

CONFIGURE_FILE(
	"${PROJECT_SOURCE_DIR}/include/predcfb/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
)


# --- c compiler flags ---

SET(CMAKE_C_FLAGS "-Wall -Wextra -std=c99 -pedantic -D_XOPEN_SOURCE=700")
SET(CMAKE_C_FLAGS_DEBUG "-g -O0")
SET(CMAKE_C_FLAGS_RELEASE "-O3")

INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}")


# --- build static libraries ---

ADD_SUBDIRECTORY(lib/minizip)
ADD_SUBDIRECTORY(lib/libcsv)
ADD_SUBDIRECTORY(lib/polarssl)
ADD_SUBDIRECTORY(lib/libb64)


# --- build project ---

ADD_SUBDIRECTORY(src)


## --- build tests ---

ADD_SUBDIRECTORY(tests)
