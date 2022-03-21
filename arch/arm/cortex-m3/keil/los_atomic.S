;
; Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
;
; Redistribution and use in source and binary forms, with or without modification,
; are permitted provided that the following conditions are met:
;
; 1. Redistributions of source code must retain the above copyright notice, this list of
;    conditions and the following disclaimer.
;
; 2. Redistributions in binary form must reproduce the above copyright notice, this list
;    of conditions and the following disclaimer in the documentation and/or other materials
;    provided with the distribution.
;
; 3. Neither the name of the copyright holder nor the names of its contributors may be used
;    to endorse or promote products derived from this software without specific prior written
;    permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
; "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
; THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
; PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
; EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
; WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
; OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
; ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;

        EXPORT  ArchAtomicRead
        EXPORT  ArchAtomicSet
        EXPORT  ArchAtomicAdd
        EXPORT  ArchAtomicSub
        EXPORT  ArchAtomicXchg32bits
        EXPORT  ArchAtomicCmpXchg32bits

    PRESERVE8
    AREA  |.text|, CODE, READONLY
    THUMB


        EXPORT  ArchAtomicRead
        EXPORT  ArchAtomicSet
        EXPORT  ArchAtomicAdd
        EXPORT  ArchAtomicSub
        EXPORT  ArchAtomicXchg32bits
        EXPORT  ArchAtomicCmpXchg32bits

    PRESERVE8
    AREA  |.text|, CODE, READONLY
    THUMB

ArchAtomicRead
    ldrex r1, [r0]
    mov r0, r1
    bx lr

ArchAtomicSet
    ldrex r2, [r0]
    strex r3, r1, [r0]
    teq r3, #0
    bne ArchAtomicSet
    bx lr

ArchAtomicAdd
    ldrex r2, [r0]
    add r2, r2, r1
    strex r3, r2, [r0]
    teq r3, #0
    bne ArchAtomicAdd
    mov r0, r2
    bx lr

ArchAtomicSub
    ldrex r2, [r0]
    sub r2, r2, r1
    strex r3, r2, [r0]
    teq r3, #0
    bne ArchAtomicSub
    mov r0, r2
    bx lr

ArchAtomicXchg32bits
    ldrex r2, [r0]
    strex r3, r1, [r0]
    teq r3, #0
    bne ArchAtomicXchg32bits
    mov r0, r2

ArchAtomicCmpXchg32bits
    ldrex r3, [r0]
    cmp r3, r2
    bne end
    strex r4, r1, [r0]
    teq r4, #0
    bne ArchAtomicCmpXchg32bits
end
