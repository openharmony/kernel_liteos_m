/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "xts_net.h"

LITE_TEST_SUIT(NET, ActsNet, ActsNetTestSuite);

static BOOL ActsNetTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL ActsNetTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number    : SUB_KERNEL_NET_0730
 * @tc.name      : test ioctl get and set IFHWADDR
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testIoctlIfhwAddr, Function | MediumTest | Level2)
{
    int udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    ICUNIT_ASSERT_NOT_EQUAL(udpFd, -1, udpFd); /* -1, common data for test, no special meaning */

    struct ifreq ifre[5]; /* 5, common data for test, no special meaning */
    struct ifconf ifcf = {0};
    (void)memset_s(&ifcf, sizeof(struct ifconf), 0, sizeof(struct ifconf));
    ifcf.ifc_len = 5 * sizeof(struct ifreq); /* 5, common data for test, no special meaning */
    ifcf.ifc_buf = (char *)ifre;
    int ret = ioctl(udpFd, SIOCGIFCONF, (char *)&ifcf);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    char rst1[18]; /* 18, common data for test, no special meaning */
    char *macPtr = NULL;
    struct ifreq ifrTmp = {0};
    struct sockaddr_in *addr = NULL;
    int ifrCount = ifcf.ifc_len / sizeof(struct ifreq);
    ICUNIT_GOTO_EQUAL(ifrCount, 2, ifrCount, EXIT); /* 2, common data for test, no special meaning */
    ICUNIT_ASSERT_WITHIN_EQUAL(ifrCount, 2, INT_MAX, ifrCount); /* 2, common data for test, no special meaning */
EXIT:
    for (int i = 0; i < ifrCount; i++) {
        addr = (struct sockaddr_in *)&ifre[i].ifr_addr;
        if (strcmp("lo", ifre[i].ifr_name) != 0) {
            (void)memset_s(&ifrTmp, sizeof(struct ifreq), 0, sizeof(struct ifreq));
            ret = strcpy_s(ifrTmp.ifr_name, sizeof(ifrTmp.ifr_name), ifre[i].ifr_name);
            ICUNIT_ASSERT_EQUAL(ret, 0, ret);
            ret = ioctl(udpFd, SIOCGIFHWADDR, &ifrTmp);
            ICUNIT_ASSERT_EQUAL(ret, 0, ret);
            macPtr = ifrTmp.ifr_hwaddr.sa_data;
            ret = sprintf_s(rst1, sizeof(rst1), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", *macPtr, *(macPtr + 1),
                *(macPtr + 2), *(macPtr + 3), *(macPtr + 4), *(macPtr + 5)); /* 1, 2, 3, 4, 5, common data for test, no special meaning */
            ICUNIT_ASSERT_EQUAL((unsigned int)ret, strlen(rst1), (unsigned int)ret);
        }
    }
    ret = close(udpFd);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_1000
 * @tc.name      : test socket operation
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testSocketOpt, Function | MediumTest | Level2)
{
    socklen_t len;
    struct timeval timeout = {0};
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd); /* -1, common data for test, no special meaning */

    int error = -1; /* -1, common data for test, no special meaning */
    len = sizeof(error);
    int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(error, 0, error);

    len = sizeof(timeout);
    ret = getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, &len);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    timeout.tv_sec = 1000; /* 1000, common data for test, no special meaning */
    len = sizeof(timeout);
    ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    (void)memset_s(&timeout, len, 0, len);
    ret = getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, &len);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(timeout.tv_sec, 1000, timeout.tv_sec); /* 1000, common data for test, no special meaning */

    int flag = 1; /* 1, common data for test, no special meaning */
    ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    flag = 0;
    len = sizeof(flag);
    ret = getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, &len);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(flag, 1, flag); /* 1, common data for test, no special meaning */

    error = -1; /* -1, common data for test, no special meaning */
    len = sizeof(error);
    ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(error, 0, error);

    ret = close(fd);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_1100
 * @tc.name      : test getsockname and getpeername invalid input
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testGetSocketNameInvalidInput, Function | MediumTest | Level3)
{
    struct sockaddr addr = {0};
    socklen_t addrLen = sizeof(addr);
    int ret = getsockname(-1, &addr, &addrLen); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getpeername(-1, &addr, &addrLen); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getsockname(0, &addr, &addrLen);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getpeername(0, &addr, &addrLen);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getsockname(1, &addr, &addrLen); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getpeername(1, &addr, &addrLen); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getsockname(130, &addr, &addrLen); /* 130, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getpeername(130, &addr, &addrLen); /* 130, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getsockname(10, NULL, &addrLen); /* 10, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getpeername(10, NULL, &addrLen); /* 10, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getsockname(10, &addr, NULL); /* 10, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = getpeername(10, &addr, NULL); /* 10, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_2400
 * @tc.name      : test convert value from host to network byte order
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testHostToNetwork, Function | MediumTest | Level2)
{
    uint32_t intInput1 = 0;
    uint32_t intRst01 = htonl(intInput1);
    uint32_t intInput2 = 65536; /* 65536, common data for test, no special meaning */
    uint32_t intRst02 = htonl(intInput2);
    uint16_t shortInput1 = 0;
    uint16_t shortRst01 = htons(shortInput1);
    uint16_t shortInput2 = 255; /* 255, common data for test, no special meaning */
    uint16_t shortRst02 = htons(shortInput2);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint32_t expectZero = 0;
    uint32_t expectForIinput2 = 256; /* 255, common data for test, no special meaning */
    uint32_t expectForSinput2 = 65280; /* 65536, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(intRst01, expectZero, intRst01);
    ICUNIT_ASSERT_EQUAL(intRst02, expectForIinput2, intRst02);
    ICUNIT_ASSERT_EQUAL(shortRst01, expectZero, shortRst01);
    ICUNIT_ASSERT_EQUAL(shortRst02, expectForSinput2, shortRst02);
#else
    ICUNIT_ASSERT_EQUAL(intRst01, intInput1, intRst01);
    ICUNIT_ASSERT_EQUAL(intRst02, intInput2, intRst02);
    ICUNIT_ASSERT_EQUAL(shortRst01, shortInput1, shortRst01);
    ICUNIT_ASSERT_EQUAL(shortRst02, shortInput2, shortRst02);
#endif
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_2500
 * @tc.name      : test convert value from network to host byte order
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testNetworkToHost, Function | MediumTest | Level2)
{
    uint32_t intInput1 = 0;
    uint32_t intRst1 = ntohl(intInput1);
    uint32_t intInput2 = 65536; /* 65536, common data for test, no special meaning */
    uint32_t intRst02 = ntohl(intInput2);
    uint16_t shortInput1 = 0;
    uint16_t shortRst01 = ntohs(shortInput1);
    uint16_t shortInput2 = 255; /* 255, common data for test, no special meaning */
    uint16_t shortRst02 = ntohs(shortInput2);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint32_t expectZero = 0;
    uint32_t expectForIinput2 = 256; /* 255, common data for test, no special meaning */
    uint32_t expectForSinput2 = 65280; /* 65536, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(intRst1, expectZero, intRst1);
    ICUNIT_ASSERT_EQUAL(intRst02, expectForIinput2, intRst02);
    ICUNIT_ASSERT_EQUAL(shortRst01, expectZero, shortRst01);
    ICUNIT_ASSERT_EQUAL(shortRst02, expectForSinput2, shortRst02);
#else
    ICUNIT_ASSERT_EQUAL(intRst1, intInput1, intRst1);
    ICUNIT_ASSERT_EQUAL(intRst02, intInput2, intRst02);
    ICUNIT_ASSERT_EQUAL(shortRst01, shortInput1, shortRst01);
    ICUNIT_ASSERT_EQUAL(shortRst02, shortInput2, shortRst02);
#endif
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_2600
 * @tc.name      : test inet_pton IPv4 normal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetPtonIpv4Normal, Function | MediumTest | Level2)
{
    int ret;
    struct in_addr rst = {0};
    char cpAddrs[4][16] = {"10.58.212.100", "0.0.0.0", "255.0.0.0", "255.255.255.255"}; /* 4, 16, common data for test, no special meaning */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    unsigned int expLittle[4] = {1691630090, 0, 255, 4294967295}; /* 4, common data for test, no special meaning */
#else
    unsigned int expBig[4] = {171627620, 0, 4278190080, 4294967295}; /* 4, common data for test, no special meaning */
#endif

    for (int i = 0; i < 4; i++) { /* 4, common data for test, no special meaning */
        ret = inet_pton(AF_INET, cpAddrs[i], &rst);
        ICUNIT_ASSERT_EQUAL(ret, 1, ret);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        ICUNIT_ASSERT_EQUAL(rst.s_addr, expLittle[i], rst.s_addr);
#else
        ICUNIT_ASSERT_EQUAL(rst.s_addr, expBig[i], rst.s_addr);
#endif
    }
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_2800
 * @tc.name      : test inet_pton IPv6 normal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetPtonIpv6Normal, Function | MediumTest | Level2)
{
    int ret;
    struct in6_addr rst = {0};
    char cpAddrs[6][40] = {"0101:0101:0101:0101:1010:1010:1010:1010", "0:0:0:0:0:0:0:0", /* 6, 40, common data for test, no special meaning */
        "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF", "::", "1::", "0011:0011:0011:0011:11:11:11:11"};
    for (int i = 0; i < 6; i++) { /* 6, common data for test, no special meaning */
        ret = inet_pton(AF_INET6, cpAddrs[i], &rst);
        ICUNIT_ASSERT_EQUAL(ret, 1, ret); /* 1, common data for test, no special meaning */
    }
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_2900
 * @tc.name      : test inet_pton IPv6 abnormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetPtonIpv6Abnormal, Function | MediumTest | Level2)
{
    int ret;
    struct in6_addr rst = {0};
    char cpAddrs[7][40] = {"127.0.0.1", "f", ":", "0:0", "1:::", ":::::::", /* 7, 40, common data for test, no special meaning */
        "1111:1111:1111:1111:1111:1111:1111:111G"};
    for (int i = 0; i < 7; i++) { /* 7, common data for test, no special meaning */
        ret = inet_pton(AF_INET6, cpAddrs[i], &rst);
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    }
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_3000
 * @tc.name      : test inet_pton with invalid family
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetPtonInvalidFamily, Function | MediumTest | Level2)
{
    struct in_addr rst = {0};
    int ret = inet_pton(AF_IPX, "127.0.0.1", &rst);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = inet_pton(-1, "127.0.0.1", &rst);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_3100
 * @tc.name      : test inet_ntop IPv4 normal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetNtopIpv4Normal, Function | MediumTest | Level2)
{
    const char *ret = NULL;
    struct in_addr inputAddr = {0};
    char rstBuff[INET_ADDRSTRLEN];
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    unsigned int inpLittle[4] = {0x64d43a0a, 0, 255, 4294967295}; /* 4, common data for test, no special meaning */
#else
    unsigned int inpBig[4] = {171627620, 0, 4278190080, 4294967295}; /* 4, common data for test, no special meaning */
#endif

    char expAddrs[4][16] = {"10.58.212.100", "0.0.0.0", "255.0.0.0", "255.255.255.255"}; /* 4, 16, common data for test, no special meaning */
    for (int i = 0; i < 4; i++) { /* 4, common data for test, no special meaning */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        inputAddr.s_addr = inpLittle[i];
#else
        inputAddr.s_addr = inpBig[i];
#endif
        ret = inet_ntop(AF_INET, &inputAddr, rstBuff, sizeof(rstBuff));
        if (ret == NULL) {
            ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
        } else {
            ICUNIT_ASSERT_STRING_EQUAL(ret, expAddrs[i], ret);
            ICUNIT_ASSERT_STRING_EQUAL(rstBuff, expAddrs[i], rstBuff);
        }
    }
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_3200
 * @tc.name      : test inet_ntop IPv4 boundary input
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetNtopIpv4Abnormal, Function | MediumTest | Level2)
{
    const char *ret = NULL;
    struct in_addr inputAddr = {0};
    char rstBuff[INET_ADDRSTRLEN];
    char expStr[2][16] = {"255.255.255.255", "0.0.0.0"}; /* 2, 16, common data for test, no special meaning */
    for (int i = 0; i < 2; i++) { /* 2, common data for test, no special meaning */
        inputAddr.s_addr = (i == 0 ? -1 : 4294967296); /* -1, 4294967296, common data for test, no special meaning */
        ret = inet_ntop(AF_INET, &inputAddr, rstBuff, sizeof(rstBuff));
        ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
        ICUNIT_ASSERT_STRING_EQUAL(ret, expStr[i], ret);
        ICUNIT_ASSERT_STRING_EQUAL(rstBuff, expStr[i], rstBuff);
    }
    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_NET_3500
 * @tc.name      : test inet_ntop with invalid family
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetNtopInvalidFamily, Function | MediumTest | Level2)
{
    int iret;
    const char *ret = NULL;
    struct in6_addr inputAddr = {0};
    char rstBuff[INET6_ADDRSTRLEN];

    iret = inet_pton(AF_INET6, "1::", &inputAddr);
    ICUNIT_ASSERT_EQUAL(iret, 1, iret); /* 1, common data for test, no special meaning */
    ret = inet_ntop(AF_IPX, &inputAddr, rstBuff, sizeof(rstBuff));
    ICUNIT_ASSERT_EQUAL(ret, NULL, ret);
    ret = inet_ntop(-1, &inputAddr, rstBuff, sizeof(rstBuff)); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, NULL, ret);
    return 0;
}

RUN_TEST_SUITE(ActsNetTestSuite);

void ActsNetTest(void)
{
    RUN_ONE_TESTCASE(testIoctlIfhwAddr);
    RUN_ONE_TESTCASE(testSocketOpt);
    RUN_ONE_TESTCASE(testGetSocketNameInvalidInput);
    RUN_ONE_TESTCASE(testHostToNetwork);
    RUN_ONE_TESTCASE(testNetworkToHost);
    RUN_ONE_TESTCASE(testInetPtonIpv4Normal);
    RUN_ONE_TESTCASE(testInetPtonInvalidFamily);
    RUN_ONE_TESTCASE(testInetNtopIpv4Normal);
    RUN_ONE_TESTCASE(testInetNtopIpv4Abnormal);
#if (LWIP_IPV6 == 1)
    RUN_ONE_TESTCASE(testInetPtonIpv6Normal);
    RUN_ONE_TESTCASE(testInetPtonIpv6Abnormal);
    RUN_ONE_TESTCASE(testInetNtopInvalidFamily);
#endif
}

