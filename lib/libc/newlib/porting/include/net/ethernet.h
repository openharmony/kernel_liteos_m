/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _ADAPT_NET_ETHERNET_H
#define _ADAPT_NET_ETHERNET_H

#include <stdint.h>
#include <sys/types.h>
#include <netinet/if_ether.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ETHERTYPE_PUP       0x0200
#define ETHERTYPE_SPRITE    0x0500
#define ETHERTYPE_IP        0x0800
#define ETHERTYPE_ARP       0x0806
#define ETHERTYPE_REVARP    0x8035
#define ETHERTYPE_AT        0x809B
#define ETHERTYPE_AARP      0x80F3
#define ETHERTYPE_VLAN      0x8100
#define ETHERTYPE_IPX       0x8137
#define ETHERTYPE_IPV6      0x86dd

#define ETHER_ADDR_LEN      ETH_ALEN
#define ETHER_TYPE_LEN      ETH_TLEN
#define ETHER_CRC_LEN       4
#define ETHER_HDR_LEN       ETH_HLEN
#define ETHER_MIN_LEN       (ETH_ZLEN + ETHER_CRC_LEN)
#define ETHER_MAX_LEN       (ETH_FRAME_LEN + ETHER_CRC_LEN)

#define ETHER_IS_VALID_LEN(foo)     ((foo) >= ETHER_MIN_LEN && (foo) <= ETHER_MAX_LEN)

#define ETHERTYPE_TRAIL     0x1000
#define ETHERTYPE_NTRAILER  16

#define ETHERMTU            ETH_DATA_LEN
#define ETHERMIN            (ETHER_MIN_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)

struct ether_addr {
    uint8_t ether_addr_octet[ETH_ALEN];
};

struct ether_header {
    uint8_t  ether_dhost[ETH_ALEN];
    uint8_t  ether_shost[ETH_ALEN];
    uint16_t ether_type;
};

#ifdef __cplusplus
}
#endif

#endif /* !_ADAPT_NET_ETHERNET_H */
