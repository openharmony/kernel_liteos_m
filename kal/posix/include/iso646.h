#ifndef _ISO646_H
#define _ISO646_H

#ifdef __ICCARM__ /* for iar */
#include_next <iso646.h>
#else

#ifndef __cplusplus

#define and    &&
#define and_eq &=
#define bitand &
#define bitor  |
#define compl  ~
#define not    !
#define not_eq !=
#define or     ||
#define or_eq  |=
#define xor    ^
#define xor_eq ^=

#endif

#endif /* __ICCARM__ */
#endif
