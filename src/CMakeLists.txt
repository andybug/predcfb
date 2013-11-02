PROJECT(predcfb)

# --- build predcfb ---

ADD_EXECUTABLE(
	predcfb
	# --- sources ---
	main.c
)

TARGET_LINK_LIBRARIES(
	predcfb
	# --- libraries ---
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libcsv.a
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libminiunz.a
	z
)

INSTALL(
	TARGETS predcfb
	RUNTIME DESTINATION bin
)
