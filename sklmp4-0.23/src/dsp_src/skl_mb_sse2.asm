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

globl Skl_Add_16x8_FF_SSE2
globl Skl_Add_16x8_FH_Rnd0_SSE2
globl Skl_Add_16x8_HF_Rnd0_SSE2
globl Skl_Add_16x8_HH_Rnd0_SSE2
globl Skl_Copy_16x8_FF_SSE2
globl Skl_Copy_16x8_FH_Rnd1_SSE2
globl Skl_Copy_16x8_HF_Rnd1_SSE2
globl Skl_Copy_16x8_HH_Rnd1_SSE2
globl Skl_Copy_16x8_FH_Rnd0_SSE2
globl Skl_Copy_16x8_HF_Rnd0_SSE2
globl Skl_Copy_16x8_HH_Rnd0_SSE2

;//////////////////////////////////////////////////////////////////////

DATA

align 16
Rounder1_SSE2 times 8  dw 1
Mask1_SSE2    times 16 db 1

;//////////////////////////////////////////////////////////////////////

TEXT

  ; all funcs are signed: (uint8_t *Dst, const uint8_t *Src,
  ;                        const uint32_t BpS, const uint32_t Rounding)
  ; Dst (ecx) is supposed *aligned* to 16

%macro PROLOG 0
  mov ecx, [esp+ 4] ; Dst
  mov eax, [esp+ 8] ; Src
  mov edx, [esp+12] ; BpS
%endmacro
%macro PROLOG0 0
  PROLOG
  push ebx
  lea ebx,[edx+2*edx] ; 3*BpS
%endmacro
%macro EPILOG 0
  pop ebx
  ret
%endmacro

;//////////////////////////////////////////////////////////////////////
; Full-Full funcs
;//////////////////////////////////////////////////////////////////////

%macro ADD_FF 0
  movdqu xmm0, [eax]
  movdqu xmm1, [eax+edx]
  movdqu xmm2, [eax+2*edx]
  movdqu xmm3, [eax+ebx]
  movdqu xmm4, [ecx]
  movdqu xmm5, [ecx+edx]
  movdqu xmm6, [ecx+2*edx]
  movdqu xmm7, [ecx+ebx]
  pavgb  xmm0, xmm4
  pavgb  xmm1, xmm5
  pavgb  xmm2, xmm6
  pavgb  xmm3, xmm7
  movdqa [ecx],      xmm0
  movdqa [ecx+edx],  xmm1
  movdqa [ecx+2*edx],xmm2
  movdqa [ecx+ebx],  xmm3
%endmacro

align 16
Skl_Add_16x8_FF_SSE2:
  PROLOG0
  ADD_FF
  lea eax, [eax+4*edx]
  lea ecx, [ecx+4*edx]
  ADD_FF
  EPILOG

;//////////////////////////////////////////////////////////////////////

%macro COPY_FF 0
  movdqu xmm0,  [eax]
  movdqu xmm1,  [eax+edx]
  movdqu xmm2,  [eax+2*edx]
  movdqu xmm3,  [eax+ebx]
  lea eax, [eax+4*edx]
  movdqu xmm4,  [eax]
  movdqu xmm5,  [eax+edx]
  movdqu xmm6,  [eax+2*edx]
  movdqu xmm7,  [eax+ebx]
  movdqa [ecx],      xmm0
  movdqa [ecx+edx],  xmm1
  movdqa [ecx+2*edx],xmm2
  movdqa [ecx+ebx],  xmm3
  lea ecx, [ecx+4*edx]
  movdqa [ecx],      xmm4
  movdqa [ecx+edx],  xmm5
  movdqa [ecx+2*edx],xmm6
  movdqa [ecx+ebx],  xmm7
%endmacro

align 16
Skl_Copy_16x8_FF_SSE2:
  PROLOG0
  COPY_FF
  EPILOG

;//////////////////////////////////////////////////////////////////////
; Full-Half funcs
;//////////////////////////////////////////////////////////////////////

