PROJECT(predcfb)

ADD_EXECUTABLE(
	predcfb
	# --- sources ---
	main.c
)

TARGET_LINK_LIBRARIES(
	predcfb
	# --- libraries ---
	archive
)

INSTALL(
	TARGETS predcfb
	RUNTIME DESTINATION bin
)
