;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
;//////////////////////////////////////////////////////////////////////
; [BITS 32]

%include "../../include/skl_syst/skl_nasm.h"

globl Skl_SAD_4x4_MMX
globl Skl_SAD_4x8_MMX
globl Skl_SAD_8x4_MMX
globl Skl_SAD_8x8_MMX
globl Skl_SAD_8x16_MMX
globl Skl_SAD_16x8_MMX
globl Skl_SAD_16x16_MMX
globl Skl_SAD_16x8_Field_MMX
globl Skl_SAD_16x7_Self_MMX

globl Skl_SSD_4x4_MMX
globl Skl_SSD_8x8_MMX
globl Skl_SSD_16x16_MMX
globl Skl_SSD_16x8_Field_MMX

globl Skl_SAD_4x4_SSE
globl Skl_SAD_4x8_SSE
globl Skl_SAD_8x4_SSE
globl Skl_SAD_8x8_SSE
globl Skl_SAD_8x16_SSE
globl Skl_SAD_16x8_SSE
globl Skl_SAD_16x16_SSE
globl Skl_SAD_16x8_Field_SSE
globl Skl_SAD_16x7_Self_SSE

globl Skl_SAD_Avrg_16x16_SSE
globl Skl_SAD_Avrg_16x8_SSE
globl Skl_SAD_Avrg_8x16_SSE
globl Skl_SAD_Avrg_8x8_SSE


globl Skl_Mean_16x16_MMX
globl Skl_Mean_8x8_MMX
globl Skl_Mean_4x4_MMX
globl Skl_Sqr_16x16_MMX
globl Skl_Sqr_8x8_MMX
globl Skl_Sqr_4x4_MMX

globl Skl_Mean_16x16_SSE
globl Skl_Mean_8x8_SSE
globl Skl_Mean_4x4_SSE
globl Skl_Abs_Dev_16x16_SSE
globl Skl_Sqr_Dev_16x16_SSE

globl Skl_SAD_16x16_SSE2
globl Skl_SAD_16x8_Field_SSE2
globl Skl_SAD_16x7_Self_SSE2
globl Skl_Mean_16x16_SSE2
globl Skl_Sqr_16x16_SSE2
globl Skl_Abs_Dev_16x16_SSE2


DATA

align 16
One:     times 8  dw 1     ; for summing 4 words

TEXT

%macro COLLAPSE_MMX 0
  movq    mm7, mm6
  pmaddwd mm6, [One]
  psrlq   mm7, 32
  pmaddwd mm7, [One]
  paddd   mm6, mm7
  movd    eax, mm6
%endmacro

%macro COLLAPSE_4_MMX 0
  COLLAPSE_MMX
%endmacro

;//////////////////////////////////////////////////////////////////////
;//
;//  MMX impl
;//
;//////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////
; Skl_SAD_16x16_MMX
;//////////////////////////////////////////////////////////////////////


%macro SAD_16x16_MMX 0
  movq mm0, [eax]
  movq mm1, [edx]
  movq mm2, [eax+8]
  movq mm3, [edx+8]

  lea edx,[edx+ecx]

    ; we do our best *not* to go 16b, here

  movq    mm4, mm0
  psubusb mm0, mm1
  movq    mm5, mm2
  psubusb mm2, mm3

  psubusb mm1, mm4
  por     mm0, mm1
  psubusb mm3, mm5
  por     mm2, mm3

  movq      mm1,mm0
  punpcklbw mm0,mm7
  punpckhbw mm1,mm7
  movq      mm3,mm2
  punpcklbw mm2,mm7
  paddusw   mm0,mm1
  lea eax,[eax+ecx]
  punpckhbw mm3,mm7
  paddusw   mm6,mm0
  paddusw   mm2,mm3
  paddusw   mm6,mm2
%endmacro

align 16
Skl_SAD_16x16_MMX:  ; 179c
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX

  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX

  COLLAPSE_MMX
  ret

align 16
Skl_SAD_16x8_MMX:
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX

  COLLAPSE_MMX
  ret

align 16
Skl_SAD_16x8_Field_MMX:  ; 179c
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS
  lea ecx, [ecx+ecx]  ; 2.BpS

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX
  SAD_16x16_MMX

  COLLAPSE_MMX
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_SAD_16x7_Self_MMX
;//////////////////////////////////////////////////////////////////////

%macro SAD_16x7_MMX 0
  movq mm0, [eax]
  movq mm1, [edx]
  movq mm2, [eax+8]
  movq mm3, [edx+8]

  lea edx,[edx+ecx]

    ; we do our best *not* to go 16b, here

  movq    mm4, mm0
  psubusb mm0, mm1
  movq    mm5, mm2
  psubusb mm2, mm3

  psubusb mm1, mm4
  por     mm0, mm1
  psubusb mm3, mm5
  por     mm2, mm3

  movq      mm1,mm0
  punpcklbw mm0,mm7
  punpckhbw mm1,mm7
  movq      mm3,mm2
  punpcklbw mm2,mm7
  paddusw   mm0,mm1
  lea eax,[eax+ecx]
  punpckhbw mm3,mm7
  paddusw   mm6,mm0
  paddusw   mm2,mm3
  paddusw   mm6,mm2
