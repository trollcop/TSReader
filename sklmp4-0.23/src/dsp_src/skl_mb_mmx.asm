;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
;//////////////////////////////////////////////////////////////////////
;// Macro-block processing
;//////////////////////////////////////////////////////////////////////
; [BITS 32]

%include "../../include/skl_syst/skl_nasm.h"

globl Skl_Add_8x4_FF_MMX
globl Skl_Add_8x4_FH_Rnd0_MMX
globl Skl_Add_8x4_HF_Rnd0_MMX
globl Skl_Add_8x4_HH_Rnd0_MMX
globl Skl_Add_8x8_FF_MMX
globl Skl_Add_8x8_FH_Rnd0_MMX
globl Skl_Add_8x8_HF_Rnd0_MMX
globl Skl_Add_8x8_HH_Rnd0_MMX
globl Skl_Add_16x8_FF_MMX
globl Skl_Add_16x8_FH_Rnd0_MMX
globl Skl_Add_16x8_HF_Rnd0_MMX
globl Skl_Add_16x8_HH_Rnd0_MMX
globl Skl_Copy_8x4_FF_MMX
globl Skl_Copy_8x4_FH_Rnd1_MMX
globl Skl_Copy_8x4_HF_Rnd1_MMX
globl Skl_Copy_8x4_HH_Rnd1_MMX
globl Skl_Copy_8x4_FH_Rnd0_MMX
globl Skl_Copy_8x4_HF_Rnd0_MMX
globl Skl_Copy_8x4_HH_Rnd0_MMX
globl Skl_Copy_8x8_FF_MMX
globl Skl_Copy_8x8_FH_Rnd1_MMX
globl Skl_Copy_8x8_HF_Rnd1_MMX
globl Skl_Copy_8x8_HH_Rnd1_MMX
globl Skl_Copy_8x8_FH_Rnd0_MMX
globl Skl_Copy_8x8_HF_Rnd0_MMX
globl Skl_Copy_8x8_HH_Rnd0_MMX
globl Skl_Copy_16x8_FF_MMX
globl Skl_Copy_16x8_FH_Rnd1_MMX
globl Skl_Copy_16x8_HF_Rnd1_MMX
globl Skl_Copy_16x8_HH_Rnd1_MMX
globl Skl_Copy_16x8_FH_Rnd0_MMX
globl Skl_Copy_16x8_HF_Rnd0_MMX
globl Skl_Copy_16x8_HH_Rnd0_MMX

globl Skl_H_Pass_2Taps_MMX
globl Skl_V_Pass_2Taps_MMX
globl Skl_HV_Pass_2Taps_MMX

globl Skl_Copy_16x8_8To16_MMX
globl Skl_Copy_8x8_8To16_MMX
globl Skl_Diff_16x8_8To16_MMX
globl Skl_Diff_8x8_8To16_MMX
globl Skl_Diff_16x8_88To16_MMX
globl Skl_Diff_8x8_88To16_MMX

globl Skl_Move_16x8_MMX
globl Skl_Move_8x8_MMX

globl Skl_Copy_Upsampled_8x8_16To8_MMX
globl Skl_Add_Upsampled_8x8_16To8_MMX
globl Skl_HFilter_31_MMX
globl Skl_VFilter_31_x86
globl Skl_HFilter_31_x86

globl Skl_Filter_18x18_To_8x8_MMX
globl Skl_Filter_Diff_18x18_To_8x8_MMX

;//////////////////////////////////////////////////////////////////////

DATA

align 16
Rounder2_MMX times 4 dw 2
Rounder1_MMX times 4 dw 1
Rounder0_MMX times 4 dw 0

Up31 dw  3, 1, 3, 1
Up13 dw  1, 3, 1, 3
Cst0 dw  0, 0, 0, 0
Cst2 dw  2, 2, 2, 2
Cst3 dw  3, 3, 3, 3
Cst32 dw 32,32,32,32
Cst2000 dw  2, 0, 0, 0
Cst0002 dw  0, 0, 0, 2

Mask_ff dw 0xff,0xff,0xff,0xff

TEXT

;//////////////////////////////////////////////////////////////////////
;//
;//   Half-pixel interpolation functions
;//
;//////////////////////////////////////////////////////////////////////

%macro PROLOG0 0
  mov ecx, [esp+ 4] ; Dst
  mov eax, [esp+ 8] ; Src
  mov edx, [esp+12] ; BpS
%endmacro

