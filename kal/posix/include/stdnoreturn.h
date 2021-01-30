#ifndef _STDNORETURN_H
#define _STDNORETURN_H

#ifdef __ICCARM__ /* for iar */
#include_next <stdnoreturn.h>
#else

#ifndef __cplusplus
#include <features.h>
#define noreturn _Noreturn
#endif

#endif /* __ICCARM__ */
#endif