%endmacro

align 16
Skl_SAD_16x7_Self_MMX:
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS
  lea edx, [eax+ecx]; Src2

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  SAD_16x7_MMX
  SAD_16x7_MMX
  SAD_16x7_MMX
  SAD_16x7_MMX
  SAD_16x7_MMX
  SAD_16x7_MMX
  SAD_16x7_MMX

  COLLAPSE_MMX
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_SAD_8x8_MMX
;//////////////////////////////////////////////////////////////////////

%macro SAD_8x8_MMX  0
  movq mm0, [eax]
  movq mm1, [edx]
  movq mm2, [eax+ecx]
  movq mm3, [edx+ecx]

  lea edx,[edx+2*ecx]

    ; we do our best *not* to go 16b, here

  movq    mm4, mm0
  psubusb mm0, mm1
  movq    mm5, mm2
  psubusb mm2, mm3

  psubusb mm1, mm4
  por     mm0, mm1
  psubusb mm3, mm5
  por     mm2, mm3

  movq      mm1,mm0
  punpcklbw mm0,mm7
  punpckhbw mm1,mm7
  movq      mm3,mm2
  punpcklbw mm2,mm7
  paddusw   mm0,mm1
  lea eax,[eax+2*ecx]
  punpckhbw mm3,mm7
  paddusw   mm6,mm0
  paddusw   mm2,mm3
  paddusw   mm6,mm2
%endmacro

align 16
Skl_SAD_8x4_MMX:
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  SAD_8x8_MMX
  SAD_8x8_MMX

  COLLAPSE_MMX
  ret

align 16
Skl_SAD_8x8_MMX:    ; 57c
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  SAD_8x8_MMX
  SAD_8x8_MMX
  SAD_8x8_MMX
  SAD_8x8_MMX

  COLLAPSE_MMX
  ret

align 16
Skl_SAD_8x16_MMX:
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  SAD_8x8_MMX
  SAD_8x8_MMX
  SAD_8x8_MMX
  SAD_8x8_MMX

  SAD_8x8_MMX
  SAD_8x8_MMX
  SAD_8x8_MMX
  SAD_8x8_MMX

  COLLAPSE_MMX
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_SAD_4x4_MMX
;//////////////////////////////////////////////////////////////////////

%macro SAD_4x4_MMX  0
  movd mm0, [eax]
  movd mm1, [edx]
  movd mm2, [eax+ecx]
  movd mm3, [edx+ecx]

  lea edx,[edx+2*ecx]

    ; we do our best *not* to go 16b, here

  movq    mm4, mm0
  psubusb mm0, mm1
  movq    mm5, mm2
  psubusb mm2, mm3

  psubusb mm1, mm4
  por     mm0, mm1
  psubusb mm3, mm5
  por     mm2, mm3

  punpcklbw mm0,mm7
  punpcklbw mm2,mm7
  paddusw  mm6,mm0
  lea eax,[eax+2*ecx]
  paddusw  mm6,mm2
%endmacro

align 16
Skl_SAD_4x4_MMX:    ; 57c
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  SAD_4x4_MMX
  SAD_4x4_MMX

  COLLAPSE_4_MMX
  ret

align 16
Skl_SAD_4x8_MMX:
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  SAD_4x4_MMX
  SAD_4x4_MMX
  SAD_4x4_MMX
  SAD_4x4_MMX

  COLLAPSE_4_MMX
  ret


;//////////////////////////////////////////////////////////////////////
; Skl_SSD_16x16_MMX
;//////////////////////////////////////////////////////////////////////

%macro SSD_16x16_MMX 0
  movq mm0, [eax]
  movq mm1, [ecx]
  movq mm2, [eax+8]
  movq mm3, [ecx+8]
  lea eax,[eax+edx]
  lea ecx,[ecx+edx]
  movq mm4, mm0
  movq mm5, mm1
  punpcklbw mm0, mm6
  punpcklbw mm1, mm6
  punpckhbw mm4, mm6
  punpckhbw mm5, mm6
  psubw mm0, mm1
  psubw mm4, mm5
  pmaddwd mm0, mm0
  pmaddwd mm4, mm4
  paddd mm7, mm0
  paddd mm7, mm4
  movq mm4, mm2
  movq mm5, mm3
  punpcklbw mm2, mm6
  punpcklbw mm3, mm6
  punpckhbw mm4, mm6
  punpckhbw mm5, mm6
  psubw mm2, mm3
  psubw mm4, mm5
  pmaddwd mm2, mm2
  pmaddwd mm4, mm4
  paddd mm7, mm2
  paddd mm7, mm4