%macro PROLOG 2   ; %1: Rounder, %2 load Dst-Rounder
  pxor mm6, mm6
  movq mm7, [%1]    ; TODO: dangerous! (eax isn't checked)
%if %2
  movq mm5, [Rounder1_MMX]
%endif

  PROLOG0
%endmacro

  ; performs: mm0 == (mm0+mm2)  mm1 == (mm1+mm3)
%macro MIX 0
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm0, mm2
  paddusw mm1, mm3
%endmacro

%macro MIX_DST 0
  movq mm3, mm2
  paddusw mm0, mm7  ; rounder
  paddusw mm1, mm7  ; rounder
  punpcklbw mm2, mm6
  punpckhbw mm3, mm6
  psrlw mm0, 1
  psrlw mm1, 1

  paddusw mm0, mm2  ; mix Src(mm0/mm1) with Dst(mm2/mm3)
  paddusw mm1, mm3
  paddusw mm0, mm5
  paddusw mm1, mm5
  psrlw mm0, 1
  psrlw mm1, 1

  packuswb mm0, mm1
%endmacro

%macro MIX2 0
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm0, mm2
  paddusw mm0, mm7
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm1, mm7
  paddusw mm1, mm3
  psrlw mm0, 1
  psrlw mm1, 1

  packuswb mm0, mm1
%endmacro

;//////////////////////////////////////////////////////////////////////
;// Add functions
;//////////////////////////////////////////////////////////////////////

%macro ADD_FF_MMX 1
  movq mm0, [eax]
  movq mm2, [ecx]
  movq mm1, mm0
  movq mm3, mm2
%if (%1!=0)
  lea eax,[eax+%1*edx]
%endif
  MIX
  paddusw mm0, mm5  ; rounder
  paddusw mm1, mm5  ; rounder
  psrlw mm0, 1
  psrlw mm1, 1

  packuswb mm0, mm1
  movq [ecx], mm0
%if (%1!=0)
  lea ecx,[ecx+%1*edx]
%endif
%endmacro

align 16
Skl_Add_8x4_FF_MMX:
  PROLOG Rounder1_MMX, 1
  ADD_FF_MMX 1
  ADD_FF_MMX 1
  ADD_FF_MMX 1
  ADD_FF_MMX 0
  ret

align 16
Skl_Add_8x8_FF_MMX:
  PROLOG Rounder1_MMX, 1
  ADD_FF_MMX 1
  ADD_FF_MMX 1
  ADD_FF_MMX 1
  ADD_FF_MMX 1
  ADD_FF_MMX 1
  ADD_FF_MMX 1
  ADD_FF_MMX 1
  ADD_FF_MMX 0
  ret


%macro ADD_16x8_FF_MMX 0
  movq mm0, [eax]
  movq mm2, [ecx]
  movq mm1, mm0
  movq mm3, mm2

  MIX
  paddusw mm0, mm5  ; dst-rounder
  paddusw mm1, mm5  ; dst-rounder
  psrlw mm0, 1
  psrlw mm1, 1

  packuswb mm0, mm1
  movq [ecx], mm0

  movq mm0, [eax+8]
  movq mm2, [ecx+8]
  movq mm1, mm0
  movq mm3, mm2

  lea eax,[eax+edx]

  MIX
  paddusw mm0, mm5  ; dst-rounder
  paddusw mm1, mm5  ; dst-rounder
  psrlw mm0, 1
  psrlw mm1, 1

  packuswb mm0, mm1
  movq [ecx+8], mm0
%endmacro

align 16
Skl_Add_16x8_FF_MMX:
  PROLOG Rounder1_MMX, 1

  ADD_16x8_FF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FF_MMX

  ret

;//////////////////////////////////////////////////////////////////////

%macro ADD_FH_MMX 0
  movq mm0, [eax]
  movq mm2, [eax+1]
  movq mm1, mm0
  movq mm3, mm2

  lea eax,[eax+edx]

  MIX
  movq mm2, [ecx]   ; prepare mix with Dst[0]
  MIX_DST
  movq [ecx], mm0
%endmacro

align 16
Skl_Add_8x4_FH_Rnd0_MMX:
  PROLOG Rounder1_MMX, 1

  ADD_FH_MMX
  lea ecx,[ecx+edx]
  ADD_FH_MMX
  lea ecx,[ecx+edx]
  ADD_FH_MMX
  lea ecx,[ecx+edx]
  ADD_FH_MMX

  ret

align 16
Skl_Add_8x8_FH_Rnd0_MMX:
  PROLOG Rounder1_MMX, 1

  ADD_FH_MMX
  lea ecx,[ecx+edx]
  ADD_FH_MMX
  lea ecx,[ecx+edx]
  ADD_FH_MMX
  lea ecx,[ecx+edx]
  ADD_FH_MMX
  lea ecx,[ecx+edx]
  ADD_FH_MMX
  lea ecx,[ecx+edx]
  ADD_FH_MMX
  lea ecx,[ecx+edx]
  ADD_FH_MMX
  lea ecx,[ecx+edx]
  ADD_FH_MMX
  ret

%macro ADD_16x8_FH_MMX 0
  movq mm0, [eax]
  movq mm2, [eax+1]
  movq mm1, mm0
  movq mm3, mm2

  MIX
  movq mm2, [ecx]   ; prepare mix with Dst[0]
  MIX_DST
  movq [ecx], mm0

  movq mm0, [eax+8]
  movq mm2, [eax+9]
  movq mm1, mm0
  movq mm3, mm2

  lea eax,[eax+edx]

  MIX
  movq mm2, [ecx+8]   ; prepare mix with Dst[0]
  MIX_DST
  movq [ecx+8], mm0
%endmacro

align 16
Skl_Add_16x8_FH_Rnd0_MMX:
  PROLOG Rounder1_MMX, 1

  ADD_16x8_FH_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FH_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FH_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FH_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FH_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FH_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FH_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_FH_MMX
  ret

;//////////////////////////////////////////////////////////////////////

%macro ADD_HF_MMX 0
  movq mm0, [eax]
  movq mm2, [eax+edx]
  movq mm1, mm0
  movq mm3, mm2

  lea eax,[eax+edx]

  MIX
  movq mm2, [ecx]   ; prepare mix with Dst[0]
  MIX_DST
  movq [ecx], mm0

%endmacro

align 16
Skl_Add_8x4_HF_Rnd0_MMX:
  PROLOG Rounder1_MMX, 1

  ADD_HF_MMX
  lea ecx,[ecx+edx]
  ADD_HF_MMX
  lea ecx,[ecx+edx]
  ADD_HF_MMX
  lea ecx,[ecx+edx]
  ADD_HF_MMX
  ret

align 16
Skl_Add_8x8_HF_Rnd0_MMX:
  PROLOG Rounder1_MMX, 1

  ADD_HF_MMX
  lea ecx,[ecx+edx]
  ADD_HF_MMX
  lea ecx,[ecx+edx]
  ADD_HF_MMX
  lea ecx,[ecx+edx]
  ADD_HF_MMX
  lea ecx,[ecx+edx]
  ADD_HF_MMX
  lea ecx,[ecx+edx]
  ADD_HF_MMX
  lea ecx,[ecx+edx]
  ADD_HF_MMX
  lea ecx,[ecx+edx]
  ADD_HF_MMX
  ret

%macro ADD_16x8_HF_MMX 0
  movq mm0, [eax]
  movq mm2, [eax+edx]
  movq mm1, mm0
  movq mm3, mm2

  MIX
  movq mm2, [ecx]   ; prepare mix with Dst[0]
  MIX_DST
  movq [ecx], mm0

  movq mm0, [eax+8]
  movq mm2, [eax+edx+8]
  movq mm1, mm0
  movq mm3, mm2

  lea eax,[eax+edx]

  MIX
  movq mm2, [ecx+8]   ; prepare mix with Dst[0]
  MIX_DST
  movq [ecx+8], mm0
%endmacro

align 16
Skl_Add_16x8_HF_Rnd0_MMX:
  PROLOG Rounder1_MMX, 1

  ADD_16x8_HF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_HF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_HF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_HF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_HF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_HF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_HF_MMX
  lea ecx,[ecx+edx]
  ADD_16x8_HF_MMX
  ret

;//////////////////////////////////////////////////////////////////////

%macro ADD_HH_MMX 0
  lea eax,[eax+edx]

    ; transfert prev line to mm0/mm1
  movq mm0, mm2
  movq mm1, mm3

    ; load new line in mm2/mm3
  movq mm2, [eax]
  movq mm4, [eax+1]
  movq mm3, mm2
  movq mm5, mm4

  punpcklbw mm2, mm6
  punpcklbw mm4, mm6
  paddusw mm2, mm4
  punpckhbw mm3, mm6
  punpckhbw mm5, mm6
  paddusw mm3, mm5

    ; mix current line (mm2/mm3) with previous (mm0,mm1); 
    ; we'll preserve mm2/mm3 for next line...

  paddusw mm0, mm2  
  paddusw mm1, mm3  

  movq mm4, [ecx]   ; prepare mix with Dst[0]
  movq mm5, mm4

  paddusw mm0, mm7  ; finish mixing current line
  paddusw mm1, mm7

  punpcklbw mm4, mm6
  punpckhbw mm5, mm6

  psrlw mm0, 2
  psrlw mm1, 2

  paddusw mm0, mm4  ; mix Src(mm0/mm1) with Dst(mm2/mm3)
  paddusw mm1, mm5

  paddusw mm0, [Rounder1_MMX]
  paddusw mm1, [Rounder1_MMX]

  psrlw mm0, 1
  psrlw mm1, 1

  packuswb mm0, mm1

  movq [ecx], mm0
%endmacro

align 16
Skl_Add_8x4_HH_Rnd0_MMX:
  PROLOG Rounder2_MMX, 0    ; mm5 is busy. Don't load dst-rounder

    ; preprocess first line
  movq mm0, [eax]
  movq mm2, [eax+1]
  movq mm1, mm0
  movq mm3, mm2

  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm2, mm0
  paddusw mm3, mm1

   ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line

  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX

  ret


align 16
Skl_Add_8x8_HH_Rnd0_MMX:
  PROLOG Rounder2_MMX, 0    ; mm5 is busy. Don't load dst-rounder

    ; preprocess first line
  movq mm0, [eax]
  movq mm2, [eax+1]
  movq mm1, mm0
  movq mm3, mm2

  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm2, mm0
  paddusw mm3, mm1

   ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line

  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX

  ret

align 16
Skl_Add_16x8_HH_Rnd0_MMX:
  PROLOG Rounder2_MMX, 0

    ; preprocess first line
  movq mm0, [eax]
  movq mm2, [eax+1]
  movq mm1, mm0
  movq mm3, mm2

  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm2, mm0
  paddusw mm3, mm1

   ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line

  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX

    ; second column
  mov ecx, [esp+ 4] ; Dst
  mov eax, [esp+ 8] ; Src
  lea ecx, [ecx+8]
  lea eax, [eax+8]

    ; preprocess first line
  movq mm0, [eax]
  movq mm2, [eax+1]
  movq mm1, mm0
  movq mm3, mm2

  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm2, mm0
  paddusw mm3, mm1

   ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line

  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX
  lea ecx,[ecx+edx]
  ADD_HH_MMX

  ret

;//////////////////////////////////////////////////////////////////////
;// Copy functions
;//////////////////////////////////////////////////////////////////////

%macro COPY_FF_8 1      ; %1:phase
    movq mm0,  [eax]
    movq mm1,  [eax+edx]
    movq [ecx], mm0
%if (%1!=1)
    lea eax, [eax+2*edx]
%endif
    movq [ecx+edx], mm1
%if (%1!=1)
    lea ecx, [ecx+2*edx]
%endif
%endmacro

align 16
Skl_Copy_8x4_FF_MMX:  ; 9c
  PROLOG0
  movq mm0,  [eax      ]
  movq mm1,  [eax+edx  ]
  movq mm2,  [eax+2*edx]
  movq [ecx      ], mm0
  movq [ecx+edx  ], mm1
  movq [ecx+2*edx], mm2
  lea edx, [edx+2*edx]
  movq mm1,  [eax+edx]
  movq [ecx+edx], mm1
  ret


align 16
Skl_Copy_8x8_FF_MMX:  ; 14c
  PROLOG0
  COPY_FF_8 0
  COPY_FF_8 0
  COPY_FF_8 0
  COPY_FF_8 1
  ret

%macro COPY_FF_16 1
  movq mm0, [eax]
  movq mm1, [eax+8]
  movq mm2, [eax+edx]
  movq mm3, [eax+edx+8]
  movq [ecx], mm0
  movq [ecx+8], mm1
  movq [ecx+edx], mm2
%if (%1!=1)
  lea eax, [eax+2*edx]
%endif
  movq [ecx+edx+8], mm3
%if (%1!=1)
  lea ecx, [ecx+2*edx]
%endif
%endmacro

align 16
Skl_Copy_16x8_FF_MMX: ; 26c
  PROLOG0
  COPY_FF_16 0
  COPY_FF_16 0
  COPY_FF_16 0
  COPY_FF_16 1
  ret

;//////////////////////////////////////////////////////////////////////

%macro COPY_FH_MMX 0
  movq mm0, [eax]
  movq mm2, [eax+1]
  movq mm1, mm0
  movq mm3, mm2

  lea eax,[eax+edx]

  MIX2
  movq [ecx], mm0
%endmacro

align 16
Skl_Copy_8x4_FH_Rnd0_MMX:
  PROLOG Rounder1_MMX, 0

  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  ret

align 16
Skl_Copy_8x4_FH_Rnd1_MMX:
  PROLOG Rounder0_MMX, 0

  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  ret

align 16
Skl_Copy_8x8_FH_Rnd0_MMX:
  PROLOG Rounder1_MMX, 0

  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  ret

align 16
Skl_Copy_8x8_FH_Rnd1_MMX:
  PROLOG Rounder0_MMX, 0

  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  lea ecx,[ecx+edx]
  COPY_FH_MMX
  ret

%macro COPY_16x8_FH_MMX 0
  movq mm0, [eax]
  movq mm2, [eax+1]
  movq mm1, mm0
  movq mm3, mm2

  MIX2
  movq [ecx], mm0

  movq mm0, [eax+8]
  movq mm2, [eax+9]
  movq mm1, mm0
  movq mm3, mm2

  lea eax,[eax+edx]

  MIX2
  movq [ecx+8], mm0
%endmacro

align 16
Skl_Copy_16x8_FH_Rnd0_MMX:
  PROLOG Rounder1_MMX, 0

  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  ret

align 16
Skl_Copy_16x8_FH_Rnd1_MMX:
  PROLOG Rounder0_MMX, 0

  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  ret

align 16
Skl_H_Pass_2Taps_MMX:
Skl_Copy_16x16_FH_Rnd0_MMX:
  PROLOG Rounder1_MMX, 0

  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_FH_MMX
  ret


;//////////////////////////////////////////////////////////////////////

%macro COPY_HF_MMX 0
  movq mm0, [eax]
  movq mm2, [eax+edx]
  movq mm1, mm0
  movq mm3, mm2

  lea eax,[eax+edx]

  MIX2
  movq [ecx], mm0
%endmacro

align 16
Skl_Copy_8x4_HF_Rnd0_MMX:
  PROLOG Rounder1_MMX, 0

  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  ret

align 16
Skl_Copy_8x4_HF_Rnd1_MMX:
  PROLOG Rounder0_MMX, 0

  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  ret

align 16
Skl_Copy_8x8_HF_Rnd0_MMX:
  PROLOG Rounder1_MMX, 0

  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  ret

align 16
Skl_Copy_8x8_HF_Rnd1_MMX:
  PROLOG Rounder0_MMX, 0

  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  lea ecx,[ecx+edx]
  COPY_HF_MMX
  ret

%macro COPY_16x8_HF_MMX 0
  movq mm0, [eax]
  movq mm2, [eax+edx]
  movq mm1, mm0
  movq mm3, mm2

  MIX2
  movq [ecx], mm0

  movq mm0, [eax+8]
  movq mm2, [eax+edx+8]
  movq mm1, mm0
  movq mm3, mm2

  lea eax,[eax+edx]

  MIX2
  movq [ecx+8], mm0
%endmacro

align 16
Skl_Copy_16x8_HF_Rnd0_MMX:
  PROLOG Rounder1_MMX, 0

  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  ret

align 16
Skl_Copy_16x8_HF_Rnd1_MMX:
  PROLOG Rounder0_MMX, 0

  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  ret

align 16
Skl_V_Pass_2Taps_MMX:
Skl_Copy_16x16_HF_Rnd0_MMX:
  PROLOG Rounder1_MMX, 0

  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  lea ecx,[ecx+edx]
  COPY_16x8_HF_MMX
  ret

;//////////////////////////////////////////////////////////////////////

%macro COPY_HH_MMX 0
  lea eax,[eax+edx]

    ; transfert prev line to mm0/mm1
  movq mm0, mm2
  movq mm1, mm3

    ; load new line in mm2/mm3
  movq mm2, [eax]
  movq mm4, [eax+1]
  movq mm3, mm2
  movq mm5, mm4

  punpcklbw mm2, mm6
  paddusw mm0, mm7    ; rounder
  punpcklbw mm4, mm6
  paddusw mm1, mm7    ; rounder
  punpckhbw mm3, mm6
  paddusw mm2, mm4
  punpckhbw mm5, mm6
  paddusw mm0, mm2
  paddusw mm3, mm5
  psrlw mm0, 2
  paddusw mm1, mm3
  psrlw mm1, 2

  packuswb mm0, mm1
  movq [ecx], mm0
%endmacro

align 16
Skl_Copy_8x4_HH_Rnd0_MMX:
  PROLOG Rounder2_MMX, 0

  ; preprocess first line
  movq mm0, [eax]
  movq mm1, mm0
  movq mm2, [eax+1]
  movq mm3, mm2
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm2, mm0
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm3, mm1

    ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  ret

align 16
Skl_Copy_8x4_HH_Rnd1_MMX:
  PROLOG Rounder1_MMX, 0

  ; preprocess first line
  movq mm0, [eax]
  movq mm1, mm0
  movq mm2, [eax+1]
  movq mm3, mm2
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm2, mm0
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm3, mm1

    ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  ret

align 16
Skl_Copy_8x8_HH_Rnd0_MMX:
  PROLOG Rounder2_MMX, 0

  ; preprocess first line
  movq mm0, [eax]
  movq mm1, mm0
  movq mm2, [eax+1]
  movq mm3, mm2
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm2, mm0
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm3, mm1

    ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  ret

align 16
Skl_Copy_8x8_HH_Rnd1_MMX:
  PROLOG Rounder1_MMX, 0

  ; preprocess first line
  movq mm0, [eax]
  movq mm1, mm0
  movq mm2, [eax+1]
  movq mm3, mm2
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm2, mm0
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm3, mm1

    ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  ret


align 16
Skl_Copy_16x8_HH_Rnd0_MMX:
  PROLOG Rounder2_MMX, 0

  ; preprocess first line
  movq mm0, [eax]
  movq mm1, mm0
  movq mm2, [eax+1]
  movq mm3, mm2
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm2, mm0
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm3, mm1

   ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX


    ; second column
  mov ecx, [esp+ 4] ; Dst
  mov eax, [esp+ 8] ; Src
  lea ecx, [ecx+8]
  lea eax, [eax+8]

  ; preprocess first line
  movq mm0, [eax]
  movq mm1, mm0
  movq mm2, [eax+1]
  movq mm3, mm2
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm2, mm0
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm3, mm1

   ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX

  ret

align 16
Skl_Copy_16x8_HH_Rnd1_MMX:
  PROLOG Rounder1_MMX, 0

  ; preprocess first line
  movq mm0, [eax]
  movq mm1, mm0
  movq mm2, [eax+1]
  movq mm3, mm2
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm2, mm0
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm3, mm1

   ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX


    ; second column
  mov ecx, [esp+ 4] ; Dst
  mov eax, [esp+ 8] ; Src
  lea ecx, [ecx+8]
  lea eax, [eax+8]

  ; preprocess first line
  movq mm0, [eax]
  movq mm1, mm0
  movq mm2, [eax+1]
  movq mm3, mm2
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm2, mm0
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm3, mm1

   ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX

  ret

align 16
Skl_HV_Pass_2Taps_MMX:
Skl_Copy_16x16_HH_Rnd0_MMX:
  PROLOG Rounder2_MMX, 0

  ; preprocess first line
  movq mm0, [eax]
  movq mm1, mm0
  movq mm2, [eax+1]
  movq mm3, mm2
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm2, mm0
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm3, mm1

   ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX

    ; second column
  mov ecx, [esp+ 4] ; Dst
  mov eax, [esp+ 8] ; Src
  lea ecx, [ecx+8]
  lea eax, [eax+8]

  ; preprocess first line
  movq mm0, [eax]
  movq mm1, mm0
  movq mm2, [eax+1]
  movq mm3, mm2
  punpcklbw mm0, mm6
  punpcklbw mm2, mm6
  paddusw mm2, mm0
  punpckhbw mm1, mm6
  punpckhbw mm3, mm6
  paddusw mm3, mm1

   ; Input: mm2/mm3 contains the value (Src[0]+Src[1]) of previous line
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX
  lea ecx,[ecx+edx]
  COPY_HH_MMX

  ret

;//////////////////////////////////////////////////////////////////////
;//
;// 8b to 16b transfer ops
;//
;//////////////////////////////////////////////////////////////////////

%macro UPLOAD 2
  movq mm0, [eax+%2]
  movq mm2, mm0
  movq mm1, [eax+%2+edx]
  movq mm3, mm1
  punpcklbw mm0, mm7
  punpckhbw mm2, mm7  
  movq [ecx+%1*32], mm0  
  movq [ecx+%1*32+8], mm2  
  punpcklbw mm1, mm7
  punpckhbw mm3, mm7
  movq [ecx+%1*32+16], mm1
  movq [ecx+%1*32+24], mm3
%endmacro

align 16
Skl_Copy_16x8_8To16_MMX:   ;
  PROLOG0
  pxor mm7, mm7

  UPLOAD 0, 0
  UPLOAD 4, 8
  lea eax,[eax+2*edx]
  UPLOAD 1, 0
  UPLOAD 5, 8
  lea eax,[eax+2*edx]
  UPLOAD 2, 0
  UPLOAD 6, 8
  lea eax,[eax+2*edx]
  UPLOAD 3, 0
  UPLOAD 7, 8

  ret

align 16
Skl_Copy_8x8_8To16_MMX:   ; 31c
  PROLOG0
  pxor mm7, mm7

  UPLOAD 0, 0
  lea eax,[eax+2*edx]
  UPLOAD 1, 0
  lea eax,[eax+2*edx]
  UPLOAD 2, 0
  lea eax,[eax+2*edx]
  UPLOAD 3, 0

  ret

;//////////////////////////////////////////////////////////////////////
;//
;// Diffs (8b->16b)
;//
;//////////////////////////////////////////////////////////////////////

%macro DIFF 2
  movq mm0, [eax+%2]    ; Src
  movq mm1, mm0
  movq mm2, [eax+%2+edx]
  movq mm3, mm2
  punpcklbw mm0, mm7
  punpcklbw mm2, mm7
  movq mm4, [ecx+%1*32+ 0] ; Dst
  punpckhbw mm1, mm7
  punpckhbw mm3, mm7
  movq mm5, [ecx+%1*32+ 8] ; Dst2
  psubw mm4, mm0
  psubw mm5, mm1
  movq [ecx+%1*32+ 0], mm4
  movq [ecx+%1*32+ 8], mm5

  movq mm0, [ecx+%1*32+16] ; Dst
  movq mm1, [ecx+%1*32+24] ; Dst2
  psubw mm0, mm2
  psubw mm1, mm3
  movq [ecx+%1*32+16], mm0
  movq [ecx+%1*32+24], mm1
%endmacro

align 16
Skl_Diff_16x8_8To16_MMX:   ;  89c
  PROLOG0
  pxor mm7, mm7

  DIFF 0, 0
  DIFF 4, 8
  lea eax,[eax+2*edx]
  DIFF 1, 0
  DIFF 5, 8
  lea eax,[eax+2*edx]
  DIFF 2, 0
  DIFF 6, 8
  lea eax,[eax+2*edx]
  DIFF 3, 0
  DIFF 7, 8
  ret

align 16
Skl_Diff_8x8_8To16_MMX:   ; 46c
  PROLOG0
  pxor mm7, mm7

  DIFF 0, 0
  lea eax,[eax+2*edx]
  DIFF 1, 0
  lea eax,[eax+2*edx]
  DIFF 2, 0
  lea eax,[eax+2*edx]
  DIFF 3, 0
  ret

;//////////////////////////////////////////////////////////////////////

%macro DIFF2 2
  movq mm0, [eax+%2]     ; Src1
  movq mm1, mm0
  movq mm2, [eax+%2+edx]
  movq mm3, mm2
  punpcklbw mm0, mm7
  punpcklbw mm2, mm7
  movq mm4, [ebx+%2]     ; Src2  
  punpckhbw mm1, mm7
  punpckhbw mm3, mm7  
  movq mm6, [ebx+%2+edx] ; Src2
  movq mm5, mm4
  punpcklbw mm4, mm7  
  punpckhbw mm5, mm7
  psubw mm0, mm4
  psubw mm1, mm5
  movq mm4, mm6
  movq [ecx+%1*32+ 0], mm0
  punpcklbw mm6, mm7
  punpckhbw mm4, mm7  
  movq [ecx+%1*32+ 8], mm1
  psubw mm2, mm6
  psubw mm3, mm4
  movq [ecx+%1*32+16], mm2
  movq [ecx+%1*32+24], mm3
%endmacro

align 16
Skl_Diff_16x8_88To16_MMX:   ; 101c
;SKL_RDTSC_IN

  push ebx
  mov ecx, [esp+ 4 +4] ; Dst
  mov eax, [esp+ 8 +4] ; Src1
  mov ebx, [esp+12 +4] ; Src2
  mov edx, [esp+16 +4] ; BpS
  pxor mm7, mm7

  DIFF2 0, 0
  DIFF2 4, 8
  lea eax,[eax+2*edx]
  lea ebx,[ebx+2*edx]
  DIFF2 1, 0
  DIFF2 5, 8
  lea eax,[eax+2*edx]
  lea ebx,[ebx+2*edx]
  DIFF2 2, 0
  DIFF2 6, 8
  lea eax,[eax+2*edx]
  lea ebx,[ebx+2*edx]
  DIFF2 3, 0
  DIFF2 7, 8

  pop ebx
;SKL_RDTSC_OUT
  ret

align 16
Skl_Diff_8x8_88To16_MMX:   ;56
  push ebx
  mov ecx, [esp+ 4 +4] ; Dst
  mov eax, [esp+ 8 +4] ; Src1
  mov ebx, [esp+12 +4] ; Src2
  mov edx, [esp+16 +4] ; BpS
  pxor mm7, mm7

  DIFF2 0, 0
  lea eax,[eax+2*edx]
  lea ebx,[ebx+2*edx]
  DIFF2 1, 0
  lea eax,[eax+2*edx]
  lea ebx,[ebx+2*edx]
  DIFF2 2, 0
  lea eax,[eax+2*edx]
  lea ebx,[ebx+2*edx]
  DIFF2 3, 0

  pop ebx
  ret

;//////////////////////////////////////////////////////////////////////
;//
;// 8x8 -> 16x16 upsampling (16b)
;//
;//////////////////////////////////////////////////////////////////////

%macro MUL_PACK 4     ; %1/%2: regs   %3/%4: Up13/Up31
  pmullw %1,  %3 ; [Up13]
  pmullw mm4, %4 ; [Up31]
  pmullw %2,  %3 ; [Up13]
  pmullw mm5, %4 ; [Up31]
  paddsw %1, [Cst2]
  paddsw %2, [Cst2]
  paddsw %1, mm4
  paddsw %2, mm5
%endmacro

%macro COL03 3    ;%1/%2: regs, %3: row   -output: mm4/mm5
  movq %1, [edx+%3*16+0*2]   ; %1  = 0|1|2|3
  movq %2,[edx+%3*16+1*2]    ; %2  = 1|2|3|4
  movq mm5, %1               ; mm5 = 0|1|2|3
  movq mm4, %1               ; mm4 = 0|1|2|3
  punpckhwd mm5,%2           ; mm5 = 2|3|3|4
  punpcklwd mm4,%2           ; mm4 = 0|1|1|2
  punpcklwd %1,%1            ; %1  = 0|0|1|1
  punpcklwd %2, mm5          ; %2  = 1|2|2|3
  punpcklwd %1, mm4          ; %1  = 0|0|0|1
%endmacro

%macro COL47 3    ;%1-%2: regs, %3: row   -output: mm4/mm5
  movq mm5, [edx+%3*16+4*2]   ; mm5 = 4|5|6|7
  movq %1, [edx+%3*16+3*2]    ; %1  = 3|4|5|6
  movq %2,  mm5               ; %2  = 4|5|6|7
  movq mm4, mm5               ; mm4 = 4|5|6|7
  punpckhwd %2, %2            ; %2  = 6|6|7|7  
  punpckhwd mm5, %2           ; mm5 = 6|7|7|7
  movq %2,  %1                ; %2  = 3|4|5|6  
  punpcklwd %1, mm4           ; %1  = 3|4|4|5
  punpckhwd %2, mm4           ; %2  = 5|6|6|7
  punpcklwd mm4, %2           ; mm4 = 4|5|5|6
%endmacro

%macro MIX_ROWS 4   ; %1/%2:prev %3/4:cur (preserved)  mm4/mm5: output
  ; we need to perform: (%1,%3) -> (%1 = 3*%1+%3, mm4 = 3*%3+%1), %3 preserved.
  movq mm4, [Cst3]
  movq mm5, [Cst3]
  pmullw mm4, %3
  pmullw mm5, %4
  paddsw mm4, %1
  paddsw mm5, %2
  pmullw %1, [Cst3]
  pmullw %2, [Cst3]
  paddsw %1, %3
  paddsw %2, %4
%endmacro

;//////////////////////////////////////////////////////////////////////

  ; Note: we can use ">>2" instead of "/4" here, since we 
  ; are (supposed to be) averaging positive values

%macro STORE_1 2
  psraw %1, 2
  psraw %2, 2
  packuswb %1,%2   
  movq [ecx], %1
%endmacro

%macro STORE_2 2    ; pack and store (%1,%2) + (mm4,mm5)
  psraw %1, 4
  psraw %2, 4
  psraw mm4, 4
  psraw mm5, 4
  packuswb %1,%2
  packuswb mm4, mm5
  movq [ecx], %1
  movq [ecx+eax], mm4
  lea ecx, [ecx+2*eax]
%endmacro

align 16
Skl_Copy_Upsampled_8x8_16To8_MMX:  ; 344c

  mov ecx, [esp+4]  ; Dst
  mov edx, [esp+8]  ; Src
  mov eax, [esp+12] ; BpS

  movq mm6, [Up13]
  movq mm7, [Up31]

  COL03 mm0, mm1, 0
  MUL_PACK mm0,mm1, mm6, mm7
  movq mm4, mm0
  movq mm5, mm1
  STORE_1 mm4, mm5
  add ecx, eax

  COL03 mm2, mm3, 1
  MUL_PACK mm2,mm3, mm6, mm7
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_2 mm0, mm1

  COL03 mm0, mm1, 2
  MUL_PACK mm0,mm1, mm6, mm7
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_2 mm2, mm3

  COL03 mm2, mm3, 3
  MUL_PACK mm2,mm3, mm6, mm7
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_2 mm0, mm1

  COL03 mm0, mm1, 4
  MUL_PACK mm0,mm1, mm6, mm7
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_2 mm2, mm3

  COL03 mm2, mm3, 5
  MUL_PACK mm2,mm3, mm6, mm7
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_2 mm0, mm1

  COL03 mm0, mm1, 6
  MUL_PACK mm0,mm1, mm6, mm7
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_2 mm2, mm3

  COL03 mm2, mm3, 7
  MUL_PACK mm2,mm3, mm6, mm7
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_2 mm0, mm1

  STORE_1 mm2, mm3

  mov ecx, [esp+4]
  add ecx, 8

  COL47 mm0, mm1, 0
  MUL_PACK mm0,mm1, mm6, mm7
  movq mm4, mm0
  movq mm5, mm1
  STORE_1 mm4, mm5
  add ecx, eax

  COL47 mm2, mm3, 1
  MUL_PACK mm2,mm3, mm6, mm7
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_2 mm0, mm1

  COL47 mm0, mm1, 2
  MUL_PACK mm0,mm1, mm6, mm7
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_2 mm2, mm3

  COL47 mm2, mm3, 3
  MUL_PACK mm2,mm3, mm6, mm7
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_2 mm0, mm1

  COL47 mm0, mm1, 4
  MUL_PACK mm0,mm1, mm6, mm7
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_2 mm2, mm3

  COL47 mm2, mm3, 5
  MUL_PACK mm2,mm3, mm6, mm7
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_2 mm0, mm1

  COL47 mm0, mm1, 6
  MUL_PACK mm0,mm1, mm6, mm7
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_2 mm2, mm3

  COL47 mm2, mm3, 7
  MUL_PACK mm2,mm3, mm6, mm7
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_2 mm0, mm1

  STORE_1 mm2, mm3

  ret

;//////////////////////////////////////////////////////////////////////

    ; Note: grrr... the 'pcmpgtw' stuff are the "/4" and "/16" operators
    ; implemented with ">>2" and ">>4" using: 
    ;       x/4  = ( (x-(x<0))>>2 ) + (x<0)
    ;       x/16 = ( (x-(x<0))>>4 ) + (x<0)

%macro STORE_ADD_1 2
    ; We substract the rounder '2' for corner pixels,
    ; since when 'x' is negative, (x*4 + 2)/4 is *not*
    ; equal to 'x'. In fact, the correct relation is:
    ;         (x*4 + 2)/4 = x - (x<0)
    ; So, better revert to (x*4)/4 = x.

  psubsw %1, [Cst2000]
  psubsw %2, [Cst0002]
  pxor mm6, mm6
  pxor mm7, mm7
  pcmpgtw mm6, %1
  pcmpgtw mm7, %2
  paddsw %1, mm6
  paddsw %2, mm7
  psraw %1, 2
  psraw %2, 2
  psubsw %1, mm6
  psubsw %2, mm7

    ; mix with destination [ecx]
  movq mm6, [ecx]
  movq mm7, [ecx]
  punpcklbw mm6, [Cst0]
  punpckhbw mm7, [Cst0]
  paddsw %1, mm6
  paddsw %2, mm7
  packuswb %1,%2
  movq [ecx], %1
%endmacro

%macro STORE_ADD_2 2
  pxor mm6, mm6
  pxor mm7, mm7
  pcmpgtw mm6, %1
  pcmpgtw mm7, %2
  paddsw %1, mm6
  paddsw %2, mm7
  psraw %1, 4
  psraw %2, 4
  psubsw %1, mm6
  psubsw %2, mm7

  pxor mm6, mm6
  pxor mm7, mm7
  pcmpgtw mm6, mm4
  pcmpgtw mm7, mm5
  paddsw mm4, mm6
  paddsw mm5, mm7
  psraw mm4, 4
  psraw mm5, 4
  psubsw mm4, mm6
  psubsw mm5, mm7

    ; mix with destination
  movq mm6, [ecx]
  movq mm7, [ecx]
  punpcklbw mm6, [Cst0]
  punpckhbw mm7, [Cst0]
  paddsw %1, mm6
  paddsw %2, mm7

  movq mm6, [ecx+eax]
  movq mm7, [ecx+eax]

  punpcklbw mm6, [Cst0]
  punpckhbw mm7, [Cst0]
  paddsw mm4, mm6
  paddsw mm5, mm7

  packuswb %1,%2
  packuswb mm4, mm5

  movq [ecx], %1
  movq [ecx+eax], mm4

  lea ecx, [ecx+2*eax]
%endmacro

align 16
Skl_Add_Upsampled_8x8_16To8_MMX:  ; 579c

  mov ecx, [esp+4]  ; Dst
  mov edx, [esp+8]  ; Src
  mov eax, [esp+12] ; BpS

  COL03 mm0, mm1, 0
  MUL_PACK mm0,mm1, [Up13], [Up31]  
  movq mm4, mm0
  movq mm5, mm1
  STORE_ADD_1 mm4, mm5
  add ecx, eax

  COL03 mm2, mm3, 1
  MUL_PACK mm2,mm3, [Up13], [Up31]
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_ADD_2 mm0, mm1

  COL03 mm0, mm1, 2
  MUL_PACK mm0,mm1, [Up13], [Up31]
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_ADD_2 mm2, mm3

  COL03 mm2, mm3, 3
  MUL_PACK mm2,mm3, [Up13], [Up31]
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_ADD_2 mm0, mm1

  COL03 mm0, mm1, 4
  MUL_PACK mm0,mm1, [Up13], [Up31]
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_ADD_2 mm2, mm3

  COL03 mm2, mm3, 5
  MUL_PACK mm2,mm3, [Up13], [Up31]
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_ADD_2 mm0, mm1

  COL03 mm0, mm1, 6
  MUL_PACK mm0,mm1, [Up13], [Up31]
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_ADD_2 mm2, mm3

  COL03 mm2, mm3, 7
  MUL_PACK mm2,mm3, [Up13], [Up31]
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_ADD_2 mm0, mm1

  STORE_ADD_1 mm2, mm3


  mov ecx, [esp+4]
  add ecx, 8

  COL47 mm0, mm1, 0
  MUL_PACK mm0,mm1, [Up13], [Up31]  
  movq mm4, mm0
  movq mm5, mm1  
  STORE_ADD_1 mm4, mm5
  add ecx, eax

  COL47 mm2, mm3, 1
  MUL_PACK mm2,mm3, [Up13], [Up31]
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_ADD_2 mm0, mm1

  COL47 mm0, mm1, 2
  MUL_PACK mm0,mm1, [Up13], [Up31]
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_ADD_2 mm2, mm3

  COL47 mm2, mm3, 3
  MUL_PACK mm2,mm3, [Up13], [Up31]
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_ADD_2 mm0, mm1

  COL47 mm0, mm1, 4
  MUL_PACK mm0,mm1, [Up13], [Up31]
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_ADD_2 mm2, mm3

  COL47 mm2, mm3, 5
  MUL_PACK mm2,mm3, [Up13], [Up31]
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_ADD_2 mm0, mm1

  COL47 mm0, mm1, 6
  MUL_PACK mm0,mm1, [Up13], [Up31]
  MIX_ROWS mm2, mm3, mm0, mm1
  STORE_ADD_2 mm2, mm3

  COL47 mm2, mm3, 7
  MUL_PACK mm2,mm3, [Up13], [Up31]
  MIX_ROWS mm0, mm1, mm2, mm3
  STORE_ADD_2 mm0, mm1

  STORE_ADD_1 mm2, mm3

  ret

;//////////////////////////////////////////////////////////////////////
;//
;// horizontal/vertical filtering: [x,y] -> [ (3x+y+2)>>2, (x+3y+2)>>2]
;// We use the trick: tmp = (x+y+2) -> [x = (tmp+2x)>>2, y = (tmp+2y)>>2]
;//
;//////////////////////////////////////////////////////////////////////

align 16
Skl_HFilter_31_MMX:
  push esi
  push edi
  mov esi, [esp+4  +8]  ; Src1
  mov edi, [esp+8  +8]  ; Src2
  mov eax, [esp+12 +8] ; Nb_Blks
  lea eax,[eax*2]
  movq mm5, [Cst2]
  pxor mm7, mm7

  lea esi, [esi+eax*4]
  lea edi, [edi+eax*4]

  neg eax

.Loop:  ;12c
  movd mm0, [esi+eax*4]  
  movd mm1, [edi+eax*4]
  movq mm2, mm5  
  punpcklbw mm0, mm7
  punpcklbw mm1, mm7
  paddsw mm2, mm0
  paddsw mm0, mm0
  paddsw mm2, mm1
  paddsw mm1, mm1
  paddsw mm0, mm2
  paddsw mm1, mm2
  psraw mm0, 2
  psraw mm1, 2
  packuswb mm0, mm7
  packuswb mm1, mm7
  movd [esi+eax*4], mm0
  movd [edi+eax*4], mm1
  add eax,1
  jl .Loop

  pop edi
  pop esi
  ret

  ; MMX is of no use here. Better use plain ASM. Moreover,
  ; this is for the fun of ASM coding, coz' every modern compiler can
  ; end up with a code that looks very much like this one...

align 16
Skl_VFilter_31_x86:
  push esi
  push edi
  push ebx
  push ebp
  mov esi, [esp+4  +16]  ; Src1
  mov edi, [esp+8  +16]  ; Src2
  mov ebp, [esp+12 +16]  ; BpS
  mov eax, [esp+16 +16]  ; Nb_Blks
  lea eax,[eax*8]

.Loop:  ;7c
  movzx ecx, byte [esi]
  movzx edx, byte [edi]

  lea ebx, [ecx+edx+2]
  lea ecx,[ebx+2*ecx]
  lea edx,[ebx+2*edx]

  shr ecx,2
  shr edx,2
  mov [esi], cl
  mov [edi], dl
  lea esi, [esi+ebp]
  lea edi, [edi+ebp]
  dec eax
  jg .Loop

  pop ebp
  pop ebx
  pop edi
  pop esi
  ret

  ; this one's just a little faster than gcc's code. Very little.

align 16
Skl_HFilter_31_x86:
  push esi
  push edi
  push ebx
  mov esi, [esp+4  +12]  ; Src1
  mov edi, [esp+8  +12]  ; Src2
  mov eax, [esp+12 +12]  ; Nb_Blks

  lea eax,[eax*8]
  lea esi, [esi+eax]
  lea edi, [esi+eax]
  neg eax

.Loop:  ; 6c
  movzx ecx, byte [esi+eax]
  movzx edx, byte [edi+eax]

  lea ebx, [ecx+edx+2]
  lea ecx,[ebx+2*ecx]
  lea edx,[ebx+2*edx]
  shr ecx,2
  shr edx,2
  mov [esi+eax], cl
  mov [edi+eax], dl
  inc eax

  jl .Loop

  pop ebx
  pop edi
  pop esi
  ret

;//////////////////////////////////////////////////////////////////////
;//
;// 16b downsampling 16x16 -> 8x8
;//
;//////////////////////////////////////////////////////////////////////

%macro HFILTER_1331 2  ;%1:src %2:dst reg. -trashes mm0/mm1/mm2
  movq mm2, [Mask_ff]
  movq %2,  [%1-1]    ;-10123456
  movq mm0, [%1]      ; 01234567
  movq mm1, [%1+1]    ; 12345678
  pand  %2, mm2       ;-1|1|3|5
  pand mm0, mm2       ; 0|2|4|6
  pand mm1, mm2       ; 1|3|5|7
  pand mm2, [%1+2]    ; 2|4|6|8
  paddusw mm0, mm1
  paddusw %2, mm2
  pmullw mm0,  mm7
  paddusw %2, mm0
%endmacro

%macro VFILTER_1331 4  ; %1-4: regs  %1-%2: trashed
  paddsw %1, [Cst32]
  paddsw %2, %3    
  pmullw %2, mm7
  paddsw %1, %4
  paddsw %1, %2
  psraw %1, 6
%endmacro

;//////////////////////////////////////////////////////////////////////

%macro COPY_TWO_LINES_1331 1     ; %1: dst
  HFILTER_1331 edx    , mm5
  HFILTER_1331 edx+eax, mm6
  lea edx, [edx+2*eax]
  VFILTER_1331 mm3,mm4,mm5, mm6
  movq [%1], mm3

  HFILTER_1331 edx    , mm3
  HFILTER_1331 edx+eax, mm4
  lea edx, [edx+2*eax]
  VFILTER_1331 mm5,mm6,mm3,mm4
  movq [%1+16], mm5
%endmacro

align 16
Skl_Filter_18x18_To_8x8_MMX:  ; 283c   (~4.4c per output pixel)

  mov ecx, [esp+4]  ; Dst
  mov edx, [esp+8]  ; Src
  mov eax, [esp+12] ; BpS

  movq mm7, [Cst3]
  sub edx, eax

    ; mm3/mm4/mm5/mm6 is used as a 4-samples delay line.

      ; process columns 0-3

  HFILTER_1331 edx    , mm3   ; pre-load mm3/mm4
  HFILTER_1331 edx+eax, mm4
  lea edx, [edx+2*eax]

  COPY_TWO_LINES_1331 ecx + 0*16
  COPY_TWO_LINES_1331 ecx + 2*16
  COPY_TWO_LINES_1331 ecx + 4*16
  COPY_TWO_LINES_1331 ecx + 6*16

      ; process columns 4-7

  mov edx, [esp+8]
  sub edx, eax
  add edx, 8

  HFILTER_1331 edx    , mm3   ; pre-load mm3/mm4
  HFILTER_1331 edx+eax, mm4
  lea edx, [edx+2*eax]

  COPY_TWO_LINES_1331 ecx + 0*16 +8
  COPY_TWO_LINES_1331 ecx + 2*16 +8
  COPY_TWO_LINES_1331 ecx + 4*16 +8
  COPY_TWO_LINES_1331 ecx + 6*16 +8
  ret

;//////////////////////////////////////////////////////////////////////

%macro DIFF_TWO_LINES_1331 1     ; %1: dst
  HFILTER_1331 edx    , mm5
  HFILTER_1331 edx+eax, mm6
  lea edx, [edx+2*eax]
  movq mm2, [%1]
  VFILTER_1331 mm3,mm4,mm5, mm6
  psubsw mm2, mm3
  movq [%1], mm2

  HFILTER_1331 edx    , mm3
  HFILTER_1331 edx+eax, mm4
  lea edx, [edx+2*eax]
  movq mm2, [%1+16]
  VFILTER_1331 mm5,mm6,mm3,mm4
  psubsw mm2, mm5
  movq [%1+16], mm2
%endmacro

align 16
Skl_Filter_Diff_18x18_To_8x8_MMX:  ; 302c
;SKL_RDTSC_IN
  mov ecx, [esp+4]  ; Dst
  mov edx, [esp+8]  ; Src
  mov eax, [esp+12] ; BpS

  movq mm7, [Cst3]
  sub edx, eax

    ; mm3/mm4/mm5/mm6 is used as a 4-samples delay line.

      ; process columns 0-3

  HFILTER_1331 edx    , mm3   ; pre-load mm3/mm4
  HFILTER_1331 edx+eax, mm4
  lea edx, [edx+2*eax]

  DIFF_TWO_LINES_1331 ecx + 0*16
  DIFF_TWO_LINES_1331 ecx + 2*16
  DIFF_TWO_LINES_1331 ecx + 4*16
  DIFF_TWO_LINES_1331 ecx + 6*16

      ; process columns 4-7

  mov edx, [esp+8]
  sub edx, eax
  add edx, 8

  HFILTER_1331 edx    , mm3   ; pre-load mm3/mm4
  HFILTER_1331 edx+eax, mm4
  lea edx, [edx+2*eax]

  DIFF_TWO_LINES_1331 ecx + 0*16 +8
  DIFF_TWO_LINES_1331 ecx + 2*16 +8
  DIFF_TWO_LINES_1331 ecx + 4*16 +8
  DIFF_TWO_LINES_1331 ecx + 6*16 +8
;SKL_RDTSC_OUT
  ret

;//////////////////////////////////////////////////////////////////////
