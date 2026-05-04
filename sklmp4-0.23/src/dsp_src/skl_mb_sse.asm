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

globl Skl_Add_8x4_FF_SSE
globl Skl_Add_8x4_FH_Rnd0_SSE
globl Skl_Add_8x4_HF_Rnd0_SSE
globl Skl_Add_8x4_HH_Rnd0_SSE
globl Skl_Add_8x8_FF_SSE
globl Skl_Add_8x8_FH_Rnd0_SSE
globl Skl_Add_8x8_HF_Rnd0_SSE
globl Skl_Add_8x8_HH_Rnd0_SSE
globl Skl_Add_16x8_FF_SSE
globl Skl_Add_16x8_FH_Rnd0_SSE
globl Skl_Add_16x8_HF_Rnd0_SSE
globl Skl_Add_16x8_HH_Rnd0_SSE
globl Skl_Copy_8x4_FH_Rnd1_SSE
globl Skl_Copy_8x4_HF_Rnd1_SSE
globl Skl_Copy_8x4_HH_Rnd1_SSE
globl Skl_Copy_8x4_FH_Rnd0_SSE
globl Skl_Copy_8x4_HF_Rnd0_SSE
globl Skl_Copy_8x4_HH_Rnd0_SSE
globl Skl_Copy_8x8_FH_Rnd1_SSE
globl Skl_Copy_8x8_HF_Rnd1_SSE
globl Skl_Copy_8x8_HH_Rnd1_SSE
globl Skl_Copy_8x8_FH_Rnd0_SSE
globl Skl_Copy_8x8_HF_Rnd0_SSE
globl Skl_Copy_8x8_HH_Rnd0_SSE
globl Skl_Copy_16x8_FH_Rnd1_SSE
globl Skl_Copy_16x8_HF_Rnd1_SSE
globl Skl_Copy_16x8_HH_Rnd1_SSE
globl Skl_Copy_16x8_FH_Rnd0_SSE
globl Skl_Copy_16x8_HF_Rnd0_SSE
globl Skl_Copy_16x8_HH_Rnd0_SSE

globl Skl_H_Pass_2Taps_SSE
globl Skl_V_Pass_2Taps_SSE
globl Skl_HV_Pass_2Taps_SSE

globl Skl_Copy_Upsampled_8x8_16To8_SSE
globl Skl_Add_Upsampled_8x8_16To8_SSE

;//////////////////////////////////////////////////////////////////////

DATA

align 16
Mask    times 8 db 1

Up31 dw  3, 1, 3, 1
Up13 dw  1, 3, 1, 3
Cst2 dw  2, 2, 2, 2
Cst3 dw  3, 3, 3, 3
Cst0 dw  0, 0, 0, 0
Cst2000 dw  2, 0, 0, 0
Cst0002 dw  0, 0, 0, 2

TEXT

;//////////////////////////////////////////////////////////////////////
;//
;//   Half-pixel interpolation functions
;//
;// all funcs are signed: (uint8_t *Dst, const uint8_t *Src,
;//                        const uint32_t BpS, const uint32_t Rounding)
;// even if 'rounding' is not used (Copy_FF/Add_FF)
;//
;//////////////////////////////////////////////////////////////////////

%macro PROLOG0 0
  mov ecx, [esp+ 4] ; Dst
  mov eax, [esp+ 8] ; Src
  mov edx, [esp+12] ; BpS
%endmacro

%macro PROLOG1 0
  mov ecx, [esp+ 4] ; Dst
  mov eax, [esp+ 8] ; Src
  mov edx, [esp+12] ; BpS
  push ebx
  lea ebx, [edx+2*edx]
%endmacro

;//////////////////////////////////////////////////////////////////////
; Full-Full funcs
;//////////////////////////////////////////////////////////////////////