%endmacro

align 16
Skl_SSD_16x16_MMX:
  mov eax, [esp+ 4] ; Src1
  mov ecx, [esp+ 8] ; Src2
  mov edx, [esp+12] ; BpS

  pxor mm7, mm7 ; accum
  pxor mm6, mm6 ; zero

  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX

  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX

  movq mm6, mm7
  psrlq mm7, 32
  paddd mm6, mm7

  movd eax, mm6
  ret

align 16
Skl_SSD_16x8_Field_MMX:
  mov eax, [esp+ 4] ; Src1
  mov ecx, [esp+ 8] ; Src2
  mov edx, [esp+12] ; BpS
  lea edx, [edx+edx]  ; 2.BpS

  pxor mm7, mm7 ; accum
  pxor mm6, mm6 ; zero

  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX
  SSD_16x16_MMX

  movq mm6, mm7
  psrlq mm7, 32
  paddd mm6, mm7

  movd eax, mm6
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_SSD_8x8_MMX
;//////////////////////////////////////////////////////////////////////

%macro SSD_8x8_MMX  0
  movq mm0, [eax]
  movq mm1, [ecx]
  movq mm2, [eax+edx]
  movq mm3, [ecx+edx]
  lea eax,[eax+2*edx]
  lea ecx,[ecx+2*edx]
  movq mm4, mm0
  movq mm5, mm1
  punpcklbw mm0, mm6
  punpcklbw mm1, mm6
  punpckhbw mm4, mm6
  punpckhbw mm5, mm6
  psubw mm0, mm1
  psubw mm4, mm5
  pmaddwd mm0, mm0
  pmaddwd mm4, mm4
  paddd mm7, mm0
  paddd mm7, mm4
  movq mm4, mm2
  movq mm5, mm3
  punpcklbw mm2, mm6
  punpcklbw mm3, mm6
  punpckhbw mm4, mm6
  punpckhbw mm5, mm6
  psubw mm2, mm3
  psubw mm4, mm5
  pmaddwd mm2, mm2
  pmaddwd mm4, mm4
  paddd mm7, mm2
  paddd mm7, mm4
%endmacro

align 16
Skl_SSD_8x8_MMX:
  mov eax, [esp+ 4] ; Src1
  mov ecx, [esp+ 8] ; Src2
  mov edx, [esp+12] ; BpS

  pxor mm7, mm7 ; accum
  pxor mm6, mm6 ; zero

  SSD_8x8_MMX
  SSD_8x8_MMX
  SSD_8x8_MMX
  SSD_8x8_MMX

  movq mm6, mm7
  psrlq mm7, 32
  paddd mm6, mm7
  movd eax, mm6
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_SSD_4x4_MMX
;//////////////////////////////////////////////////////////////////////

%macro SSD_4x4_MMX  0
  movd mm0, [eax]
  movd mm1, [ecx]
  movd mm2, [eax+edx]
  movd mm3, [ecx+edx]
  punpcklbw mm0, mm6
  punpcklbw mm1, mm6
  punpcklbw mm2, mm6
  punpcklbw mm3, mm6
  psubw mm0, mm1
  psubw mm2, mm3
  pmaddwd mm0, mm0
  pmaddwd mm2, mm2
  paddd mm7, mm0
  paddd mm7, mm2
%endmacro

align 16
Skl_SSD_4x4_MMX:
  mov eax, [esp+ 4] ; Src1
  mov ecx, [esp+ 8] ; Src2
  mov edx, [esp+12] ; BpS

  pxor mm7, mm7 ; accum
  pxor mm6, mm6 ; zero

  SSD_4x4_MMX
  lea eax,[eax+2*edx]
  lea ecx,[ecx+2*edx]
  SSD_4x4_MMX

  movq mm6, mm7
  psrlq mm7, 32
  paddd mm6, mm7
  movd eax, mm6
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Mean_16x16_MMX
;//////////////////////////////////////////////////////////////////////

%macro MEAN_16x16_MMX 0
  movq mm0, [eax]
  movq mm1, [eax+8]
  lea eax,[eax+ecx]
  movq mm2, mm0
  movq mm3, mm1
  punpcklbw mm0,mm7
  punpcklbw mm1,mm7
  punpckhbw mm2,mm7
  punpckhbw mm3,mm7
  paddw mm5, mm0
  paddw mm6, mm1
  paddw mm5, mm2
  paddw mm6, mm3
%endmacro

align 16
Skl_Mean_16x16_MMX:
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor mm5, mm5 ; accums
  pxor mm6, mm6 ; accums
  pxor mm7, mm7 ; zero

  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX

  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX
  MEAN_16x16_MMX

  paddusw mm6, mm5
  COLLAPSE_MMX
  shr eax, 8
  ret


;//////////////////////////////////////////////////////////////////////
; Skl_Mean_8x8_MMX
;//////////////////////////////////////////////////////////////////////

