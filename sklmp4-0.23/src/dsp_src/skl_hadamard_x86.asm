;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
; [BITS 32]

%include "../../include/skl_syst/skl_nasm.h"

globl Skl_Hadamard_MMX
globl Skl_Hadamard_SSE

globl Skl_Hadamard_4x4_SSE

globl Skl_Hadamard_SAD_4x4_MMX
globl Skl_Hadamard_SAD_8x8_MMX
globl Skl_Hadamard_SAD_16x8_Field_MMX
globl Skl_Hadamard_SAD_16x16_MMX

;//////////////////////////////////////////////////////////////////////

DATA
One:     times 8  dw 1     ; for summing 4 words

TEXT

%macro BUTF2 4   ; a, b, c, d
  paddw %2, %1   ; a+b
  paddw %4, %3   ; c+d
  paddw %1, %1   ; 2a
  paddw %3, %3   ; 2c
  psubw %1, %2   ; a-b
  psubw %3, %4   ; c-d
%endmacro

;//////////////////////////////////////////////////////////////////////

%macro HADAMARD_HPASS 4   ; %1: offset  %2:reorder?, %3/%4:work reg
  movq   %3, [eax+%1]    ; [0123]

  movq   %4, %3
  movq   mm7, [eax+%1+8]  ; [4567]

  paddw %3, mm7      ; [abcd]
  psubw %4, mm7      ; [efgh]

  movq      mm7,%3
  punpcklwd %3,%4   ; [aebf]
  punpckhwd mm7,%4   ; [cgdh]

  movq  %4,%3
  paddw %3,mm7       ; [ABCD]
  psubw %4,mm7       ; [EFGH]

  movq      mm7,%3
  punpcklwd %3,%4   ; [ABEF]
  punpckhwd mm7,%4   ; [CDGH]

  movq  %4,%3
  paddw %3,mm7       ; [0312]
  psubw %4,mm7       ; [7465]

%if (%2==1)   ; SSE only

  pshufw %3,%3, 01111000b ; [0123]
  movq [eax+%1  ], %3
  pshufw %4,%4, 00101101b ; [4567]

%else
  movq [eax+%1  ], %3
%else

  movq [eax+%1+8], %4
%endif

%endmacro

%macro HADAMARD_VPASS 1   ; src/dst   ; 27c
  movq  mm0, [%1+0*16]
  movq  mm1, [%1+1*16]
  movq  mm2, [%1+2*16]
  movq  mm3, [%1+3*16]
  movq  mm4, [%1+4*16]
  movq  mm5, [%1+5*16]
  movq  mm6, [%1+6*16]
  movq  mm7, [%1+7*16]

  BUTF2  mm0, mm1,  mm2, mm3
  BUTF2  mm1, mm3,  mm0, mm2

  BUTF2  mm4, mm5,  mm6, mm7
  BUTF2  mm4, mm6,  mm5, mm7

  BUTF2  mm3, mm7,  mm0, mm4
  BUTF2  mm2, mm6,  mm1, mm5

  movq  [%1+0*16], mm7
  movq  [%1+1*16], mm3
  movq  [%1+2*16], mm1
  movq  [%1+3*16], mm5
  movq  [%1+4*16], mm4
  movq  [%1+5*16], mm0
  movq  [%1+6*16], mm2
  movq  [%1+7*16], mm6
%endmacro

;//////////////////////////////////////////////////////////////////////

  ; SSE: 135c  (131c without the pshufw reordering)
align 16
Skl_Hadamard_MMX:
  mov eax,[esp+4] ; In

  HADAMARD_HPASS 0*16, 0, mm0,mm1
  HADAMARD_HPASS 1*16, 0, mm0,mm1
  HADAMARD_HPASS 2*16, 0, mm0,mm1
  HADAMARD_HPASS 3*16, 0, mm0,mm1
  HADAMARD_HPASS 4*16, 0, mm0,mm1
  HADAMARD_HPASS 5*16, 0, mm0,mm1
  HADAMARD_HPASS 6*16, 0, mm0,mm1
  HADAMARD_HPASS 7*16, 0, mm0,mm1
  HADAMARD_VPASS eax
  HADAMARD_VPASS eax+8
  ret

