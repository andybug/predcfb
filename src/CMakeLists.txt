PROJECT(predcfb)

# --- build predcfb ---

ADD_LIBRARY(
	libpredcfb
	OBJECT
	# --- sources ---
	conference.c
	csvparse.c
	fieldlist.c
	objectdb.c
	parse.c
	zipfile.c
)

SET_TARGET_PROPERTIES(libpredcfb PROPERTIES PREFIX "")


ADD_EXECUTABLE(
	predcfb
	# --- sources ---
	main.c
	# --- predcfb objects ---
	$<TARGET_OBJECTS:libpredcfb>
)

TARGET_LINK_LIBRARIES(
	predcfb
	# --- static libraries ---
	libcsv
	miniunz
	polarssl
	libb64
	# --- shared libraries ---
	z
)

INSTALL(
	TARGETS predcfb
	RUNTIME DESTINATION bin
)