%macro MEAN_8x8_MMX  0
  movq mm0, [eax]
  movq mm1, [eax+ecx]
  lea eax,[eax+2*ecx]
  movq mm2, mm0
  movq mm3, mm1
  punpcklbw mm0,mm7
  punpcklbw mm1,mm7
  punpckhbw mm2,mm7
  punpckhbw mm3,mm7
  paddw mm5, mm0
  paddw mm6, mm1
  paddw mm5, mm2
  paddw mm6, mm3
%endmacro

align 16
Skl_Mean_8x8_MMX:
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS
  
  pxor mm5, mm5 ; accums
  pxor mm6, mm6 ; accums
  pxor mm7, mm7 ; zero

  MEAN_8x8_MMX
  MEAN_8x8_MMX
  MEAN_8x8_MMX
  MEAN_8x8_MMX

  paddw mm6, mm5
  COLLAPSE_MMX
  shr eax,6
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Mean_4x4_MMX
;//////////////////////////////////////////////////////////////////////

%macro MEAN_4x4_MMX  0
  movd mm0, [eax]
  movd mm1, [eax+ecx]
  lea eax,[eax+2*ecx]
  punpcklbw mm0,mm7
  punpcklbw mm1,mm7
  paddw mm5, mm0
  paddw mm6, mm1
%endmacro

align 16
Skl_Mean_4x4_MMX:
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS
  
  pxor mm5, mm5 ; accums
  pxor mm6, mm6 ; accums
  pxor mm7, mm7 ; zero

  MEAN_4x4_MMX
  MEAN_4x4_MMX

  paddw mm6, mm5
  COLLAPSE_MMX
  shr eax,4
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Sqr_16x16_MMX
;//////////////////////////////////////////////////////////////////////

%macro SQR_16x16_MMX 0
  movq mm0, [eax]
  movq mm1, [eax+8]
  lea eax,[eax+ecx]
  movq mm2, mm0
  movq mm3, mm1
  punpcklbw mm0, mm6
  punpcklbw mm1, mm6
  punpckhbw mm2, mm6
  punpckhbw mm3, mm6
  pmaddwd mm0, mm0
  pmaddwd mm1, mm1
  pmaddwd mm2, mm2
  pmaddwd mm3, mm3
  paddd mm7, mm0
  paddd mm7, mm1
  paddd mm7, mm2
  paddd mm7, mm3
%endmacro

align 16
Skl_Sqr_16x16_MMX:
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor mm7, mm7 ; accum
  pxor mm6, mm6 ; zero

  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX

  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX
  SQR_16x16_MMX

  movq mm6, mm7
  psrlq mm7, 32
  paddd mm6, mm7

  movd eax, mm6
  shr eax, 8
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Sqr_8x8_MMX
;//////////////////////////////////////////////////////////////////////

%macro SQR_8x8_MMX  0
  movq mm0, [eax]
  movq mm1, [eax+ecx]
  lea eax,[eax+2*ecx]
  movq mm2, mm0
  movq mm3, mm1
  punpcklbw mm0, mm6
  punpcklbw mm1, mm6
  punpckhbw mm2, mm6
  punpckhbw mm3, mm6
  pmaddwd mm0, mm0
  pmaddwd mm1, mm1
  pmaddwd mm2, mm2
  pmaddwd mm3, mm3
  paddd mm7, mm0
  paddd mm7, mm1
  paddd mm7, mm2
  paddd mm7, mm3
%endmacro

align 16
Skl_Sqr_8x8_MMX:

  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor mm7, mm7 ; accum
  pxor mm6, mm6 ; zero

  SQR_8x8_MMX
  SQR_8x8_MMX
  SQR_8x8_MMX
  SQR_8x8_MMX

  movq mm6, mm7
  psrlq mm7, 32
  paddd mm6, mm7
  movd eax, mm6
  shr eax,6
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Sqr_4x4_MMX
;//////////////////////////////////////////////////////////////////////

%macro SQR_4x4_MMX  0
  movq mm0, [eax]
  movq mm1, [eax+ecx]
  punpcklbw mm0, mm6
  punpcklbw mm1, mm6
  pmaddwd mm0, mm0
  pmaddwd mm1, mm1
  paddd mm7, mm0
  paddd mm7, mm1
%endmacro

align 16
Skl_Sqr_4x4_MMX:

  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor mm7, mm7 ; accum
  pxor mm6, mm6 ; zero

  SQR_4x4_MMX
  lea eax,[eax+2*ecx]
  SQR_4x4_MMX

  movq mm6, mm7
  psrlq mm7, 32
  paddd mm6, mm7
  movd eax, mm6
  shr eax,4
  ret

;//////////////////////////////////////////////////////////////////////
;//
;//  SSE impl
;//
;//////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////
; Skl_SAD_16x16_SSE
;//////////////////////////////////////////////////////////////////////

