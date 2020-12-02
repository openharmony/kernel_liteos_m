/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
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

#include "stdarg.h"
#include <stdio.h>
#include "los_debug.h"
#include "uart.h"

int putchar(int n)
{
    return UartOut(n, NULL);
}

int puts(const char *string)
{
    int count = 0;
    char *s = (char *)string;
    while (*s != '\0') {
        putchar(*s);
        s++;
        count++;
    }

    return count;
}

static int hex2asc(int n)
{
    n &= 15;
    if(n > 9){
        return ('a' - 10) + n;
    } else {
        return '0' + n;
    }
}

static void dputs(char const *s, int (*pFputc)(int n, void *cookie), void *cookie)
{
    while (*s) {
        pFputc(*s++, cookie);
    }
}

void __dprintf(char const *fmt, va_list ap,
        int (*pFputc)(int n, void *cookie),
        void *cookie)
{
    char scratch[256];

    for(;;){
        switch(*fmt){
            case 0:
                va_end(ap);
                return;
            case '%':
                switch(fmt[1]) {
                    case 'c': {
                                  unsigned n = va_arg(ap, unsigned);
                                  pFputc(n, cookie);
                                  fmt += 2;
                                  continue;
                              }
                    case 'h': {
                                  unsigned n = va_arg(ap, unsigned);
                                  pFputc(hex2asc(n >> 12), cookie);
                                  pFputc(hex2asc(n >> 8), cookie);
                                  pFputc(hex2asc(n >> 4), cookie);
                                  pFputc(hex2asc(n >> 0), cookie);
                                  fmt += 2;
                                  continue;
                              }
                    case 'b': {
                                  unsigned n = va_arg(ap, unsigned);
                                  pFputc(hex2asc(n >> 4), cookie);
                                  pFputc(hex2asc(n >> 0), cookie);
                                  fmt += 2;
                                  continue;
                              }
                    case 'p':
                    case 'X':
                    case 'x': {
                                  unsigned n = va_arg(ap, unsigned);
                                  char *p = scratch + 15;
                                  *p = 0;
                                  do {
                                      *--p = hex2asc(n);
                                      n = n >> 4;
                                  } while(n != 0);
                                  while(p > (scratch + 7)) *--p = '0';
                                  dputs(p, pFputc, cookie);
                                  fmt += 2;
                                  continue;
                              }
                    case 'd': {
                                  int n = va_arg(ap, int);
                                  char *p = scratch + 15;
                                  *p = 0;
                                  if(n < 0) {
                                      pFputc('-', cookie);
                                      n = -n;
                                  }
                                  do {
                                      *--p = (n % 10) + '0';
                                      n /= 10;
                                  } while(n != 0);
                                  dputs(p, pFputc, cookie);
                                  fmt += 2;
                                  continue;
                              }
                    case 'u': {
                                  unsigned int n = va_arg(ap, unsigned int);
                                  char *p = scratch + 15;
                                  *p = 0;
                                  do {
                                      *--p = (n % 10) + '0';
                                      n /= 10;
                                  } while(n != 0);
                                  dputs(p, pFputc, cookie);
                                  fmt += 2;
                                  continue;
                              }
                    case 's': {
                                  char *s = va_arg(ap, char*); /*lint !e64*/
                                  if(s == 0) s = "(null)";
                                  dputs(s, pFputc, cookie);
                                  fmt += 2;
                                  continue;
                              }
                }
                pFputc(*fmt++, cookie);
                break;
            case '\n':
                pFputc('\r', cookie);
            default: /*lint !e616*/
                pFputc(*fmt++, cookie);
        }
    }
}

int printf(char const *str, ...)
{
    va_list ap;
    va_start(ap, str);
    __dprintf(str, ap, UartPutc, 0);
    va_end(ap);
    return 0;
}
