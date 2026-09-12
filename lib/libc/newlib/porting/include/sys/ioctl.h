/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ADAPT_SYS_IOCTL_H
#define _ADAPT_SYS_IOCTL_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TCGETS          0x5401
#define TCSETS          0x5402
#define TCSETSW         0x5403
#define TCSETSF         0x5404
#define TCGETA          0x5405
#define TCSETA          0x5406
#define TCSETAW         0x5407
#define TCSETAF         0x5408
#define TCSBRK          0x5409
#define TCXONC          0x540A
#define TCFLSH          0x540B
#define TIOCEXCL        0x540C
#define TIOCNXCL        0x540D
#define TIOCSCTTY       0x540E
#define TIOCGPGRP       0x540F
#define TIOCSPGRP       0x5410
#define TIOCOUTQ        0x5411
#define TIOCSTI         0x5412
#define TIOCGWINSZ      0x5413
#define TIOCSWINSZ      0x5414
#define TIOCMGET        0x5415
#define TIOCMBIS        0x5416
#define TIOCMBIC        0x5417
#define TIOCMSET        0x5418
#define TIOCGSOFTCAR    0x5419
#define TIOCSSOFTCAR    0x541A
#define FIONREAD        0x541B
#define TIOCINQ         FIONREAD
#define TIOCLINUX       0x541C
#define TIOCCONS        0x541D
#define TIOCGSERIAL     0x541E
#define TIOCSSERIAL     0x541F
#define TIOCPKT         0x5420
#define FIONBIO         0x5421

#define SIOCADDRT          0x890B
#define SIOCDELRT          0x890C
#define SIOCRTMSG          0x890D

#define SIOCGIFNAME        0x8910
#define SIOCSIFLINK        0x8911
#define SIOCGIFCONF        0x8912
#define SIOCGIFFLAGS       0x8913
#define SIOCSIFFLAGS       0x8914
#define SIOCGIFADDR        0x8915
#define SIOCSIFADDR        0x8916
#define SIOCGIFDSTADDR     0x8917
#define SIOCSIFDSTADDR     0x8918
#define SIOCGIFBRDADDR     0x8919
#define SIOCSIFBRDADDR     0x891a
#define SIOCGIFNETMASK     0x891b
#define SIOCSIFNETMASK     0x891c
#define SIOCGIFMETRIC      0x891d
#define SIOCSIFMETRIC      0x891e
#define SIOCGIFMEM         0x891f
#define SIOCSIFMEM         0x8920
#define SIOCGIFMTU         0x8921
#define SIOCSIFMTU         0x8922
#define SIOCSIFNAME        0x8923
#define SIOCSIFHWADDR      0x8924
#define SIOCGIFENCAP       0x8925
#define SIOCSIFENCAP       0x8926
#define SIOCGIFHWADDR      0x8927
#define SIOCGIFSLAVE       0x8929
#define SIOCSIFSLAVE       0x8930
#define SIOCADDMULTI       0x8931
#define SIOCDELMULTI       0x8932
#define SIOCGIFINDEX       0x8933
#define SIOGIFINDEX        SIOCGIFINDEX

#define _IOC(a, b, c, d)  (((a) << 30) | ((b) << 8) | (c) | ((d) << 16))
#define _IOC_NONE  0U
#define _IOC_WRITE 1U
#define _IOC_READ  2U

#define _IO(a, b)       _IOC(_IOC_NONE, (a), (b), 0)
#define _IOW(a, b, c)   _IOC(_IOC_WRITE, (a), (b), sizeof(c))
#define _IOR(a, b, c)   _IOC(_IOC_READ, (a), (b), sizeof(c))
#define _IOWR(a, b, c)  _IOC(_IOC_READ | _IOC_WRITE, (a), (b), sizeof(c))

int ioctl(int, int, ...);

#ifdef __cplusplus
}
#endif

#endif /* !_ADAPT_SYS_IOCTL_H */