%macro ADD_FF 5   ;%1-%4:offsets, %5:phase
  movq  mm0, [eax+%1]
  pavgb mm0, [ecx+%1]
  movq  mm1, [eax+%2]
  pavgb mm1, [ecx+%2]
  movq  mm2, [eax+%3]
  pavgb mm2, [ecx+%3]
  movq  mm3, [eax+%4]
  pavgb mm3, [ecx+%4]
  movq [ecx+%1], mm0
  movq [ecx+%2], mm1
  movq [ecx+%3], mm2
  movq [ecx+%4], mm3
%if (%5!=0)
  lea eax,[eax+%5*edx]
  lea ecx,[ecx+%5*edx]
%endif
%endmacro

align 16
Skl_Add_8x4_FF_SSE:
  PROLOG1
  ADD_FF 0, edx, 2*edx, ebx, 0
  pop ebx
  ret

align 16
Skl_Add_8x8_FF_SSE:  ; 22c
  PROLOG1
  ADD_FF 0, edx, 2*edx, ebx, 4
  ADD_FF 0, edx, 2*edx, ebx, 0
  pop ebx
  ret

align 16
Skl_Add_16x8_FF_SSE:  ; 42c
  PROLOG0
  ADD_FF 0, 8, edx, edx+8, 2
  ADD_FF 0, 8, edx, edx+8, 2
  ADD_FF 0, 8, edx, edx+8, 2
  ADD_FF 0, 8, edx, edx+8, 0
  ret

;//////////////////////////////////////////////////////////////////////
; Full-Half funcs
;//////////////////////////////////////////////////////////////////////

%macro COPY_ADD_FH_RND0 6   ; %1-%4=offsets  %5: =0->COPY, =1->AVRG %6:mult
  movq  mm0, [eax+%1]
  movq  mm1, [eax+%2]
  movq  mm2, [eax+%3]
  movq  mm3, [eax+%4]
  pavgb mm0, [eax+%1+1]
  pavgb mm1, [eax+%2+1]
  pavgb mm2, [eax+%3+1]
  pavgb mm3, [eax+%4+1]
%if (%5!=0)
  pavgb mm0, [ecx+%1]
  pavgb mm1, [ecx+%2]
  pavgb mm2, [ecx+%3]
  pavgb mm3, [ecx+%4]
%endif
  movq  [ecx+%1], mm0
  movq  [ecx+%2], mm1
  movq  [ecx+%3], mm2
  movq  [ecx+%4], mm3
%if (%6!=0)
  lea eax,[eax+%6*edx]
  lea ecx,[ecx+%6*edx]
%endif
%endmacro

%macro COPY_ADD_FH_RND1 6   ; we use: (i+j)/2 = ( i+j+1 )/2 - (i^j)&1
  movq  mm0, [eax+%1]
  movq  mm1, [eax+%2]
  movq  mm2, [eax+%1+1]
  movq  mm3, [eax+%2+1]

  movq  mm4, mm0
  movq  mm5, mm1
  pavgb mm0, mm2
  pxor  mm2, mm4
  pavgb mm1, mm3
  pxor  mm3, mm5

  pand  mm2, mm7   ; Mask
  pand  mm3, mm7
  psubb mm0, mm2
  psubb mm1, mm3
%if (%5!=0)
  pavgb mm0, [ecx+%1]
  pavgb mm1, [ecx+%2]
%endif
  movq [ecx+%1],mm0
  movq [ecx+%2],mm1

  movq  mm0, [eax+%3]
  movq  mm1, [eax+%4]
  movq  mm4, mm0
  movq  mm5, mm1
  movq  mm2, [eax+%3+1]
  movq  mm3, [eax+%4+1]

  pavgb mm0, mm2
  pxor  mm2, mm4
  pavgb mm1, mm3
  pxor  mm3, mm5

  pand  mm2, mm7
  pand  mm3, mm7
  psubb mm0, mm2
  psubb mm1, mm3
%if (%5!=0)
  pavgb mm0, [ecx+%3]
  pavgb mm1, [ecx+%4]
%endif
  movq [ecx+%3],mm0
  movq [ecx+%4],mm1
%if (%6!=0)
  lea eax,[eax+%6*edx]
  lea ecx,[ecx+%6*edx]
%endif
%endmacro

;//////////////////////////////////////////////////////////////////////

