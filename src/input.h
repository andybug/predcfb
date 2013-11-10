#ifndef INPUT_H
#define INPUT_H

#define INPUT_OK			0
#define INPUT_ARCHIVE_ERROR		(-1)
#define INPUT_MEMORY_ERROR		(-2)
#define INPUT_ACCESS_ERROR		(-3)
#define INPUT_INVALID_ARCHIVE_ERROR	(-4)

extern int input_read_archive(const char *path);

#endif
