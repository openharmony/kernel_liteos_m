/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#include "lwip/sockets.h"
#include "lwip/priv/tcpip_priv.h"
#include "lwip/priv/sockets_priv.h"
#include "lwip/prot/dhcp.h"
#include "lwip/dhcp.h"

#if !LWIP_COMPAT_SOCKETS
#if LWIP_SOCKET

#define CHECK_NULL_PTR(ptr) do { if ((ptr) == NULL) { set_errno(EFAULT); return -1; } } while (0)

int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    return lwip_accept(s, addr, addrlen);
}

int bind(int s, const struct sockaddr *name, socklen_t namelen)
{
    CHECK_NULL_PTR(name);
    if (namelen < sizeof(*name)) {
        set_errno(EINVAL);
        return -1;
    }
    return lwip_bind(s, name, namelen);
}

int shutdown(int s, int how)
{
    return lwip_shutdown(s, how);
}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
    CHECK_NULL_PTR(name);
    CHECK_NULL_PTR(namelen);
    return lwip_getpeername(s, name, namelen);
}

int getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
    CHECK_NULL_PTR(name);
    CHECK_NULL_PTR(namelen);
    return lwip_getsockname(s, name, namelen);
}

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
{
    return lwip_getsockopt(s, level, optname, optval, optlen);
}

int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
    return lwip_setsockopt(s, level, optname, optval, optlen);
}

int closesocket(int s)
{
    return lwip_close(s);
}

int connect(int s, const struct sockaddr *name, socklen_t namelen)
{
    CHECK_NULL_PTR(name);
    if (namelen < sizeof(*name)) {
        set_errno(EINVAL);
        return -1;
    }
    return lwip_connect(s, name, namelen);
}

int listen(int s, int backlog)
{
    return lwip_listen(s, backlog);
}

ssize_t recv(int s, void *mem, size_t len, int flags)
{
    CHECK_NULL_PTR(mem);
    return lwip_recv(s, mem, len, flags);
}

ssize_t recvfrom(int s, void *mem, size_t len, int flags,
                 struct sockaddr *from, socklen_t *fromlen)
{
    CHECK_NULL_PTR(mem);
    return lwip_recvfrom(s, mem, len, flags, from, fromlen);
}

ssize_t recvmsg(int s, struct msghdr *message, int flags)
{
    CHECK_NULL_PTR(message);
    if (message->msg_iovlen) {
        CHECK_NULL_PTR(message->msg_iov);
    }
    return lwip_recvmsg(s, message, flags);
}

ssize_t send(int s, const void *dataptr, size_t size, int flags)
{
    CHECK_NULL_PTR(dataptr);
    return  lwip_send(s, dataptr, size, flags);
}

ssize_t sendmsg(int s, const struct msghdr *message, int flags)
{
    return lwip_sendmsg(s, message, flags);
}

ssize_t sendto(int s, const void *dataptr, size_t size, int flags,
               const struct sockaddr *to, socklen_t tolen)
{
    CHECK_NULL_PTR(dataptr);
    if (to && tolen < sizeof(*to)) {
        set_errno(EINVAL);
        return -1;
    }
    return lwip_sendto(s, dataptr, size, flags, to, tolen);
}

int socket(int domain, int type, int protocol)
{
    return lwip_socket(domain, type, protocol);
}

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    return lwip_inet_ntop(af, src, dst, size);
}

int inet_pton(int af, const char *src, void *dst)
{
    return lwip_inet_pton(af, src, dst);
}

#ifndef LWIP_INET_ADDR_FUNC
in_addr_t inet_addr(const char *cp)
{
    return ipaddr_addr(cp);
}
#endif

#ifndef LWIP_INET_NTOA_FUNC
char *inet_ntoa(struct in_addr addr)
{
    return ip4addr_ntoa((const ip4_addr_t*)&(addr));
}
#endif

#ifndef LWIP_INET_ATON_FUNC
int inet_aton(const char *cp, struct in_addr *addr)
{
    return ip4addr_aton(cp, (ip4_addr_t*)addr);
}
#endif

int ioctlsocket(int s, long cmd, void *argp)
{
    return lwip_ioctl(s, cmd, argp);
}

