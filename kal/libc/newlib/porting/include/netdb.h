/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _ADAPT_NETDB_H
#define _ADAPT_NETDB_H

#include <sys/features.h>
#include <sys/types.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AI_PASSIVE      0x01
#define AI_CANONNAME    0x02
#define AI_NUMERICHOST  0x04
#define AI_V4MAPPED     0x08
#define AI_ALL          0x10
#define AI_ADDRCONFIG   0x20
#define AI_NUMERICSERV  0x400

#define NI_NUMERICHOST  0x01
#define NI_NUMERICSERV  0x02
#define NI_NOFQDN       0x04
#define NI_NAMEREQD     0x08
#define NI_DGRAM        0x10
#define NI_NUMERICSCOPE 0x100
#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define NI_MAXHOST 255
#define NI_MAXSERV 32
#endif

#define EAI_BADFLAGS   -1
#define EAI_NONAME     -2
#define EAI_AGAIN      -3
#define EAI_FAIL       -4
#define EAI_FAMILY     -6
#define EAI_SOCKTYPE   -7
#define EAI_SERVICE    -8
#define EAI_MEMORY     -10
#define EAI_SYSTEM     -11
#define EAI_OVERFLOW   -12
#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define EAI_NODATA     -5
#define EAI_ADDRFAMILY -9
#define EAI_INPROGRESS -100
#define EAI_CANCELED   -101
#define EAI_NOTCANCELED -102
#define EAI_ALLDONE    -103
#define EAI_INTR       -104
#define EAI_IDN_ENCODE -105
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define HOST_NOT_FOUND 1
#define TRY_AGAIN      2
#define NO_RECOVERY    3
#define NO_DATA        4
#define NO_ADDRESS     NO_DATA
#endif

struct addrinfo {
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    socklen_t ai_addrlen;
    struct sockaddr *ai_addr;
    char *ai_canonname;
    struct addrinfo *ai_next;
};

int getaddrinfo(const char *__restrict nodename, const char *__restrict servname, \
                 const struct addrinfo *__restrict hints, struct addrinfo **__restrict res);
void freeaddrinfo(struct addrinfo *res);
int getnameinfo(const struct sockaddr * __restrict sa, socklen_t salen, char * __restrict host, \
                socklen_t hostlen, char * __restrict serv, socklen_t servlen, int flags);
const char *gai_strerror(int errcode);

struct netent {
    char *n_name;
    char **n_aliases;
    int n_addrtype;
    uint32_t n_net;
};

struct hostent {
    char *h_name;
    char **h_aliases;
    int h_addrtype;
    int h_length;
    char **h_addr_list;
};
#define h_addr h_addr_list[0]

struct servent {
    char *s_name;
    char **s_aliases;
    int s_port;
    char *s_proto;
};

struct protoent {
    char *p_name;
    char **p_aliases;
    int p_proto;
};

void sethostent(int stay_open);
void endhostent(void);
struct hostent *gethostent(void);

void setnetent(int stay_open);
void endnetent(void);
struct netent *getnetent(void);
struct netent *getnetbyaddr(uint32_t net, int type);
struct netent *getnetbyname(const char *name);

void setservent(int stay_open);
void endservent(void);
struct servent *getservent(void);
struct servent *getservbyname(const char *name, const char *proto);
struct servent *getservbyport(int port, const char *proto);

void setprotoent(int stay_open);
void endprotoent(void);
struct protoent *getprotoent(void);
struct protoent *getprotobyname(const char *name);
struct protoent *getprotobynumber(int proto);

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE) || defined(_POSIX_SOURCE) \
    || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 < 200809L) \
    || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 < 700)
struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type);
int *__h_errno_location(void);
#define h_errno (*__h_errno_location())
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
void herror(const char *str);
const char *hstrerror(int errnum);
int gethostbyname_r(const char * name, struct hostent * ret, char * buf, size_t buflen, \
                    struct hostent ** result, int * h_errnop);
int gethostbyname2_r(const char * __restrict name, int af, struct hostent * __restrict result_buf, \
                     char * __restrict buf, size_t buflen, struct hostent ** __restrict result, \
                     int * __restrict h_errnop);
struct hostent *gethostbyname2(const char *name, int af);
int gethostbyaddr_r(const void * __restrict addr, __socklen_t len, int type, struct hostent * __restrict result_buf, \
                    char * __restrict buf, size_t buflen, struct hostent ** __restrict result, \
                    int * __restrict h_errnop);
int getservbyport_r(int port, const char * __restrict proto, struct servent * __restrict result_buf, \
                    char * __restrict buf, size_t buflen, struct servent ** __restrict result);
int getservbyname_r(const char *name, const char *proto, struct servent *result_buf, char *buf, size_t buflen, \
                    struct servent **result);
#endif

#ifdef __cplusplus
}
#endif

#endif /* !_ADAPT_NETDB_H */