%macro COPY_ADD_FH_RND0 1  ; %1=0->COPY, =1->AVRG
  movdqu xmm0, [eax]
  movdqu xmm1, [eax+edx]
  movdqu xmm2, [eax+1]
  movdqu xmm3, [eax+edx+1]
  movdqu xmm4, [eax+2*edx]
  movdqu xmm5, [eax+ebx]
  movdqu xmm6, [eax+2*edx+1]
  movdqu xmm7, [eax+ebx+1]
  pavgb  xmm0, xmm2
  pavgb  xmm1, xmm3
  pavgb  xmm4, xmm6
  pavgb  xmm5, xmm7
%if (%1!=0)
  pavgb  xmm0, [ecx]
  pavgb  xmm1, [ecx+edx]
  pavgb  xmm4, [ecx+2*edx]
  pavgb  xmm5, [ecx+ebx]
%endif
  movdqa [ecx],      xmm0
  movdqa [ecx+edx],  xmm1
  movdqa [ecx+2*edx],xmm4
  movdqa [ecx+ebx],  xmm5
%endmacro

%macro COPY_ADD_FH_RND1 1
  movdqu xmm0, [eax]
  movdqu xmm1, [eax+edx]
  movdqu xmm4, xmm0
  movdqu xmm5, xmm1
  movdqu xmm2, [eax+1]
  movdqu xmm3, [eax+edx+1]
  pavgb  xmm0, xmm2
  pxor   xmm2, xmm4
  pavgb  xmm1, xmm3
  pxor   xmm3, xmm5
  pand   xmm2, xmm7   ; [Mask1_SSE2]
  pand   xmm3, xmm7
  psubb  xmm0, xmm2
  psubb  xmm1, xmm3
%if (%1!=0)
  pavgb  xmm0, [ecx]
  pavgb  xmm1, [ecx+edx]
%endif
  movdqa [ecx],xmm0
  movdqa [ecx+edx],xmm1
%endmacro

align 16
Skl_Add_16x8_FH_Rnd0_SSE2:
  PROLOG0
  COPY_ADD_FH_RND0 1
  lea eax,[eax+4*edx]
  lea ecx,[ecx+4*edx]
  COPY_ADD_FH_RND0 1
  EPILOG

;//////////////////////////////////////////////////////////////////////
  
align 16
Skl_Copy_16x8_FH_Rnd0_SSE2:
  PROLOG0
  COPY_ADD_FH_RND0 0
  lea eax,[eax+4*edx]
  lea ecx,[ecx+4*edx]
  COPY_ADD_FH_RND0 0
  EPILOG

align 16
Skl_Copy_16x8_FH_Rnd1_SSE2:
  PROLOG0
  movdqa xmm7, [Mask1_SSE2]
  COPY_ADD_FH_RND1 0
  lea eax,[eax+2*edx]
  lea ecx,[ecx+2*edx]
  COPY_ADD_FH_RND1 0
  lea eax,[eax+2*edx]
  lea ecx,[ecx+2*edx]
  COPY_ADD_FH_RND1 0
  lea eax,[eax+2*edx]
  lea ecx,[ecx+2*edx]
  COPY_ADD_FH_RND1 0
  EPILOG

;//////////////////////////////////////////////////////////////////////
; Half-Full funcs
;//////////////////////////////////////////////////////////////////////

%macro COPY_ADD_16x8_HF_RND0 1     ; %1=0->COPY, =1->AVRG
    ; xmm0 = [eax]
  movdqu xmm1, [eax+  edx]
  movdqu xmm2, [eax+2*edx]
  movdqu xmm3, [eax+  ebx]
  movdqu xmm4, [eax+4*edx]
  pavgb  xmm0, xmm1
  pavgb  xmm1, xmm2
  pavgb  xmm2, xmm3
  pavgb  xmm3, xmm4
%if (%1!=0)
  pavgb  xmm0, [ecx      ]
  pavgb  xmm1, [ecx+  edx]
  pavgb  xmm2, [ecx+2*edx]
  pavgb  xmm3, [ecx+ebx  ]
%endif
  movdqa [ecx],      xmm0
  movdqa [ecx+edx],  xmm1
  movdqa [ecx+2*edx],xmm2
  movdqa [ecx+ebx],  xmm3
%endmacro