align 16
Skl_Add_8x4_FH_Rnd0_SSE:
  PROLOG1
  COPY_ADD_FH_RND0 0, edx, 2*edx, ebx, 1, 0
  pop ebx
  ret

align 16
Skl_Add_8x8_FH_Rnd0_SSE:
  PROLOG1
  COPY_ADD_FH_RND0 0, edx, 2*edx, ebx, 1, 4
  COPY_ADD_FH_RND0 0, edx, 2*edx, ebx, 1, 0
  pop ebx
  ret


align 16
Skl_Add_16x8_FH_Rnd0_SSE:
  PROLOG0
    ; 60c
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 1, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 1, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 1, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 1, 0
  ret


align 16
Skl_Copy_8x4_FH_Rnd0_SSE:
  PROLOG1
  COPY_ADD_FH_RND0 0, edx, 2*edx, ebx, 0, 0
  pop ebx
  ret

align 16
Skl_Copy_8x8_FH_Rnd0_SSE:
  PROLOG1
  COPY_ADD_FH_RND0 0, edx, 2*edx, ebx, 0, 4
  COPY_ADD_FH_RND0 0, edx, 2*edx, ebx, 0, 0
  pop ebx
  ret

align 16
Skl_Copy_16x8_FH_Rnd0_SSE:
  PROLOG0
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 0
  ret

align 16
Skl_H_Pass_2Taps_SSE:
Skl_Copy_16x16_FH_Rnd0_SSE:
  PROLOG0
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 2

  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND0 0, 8, edx, edx+8, 0, 0
  ret

;//////////////////////////////////////////////////////////////////////

align 16
Skl_Copy_8x4_FH_Rnd1_SSE:
  PROLOG1
  movq mm7, [Mask]
  COPY_ADD_FH_RND1 0, edx, 2*edx, ebx, 0, 0
  pop ebx
  ret
align 16
Skl_Copy_8x8_FH_Rnd1_SSE:
  PROLOG1
  movq mm7, [Mask]
  COPY_ADD_FH_RND1 0, edx, 2*edx, ebx, 0, 4
  COPY_ADD_FH_RND1 0, edx, 2*edx, ebx, 0, 0
  pop ebx
  ret

align 16
Skl_Copy_16x8_FH_Rnd1_SSE:
  PROLOG0
  movq mm7, [Mask]
  COPY_ADD_FH_RND1 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND1 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND1 0, 8, edx, edx+8, 0, 2
  COPY_ADD_FH_RND1 0, 8, edx, edx+8, 0, 0
  ret

;//////////////////////////////////////////////////////////////////////
; Half-Full funcs
;//////////////////////////////////////////////////////////////////////

%macro COPY_ADD_8x8_HF_RND0 5 ; %1:COPY/AVRG %2:offset, %3-%4:in/out regs %5:mult
  movq  mm1, [eax+edx+%2]
  movq  mm2, [eax+2*edx+%2]
  movq  mm3, [eax+ebx+%2]
  movq  %4,  [eax+4*edx+%2]
  pavgb %3,  mm1
  pavgb mm1, mm2
  pavgb mm2, mm3
  pavgb mm3, %4
%if (%1!=0)
  pavgb %3,  [ecx+%2]
  pavgb mm1, [ecx+edx+%2]
  pavgb mm2, [ecx+2*edx+%2]
  pavgb mm3, [ecx+ebx+%2]
%endif
  movq [ecx+%2],      %3
  movq [ecx+edx+%2],  mm1
  movq [ecx+2*edx+%2],mm2
  movq [ecx+ebx+%2],  mm3
%if (%5!=0)
  lea eax,[eax+%5*edx]
  lea ecx,[ecx+%5*edx]
%endif
%endmacro

%macro COPY_ADD_8x8_HF_RND1 5 ; %1:COPY/AVRG %2:offset %3-%4:in/out regs %5:mult
  movq  mm1, [eax+edx+%2]
  movq  %4, [eax+2*edx+%2]
  movq  mm2, %3
  movq  mm3, mm1
  pavgb %3, mm1
  pxor  mm2, mm1  
  pavgb mm1, %4
  pxor  mm3, %4
  pand  mm2, mm7    ; lsb's of (i^j)...
  psubb %3, mm2     ; ...are substracted from result of pavgb
  pand  mm3, mm7
  psubb mm1, mm3
