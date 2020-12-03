;/*----------------------------------------------------------------------------
;*      Huawei - LiteOS
;*----------------------------------------------------------------------------
;*      Name:    LOS_VENDOR.S
;*      Purpose: Thread scheduler
;*      Rev.:    V1.0.0
;*----------------------------------------------------------------------------
;*

;* Copyright (c) 2014, Huawei Technologies Co., Ltd.
;* All rights reserved.
;* Permission to use, copy, modify, and distribute this software for any
;* purpose with or without fee is hereby granted, provided that the above
;* copyright notice and this permission notice appear in all copies.

;*THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
;*WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
;*MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
;*ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
;*WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
;*ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
;*OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
;*---------------------------------------------------------------------------*/


        PRESERVE8

    AREA    RESET, CODE, READONLY
    THUMB

    IMPORT  ||Image$$ARM_LIB_STACKHEAP$$ZI$$Limit||
    IMPORT  OsHwiDefaultHandler

    EXPORT  _BootVectors
    EXPORT  Reset_Handler

_BootVectors
    DCD     ||Image$$ARM_LIB_STACKHEAP$$ZI$$Limit||
    DCD     Reset_Handler
    DCD     OsHwiDefaultHandler
    DCD     OsHwiDefaultHandler


Reset_Handler
    CPSID   I

    IMPORT  LOS_HardBootInit
    BL      LOS_HardBootInit
    IMPORT  __main
    LDR     R0, =__main
    BX      R0

    ALIGN
    END
