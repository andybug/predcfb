#ifndef ZIPFILE_H
#define ZIPFILE_H

#define ZIPFILE_OK	0
#define ZIPFILE_ERROR	(-1)

enum zipfile_err {
	ZIPFILE_ENONE,
	ZIPFILE_EACCESS,
	ZIPFILE_EPATH,
	ZIPFILE_EFILEBAD,
	ZIPFILE_EINTERNAL,
	ZIPFILE_ENOENT,
	ZIPFILE_ENOTYPE,
	ZIPFILE_EPARSE,
	ZIPFILE_ESTILLOPEN,
	ZIPFILE_ENOTOPEN,
	ZIPFILE_EUNKNOWN
};

typedef struct zipfile_read_context zf_readctx;

extern zf_readctx *zipfile_open_archive(const char *path);
extern int zipfile_close_archive(zf_readctx *z);
extern int zipfile_open_file(zf_readctx *z, const char *file);
extern int zipfile_close_file(zf_readctx *z);
extern ssize_t zipfile_read_file(zf_readctx *z, char *buf, size_t count);
extern const char *zipfile_strerr(void);

#endif