#ifdef LWIP_SOCKET_READ_FUNC
ssize_t read(int fd, void *buf, size_t len)
{
    return lwip_read(fd, buf, len);
}
#endif

#ifdef LWIP_SOCKET_WRITE_FUNC
ssize_t write(int fd, const void *buf, size_t len)
{
    return lwip_write(fd, buf, len);
}
#endif

#ifdef LWIP_SOCKET_CLOSE_FUNC
int close(int fd)
{
    return lwip_close(fd);
}
#endif

#ifdef LWIP_SOCKET_IOCTL_FUNC
int ioctl(int fd, int req, ...)
{
    UINTPTR arg = 0;
    va_list ap;
    va_start(ap, req);
    arg = va_arg(ap, UINTPTR);
    va_end(ap);
    return lwip_ioctl(fd, (long)req, (void *)arg);
}
#endif

#ifdef LWIP_SOCKET_FCNTL_FUNC
int fcntl(int fd, int cmd, ...)
{
    int val = 0;
    va_list ap;
    va_start(ap, cmd);
    val = va_arg(ap, int);
    va_end(ap);
    return lwip_fcntl(fd, cmd, val);
}
#endif

#if LWIP_SOCKET_SELECT
#ifdef LWIP_SOCKET_SELECT_FUNC
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
    return lwip_select(maxfdp1, readset, writeset, exceptset, timeout);
}
#endif
#endif

#if LWIP_SOCKET_POLL
#ifdef LWIP_SOCKET_POLL_FUNC
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    return lwip_poll(fds, nfds, timeout);
}
#endif
#endif

unsigned int if_nametoindex(const char *ifname)
{
    return lwip_if_nametoindex(ifname);
}

#endif
#endif /* !LWIP_COMPAT_SOCKETS */

#define IOCTL_CMD_CASE_HANDLER() do {                      \
    err_t  err;                                            \
    struct lwip_ioctl_apimsg msg;                          \
    msg.sock = sock;                                       \
    msg.cmd = cmd;                                         \
    msg.argp = argp;                                       \
                                                           \
    err = tcpip_api_call(lwip_do_ioctl_impl, &msg.call);   \
    if (err != ENOSYS) {                                   \
        sock_set_errno(sock, err);                         \
        done_socket(sock);                                 \
        return -(err != ERR_OK);                           \
    }                                                      \
} while (0)

struct lwip_ioctl_apimsg {
    struct tcpip_api_call_data call;
    struct lwip_sock *sock;
    long cmd;
    void *argp;
};

static err_t lwip_do_ioctl_impl(struct tcpip_api_call_data *call);

#include "../api/sockets.c"

static u8_t lwip_ioctl_internal_SIOCGIFCONF(struct ifreq *ifr)
{
    struct ifconf *ifc = NULL;
    struct netif *netif = NULL;
    struct ifreq ifreq;
    struct sockaddr_in *sock_in = NULL;
    int pos;
    int len;
    int ret;

    /* Format the caller's buffer. */
    ifc = (struct ifconf *)ifr;
    len = ifc->ifc_len;

    /* Loop over the interfaces, and write an info block for each. */
    pos = 0;
    for (netif = netif_list; netif != NULL; netif = netif->next) {
        if (ifc->ifc_buf == NULL) {
            pos = (pos + (int)sizeof(struct ifreq));
            continue;
        }

        if (len < (int)sizeof(ifreq)) {
            break;
        }
        (void)memset_s(&ifreq, sizeof(struct ifreq), 0, sizeof(struct ifreq));
        if (netif->link_layer_type == LOOPBACK_IF) {
            ret = snprintf_s(ifreq.ifr_name, IFNAMSIZ, (IFNAMSIZ - 1), "%.2s", netif->name);
            if ((ret <= 0) || (ret >= IFNAMSIZ)) {
                LWIP_DEBUGF(NETIF_DEBUG, ("lwip_ioctl: snprintf_s ifr_name failed."));
                return ENOBUFS;
            }
        } else {
            ret = snprintf_s(ifreq.ifr_name, IFNAMSIZ, (IFNAMSIZ - 1), "%s", netif_get_name(netif));
            if ((ret <= 0) || (ret >= IFNAMSIZ)) {
                LWIP_DEBUGF(NETIF_DEBUG, ("lwip_ioctl: snprintf_s ifr_name failed."));
                return ENOBUFS;
            }
        }

        sock_in = (struct sockaddr_in *)&ifreq.ifr_addr;
        sock_in->sin_family = AF_INET;
        sock_in->sin_addr.s_addr = ip_2_ip4(&netif->ip_addr)->addr;
        if (memcpy_s(ifc->ifc_buf + pos, sizeof(struct ifreq), &ifreq, sizeof(struct ifreq)) != EOK) {
            return ENOBUFS;
        }
        pos = pos + (int)sizeof(struct ifreq);
        len = len - (int)sizeof(struct ifreq);
    }

    ifc->ifc_len = pos;

    return 0;
}