%if (%1!=0)
  pavgb %3, [ecx+%2]
  pavgb mm1, [ecx+edx+%2]
%endif
  movq [ecx+%2], %3
  movq [ecx+edx+%2], mm1
%if (%5!=0)
  lea eax,[eax+%5*edx]
  lea ecx,[ecx+%5*edx]
%endif
%endmacro

;//////////////////////////////////////////////////////////////////////

align 16
Skl_Add_8x4_HF_Rnd0_SSE:
  PROLOG1
  movq mm0, [eax] ; loop invariant
  COPY_ADD_8x8_HF_RND0 1,0, mm0,mm4, 0
  pop ebx
  ret

align 16
Skl_Copy_8x4_HF_Rnd0_SSE:
  PROLOG1
  movq mm0, [eax] ; loop invariant
  COPY_ADD_8x8_HF_RND0 0,0, mm0,mm4, 0
  pop ebx
  ret


align 16
Skl_Add_8x8_HF_Rnd0_SSE:
  PROLOG1
  movq mm0, [eax] ; loop invariant
  COPY_ADD_8x8_HF_RND0 1,0, mm0,mm4, 4
  COPY_ADD_8x8_HF_RND0 1,0, mm4,mm0, 0
  pop ebx
  ret

align 16
Skl_Copy_8x8_HF_Rnd0_SSE:
  PROLOG1
  movq mm0, [eax] ; loop invariant
  COPY_ADD_8x8_HF_RND0 0,0, mm0,mm4, 4
  COPY_ADD_8x8_HF_RND0 0,0, mm4,mm0, 0
  pop ebx
  ret


align 16
Skl_Add_16x8_HF_Rnd0_SSE:
  PROLOG1
  movq mm0, [eax]   ; loop invariants
  movq mm5, [eax+8] ; loop invariants
  COPY_ADD_8x8_HF_RND0 1,0, mm0,mm4, 0
  COPY_ADD_8x8_HF_RND0 1,8, mm5,mm6, 4
  COPY_ADD_8x8_HF_RND0 1,0, mm4,mm0, 0
  COPY_ADD_8x8_HF_RND0 1,8, mm6,mm5, 0
  pop ebx
  ret

align 16
Skl_Copy_16x8_HF_Rnd0_SSE:
  PROLOG1
  movq mm0, [eax]   ; loop invariants
  movq mm5, [eax+8] ; loop invariants
  COPY_ADD_8x8_HF_RND0 0,0, mm0,mm4, 0
  COPY_ADD_8x8_HF_RND0 0,8, mm5,mm6, 4
  COPY_ADD_8x8_HF_RND0 0,0, mm4,mm0, 0
  COPY_ADD_8x8_HF_RND0 0,8, mm6,mm5, 0
  pop ebx
  ret

align 16
Skl_V_Pass_2Taps_SSE:
Skl_Copy_16x16_HF_Rnd0_SSE:
  PROLOG1
  movq mm0, [eax]   ; loop invariants
  movq mm5, [eax+8] ; loop invariants
  COPY_ADD_8x8_HF_RND0 0,0, mm0,mm4, 0
  COPY_ADD_8x8_HF_RND0 0,8, mm5,mm6, 4
  COPY_ADD_8x8_HF_RND0 0,0, mm4,mm0, 0
  COPY_ADD_8x8_HF_RND0 0,8, mm6,mm5, 4

  COPY_ADD_8x8_HF_RND0 0,0, mm0,mm4, 0
  COPY_ADD_8x8_HF_RND0 0,8, mm5,mm6, 4
  COPY_ADD_8x8_HF_RND0 0,0, mm4,mm0, 0
  COPY_ADD_8x8_HF_RND0 0,8, mm6,mm5, 0
  pop ebx
  ret