align 16
Skl_Hadamard_SSE:
  mov eax,[esp+4] ; In
  HADAMARD_HPASS 0*16, 1, mm0,mm1
  HADAMARD_HPASS 1*16, 1, mm0,mm1
  HADAMARD_HPASS 2*16, 1, mm0,mm1
  HADAMARD_HPASS 3*16, 1, mm0,mm1
  HADAMARD_HPASS 4*16, 1, mm0,mm1
  HADAMARD_HPASS 5*16, 1, mm0,mm1
  HADAMARD_HPASS 6*16, 1, mm0,mm1
  HADAMARD_HPASS 7*16, 1, mm0,mm1
  HADAMARD_VPASS eax
  HADAMARD_VPASS eax+8
  ret

;//////////////////////////////////////////////////////////////////////
;// Hadamard SAD

%macro ADD_ABS 2   ; %1/%2:in reg
  pxor    mm7, mm7
  pcmpgtw mm7, %1
  pxor    mm5, mm5
  pcmpgtw mm5, %2
  psubw   mm6, mm7
  pxor    mm7, %1
  psubw   mm6, mm5
  paddw   mm6, mm7
  pxor    mm5, %2
  paddw   mm6, mm5
%endmacro

%macro HADAMARD_SAD_HPASS 3   ; %1:dst offset1 %2:dst offset2  %3:src offset [eax=cur,ecx=ref]

    ; first, upload 8b->16b the diff Src1[i]-Src2[i]

  movd   mm0, [eax+%3]    ; [0123]
  punpcklbw mm0, mm6
  movd   mm2, [ecx+%3]    ; [0123]
  punpcklbw mm2, mm6
  movd   mm1, [eax+%3+4]  ; [4567]
  movd   mm3, [ecx+%3+4]  ; [4567]

  punpcklbw mm1, mm6
  punpcklbw mm3, mm6

  psubw mm0, mm2
  psubw mm1, mm3

    ; now, go with the transform

  movq  mm7, mm0
  paddw mm0, mm1      ; [abcd]
  psubw mm7, mm1      ; [efgh]

  movq      mm1,mm0
  punpcklwd mm0,mm7   ; [aebf]
  punpckhwd mm1,mm7   ; [cgdh]

  movq  mm7,mm0
  paddw mm0,mm1       ; [ABCD]
  psubw mm7,mm1       ; [EFGH]

  movq      mm1,mm0
  punpcklwd mm0,mm7   ; [ABEF]
  punpckhwd mm1,mm7   ; [CDGH]

  movq  mm7,mm0
  paddw mm0,mm1       ; [0312]
  psubw mm7,mm1       ; [7465]

  movq [esp+%1], mm0
  movq [esp+%2], mm7
%endmacro

%macro HADAMARD_SAD_VPASS 2   ; %1:src/dst,  %2:SAD
  movq  mm0, [%1+0*16]
  movq  mm1, [%1+1*16]
  movq  mm2, [%1+2*16]
  movq  mm3, [%1+3*16]
  movq  mm4, [%1+4*16]
  movq  mm5, [%1+5*16]
  movq  mm6, [%1+6*16]
  movq  mm7, [%1+7*16]

  BUTF2  mm0, mm1,  mm2, mm3
  BUTF2  mm1, mm3,  mm0, mm2

  BUTF2  mm4, mm5,  mm6, mm7
  BUTF2  mm4, mm6,  mm5, mm7

  BUTF2  mm3, mm7,  mm0, mm4
  BUTF2  mm2, mm6,  mm1, mm5

    ; time to sum up the abs val of mm0..mm7
    ; -> make room for 3 regs

  movq  [esp+0*16], mm7   ; Spill
  movq  [esp+1*16], mm6   ; ...
  movq  [esp+2*16], mm5   ; ...

  movq mm6, %2
  ADD_ABS mm0,mm1
  ADD_ABS mm2,mm3
  movq  mm0, [esp+0*16]
  movq  mm1, [esp+1*16]
  movq  mm2, [esp+2*16]
  ADD_ABS mm0, mm1
  ADD_ABS mm2, mm4
%endmacro

;//////////////////////////////////////////////////////////////////////

