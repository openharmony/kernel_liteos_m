#ifndef _STDBOOL_H
#define _STDBOOL_H

#ifdef __ICCARM__ /* for iar */
#include_next <stdbool.h>
#else

#ifndef __cplusplus

#define true 1
#define false 0
#define bool _Bool

#endif

#define __bool_true_false_are_defined 1

#endif /* __ICCARM__ */
#endif