static u8_t lwip_ioctl_internal_SIOCGIFADDR(struct ifreq *ifr)
{
    struct netif *netif = NULL;
    struct sockaddr_in *sock_in = NULL;

    /* get netif ipaddr */
    netif = netif_find(ifr->ifr_name);
    if (netif == NULL) {
        return ENODEV;
    } else {
        sock_in = (struct sockaddr_in *)&ifr->ifr_addr;
        sock_in->sin_family = AF_INET;
        sock_in->sin_addr.s_addr = ip_2_ip4(&netif->ip_addr)->addr;
        return 0;
    }
}

static u8_t lwip_ioctl_internal_SIOCGIFNETMASK(struct ifreq *ifr)
{
    struct netif *netif = NULL;
    struct sockaddr_in *sock_in = NULL;

    /* get netif netmask */
    netif = netif_find(ifr->ifr_name);
    if (netif == NULL) {
        return ENODEV;
    } else {
        sock_in = (struct sockaddr_in *)&ifr->ifr_netmask;
        sock_in->sin_family = AF_INET;
        sock_in->sin_addr.s_addr = ip_2_ip4(&netif->netmask)->addr;
        return 0;
    }
}

static u8_t lwip_ioctl_internal_SIOCGIFHWADDR(struct ifreq *ifr)
{
    struct netif *netif = NULL;

    /* get netif hw addr */
    netif = netif_find(ifr->ifr_name);
    if (netif == NULL) {
        return ENODEV;
    }
#if LWIP_HAVE_LOOPIF
    else if (netif->link_layer_type == LOOPBACK_IF) {
        return EPERM;
    }
#endif /* LWIP_HAVE_LOOPIF */
    else {
        if (memcpy_s((void *)ifr->ifr_hwaddr.sa_data, sizeof(ifr->ifr_hwaddr.sa_data),
                     (void *)netif->hwaddr, netif->hwaddr_len) != EOK) {
            return EINVAL;
        }
        return 0;
    }
}

