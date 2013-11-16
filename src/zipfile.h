#ifndef ZIPFILE_H
#define ZIPFILE_H

#define ZIPFILE_OK	0
#define ZIPFILE_ERROR	(-1)

enum zipfile_err {
	ZIPFILE_EOK,
	ZIPFILE_EACCESS,
	ZIPFILE_EPATH,
	ZIPFILE_EFILEBAD,
	ZIPFILE_ENOENT
};

extern int zipfile_read(const char *file);
extern const char *zipfile_strerr(void);

#endif
