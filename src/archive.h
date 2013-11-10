#ifndef ARCHIVE_H
#define ARCHIVE_H

#define ARCHIVE_OK			0
#define ARCHIVE_EOF			1
#define ARCHIVE_OPEN_ERROR		(-1)
#define ARCHIVE_MISSING_FILE_ERROR	(-2)
#define ARCHIVE_INTERNAL_ERROR		(-3)
#define ARCHIVE_MEMORY_ERROR		(-4)

typedef void* archive_ctx;

extern int archive_open(archive_ctx *a, const char *path);
extern int archive_close(archive_ctx a);
extern int archive_open_file(archive_ctx a, const char *file);
extern int archive_read_file(archive_ctx a, char *buf, size_t len, size_t *read);
extern int archive_close_file(archive_ctx a);

#endif
