
INCLUDE(CheckLibraryExists)
INCLUDE(CheckIncludeFiles)

# check for libarchive
CHECK_INCLUDE_FILES(archive.h HAS_ARCHIVE_H)
CHECK_LIBRARY_EXISTS(archive archive_version_number "" HAS_LIBARCHIVE)
IF(NOT HAS_ARCHIVE_H OR NOT HAS_LIBARCHIVE)
	MESSAGE(FATAL_ERROR "Missing libarchive - check the README for install instructions")
ENDIF(NOT HAS_ARCHIVE_H OR NOT HAS_LIBARCHIVE)

