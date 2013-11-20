PROJECT(predcfb)

# --- build predcfb ---

ADD_LIBRARY(
	predcfb_lib
	OBJECT
	# --- sources ---
	conference.c
	csvparse.c
	fieldlist.c
	parse.c
	zipfile.c
)

ADD_EXECUTABLE(
	predcfb
	# --- sources ---
	main.c
	# --- predcfb objects ---
	$<TARGET_OBJECTS:predcfb_lib>
)

TARGET_LINK_LIBRARIES(
	predcfb
	# --- static libraries ---
	csv
	miniunz
	polarssl
	# --- shared libraries ---
	z
)

INSTALL(
	TARGETS predcfb
	RUNTIME DESTINATION bin
)
