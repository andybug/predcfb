#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include "config.h"

#ifndef HAVE_STRLCPY
size_t strlcpy(char * restrict dst, const char * restrict src, size_t size);
#endif

#ifndef HAVE_STRLCAT
size_t strlcat(char * restrict dst, const char * restrict src, size_t size);
#endif

#endif
