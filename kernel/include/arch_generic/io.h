/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _ARCH_GENERIC_IO_H
#define _ARCH_GENERIC_IO_H

#include "los_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef ArchWrite8
#define ArchWrite8(addr, value) ArchWrite8(addr, value)
STATIC INLINE VOID ArchWrite8(UINTPTR addr, UINT8 value)
{
    *(volatile UINT8 *)addr = value;
}
#endif

#ifndef ArchWrite16
#define ArchWrite16(addr, value) ArchWrite16(addr, value)
STATIC INLINE VOID ArchWrite16(UINTPTR addr, UINT16 value)
{
    *(volatile UINT16 *)addr = value;
}
#endif

#ifndef ArchWrite32
#define ArchWrite32(addr, value) ArchWrite32(addr, value)
STATIC INLINE VOID ArchWrite32(UINTPTR addr, UINT32 value)
{
    *(volatile UINT32 *)addr = value;
}
#endif

#ifndef ArchWrite64
#define ArchWrite64(addr, value) ArchWrite64(addr, value)
STATIC INLINE VOID ArchWrite64(UINTPTR addr, UINT64 value)
{
    *(volatile UINT64 *)addr = value;
}
#endif

#ifndef ArchRead8
#define ArchRead8(addr) ArchRead8(addr)
STATIC INLINE UINT8 ArchRead8(UINTPTR addr)
{
    return *(volatile UINT8 *)addr;
}
#endif

#ifndef ArchRead16
#define ArchRead16(addr) ArchRead16(addr)
STATIC INLINE UINT16 ArchRead16(UINTPTR addr)
{
    return *(volatile UINT16 *)addr;
}
#endif

#ifndef ArchRead32
#define ArchRead32(addr) ArchRead32(addr)
STATIC INLINE UINT32 ArchRead32(UINTPTR addr)
{
    return *(volatile UINT32 *)addr;
}
#endif

#ifndef ArchRead64
#define ArchRead64(addr) ArchRead64(addr)
STATIC INLINE UINT64 ArchRead64(UINTPTR addr)
{
    return *(volatile UINT64 *)addr;
}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ARCH_GENERIC_IO_H */
