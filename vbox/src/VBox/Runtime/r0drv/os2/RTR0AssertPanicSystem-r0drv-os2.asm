; $Id: RTR0AssertPanicSystem-r0drv-os2.asm 69218 2017-10-24 14:57:36Z vboxsync $
;; @file
; IPRT - RTR0AssertPanicSystem, Ring-0 Driver, OS/2.
;

;
; Copyright (c) 1999-2007 knut st. osmundsen <bird-src-spam@anduin.net>
;
; Permission is hereby granted, free of charge, to any person
; obtaining a copy of this software and associated documentation
; files (the "Software"), to deal in the Software without
; restriction, including without limitation the rights to use,
; copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the
; Software is furnished to do so, subject to the following
; conditions:
;
; The above copyright notice and this permission notice shall be
; included in all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
; OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
; NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
; OTHER DEALINGS IN THE SOFTWARE.
;


;*******************************************************************************
;* Header Files                                                                *
;*******************************************************************************
%define RT_INCL_16BIT_SEGMENTS
%include "iprt/asmdefs.mac"


;*******************************************************************************
;* Defined Constants And Macros                                                *
;*******************************************************************************
%define DevHlp_InternalError 02bh


;*******************************************************************************
;* External Symbols                                                            *
;*******************************************************************************
extern KernThunkStackTo16
extern KernThunkStackTo32
extern NAME(g_szRTAssertMsg)
extern NAME(g_cchRTAssertMsg)
extern NAME(g_fpfnDevHlp)


BEGINCODE

BEGINPROC_EXPORTED RTR0AssertPanicSystem
    push    ebp
    mov     ebp, esp
    push    esi
    push    edi
    push    ds

    ;
    ; Try detect debug kernel... later one day.
    ;


    ;
    ; Raise an IPE.
    ;
    call    KernThunkStackTo16
    ;jmp far dword NAME(RTR0AssertPanicSystem_16) wrt CODE16
    db      066h
    db      0eah
    dw      NAME(RTR0AssertPanicSystem_16) wrt CODE16
    dw      CODE16
BEGINCODE16
GLOBALNAME RTR0AssertPanicSystem_16
    ; mov     ax, seg NAME(g_szRTAssertMsg) - makes wlink crash.
    mov     ax, DATA16
    mov     ds, ax
    mov     si, NAME(g_szRTAssertMsg)
    mov     di, [NAME(g_cchRTAssertMsg)]
    mov     dl, DevHlp_InternalError
    call far [NAME(g_fpfnDevHlp)]

    ; Doesn't normally return, but in case it does...
    ;jmp far dword NAME(RTR0AssertPanicSystem_32)
    db      066h
    db      0eah
    dd      NAME(RTR0AssertPanicSystem_32)
    dw      TEXT32 wrt FLAT
BEGINCODE32:
GLOBALNAME RTR0AssertPanicSystem_32
    call KernThunkStackTo32
    mov     eax, 1
    pop     ds
    pop     edi
    pop     esi
    leave
    ret
ENDPROC RTR0AssertPanicSystem