%define LOCAL_TMP_SIZE    16*16
%define SAD esp+LOCAL_TMP_SIZE

align 16
Skl_Hadamard_SAD_8x8_MMX:  ; 221c
;SKL_RDTSC_IN
  mov eax,[esp+ 4] ; Src1
  mov ecx,[esp+ 8] ; Src2
  mov edx,[esp+12] ; BpS
  push ebp
  mov ebp, esp
  lea esp, [esp-LOCAL_TMP_SIZE-16]
  and esp, ~0xf    ; align to 16b

  pxor mm6, mm6
  movq [SAD], mm6

  HADAMARD_SAD_HPASS 0*16, 8*16, 0
  HADAMARD_SAD_HPASS 1*16, 9*16, edx
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS 2*16, 10*16, 0
  HADAMARD_SAD_HPASS 3*16, 11*16, edx
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS 4*16, 12*16, 0
  HADAMARD_SAD_HPASS 5*16, 13*16, edx
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS 6*16, 14*16, 0
  HADAMARD_SAD_HPASS 7*16, 15*16, edx

  HADAMARD_SAD_VPASS esp,      [SAD]
  movq [SAD], mm6    ; save intermediate SAD
  HADAMARD_SAD_VPASS esp+8*16, [SAD]

    ; mm6 = [SAD]. Now, collapse it.

  pmaddwd mm6, [One]
  movq    mm7, mm6
  psrlq   mm6, 32
  mov     esp, ebp
  paddd   mm6, mm7
  pop     ebp
  movd    eax, mm6
;SKL_RDTSC_OUT
  ret

%undef LOCAL_TMP_SIZE
%undef SAD

;//////////////////////////////////////////////////////////////////////

%define LOCAL_TMP_SIZE    64*16
%define SAD esp+LOCAL_TMP_SIZE
%define SAD2 SAD+8

align 16
Skl_Hadamard_SAD_16x16_MMX:  ; 831c
  mov eax,[esp+ 4] ; Src1
  mov ecx,[esp+ 8] ; Src2
  mov edx,[esp+12] ; BpS
  push ebp
  mov ebp, esp
  lea esp, [esp-LOCAL_TMP_SIZE-16]
  and esp, ~0xf    ; align to 16b

  pxor mm6, mm6
  movq [SAD], mm6

  HADAMARD_SAD_HPASS  0*16,  8*16, 0
  HADAMARD_SAD_HPASS 16*16, 24*16, 8
  HADAMARD_SAD_HPASS  1*16,  9*16, edx
  HADAMARD_SAD_HPASS 17*16, 25*16, edx+8
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS  2*16, 10*16, 0
  HADAMARD_SAD_HPASS 18*16, 26*16, 8
  HADAMARD_SAD_HPASS  3*16, 11*16, edx
  HADAMARD_SAD_HPASS 19*16, 27*16, edx+8
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS  4*16, 12*16, 0
  HADAMARD_SAD_HPASS 20*16, 28*16, 8
  HADAMARD_SAD_HPASS  5*16, 13*16, edx
  HADAMARD_SAD_HPASS 21*16, 29*16, edx+8
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS  6*16, 14*16, 0
  HADAMARD_SAD_HPASS 22*16, 30*16, 8
  HADAMARD_SAD_HPASS  7*16, 15*16, edx
  HADAMARD_SAD_HPASS 23*16, 31*16, edx+8
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS 32*16, 40*16, 0
  HADAMARD_SAD_HPASS 48*16, 56*16, 8
  HADAMARD_SAD_HPASS 33*16, 41*16, edx
  HADAMARD_SAD_HPASS 49*16, 57*16, edx+8
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS 34*16, 42*16, 0
  HADAMARD_SAD_HPASS 50*16, 58*16, 8
  HADAMARD_SAD_HPASS 35*16, 43*16, edx
  HADAMARD_SAD_HPASS 51*16, 59*16, edx+8
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS 36*16, 44*16, 0
  HADAMARD_SAD_HPASS 52*16, 60*16, 8
  HADAMARD_SAD_HPASS 37*16, 45*16, edx
  HADAMARD_SAD_HPASS 53*16, 61*16, edx+8
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS 38*16, 46*16, 0
  HADAMARD_SAD_HPASS 54*16, 62*16, 8
  HADAMARD_SAD_HPASS 39*16, 47*16, edx
  HADAMARD_SAD_HPASS 55*16, 63*16, edx+8

  HADAMARD_SAD_VPASS esp+ 0*16, [SAD]
  movq [SAD], mm6    ; save intermediate SAD
  HADAMARD_SAD_VPASS esp+ 8*16, [SAD]
  movq [SAD], mm6
  HADAMARD_SAD_VPASS esp+16*16, [SAD]
  movq [SAD], mm6
  HADAMARD_SAD_VPASS esp+24*16, [SAD]

    ; we need to split SAD accums in two, because of
    ; overflow... Store partially collapsed current SAD
  pmaddwd mm6, [One]
  pxor mm7, mm7
  movq [SAD], mm6

  movq [SAD2], mm7
  HADAMARD_SAD_VPASS esp+32*16, [SAD2]
  movq [SAD2], mm6
  HADAMARD_SAD_VPASS esp+40*16, [SAD2]
  movq [SAD2], mm6
  HADAMARD_SAD_VPASS esp+48*16, [SAD2]
  movq [SAD2], mm6
  HADAMARD_SAD_VPASS esp+56*16, [SAD2]

    ; mm6 = [SAD2]. Now, collapse it (with [SAD]).

  pmaddwd mm6, [One]
  paddd   mm6, [SAD]
  movq    mm7, mm6
  psrlq   mm6, 32
  mov     esp, ebp
  paddd   mm7, mm6
  pop     ebp
  movd    eax, mm7
  ret