%macro SAD_16x16_SSE 0
  movq    mm0, [eax]
  psadbw  mm0, [edx]
  movq    mm1, [eax+8]
  lea eax, [eax+ecx]
  psadbw  mm1, [edx+8]
  add edx, ecx
  paddusw mm6,mm0
  paddusw mm6,mm1
%endmacro

align 16
Skl_SAD_16x16_SSE:  ; 104c
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS

  pxor mm7, mm7 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE

  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE

  movd eax, mm6
  ret

align 16
Skl_SAD_16x8_SSE:
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS

  pxor mm7, mm7 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE

  movd eax, mm6
  ret

align 16
Skl_SAD_16x8_Field_SSE:  ; 104c
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS
  lea ecx, [ecx+ecx]  ; 2.BpS

  pxor mm7, mm7 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE
  SAD_16x16_SSE

  movd eax, mm6
  ret


;//////////////////////////////////////////////////////////////////////
; Skl_SAD_Avrg_xxx_SSE
;//////////////////////////////////////////////////////////////////////

%macro SAD_16x16_AVRG_SSE 0
  movq    mm0, [edx]
  movq    mm1, [edx+8]
  pavgb   mm0, [ebx]
  pavgb   mm1, [ebx+8]
  psadbw  mm0, [eax]
  lea edx, [edx+ecx]
  psadbw  mm1, [eax+8]
  add eax, ecx
  paddusw mm6,mm0
  lea ebx, [ebx+ecx]
  paddusw mm6,mm1
%endmacro

%macro SAD_8x8_AVRG_SSE 0
  movq    mm0, [edx]
  movq    mm1, [edx+ecx]
  pavgb   mm0, [ebx]
  pavgb   mm1, [ebx+ecx]
  psadbw  mm0, [eax]
  lea edx, [edx+2*ecx]
  psadbw  mm1, [eax+ecx]
  lea eax, [eax+2*ecx]
  paddusw mm6,mm0
  lea ebx, [ebx+2*ecx]
  paddusw mm6,mm1
%endmacro

align 16
Skl_SAD_Avrg_16x16_SSE:
  mov eax, [esp+ 4] ; Dst
  mov edx, [esp+ 8] ; Src1
  mov ecx, [esp+16] ; BpS
  push ebx
  mov ebx, [esp+12+4] ; Src2

  pxor mm7, mm7 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE

  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE

  pop ebx
  movd eax, mm6
  ret

align 16
Skl_SAD_Avrg_16x8_SSE:  
  mov eax, [esp+ 4] ; Dst
  mov edx, [esp+ 8] ; Src1
  mov ecx, [esp+16] ; BpS
  push ebx
  mov ebx, [esp+12+4] ; Src2

  pxor mm7, mm7 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE
  SAD_16x16_AVRG_SSE

  pop ebx
  movd eax, mm6
  ret

align 16
Skl_SAD_Avrg_8x16_SSE:
  mov eax, [esp+ 4] ; Dst
  mov edx, [esp+ 8] ; Src1
  mov ecx, [esp+16] ; BpS
  push ebx
  mov ebx, [esp+12+4] ; Src2

  pxor mm7, mm7 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_8x8_AVRG_SSE
  SAD_8x8_AVRG_SSE
  SAD_8x8_AVRG_SSE
  SAD_8x8_AVRG_SSE
  SAD_8x8_AVRG_SSE
  SAD_8x8_AVRG_SSE
  SAD_8x8_AVRG_SSE
  SAD_8x8_AVRG_SSE

  pop ebx
  movd eax, mm6
  ret

align 16
Skl_SAD_Avrg_8x8_SSE:  
  mov eax, [esp+ 4] ; Dst
  mov edx, [esp+ 8] ; Src1
  mov ecx, [esp+16] ; BpS
  push ebx
  mov ebx, [esp+12+4] ; Src2

  pxor mm7, mm7 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_8x8_AVRG_SSE
  SAD_8x8_AVRG_SSE
  SAD_8x8_AVRG_SSE
  SAD_8x8_AVRG_SSE

  pop ebx
  movd eax, mm6
  ret



;//////////////////////////////////////////////////////////////////////
; Skl_SAD_16x7_Self_SSE
;//////////////////////////////////////////////////////////////////////

%macro SAD_16x7_SSE 0
  movq    mm0, [eax]
  psadbw  mm0, [edx]
  movq    mm1, [eax+8]
  lea eax, [eax+ecx]
  psadbw  mm1, [edx+8]
  add edx, ecx
  paddusw mm6,mm0
  paddusw mm6,mm1
%endmacro

align 16
Skl_SAD_16x7_Self_SSE:  ; 104c
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS
  lea edx, [eax+ecx]

  pxor mm7, mm7 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_16x7_SSE
  SAD_16x7_SSE
  SAD_16x7_SSE
  SAD_16x7_SSE
  SAD_16x7_SSE
  SAD_16x7_SSE
  SAD_16x7_SSE

  movd eax, mm6
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_SAD_8x8_SSE
;//////////////////////////////////////////////////////////////////////

