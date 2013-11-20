PROJECT(libcsv)

# --- build libcsv ---

ADD_LIBRARY(
	libcsv
	# --- sources ---
	libcsv.c
)

SET_TARGET_PROPERTIES(libcsv PROPERTIES PREFIX "")
