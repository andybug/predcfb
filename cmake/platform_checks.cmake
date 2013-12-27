
INCLUDE(CheckLibraryExists)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckIncludeFiles)

# check for zlib
CHECK_INCLUDE_FILES(zlib.h HAS_ZLIB_H)
CHECK_LIBRARY_EXISTS(z zlibVersion "" HAS_ZLIB)
IF(NOT HAS_ZLIB_H OR NOT HAS_ZLIB)
	MESSAGE(FATAL_ERROR "Missing zlib")
ENDIF(NOT HAS_ZLIB_H OR NOT HAS_ZLIB)

# check for libyaml
CHECK_INCLUDE_FILES(yaml.h HAS_YAML_H)
CHECK_LIBRARY_EXISTS(yaml yaml_get_version_string "" HAS_YAML)
IF(NOT HAS_YAML_H OR NOT HAS_YAML)
	MESSAGE(FATAL_ERROR "Missing libyaml")
ENDIF(NOT HAS_YAML_H OR NOT HAS_YAML)

# check for strlcpy and strlcat
CHECK_SYMBOL_EXISTS(strlcpy "string.h" HAVE_STRLCPY)
CHECK_SYMBOL_EXISTS(strlcat "string.h" HAVE_STRLCAT)