static u8_t lwip_ioctl_internal_SIOCSIFFLAGS(struct ifreq *ifr)
{
    struct netif *netif = NULL;

    /* set netif hw addr */
    netif = netif_find(ifr->ifr_name);
    if (netif == NULL) {
        return ENODEV;
    }
#if LWIP_HAVE_LOOPIF
    else if (netif->link_layer_type == LOOPBACK_IF) {
        return EPERM;
    }
#endif /* LWIP_HAVE_LOOPIF */
    else {
        if (((unsigned short)ifr->ifr_flags & IFF_UP) && !(netif->flags & NETIF_FLAG_UP)) {
            (void)netif_set_up(netif);
        } else if (!((unsigned short)ifr->ifr_flags & IFF_UP) && (netif->flags & NETIF_FLAG_UP)) {
            (void)netif_set_down(netif);
        }
        if (((unsigned short)ifr->ifr_flags & IFF_RUNNING) && !(netif->flags & NETIF_FLAG_LINK_UP)) {
            (void)netif_set_link_up(netif);
        } else if (!((unsigned short)ifr->ifr_flags & IFF_RUNNING) && (netif->flags & NETIF_FLAG_LINK_UP)) {
            (void)netif_set_link_down(netif);
        }

        if ((unsigned short)ifr->ifr_flags & IFF_BROADCAST) {
            netif->flags |= NETIF_FLAG_BROADCAST;
        } else {
            netif->flags = netif->flags & (~NETIF_FLAG_BROADCAST);
        }
        if ((unsigned short)ifr->ifr_flags & IFF_NOARP) {
            netif->flags = (netif->flags & (~NETIF_FLAG_ETHARP));
        } else {
            netif->flags |= NETIF_FLAG_ETHARP;
        }

        if ((unsigned short)ifr->ifr_flags & IFF_MULTICAST) {
#if LWIP_IGMP
            netif->flags |= NETIF_FLAG_IGMP;
#endif /* LWIP_IGMP */
#if LWIP_IPV6 && LWIP_IPV6_MLD
            netif->flags |= NETIF_FLAG_MLD6;
#endif /* LWIP_IPV6_MLD */
        } else {
#if LWIP_IGMP
            netif->flags = (netif->flags & ~NETIF_FLAG_IGMP);
#endif /* LWIP_IGMP */
#if LWIP_IPV6 && LWIP_IPV6_MLD
            netif->flags = (netif->flags & ~NETIF_FLAG_MLD6);
#endif /* LWIP_IPV6_MLD */
        }

#if LWIP_DHCP
        if ((unsigned short)ifr->ifr_flags & IFF_DYNAMIC) {
            (void)dhcp_start(netif);
        } else {
            dhcp_stop(netif);
#if !LWIP_DHCP_SUBSTITUTE
            dhcp_cleanup(netif);
#endif
        }
#endif

#if LWIP_NETIF_PROMISC
        if (((unsigned short)ifr->ifr_flags & IFF_PROMISC)) {
            netif->flags |= NETIF_FLAG_PROMISC;
        } else {
            netif->flags &= ~NETIF_FLAG_PROMISC;
        }
        if (netif->drv_config) {
            netif->drv_config(netif, IFF_PROMISC, !!((unsigned short)ifr->ifr_flags & IFF_PROMISC));
        }
#endif /* LWIP_NETIF_PROMISC */
        return 0;
    }
}

static u8_t lwip_ioctl_internal_SIOCGIFFLAGS(struct ifreq *ifr)
{
    struct netif *netif = NULL;

    /* set netif hw addr */
    netif = netif_find(ifr->ifr_name);
    if (netif == NULL) {
        return ENODEV;
    } else {
        if (netif->flags & NETIF_FLAG_UP) {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) | IFF_UP;
        } else {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) & ~IFF_UP;
        }
        if (netif->flags & NETIF_FLAG_LINK_UP) {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) | IFF_RUNNING;
        } else {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) & ~IFF_RUNNING;
        }
        if (netif->flags & NETIF_FLAG_BROADCAST) {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) | IFF_BROADCAST;
        } else {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) & ~IFF_BROADCAST;
        }
        if (netif->flags & NETIF_FLAG_ETHARP) {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) & ~IFF_NOARP;
        } else {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) | IFF_NOARP;
        }

#if LWIP_IGMP || LWIP_IPV6_MLD
        if (
#if LWIP_IGMP
            (netif->flags & NETIF_FLAG_IGMP)
#endif /* LWIP_IGMP */
#if LWIP_IGMP && LWIP_IPV6_MLD
            ||
#endif /* LWIP_IGMP && LWIP_IPV6_MLD */
#if LWIP_IPV6_MLD
            (netif->flags & NETIF_FLAG_MLD6)
#endif /* LWIP_IPV6_MLD */
                ) {
            ifr->ifr_flags = (short)((unsigned short)ifr->ifr_flags | IFF_MULTICAST);
        } else {
            ifr->ifr_flags = (short)((unsigned short)ifr->ifr_flags & (~IFF_MULTICAST));
        }
#endif /* LWIP_IGMP || LWIP_IPV6_MLD */

#if LWIP_DHCP
        if (dhcp_supplied_address(netif)) {
            ifr->ifr_flags = (short)((unsigned short)ifr->ifr_flags | IFF_DYNAMIC);
        } else {
            ifr->ifr_flags = (short)((unsigned short)ifr->ifr_flags & (~IFF_DYNAMIC));
        }
