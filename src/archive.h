#ifndef ARCHIVE_H
#define ARCHIVE_H

#define ARCHIVE_OK			0
#define ARCHIVE_OPEN_ERROR		(-1)
#define ARCHIVE_MISSING_FILE_ERROR	(-2)
#define ARCHIVE_INTERNAL_ERROR		(-3)

extern int archive_read(const char *path);

#endif