%macro SAD_8x8_SSE 0
  movq    mm0, [eax]
  psadbw  mm0, [edx]
  movq    mm1, [eax+ecx]
  add eax, ebx
  psadbw  mm1, [edx+ecx]
  lea edx, [edx+ebx]
  paddusw mm6,mm0
  paddusw mm6,mm1
%endmacro

align 16
Skl_SAD_8x4_SSE:
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS
  push ebx
  lea ebx, [ecx+ecx]

  pxor mm5, mm5 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_8x8_SSE

  movq mm0, [eax]
  psadbw mm0, [edx]
  movq mm1, [eax+ecx]
  psadbw mm1, [edx+ecx]
  pop ebx
  paddusw mm6,mm0
  paddusw mm6,mm1

  movd eax, mm6
  ret

align 16
Skl_SAD_8x8_SSE:    ; 29c
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS
  push ebx
  lea ebx, [ecx+ecx]

  pxor mm5, mm5 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_8x8_SSE
  SAD_8x8_SSE
  SAD_8x8_SSE

  movq mm0, [eax]
  psadbw mm0, [edx]
  movq mm1, [eax+ecx]
  psadbw mm1, [edx+ecx]
  pop ebx
  paddusw mm6,mm0
  paddusw mm6,mm1

  movd eax, mm6
  ret

align 16
Skl_SAD_8x16_SSE:
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS
  push ebx
  lea ebx, [ecx+ecx]

  pxor mm5, mm5 ; this is a NOP
  pxor mm6, mm6 ; accum2

  SAD_8x8_SSE
  SAD_8x8_SSE
  SAD_8x8_SSE
  SAD_8x8_SSE

  SAD_8x8_SSE
  SAD_8x8_SSE
  SAD_8x8_SSE

  movq mm0, [eax]
  psadbw mm0, [edx]
  movq mm1, [eax+ecx]
  psadbw mm1, [edx+ecx]
  pop ebx
  paddusw mm6,mm0
  paddusw mm6,mm1

  movd eax, mm6
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_SAD_4x4_SSE
;//////////////////////////////////////////////////////////////////////

%macro SAD_4x4_SSE 0
  movd    mm0, [eax]
  movd    mm1, [edx]
  psadbw  mm0, mm1
  movd    mm1, [eax+ecx]
  add eax, ebx
  paddusw mm6,mm0
  movd    mm0, [edx+ecx]
  lea edx, [edx+ebx]
  psadbw  mm1, mm0
  paddusw mm6,mm1
%endmacro

align 16
Skl_SAD_4x4_SSE:    ; 
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS
  push ebx
  lea ebx, [ecx+ecx]

  pxor mm6, mm6 ; accum2
  pxor mm0, mm0
  pxor mm1, mm1

  SAD_4x4_SSE

  movd    mm0, [eax]
  movd    mm1, [edx]
  psadbw  mm0, mm1
  movd    mm1, [eax+ecx]
  pop ebx
  paddusw mm6,mm0
  movd    mm0, [edx+ecx]
  psadbw  mm1, mm0
  paddusw mm6,mm1

  movd eax, mm6
  ret

align 16
Skl_SAD_4x8_SSE:    ; 
  mov eax, [esp+ 4] ; Src1
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS
  push ebx
  lea ebx, [ecx+ecx]

  pxor mm6, mm6 ; accum2
  pxor mm0, mm0
  pxor mm1, mm1

  SAD_4x4_SSE
  SAD_4x4_SSE
  SAD_4x4_SSE

  movd    mm0, [eax]
  movd    mm1, [edx]
  psadbw  mm0, mm1
  movd    mm1, [eax+ecx]
  pop ebx
  paddusw mm6,mm0
  movd    mm0, [edx+ecx]
  psadbw  mm1, mm0
  paddusw mm6,mm1

  movd eax, mm6
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Mean_16x16_SSE
;//////////////////////////////////////////////////////////////////////

%macro MEAN_16x16_SSE 0
  movq    mm0, [eax]
  psadbw  mm0, mm7
  movq    mm1, [eax+8]
  psadbw  mm1, mm7
  paddusw  mm5, mm0
  add eax, ecx
  paddusw   mm6, mm1
%endmacro

align 16
Skl_Mean_16x16_SSE:   ; 97c
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor mm5, mm5 ; accum
  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE

  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE

  paddusw mm6,mm5
  movd    eax,mm6
  shr     eax, 8
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Mean_8x8_SSE
;//////////////////////////////////////////////////////////////////////

%macro MEAN_8x8_SSE  0
  movq mm0, [eax]
  movq mm1, [eax+ecx]
  psadbw mm0, mm7
  add eax, edx
  psadbw mm1, mm7
  paddw mm6, mm0
  paddw mm6, mm1