;//////////////////////////////////////////////////////////////////////

align 16
Skl_Copy_8x4_HF_Rnd1_SSE:
  PROLOG0
  movq mm0, [eax] ; loop invariant
  movq mm7, [Mask]
  COPY_ADD_8x8_HF_RND1 0,0, mm0,mm4, 2
  COPY_ADD_8x8_HF_RND1 0,0, mm4,mm0, 0
  ret


align 16
Skl_Copy_8x8_HF_Rnd1_SSE:
  PROLOG0
      ; 32c
  movq mm7, [Mask]
  movq mm0, [eax] ; loop invariant
  COPY_ADD_8x8_HF_RND1 0,0, mm0,mm4, 2
  COPY_ADD_8x8_HF_RND1 0,0, mm4,mm0, 2
  COPY_ADD_8x8_HF_RND1 0,0, mm0,mm4, 2
  COPY_ADD_8x8_HF_RND1 0,0, mm4,mm0, 0
  ret


align 16
Skl_Copy_16x8_HF_Rnd1_SSE:
  PROLOG0
  movq mm0, [eax]   ; loop invariants
  movq mm5, [eax+8] ; loop invariants
    ; 61c
  movq mm7, [Mask]
  COPY_ADD_8x8_HF_RND1 0,0, mm0,mm4, 0
  COPY_ADD_8x8_HF_RND1 0,8, mm5,mm6, 2

  COPY_ADD_8x8_HF_RND1 0,0, mm4,mm0, 0
  COPY_ADD_8x8_HF_RND1 0,8, mm6,mm5, 2

  COPY_ADD_8x8_HF_RND1 0,0, mm0,mm4, 0
  COPY_ADD_8x8_HF_RND1 0,8, mm5,mm6, 2

  COPY_ADD_8x8_HF_RND1 0,0, mm4,mm0, 0
  COPY_ADD_8x8_HF_RND1 0,8, mm6,mm5, 0
  ret

;//////////////////////////////////////////////////////////////////////
; Half-Half funcs
;//////////////////////////////////////////////////////////////////////

; The trick is to correct the result of 'pavgb' with some combination of the
; lsb's of the 4 input values i,j,k,l, and their intermediate 'pavgb' (s and t).
; The boolean relations are:
;   (i+j+k+l+3)/4 = (s+t+1)/2 - (ij&kl)&st
;   (i+j+k+l+2)/4 = (s+t+1)/2 - (ij|kl)&st
;   (i+j+k+l+1)/4 = (s+t+1)/2 - (ij&kl)|st
;   (i+j+k+l+0)/4 = (s+t+1)/2 - (ij|kl)|st
; with  s=(i+j+1)/2, t=(k+l+1)/2, ij = i^j, kl = k^l, st = s^t.

; Moreover, we process 2 lines at a times, for better overlapping (~15% faster).

    ; loop invariants: %1=(i+j+1)/2  and  %2= i^j
%macro HH_SETUP 3   ; %1,%2:regs, %3:offset
  movq  %1, [eax+%3] 
  movq  %2, [eax+%3+1]
  movq  mm6, %1
  pavgb %1, %2      ; (i+j+1)/2
  pxor  %2, mm6     ; i^j
%endmacro

%macro COPY_ADD_HH_RND0 4 ; %1=COPY/AVRG, %2:offset,  %3/%4: inv regs
  movq  mm0, [eax+edx+%2]
  movq  mm6, mm0
  movq  mm1, [eax+edx+%2+1]
  pavgb mm0, mm1  ; mm0=(j+k+1)/2. preserved for next step
  pxor  mm1, mm6  ; mm1=(j^k).     preserved for next step
  movq  mm6, %3
  por   %4,  mm1  ; ij |= jk
  pxor  mm6, mm0  ; mm6 = s^t
  pand  %4,  mm6  ; (ij|jk) &= st
  pavgb %3,  mm0  ; mm2 = (s+t+1)/2
  pand  %4,  mm7  ; mask lsb
  psubb %3,  %4   ; apply.

%if (%1!=0)
  pavgb %3, [ecx+%2]
