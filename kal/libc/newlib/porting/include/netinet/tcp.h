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

#ifndef _ADAPT_NETINET_TCP_H
#define _ADAPT_NETINET_TCP_H

#include <sys/features.h>

#define TCP_NODELAY     1
#define TCP_MAXSEG      2
#define TCP_CORK        3
#define TCP_KEEPIDLE    4
#define TCP_KEEPINTVL   5
#define TCP_KEEPCNT     6
#define TCP_SYNCNT      7

#define TCP_ESTABLISHED  1
#define TCP_SYN_SENT     2
#define TCP_SYN_RECV     3
#define TCP_FIN_WAIT1    4
#define TCP_FIN_WAIT2    5
#define TCP_TIME_WAIT    6
#define TCP_CLOSE        7
#define TCP_CLOSE_WAIT   8
#define TCP_LAST_ACK     9
#define TCP_LISTEN       10
#define TCP_CLOSING      11

#ifdef _GNU_SOURCE
#define TCPI_OPT_TIMESTAMPS     1
#define TCPI_OPT_SACK           2
#define TCPI_OPT_WSCALE         4
#define TCPI_OPT_ECN            8

#define TCP_CA_Open             0
#define TCP_CA_Disorder         1
#define TCP_CA_CWR              2
#define TCP_CA_Recovery         3
#define TCP_CA_Loss             4
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define TCPOPT_EOL              0
#define TCPOPT_NOP              1
#define TCPOPT_MAXSEG           2
#define TCPOPT_WINDOW           3
#define TCPOPT_SACK_PERMITTED   4
#define TCPOPT_SACK             5
#define TCPOPT_TIMESTAMP        8
#define TCPOLEN_SACK_PERMITTED  2
#define TCPOLEN_WINDOW          3
#define TCPOLEN_MAXSEG          4
#define TCPOLEN_TIMESTAMP       10

#define SOL_TCP 6

#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>

#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20

typedef uint32_t tcp_seq;

struct tcphdr {
#ifdef _GNU_SOURCE
#ifdef __GNUC__
    __extension__
#endif
    union { struct {

    uint16_t source;
    uint16_t dest;
    uint32_t seq;
    uint32_t ack_seq;
#if BYTE_ORDER == LITTLE_ENDIAN
    uint16_t res1:4;
    uint16_t doff:4;
    uint16_t fin:1;
    uint16_t syn:1;
    uint16_t rst:1;
    uint16_t psh:1;
    uint16_t ack:1;
    uint16_t urg:1;
    uint16_t res2:2;
#else
    uint16_t doff:4;
    uint16_t res1:4;
    uint16_t res2:2;
    uint16_t urg:1;
    uint16_t ack:1;
    uint16_t psh:1;
    uint16_t rst:1;
    uint16_t syn:1;
    uint16_t fin:1;
#endif
    uint16_t window;
    uint16_t check;
    uint16_t urg_ptr;

    }; struct {
#endif

    uint16_t th_sport;
    uint16_t th_dport;
    uint32_t th_seq;
    uint32_t th_ack;
#if BYTE_ORDER == LITTLE_ENDIAN
    uint8_t th_x2:4;
    uint8_t th_off:4;
#else
    uint8_t th_off:4;
    uint8_t th_x2:4;
#endif
    uint8_t th_flags;
    uint16_t th_win;
    uint16_t th_sum;
    uint16_t th_urp;

#ifdef _GNU_SOURCE
    }; };
#endif
};
#endif /* _GNU_SOURCE || _BSD_SOURCE */

#ifdef _GNU_SOURCE
#define TCP_MD5SIG_MAXKEYLEN    80
#define TCP_MD5SIG_FLAG_PREFIX  1

#define TCP_REPAIR_ON        1
#define TCP_REPAIR_OFF        0
#define TCP_REPAIR_OFF_NO_WP    -1

struct tcp_info {
    uint8_t tcpi_state;
    uint8_t tcpi_ca_state;
    uint8_t tcpi_retransmits;
    uint8_t tcpi_probes;
    uint8_t tcpi_backoff;
    uint8_t tcpi_options;
    uint8_t tcpi_snd_wscale : 4, tcpi_rcv_wscale : 4;
    uint8_t tcpi_delivery_rate_app_limited : 1;
    uint32_t tcpi_rto;
    uint32_t tcpi_ato;
    uint32_t tcpi_snd_mss;
    uint32_t tcpi_rcv_mss;
    uint32_t tcpi_unacked;
    uint32_t tcpi_sacked;
    uint32_t tcpi_lost;
    uint32_t tcpi_retrans;
    uint32_t tcpi_fackets;
    uint32_t tcpi_last_data_sent;
    uint32_t tcpi_last_ack_sent;
    uint32_t tcpi_last_data_recv;
    uint32_t tcpi_last_ack_recv;
    uint32_t tcpi_pmtu;
    uint32_t tcpi_rcv_ssthresh;
    uint32_t tcpi_rtt;
    uint32_t tcpi_rttvar;
    uint32_t tcpi_snd_ssthresh;
    uint32_t tcpi_snd_cwnd;
    uint32_t tcpi_advmss;
    uint32_t tcpi_reordering;
    uint32_t tcpi_rcv_rtt;
    uint32_t tcpi_rcv_space;
    uint32_t tcpi_total_retrans;
    uint64_t tcpi_pacing_rate;
    uint64_t tcpi_max_pacing_rate;
    uint64_t tcpi_bytes_acked;
    uint64_t tcpi_bytes_received;
    uint32_t tcpi_segs_out;
    uint32_t tcpi_segs_in;
    uint32_t tcpi_notsent_bytes;
    uint32_t tcpi_min_rtt;
    uint32_t tcpi_data_segs_in;
    uint32_t tcpi_data_segs_out;
    uint64_t tcpi_delivery_rate;
    uint64_t tcpi_busy_time;
    uint64_t tcpi_rwnd_limited;
    uint64_t tcpi_sndbuf_limited;
    uint32_t tcpi_delivered;
    uint32_t tcpi_delivered_ce;
    uint64_t tcpi_bytes_sent;
    uint64_t tcpi_bytes_retrans;
    uint32_t tcpi_dsack_dups;
    uint32_t tcpi_reord_seen;
    uint32_t tcpi_rcv_ooopack;
    uint32_t tcpi_snd_wnd;
};

struct tcp_md5sig {
    struct sockaddr_storage tcpm_addr;
    uint8_t tcpm_flags;
    uint8_t tcpm_prefixlen;
    uint16_t tcpm_keylen;
    uint32_t __tcpm_pad;
    uint8_t tcpm_key[TCP_MD5SIG_MAXKEYLEN];
};

struct tcp_diag_md5sig {
    uint8_t tcpm_family;
    uint8_t tcpm_prefixlen;
    uint16_t tcpm_keylen;
    uint32_t tcpm_addr[4];
    uint8_t tcpm_key[TCP_MD5SIG_MAXKEYLEN];
};

struct tcp_repair_window {
    uint32_t snd_wl1;
    uint32_t snd_wnd;
    uint32_t max_window;
    uint32_t rcv_wnd;
    uint32_t rcv_wup;
};

struct tcp_zerocopy_receive {
    uint64_t address;
    uint32_t length;
    uint32_t recv_skip_hint;
};
#endif /* _GNU_SOURCE */

#endif /* !_ADAPT_NETINET_TCP_H */