#endif

#if LWIP_HAVE_LOOPIF
        if (netif->link_layer_type == LOOPBACK_IF) {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) | IFF_LOOPBACK;
        }
#endif

#if LWIP_NETIF_PROMISC
        if (netif->flags & NETIF_FLAG_PROMISC) {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) | IFF_PROMISC;
        } else {
            ifr->ifr_flags = ((unsigned short)(ifr->ifr_flags)) & ~IFF_PROMISC;
        }
#endif /* LWIP_NETIF_PROMISC */

        return 0;
    }
}

static u8_t lwip_ioctl_internal_SIOCGIFNAME(struct ifreq *ifr)
{
    struct netif *netif = NULL;
    int ret;

    for (netif = netif_list; netif != NULL; netif = netif->next) {
        if (ifr->ifr_ifindex == netif_get_index(netif)) {
            break;
        }
    }

    if (netif == NULL) {
        return ENODEV;
    } else {
        if (netif->link_layer_type == LOOPBACK_IF) {
            ret = snprintf_s(ifr->ifr_name, IFNAMSIZ, (IFNAMSIZ - 1), "%.2s", netif->name);
            if ((ret <= 0) || (ret >= IFNAMSIZ)) {
                return ENOBUFS;
            }
        } else {
            ret = snprintf_s(ifr->ifr_name, IFNAMSIZ, (IFNAMSIZ - 1), "%s", netif_get_name(netif));
            if ((ret <= 0) || (ret >= IFNAMSIZ)) {
                return ENOBUFS;
            }
        }
        return 0;
    }
}

