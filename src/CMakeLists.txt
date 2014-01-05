PROJECT(predcfb)

# --- build predcfb ---

ADD_LIBRARY(
	libpredcfb
	OBJECT
	# --- sources ---
	cfbstats/core.c
	cfbstats/fielddesc.c
	cfbstats/id_map.c
	cfbstats/linehandler.c
	cfbstats/parsers.c
	cfbstats/unzip.c
	csvline.c
	csvparse.c
	objectdb/core.c
	objectdb/objectid.c
	objectdb/write.c
	options.c
	schedule.c
	string.c
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
	# --- shared libraries ---
	yaml
	z
)

INSTALL(
	TARGETS predcfb
	RUNTIME DESTINATION bin
)