%macro COPY_ADD_16x8_HF_RND1 1     ; %1=0->COPY, =1->AVRG
    ; xmm0 = [eax]
  movdqu xmm1, [eax+edx]
  movdqu xmm2, [eax+2*edx]
  lea eax,[eax+2*edx]
  movdqu xmm3, xmm0
  movdqu xmm4, xmm1
  pavgb  xmm0, xmm1
  pxor   xmm3, xmm1
  pavgb  xmm1, xmm2
  pxor   xmm4, xmm2
  pand   xmm3, xmm7  ; lsb's of (i^j)...
  pand   xmm4, xmm7  ; lsb's of (i^j)...
  psubb  xmm0, xmm3 ; ...are substracted from result of pavgb
  psubb  xmm1, xmm4 ; ...are substracted from result of pavgb
%if (%1!=0)
  pavgb xmm0, [ecx]
  pavgb xmm1, [ecx+edx]
%endif
  movdqa [ecx], xmm0
  movdqa  xmm0, xmm2     ; preserved
  movdqa [ecx+edx], xmm1
%endmacro

align 16
Skl_Add_16x8_HF_Rnd0_SSE2:
  PROLOG0
  movdqu xmm0, [eax]   ; loop invariant

  COPY_ADD_16x8_HF_RND0 1
  movdqa xmm0, xmm4  ; next invariant
  lea eax,[eax+4*edx]
  lea ecx,[ecx+4*edx]
  COPY_ADD_16x8_HF_RND0 1
  EPILOG

;//////////////////////////////////////////////////////////////////////

align 16
Skl_Copy_16x8_HF_Rnd0_SSE2:
  PROLOG0
  movdqu xmm0, [eax]   ; loop invariant

  COPY_ADD_16x8_HF_RND0 0
  movdqa xmm0, xmm4  ; next invariant
  lea eax,[eax+4*edx]
  lea ecx,[ecx+4*edx]
  COPY_ADD_16x8_HF_RND0 0
  EPILOG

align 16
Skl_Copy_16x8_HF_Rnd1_SSE2:
  PROLOG0
  movdqu xmm0, [eax]   ; loop invariant
  movdqa xmm7, [Mask1_SSE2]
  COPY_ADD_16x8_HF_RND1 0
  lea ecx,[ecx+2*edx]
  COPY_ADD_16x8_HF_RND1 0
  lea ecx,[ecx+2*edx]
  COPY_ADD_16x8_HF_RND1 0
  lea ecx,[ecx+2*edx]
  COPY_ADD_16x8_HF_RND1 0
  EPILOG

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

; Moreover, we process 2 lines at a times

%macro COPY_ADD_HH_RND0 1     ; %1=0->COPY, =1->AVRG
  movdqu xmm0, [eax  ]
  movdqu xmm1, [eax+1]

  movdqa xmm6, xmm0
  pavgb  xmm0, xmm1  ; mm0=(j+k+1)/2. preserved for next step
  pxor   xmm1, xmm6  ; mm1=(j^k).     preserved for next step

  por    xmm3, xmm1  ; ij |= jk
  movdqa xmm6, xmm2
  pxor   xmm6, xmm0  ; mm6 = s^t
  pand   xmm3, xmm6  ; (ij|jk) &= st
  pavgb  xmm2, xmm0  ; mm2 = (s+t+1)/2
  pand   xmm3, xmm7  ; mask lsb
  psubb  xmm2, xmm3  ; apply.
%if (%1!=0)
  pavgb  xmm2, [ecx]
%endif
  movdqa [ecx], xmm2

  movdqu xmm2, [eax+edx  ]
  movdqu xmm3, [eax+edx+1]
  movdqa xmm6, xmm2
  pavgb  xmm2, xmm3  ; preserved for next iteration
  pxor   xmm3, xmm6  ; preserved for next iteration

  por    xmm1, xmm3
  movdqa xmm6, xmm0
  pxor   xmm6, xmm2
  pand   xmm1, xmm6
  pavgb  xmm0, xmm2

  pand   xmm1, xmm7 
  psubb  xmm0, xmm1
%if (%1!=0)
  pavgb xmm0, [ecx+edx]
%endif
  movdqa [ecx+edx], xmm0
%endmacro