static u8_t lwip_validate_ifname(const char *name, u8_t *let_pos)
{
    unsigned short num_pos = 0;
    unsigned short letter_pos = 0;
    unsigned short pos = 0;
    u8_t have_num = 0;

    /* if the first position of variable name is not letter, such as '6eth2' */
    if (!((*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z'))) {
        return 0;
    }

    /* check if the position of letter is bigger than the the position of digital */
    while (*name != '\0') {
        if ((*name >= '0') && (*name <= '9')) {
            num_pos = pos;
            have_num = 1;
        } else if (((*name >= 'a') && (*name <= 'z')) || ((*name >= 'A') && (*name <= 'Z'))) {
            letter_pos = pos;
            if (have_num != 0) {
                return 0;
            }
        } else {
            return 0;
        }
        pos++;
        name++;
    }

    /* for the speacil case as all position of variable name is letter, such as 'ethabc' */
    if (num_pos == 0) {
        return 0;
    }

    /* cheak if the digital in the variable name is bigger than 255, such as 'eth266' */
    if (atoi(name - (pos - letter_pos - 1)) > 255) {
        return 0;
    }

    *let_pos = (u8_t)letter_pos;

    return 1;
}

static u8_t lwip_ioctl_internal_SIOCSIFNAME(struct ifreq *ifr)
{
    struct netif *netif = NULL;
    u8_t letter_pos = 0;

    netif = netif_find(ifr->ifr_name);
    if (netif == NULL) {
        return ENODEV;
    } else if (netif->link_layer_type == LOOPBACK_IF) {
        return EPERM;
    } else if ((netif->flags & IFF_UP) != 0) {
        return EBUSY;
    } else {
        if (strncmp(ifr->ifr_name, ifr->ifr_newname, IFNAMSIZ) == 0) {
            /* not change */
            return 0;
        }

        ifr->ifr_newname[IFNAMSIZ - 1] = '\0';
        if ((lwip_validate_ifname(ifr->ifr_newname, &letter_pos) == 0) || (strlen(ifr->ifr_newname) > (IFNAMSIZ - 1))) {
            return EINVAL;
        }

        if (strncpy_s(netif->full_name, sizeof(netif->full_name), ifr->ifr_newname, strlen(ifr->ifr_newname)) != EOK) {
            return EINVAL;
        }
    }

    return 0;
}

static u8_t lwip_ioctl_internal_SIOCGIFINDEX(struct ifreq *ifr)
{
    struct netif *netif = NULL;

    netif = netif_find(ifr->ifr_name);
    if (netif == NULL) {
        return ENODEV;
    } else {
        ifr->ifr_ifindex = netif_get_index(netif);
        return 0;
    }
}

static u8_t lwip_ioctl_internal_SIOCGIFMTU(struct ifreq *ifr)
{
    struct netif *netif = NULL;

    /* get netif hw addr */
    netif = netif_find(ifr->ifr_name);
    if (netif == NULL) {
        return ENODEV;
    } else {
        ifr->ifr_mtu = netif->mtu;
        return 0;
    }
}

static u8_t lwip_ioctl_internal_SIOCGIFBRDADDR(struct ifreq *ifr)
{
    struct netif *netif = NULL;
    struct sockaddr_in *sock_in = NULL;

    /* get netif subnet broadcast addr */
    netif = netif_find(ifr->ifr_name);
    if (netif == NULL) {
        return ENODEV;
    }
    if (ip4_addr_isany_val(*(ip_2_ip4(&netif->netmask)))) {
        return ENXIO;
    }
    sock_in = (struct sockaddr_in *)&ifr->ifr_addr;
    sock_in->sin_family = AF_INET;
    sock_in->sin_addr.s_addr = (ip_2_ip4(&((netif)->ip_addr))->addr | ~(ip_2_ip4(&netif->netmask)->addr));
    return 0;
}

static u8_t lwip_ioctl_impl(const struct lwip_sock *sock, long cmd, void *argp)
{
    u8_t err = 0;
    struct ifreq *ifr = (struct ifreq *)argp;
    bool is_ipv6 = 0;

    /* allow it only on IPv6 sockets... */
    is_ipv6 = NETCONNTYPE_ISIPV6((unsigned int)(sock->conn->type));

    switch ((u32_t)cmd) {
        case SIOCGIFCONF:
            if (is_ipv6 != 0) {
                err = EINVAL;
            } else {
                err = lwip_ioctl_internal_SIOCGIFCONF(ifr);
            }
            break;
        case SIOCGIFADDR:
            if (is_ipv6 != 0) {
                err = EINVAL;
            } else {
                err = lwip_ioctl_internal_SIOCGIFADDR(ifr);
            }
            break;
        case SIOCGIFNETMASK:
            if (is_ipv6 != 0) {
                err = EINVAL;
            } else {
                err = lwip_ioctl_internal_SIOCGIFNETMASK(ifr);
            }
            break;
        case SIOCGIFHWADDR:
            err = lwip_ioctl_internal_SIOCGIFHWADDR(ifr);
            break;
        case SIOCSIFFLAGS:
            err = lwip_ioctl_internal_SIOCSIFFLAGS(ifr);
            break;
        case SIOCGIFFLAGS:
            err = lwip_ioctl_internal_SIOCGIFFLAGS(ifr);
            break;
        case SIOCGIFNAME:
            err = lwip_ioctl_internal_SIOCGIFNAME(ifr);
            break;
        case SIOCSIFNAME:
            err = lwip_ioctl_internal_SIOCSIFNAME(ifr);
            break;
        case SIOCGIFINDEX:
            err = lwip_ioctl_internal_SIOCGIFINDEX(ifr);
            break;
        case SIOCGIFMTU:
            err = lwip_ioctl_internal_SIOCGIFMTU(ifr);
            break;
        case SIOCGIFBRDADDR:
            if (is_ipv6 != 0) {
                err = EINVAL;
            } else {
                err = lwip_ioctl_internal_SIOCGIFBRDADDR(ifr);
            }
            break;
        default:
            err = ENOSYS;
            LWIP_DEBUGF(SOCKETS_DEBUG, ("lwip_ioctl(UNIMPL: 0x%lx)\n", cmd));
            break;
    }

    return err;
}

static err_t lwip_do_ioctl_impl(struct tcpip_api_call_data *call)
{
    struct lwip_ioctl_apimsg *msg = (struct lwip_ioctl_apimsg *)(void *)call;
    return lwip_ioctl_impl(msg->sock, msg->cmd, msg->argp);
}