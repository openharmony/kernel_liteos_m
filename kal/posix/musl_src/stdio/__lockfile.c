#include "stdio_impl.h"
#include "pthread.h"

int __lockfile(FILE *f)
{
	return pthread_mutex_lock((pthread_mutex_t *)f->lock);
}

int __unlockfile(FILE *f)
{
	return pthread_mutex_unlock((pthread_mutex_t *)f->lock);
}