%endif
  movq [ecx+%2], %3

  movq  %3, [eax+2*edx+%2]
  movq  %4, [eax+2*edx+%2+1]
  movq  mm6, %3
  pavgb %3, %4    ; preserved for next iteration
  pxor  %4, mm6   ; preserved for next iteration
  movq  mm6, mm0
  por   mm1, %4
  pxor  mm6, %3
  pand  mm1, mm6
  pavgb mm0, %3
  pand  mm1, mm7 
  psubb mm0, mm1

%if (%1!=0)
  pavgb mm0, [ecx+edx+%2]
%endif
  movq [ecx+edx+%2], mm0
%endmacro

%macro COPY_ADD_HH_RND1 4 ; %1=COPY/AVRG, %2:offset,  %3/%4: inv regs
  movq  mm0, [eax+edx+%2]
  movq  mm6, mm0
  movq  mm1, [eax+edx+%2+1]
  pavgb mm0, mm1  ; mm0=(j+k+1)/2. preserved for next step
  pxor  mm1, mm6  ; mm1=(j^k).     preserved for next step
  movq  mm6, %3
  pand  %4, mm1   ; ij &= jk
  pxor  mm6, mm0
  por   %4, mm6
  pavgb %3, mm0
  pand  %4, mm7
  psubb %3, %4

%if (%1!=0)
  pavgb %3, [ecx+%2]
%endif
  movq [ecx+%2], %3

  movq  %3, [eax+2*edx+%2]
  movq  %4, [eax+2*edx+%2+1]
  movq  mm6, %3
  pavgb %3, %4    ; preserved for next iteration
  pxor  %4, mm6   ; preserved for next iteration
  movq  mm6, mm0
  pand  mm1, %4
  pxor  mm6, %3
  por   mm1, mm6
  pavgb mm0, %3
  pand  mm1, mm7
  psubb mm0, mm1

%if (%1!=0)
  pavgb mm0, [ecx+edx+%2]
%endif
  movq [ecx+edx+%2], mm0
%endmacro

;//////////////////////////////////////////////////////////////////////

align 16
Skl_Add_8x4_HH_Rnd0_SSE:
  PROLOG0
  movq mm7, [Mask]
  HH_SETUP mm2, mm3, 0

  COPY_ADD_HH_RND0 1,0, mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 1,0, mm2,mm3
  ret

align 16
Skl_Copy_8x4_HH_Rnd0_SSE:
  PROLOG0
  movq mm7, [Mask]
  HH_SETUP mm2, mm3, 0

  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  ret

align 16
Skl_Add_8x8_HH_Rnd0_SSE:
  PROLOG0
  movq mm7, [Mask]
  HH_SETUP mm2, mm3, 0

  COPY_ADD_HH_RND0 1,0,  mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 1,0,  mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 1,0,  mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 1,0,  mm2,mm3
  ret

align 16
Skl_Copy_8x8_HH_Rnd0_SSE:
  PROLOG0
  movq mm7, [Mask]
  HH_SETUP mm2, mm3, 0

    ; 55c
  nop
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  ret


align 16
Skl_Add_16x8_HH_Rnd0_SSE:
  PROLOG0
  movq mm7, [Mask]
  HH_SETUP mm2, mm3, 0
  HH_SETUP mm4, mm5, 8

  COPY_ADD_HH_RND0 1,0,  mm2,mm3
  COPY_ADD_HH_RND0 1,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 1,0,  mm2,mm3
  COPY_ADD_HH_RND0 1,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 1,0,  mm2,mm3
  COPY_ADD_HH_RND0 1,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 1,0,  mm2,mm3
  COPY_ADD_HH_RND0 1,8,  mm4,mm5
  ret

align 16
Skl_Copy_16x8_HH_Rnd0_SSE:
  PROLOG0
  movq mm7, [Mask]
  HH_SETUP mm2, mm3, 0
  HH_SETUP mm4, mm5, 8

    ; 103c
  nop
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  nop
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  ret

