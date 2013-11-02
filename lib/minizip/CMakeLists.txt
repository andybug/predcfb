PROJECT(libminiunz)

# --- c compiler flags ---

ADD_DEFINITIONS("-Wno-unused-parameter")


# --- build libminiunz ---

ADD_LIBRARY(
	miniunz
	# --- sources ---
	unzip.c
	ioapi.c
)
