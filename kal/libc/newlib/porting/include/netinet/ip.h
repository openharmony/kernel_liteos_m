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

#ifndef _ADAPT_NETINET_IP_H
#define _ADAPT_NETINET_IP_H

#include <endian.h>
#include <stdint.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

struct timestamp {
    uint8_t len;
    uint8_t ptr;
#if BYTE_ORDER == LITTLE_ENDIAN
    unsigned int flags:4;
    unsigned int overflow:4;
#else
    unsigned int overflow:4;
    unsigned int flags:4;
#endif
    uint32_t data[9];
  };

struct iphdr {
#if BYTE_ORDER == LITTLE_ENDIAN
    unsigned int ihl:4;
    unsigned int version:4;
#else
    unsigned int version:4;
    unsigned int ihl:4;
#endif
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
};

struct ip {
#if BYTE_ORDER == LITTLE_ENDIAN
    unsigned int ip_hl:4;
    unsigned int ip_v:4;
#else
    unsigned int ip_v:4;
    unsigned int ip_hl:4;
#endif
    uint8_t ip_tos;
    uint16_t ip_len;
    uint16_t ip_id;
    uint16_t ip_off;
    uint8_t ip_ttl;
    uint8_t ip_p;
    uint16_t ip_sum;
    struct in_addr ip_src, ip_dst;
};

#define    IP_RF 0x8000
#define    IP_DF 0x4000
#define    IP_MF 0x2000
#define    IP_OFFMASK 0x1fff

struct ip_timestamp {
    uint8_t ipt_code;
    uint8_t ipt_len;
    uint8_t ipt_ptr;
#if BYTE_ORDER == LITTLE_ENDIAN
    unsigned int ipt_flg:4;
    unsigned int ipt_oflw:4;
#else
    unsigned int ipt_oflw:4;
    unsigned int ipt_flg:4;
#endif
    uint32_t data[9];
};

#ifdef __cplusplus
}
#endif

#endif /* !_ADAPT_NETINET_IP_H */