align 16
Skl_Hadamard_SAD_16x8_Field_MMX:
  mov eax,[esp+ 4] ; Src1
  mov ecx,[esp+ 8] ; Src2
  mov edx,[esp+12] ; BpS
  lea edx, [edx+edx]  ; 2.BpS
  push ebp
  mov ebp, esp
  lea esp, [esp-LOCAL_TMP_SIZE-16]
  and esp, ~0xf    ; align to 16b

  pxor mm6, mm6
  movq [SAD], mm6

  HADAMARD_SAD_HPASS  0*16,  8*16, 0
  HADAMARD_SAD_HPASS 16*16, 24*16, 8
  HADAMARD_SAD_HPASS  1*16,  9*16, edx
  HADAMARD_SAD_HPASS 17*16, 25*16, edx+8
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS  2*16, 10*16, 0
  HADAMARD_SAD_HPASS 18*16, 26*16, 8
  HADAMARD_SAD_HPASS  3*16, 11*16, edx
  HADAMARD_SAD_HPASS 19*16, 27*16, edx+8
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS  4*16, 12*16, 0
  HADAMARD_SAD_HPASS 20*16, 28*16, 8
  HADAMARD_SAD_HPASS  5*16, 13*16, edx
  HADAMARD_SAD_HPASS 21*16, 29*16, edx+8
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD_SAD_HPASS  6*16, 14*16, 0
  HADAMARD_SAD_HPASS 22*16, 30*16, 8
  HADAMARD_SAD_HPASS  7*16, 15*16, edx
  HADAMARD_SAD_HPASS 23*16, 31*16, edx+8

  HADAMARD_SAD_VPASS esp+ 0*16, [SAD]
  movq [SAD], mm6    ; save intermediate SAD
  HADAMARD_SAD_VPASS esp+ 8*16, [SAD]
  movq [SAD], mm6
  HADAMARD_SAD_VPASS esp+16*16, [SAD]
  movq [SAD], mm6
  HADAMARD_SAD_VPASS esp+24*16, [SAD]

  pmaddwd mm6, [One]
  movq    mm7, mm6
  psrlq   mm6, 32
  mov     esp, ebp
  paddd   mm7, mm6
  pop     ebp
  movd    eax, mm7
  ret

%undef LOCAL_TMP_SIZE
%undef SAD

;//////////////////////////////////////////////////////////////////////
;// 4x4 Hadamard transforms + SAD (for H264)