%endmacro

align 16
Skl_Mean_8x8_SSE: ; 34c
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero
  lea edx, [ecx+ecx]
  pxor mm6, mm6 ; this is a NOP

  MEAN_8x8_SSE
  MEAN_8x8_SSE
  MEAN_8x8_SSE
  MEAN_8x8_SSE

  COLLAPSE_MMX
  shr eax,6
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Mean_4x4_SSE
;//////////////////////////////////////////////////////////////////////

%macro MEAN_4x4_SSE  0
  movd mm0, [eax]
  movd mm1, [eax+ecx]
  psadbw mm0, mm7
  add eax, edx
  psadbw mm1, mm7
  paddw mm6, mm0
  paddw mm6, mm1
%endmacro

align 16
Skl_Mean_4x4_SSE:   ;
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero
  pxor mm0, mm0
  pxor mm1, mm1
  lea edx, [ecx+ecx]
  pxor mm6, mm6 ; this is a NOP

  MEAN_4x4_SSE
  MEAN_4x4_SSE

  COLLAPSE_4_MMX
  shr eax,4
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Abs_Dev_16x16_SSE
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Abs_Dev_16x16_SSE:    ; 191c
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor mm5, mm5 ; accum
  pxor mm6, mm6 ; accum
  pxor mm7, mm7 ; zero

  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE

  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE

  mov eax, [esp+ 4]   ; Src

  paddusw  mm5,mm6
  pxor     mm6, mm6     ; accum #1
  psrlw    mm5, 8       ; => Mean
  pshufw   mm7, mm5, 0  ; replicate Mean
  pxor     mm5, mm5     ; accum #2
  packuswb mm7,mm7

  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE

  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE
  MEAN_16x16_SSE

  paddusw mm6, mm5
  movd eax, mm6
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Sqr_Dev_16x16_SSE
;//////////////////////////////////////////////////////////////////////

%macro SQR_DEV_16x16_SSE 0
  movq   mm0, [eax]
  movq   mm2, mm0
  movq   mm1, [eax+8]
  movq   mm3, mm1
  psadbw mm0, mm7
  lea eax, [eax+ecx]
  psadbw mm1, mm7
  paddw  mm6, mm0
  movq   mm0, mm2
  paddw  mm6, mm1
  movq   mm1, mm3
  punpcklbw mm0, mm7
  punpcklbw mm1, mm7
  punpckhbw mm2, mm7
  punpckhbw mm3, mm7
  pmaddwd mm0, mm0
  pmaddwd mm1, mm1
  pmaddwd mm2, mm2
  pmaddwd mm3, mm3
  paddd mm5, mm0
  paddd mm5, mm1
  paddd mm5, mm2
  paddd mm5, mm3
%endmacro

align 16
Skl_Sqr_Dev_16x16_SSE:    ; 237c
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor mm5, mm5 ; accum for sqr
  pxor mm6, mm6 ; accum for mean
  pxor mm7, mm7 ; zero

  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE

  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE
  SQR_DEV_16x16_SSE

    ; we can't use a *signed* 'pmulhw mm6,mm6' here => pmaddw instead
  psrlq   mm6, 8  
  pmaddwd mm6, mm6
  movq    mm7, mm5
  psrlq   mm5, 32
  paddd   mm5, mm7
  psrld   mm5,8      ; (Sqr)>>8
  
  psubd mm5, mm6
  movd eax,mm5
  ret

;//////////////////////////////////////////////////////////////////////
;//
;//  SSE2 impl
;//
;//////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////
; Skl_SAD_16x16_SSE2
;//////////////////////////////////////////////////////////////////////

%macro SAD_16x16_SSE2 0
  movdqu  xmm0, [edx]
  movdqu  xmm1, [edx+ecx]
  lea edx,[edx+2*ecx]
  movdqa  xmm2, [eax]
  movdqa  xmm3, [eax+ecx]
  lea eax,[eax+2*ecx]
  psadbw  xmm0, xmm2
  paddusw xmm6,xmm0
  psadbw  xmm1, xmm3
  paddusw xmm6,xmm1
%endmacro

align 16
Skl_SAD_16x16_SSE2:
  mov eax, [esp+ 4] ; Src1 (assumed aligned)
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS

  pxor xmm6, xmm6 ; accum

  SAD_16x16_SSE2
  SAD_16x16_SSE2
  SAD_16x16_SSE2
  SAD_16x16_SSE2
  SAD_16x16_SSE2
  SAD_16x16_SSE2
  SAD_16x16_SSE2
  SAD_16x16_SSE2

  pshufd  xmm5, xmm6, 00000010b
  paddusw xmm6, xmm5
  pextrw  eax, xmm6, 0
  ret

