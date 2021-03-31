#include "stdio_impl.h"
#include <pthread.h>

#undef stderr

static pthread_mutex_t locallock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static unsigned char buf[UNGET];
hidden FILE __stderr_FILE = {
	.buf = buf+UNGET,
	.buf_size = 0,
	.fd = 2,
	.flags = F_PERM | F_NORD,
	.lbf = -1,
	.write = __stdio_write,
	.seek = __stdio_seek,
	.close = __stdio_close,
	.lock = &locallock,
};
FILE *const stderr = &__stderr_FILE;
FILE *volatile __stderr_used = &__stderr_FILE;
