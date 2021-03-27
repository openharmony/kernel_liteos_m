#include "stdio_impl.h"
#include <sys/ioctl.h>

size_t __stdout_write(FILE *f, const unsigned char *buf, size_t len)
{
    struct winsize wsz;
    if (!(f->flags & F_SVB) && ioctl(f->fd, TIOCGWINSZ, &wsz)) {
        f->lbf = EOF;
    }
    return __stdio_write(f, buf, len);
}
