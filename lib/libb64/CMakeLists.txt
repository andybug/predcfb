PROJECT(libb64)

# --- build libb64 ---

ADD_LIBRARY(
	libb64
	# --- sources ---
	src/cencode.c
	src/cdecode.c
)

SET_TARGET_PROPERTIES(libb64 PROPERTIES PREFIX "")