align 16
Skl_SAD_16x8_Field_SSE2:
  mov eax, [esp+ 4] ; Src1 (assumed aligned)
  mov edx, [esp+ 8] ; Src2
  mov ecx, [esp+12] ; BpS
  lea ecx, [ecx+ecx]  ; 2.BpS

  pxor xmm6, xmm6 ; accum

  SAD_16x16_SSE2
  SAD_16x16_SSE2
  SAD_16x16_SSE2
  SAD_16x16_SSE2

  pshufd  xmm5, xmm6, 00000010b
  paddusw xmm6, xmm5
  pextrw  eax, xmm6, 0
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_SAD_16x7_Self_SSE2
;//////////////////////////////////////////////////////////////////////

%macro SAD_16x7_SSE2 0
  movdqa  xmm0, [eax]
  psadbw  xmm0, [edx]
  movdqa  xmm1, [eax+ecx]
  lea eax, [eax+2*ecx]
  psadbw  xmm1, [edx+ecx]
  paddusw xmm6,xmm0
  lea edx, [edx+2*ecx]
  paddusw xmm6,xmm1
%endmacro

align 16
Skl_SAD_16x7_Self_SSE2:
  mov eax, [esp+ 4] ; Src (assumed aligned)
  mov ecx, [esp+ 8] ; BpS
  lea edx, [eax+ecx]

  pxor xmm7, xmm7 ; this is a NOP
  pxor xmm6, xmm6 ; accum2

  SAD_16x7_SSE2
  SAD_16x7_SSE2
  SAD_16x7_SSE2
  movdqa  xmm0, [eax]
  psadbw  xmm0, [edx]
  paddusw xmm6,xmm0

  pshufd  xmm5, xmm6, 00000010b
  paddusw xmm6, xmm5
  pextrw  eax, xmm6, 0

  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Mean_16x16_SSE2
;//////////////////////////////////////////////////////////////////////

%macro MEAN_16x16_SSE2 0
  movdqu  xmm0, [eax]
  movdqu  xmm1, [eax+ecx]
  lea eax, [eax+2*ecx]    ; + 2*BpS
  psadbw  xmm0, xmm7
  paddusw xmm6, xmm0
  psadbw  xmm1, xmm7
  paddusw xmm6, xmm1
%endmacro

align 16
Skl_Mean_16x16_SSE2:
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor xmm6, xmm6 ; accum
  pxor xmm7, xmm7 ; zero

  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2

  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2

  pshufd  xmm5, xmm6, 10b
  paddusw xmm6, xmm5
  pextrw  eax, xmm6, 0
  shr eax, 8

  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Abs_Dev_16x16_SSE2
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Abs_Dev_16x16_SSE2:
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor xmm6, xmm6 ; accum
  pxor xmm7, xmm7 ; zero

  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2

  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2

  mov eax, [esp+ 4]   ; Src

  pshufd   xmm7, xmm6, 10b
  paddusw  xmm7, xmm6
  pxor     xmm6, xmm6     ; zero accum
  psrlw    xmm7, 8        ; => Mean
  pshuflw  xmm7, xmm7, 0  ; replicate Mean
  packuswb xmm7, xmm7
  pshufd   xmm7, xmm7, 00000000b

  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2

  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2
  MEAN_16x16_SSE2

  pshufd   xmm7, xmm6, 10b
  paddusw  xmm7, xmm6
  pextrw eax, xmm7, 0
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Sqr_16x16_SSE2
;//////////////////////////////////////////////////////////////////////

%macro SQR_16x16_SSE2 0
  movdqu xmm0, [eax]
  movdqu xmm1, [eax+ecx]
  lea eax,[eax+2*ecx]
  movdqa xmm2, xmm0
  movdqa xmm3, xmm1
  punpcklbw xmm0, xmm6
  punpcklbw xmm1, xmm6
  punpckhbw xmm2, xmm6
  punpckhbw xmm3, xmm6
  pmaddwd xmm0, xmm0
  pmaddwd xmm1, xmm1
  pmaddwd xmm2, xmm2
  pmaddwd xmm3, xmm3
  paddd   xmm7, xmm0
  paddd   xmm7, xmm1
  paddd   xmm7, xmm2
  paddd   xmm7, xmm3
%endmacro

align 16
Skl_Sqr_16x16_SSE2:   ; 287c
  mov eax, [esp+ 4] ; Src
  mov ecx, [esp+ 8] ; BpS

  pxor xmm7, xmm7 ; accum
  pxor xmm6, xmm6 ; zero

  SQR_16x16_SSE2
  SQR_16x16_SSE2
  SQR_16x16_SSE2
  SQR_16x16_SSE2

  SQR_16x16_SSE2
  SQR_16x16_SSE2
  SQR_16x16_SSE2
  SQR_16x16_SSE2

  pshufd xmm6, xmm7, 1110b
  paddd  xmm7, xmm6
  pshufd xmm6, xmm7, 01b
  paddd  xmm7, xmm6
  movd   eax, xmm7
  shr    eax, 8

  ret

;//////////////////////////////////////////////////////////////////////