%macro HADAMARD4_HPASS 4   ; %1: offset  %2:reorder?, %3/%4:work reg

  movq   %3, [eax+%1  ]    ; [abcd]
  movq   %4, [eax+%1+8]    ; [efgh]

  movq      mm7,%3
  punpcklwd %3,%4    ; [aebf]
  punpckhwd mm7,%4   ; [cgdh]

  movq  %4,%3
  paddw %3,mm7       ; [ABCD]
  psubw %4,mm7       ; [EFGH]

  movq      mm7,%3
  punpcklwd %3,%4    ; [ABEF]
  punpckhwd mm7,%4   ; [CDGH]

  movq  %4,%3
  paddw %3,mm7       ; [01]
  psubw %4,mm7       ; [32]

%if (%2!=0)
  pshufw mm7,%4, 10110001b ; [23]  ; SSE only
%endif

  movq %4, %3
  punpckldq %3,mm7   ; [ABEF]
  punpckhdq %4,mm7   ; [CDGH]

  movq mm7, %3      ; 1rst butterfly of the following vertical pass
  psubw %3,%4
  paddw %4,mm7
%endmacro

;//////////////////////////////////////////////////////////////////////

align 16
Skl_Hadamard_4x4_SSE: ; 30c
;SKL_RDTSC_IN
  mov eax,[esp+4] ; In

  HADAMARD4_HPASS  0, 1, mm0, mm1
  HADAMARD4_HPASS 16, 1, mm2, mm3

;  BUTF2  mm0, mm1,  mm2, mm3
  BUTF2  mm0, mm2,  mm1, mm3

  movq [eax   ], mm3
  movq [eax+ 8], mm1
  movq [eax+16], mm0
  movq [eax+24], mm2
;SKL_RDTSC_OUT
  ret

;//////////////////////////////////////////////////////////////////////

%macro ADD_ABS_4x4 2   ; %1/%2:in reg
  pxor    mm7, mm7
  pcmpgtw mm7, %1
  pxor    mm5, mm5
  pcmpgtw mm5, %2
  psubw   mm6, mm7
  pxor    mm7, %1
  psubw   mm6, mm5
  paddw   mm6, mm7
  pxor    mm5, %2
  paddw   mm6, mm5
%endmacro

%macro HADAMARD4_SAD_HPASS 2   ; %1/%2:work reg

  movd %1,  [eax    ]
  movd mm4, [ecx    ]
  movd %2,  [eax+edx]
  movd mm5, [ecx+edx]
  punpcklbw %1, mm6
  punpcklbw mm4,mm6
  punpcklbw %2, mm6
  punpcklbw mm5,mm6
  psubw %1, mm4
  psubw %2, mm5
  
  movq      mm7,%1
  punpcklwd %1,%2    ; [aebf]
  punpckhwd mm7,%2   ; [cgdh]

  movq  %2,%1
  paddw %1,mm7       ; [ABCD]
  psubw %2,mm7       ; [EFGH]

  movq      mm7,%1
  punpcklwd %1,%2    ; [ABEF]
  punpckhwd mm7,%2   ; [CDGH]

  movq  %2,%1
  paddw %1,mm7       ; [01]
  psubw %2,mm7       ; [32]

  movq mm7, %1
  punpckldq %1,%2   ; [ABEF]
  punpckhdq mm7,%2   ; [CDGH]

  movq %2,mm7
  paddw %2,%1
  psubw %1,mm7


%endmacro

align 16
Skl_Hadamard_SAD_4x4_MMX: ; 63c
;SKL_RDTSC_IN
  mov eax,[esp+ 4] ; Src1
  mov ecx,[esp+ 8] ; Src2
  mov edx,[esp+12] ; BpS

  pxor mm6, mm6

  HADAMARD4_SAD_HPASS mm0, mm1
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  HADAMARD4_SAD_HPASS mm2, mm3

  BUTF2  mm0, mm2,  mm1, mm3

  ADD_ABS_4x4 mm0, mm2
  ADD_ABS_4x4 mm1, mm3

  pmaddwd mm6, [One]
  movq    mm7, mm6
  psrlq   mm6, 32
  paddd   mm6, mm7
  movd    eax, mm6

;SKL_RDTSC_OUT
  ret

;//////////////////////////////////////////////////////////////////////