%macro COPY_ADD_HH_RND1 1     ; %1=0->COPY, =1->AVRG
  movdqu xmm0, [eax]
  movdqu xmm1, [eax+1]

  movdqa xmm6, xmm0
  pavgb  xmm0, xmm1  ; mm0=(j+k+1)/2. preserved for next step
  pxor   xmm1, xmm6   ; mm1=(j^k).     preserved for next step

  pand   xmm3, xmm1
  movdqa xmm6, xmm2
  pxor   xmm6, xmm0
  por    xmm3, xmm6
  pavgb  xmm2, xmm0
  pand   xmm3, xmm7
  psubb  xmm2, xmm3
%if (%1!=0)
  pavgb  xmm2, [ecx]
%endif
  movdqa [ecx], xmm2

  movdqu xmm2, [eax+edx]
  movdqu xmm3, [eax+edx+1]
  movdqa xmm6, xmm2
  pavgb  xmm2, xmm3  ; preserved for next iteration
  pxor   xmm3, xmm6   ; preserved for next iteration

  pand   xmm1, xmm3
  movdqa xmm6, xmm0
  pxor   xmm6, xmm2
  por    xmm1, xmm6
  pavgb  xmm0, xmm2
  pand   xmm1, xmm7
  psubb  xmm0, xmm1
%if (%1!=0)
  pavgb  xmm0, [ecx+edx]
%endif
  movdqa [ecx+edx], xmm0
%endmacro


align 16
Skl_Add_16x8_HH_Rnd0_SSE2:
  PROLOG0
  movdqa xmm7, [Mask1_SSE2]

    ; loop invariants: mm2=(i+j+1)/2  and  mm3= i^j
  movdqu xmm2, [eax  ] 
  movdqu xmm3, [eax+1]
  lea eax,[eax+edx]

  movdqu xmm6, xmm2   
  pavgb  xmm2, xmm3
  pxor   xmm3, xmm6   ; xmm2/xmm3 ready

  COPY_ADD_HH_RND0 1
  lea ecx,[ecx+2*edx]
  lea eax,[eax+2*edx]
  COPY_ADD_HH_RND0 1
  lea ecx,[ecx+2*edx]
  lea eax,[eax+2*edx]
  COPY_ADD_HH_RND0 1
  lea ecx,[ecx+2*edx]
  lea eax,[eax+2*edx]
  COPY_ADD_HH_RND0 1

  EPILOG

;//////////////////////////////////////////////////////////////////////

align 16
Skl_Copy_16x8_HH_Rnd0_SSE2:
  PROLOG0
  movdqa xmm7, [Mask1_SSE2]

    ; loop invariants: mm2=(i+j+1)/2  and  mm3= i^j
  movdqu xmm2, [eax] 
  movdqu xmm3, [eax+1]
  lea eax,[eax+edx]
  movdqu xmm6, xmm2   
  pavgb  xmm2, xmm3
  pxor   xmm3, xmm6   ; xmm2/xmm3 ready


  COPY_ADD_HH_RND0 0
  lea ecx,[ecx+2*edx]
  lea eax,[eax+2*edx]
  COPY_ADD_HH_RND0 0
  lea eax,[eax+2*edx]
  lea ecx,[ecx+2*edx]
  COPY_ADD_HH_RND0 0
  lea eax,[eax+2*edx]
  lea ecx,[ecx+2*edx]
  COPY_ADD_HH_RND0 0

  EPILOG

align 16
Skl_Copy_16x8_HH_Rnd1_SSE2:
  PROLOG0
  movdqa xmm7, [Mask1_SSE2]

    ; loop invariants: mm2=(i+j+1)/2  and  mm3= i^j
  movdqu xmm2, [eax] 
  movdqu xmm3, [eax+1]
  lea eax,[eax+edx]
  movdqu xmm6, xmm2   
  pavgb  xmm2, xmm3
  pxor   xmm3, xmm6   ; xmm2/xmm3 ready

  COPY_ADD_HH_RND1 0
  lea ecx,[ecx+2*edx]
  lea eax,[eax+2*edx]
  COPY_ADD_HH_RND1 0
  lea ecx,[ecx+2*edx]
  lea eax,[eax+2*edx]
  COPY_ADD_HH_RND1 0
  lea ecx,[ecx+2*edx]
  lea eax,[eax+2*edx]
  COPY_ADD_HH_RND1 0

  EPILOG

;//////////////////////////////////////////////////////////////////////