align 16
Skl_HV_Pass_2Taps_SSE:
Skl_Copy_16x16_HH_Rnd0_SSE:
  PROLOG0
  movq mm7, [Mask]
  HH_SETUP mm2, mm3, 0
  HH_SETUP mm4, mm5, 8

  nop
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]

  nop
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]

  nop
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]

  nop
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND0 0,0,  mm2,mm3
  COPY_ADD_HH_RND0 0,8,  mm4,mm5

  ret

;//////////////////////////////////////////////////////////////////////

align 16
Skl_Copy_8x4_HH_Rnd1_SSE:
  PROLOG0
  movq mm7, [Mask]
  HH_SETUP mm2, mm3, 0

  COPY_ADD_HH_RND1 0,0, mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND1 0,0, mm2,mm3
  ret

align 16
Skl_Copy_8x8_HH_Rnd1_SSE:
  PROLOG0
  movq mm7, [Mask]
  HH_SETUP mm2, mm3, 0

    ; 53c
  COPY_ADD_HH_RND1 0,0, mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND1 0,0, mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND1 0,0, mm2,mm3
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND1 0,0, mm2,mm3
  ret

align 16
Skl_Copy_16x8_HH_Rnd1_SSE:
  PROLOG0
  movq mm7, [Mask]
  HH_SETUP mm2, mm3, 0
  HH_SETUP mm4, mm5, 8

    ; 103c
  COPY_ADD_HH_RND1 0,0, mm2,mm3
  COPY_ADD_HH_RND1 0,8, mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND1 0,0, mm2,mm3
  COPY_ADD_HH_RND1 0,8, mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND1 0,0, mm2,mm3
  COPY_ADD_HH_RND1 0,8, mm4,mm5
  lea eax, [eax+2*edx]
  lea ecx, [ecx+2*edx]
  COPY_ADD_HH_RND1 0,0, mm2,mm3
  COPY_ADD_HH_RND1 0,8, mm4,mm5
  ret

;//////////////////////////////////////////////////////////////////////
;//
;// 8x8 -> 16x16 upsampling (16b)
;//
;//////////////////////////////////////////////////////////////////////

%macro MUL_PACK 4     ; %1/%2: regs   %3/%4/%5: Up13/Up31
  pmullw %1,  %3 ; [Up13]
  pmullw mm4, %4 ; [Up31]
  pmullw %2,  %3 ; [Up13]
  pmullw mm5, %4 ; [Up31]
  paddsw %1, [Cst2]
  paddsw %2, [Cst2]
  paddsw %1, mm4
  paddsw %2, mm5
%endmacro

%macro COL03 3    ;%1/%2: regs, %3: row   -trashes mm4/mm5
  movq %2, [edx+%3*16+0*2]               ; <- 0|1|2|3
  pshufw %1,  %2,  (0+0*4+0*16+1*64)     ; %1 = 0|0|0|1
  pshufw mm4, %2,  (0+1*4+1*16+2*64)     ; mm4= 0|1|1|2
  pshufw %2,  %2,  (1+2*4+2*16+3*64)     ; %2 = 1|2|2|3
  pshufw mm5, [edx+%3*16+2*2],  (0+1*4+1*16+2*64) ; mm5 = 2|3|3|4
%endmacro

%macro COL47 3    ;%1-%2: regs, %3: row   -trashes mm4/mm5
  pshufw %1, [edx+%3*16+2*2],  (1+2*4+2*16+3*64) ; 3|4|4|5
  movq mm5, [edx+%3*16+2*4]                      ; <- 4|5|6|7
  pshufw mm4, mm5,  (0+1*4+1*16+2*64)            ; 4|5|5|6
  pshufw %2,  mm5,  (1+2*4+2*16+3*64)            ; 5|6|6|7
  pshufw mm5, mm5,  (2+3*4+3*16+3*64)            ; 6|7|7|7
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
Skl_Copy_Upsampled_8x8_16To8_SSE:  ; 315c

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
Skl_Add_Upsampled_8x8_16To8_SSE:  ; 549c

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

  ; pfeewwww... Never Do That On Stage Again. :)

