;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
;//////////////////////////////////////////////////////////////////////
; [BITS 32]

%include "../../include/skl_syst/skl_nasm.h"

globl Skl_Smooth_18x18_To_8x8_MMX
globl Skl_Gradx_18x18_To_8x8_MMX
globl Skl_Grady_18x18_To_8x8_MMX
globl Skl_Grad2_18x18_To_8x8_MMX

DATA

align 16
Cst3:    times 8  dw 3
Cst32:   times 8  dw 32
Cst64:   times 8  dw 64
Mask_ff: times 8  dw 0xff
Thresh:  times 8  dw 24

TEXT

;//////////////////////////////////////////////////////////////////////
;//
;//  MMX impl
;//
;//////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////
;// 18x18 -> 8x8 block filtering
;//////////////////////////////////////////////////////////////////////

%macro ADD_TIMES 5   ; (%1) = (%1)+(%5)*(%3)   (%2) = (%2)+(%5)*(%4)
  pmullw %3, %5
  pmullw %4, %5
  paddsw %1, %3
  paddsw %2, %4
%endmacro

%macro SUB_TIMES 5   ; (%1) = (%1)-3*(%3)   (%2) = (%2)-(%5)*(%4)
  pmullw %3, %5
  pmullw %4, %5
  psubsw %1, %3
  psubsw %2, %4
%endmacro


%macro LOAD_G2 3  ;%1-%2: Dst, %3:src    mm6-mm7: trashed
  movq mm6, [Mask_ff]
  movq %1,  [%3]      ; 01234567
  movq %2,  [%3+1]    ; 12345678
  movq mm7, %2
  psrlq %2, 8
  pand  %1, mm6       ; 0|2|4|6
  pand mm7, mm6       ; 1|3|5|7
  pand  %2, mm6       ; 2|4|6|8
  pand mm6, [%3-1]    ;-1|1|3|5
  pmullw mm7,  [Cst3]
  pmullw %1,  [Cst3]
  paddsw %2, mm7
  paddsw %1, mm6
%endmacro

%macro MIX_G2 2    ; %1-2: regs.  output:%1
  paddsw %1, [Cst32]
  paddsw %2, [Cst32]
  psraw %1, 6
  psraw %2, 6
  pmullw %1, %1
  pmullw %2, %2
  paddusw %1, %2

; uncomment the following instr for thresholding:
;  pcmpgtw %1, [Thresh]
; and change 'packuswb' into 'packsswb' in STORE_G2 below

%endmacro

%macro STORE_G2 2  ; %1:op type (0:tmp store, 1:final pack), %2:src offset
  LOAD_G2 mm2,mm3, edx+%2

  movq mm4,mm2
  movq mm5,mm3
  SUB_TIMES mm0,mm1, mm5,mm4, [Cst3]

  LOAD_G2 mm4,mm5, edx+eax+%2
  lea edx, [edx+2*eax]

  psubsw mm0, mm5
  psubsw mm1, mm4
  ADD_TIMES mm2,mm3, mm4,mm5, [Cst3]

  MIX_G2 mm0,mm1
%if (%1==1)
  packuswb mm0, [ecx]
  ; packsswb mm0, [ecx]     ; <= use instead, for threshold
%endif
  movq [ecx], mm0

  LOAD_G2 mm0, mm1, edx+%2

  movq mm4,mm0
  movq mm5,mm1
  SUB_TIMES mm2,mm3, mm5,mm4, [Cst3]

  LOAD_G2 mm4,mm5, edx+eax+%2
  lea edx, [edx+2*eax]

  psubsw mm2, mm5
  psubsw mm3, mm4
  ADD_TIMES mm0,mm1, mm4,mm5, [Cst3]

  MIX_G2 mm2,mm3
%if (%1==1)
  packuswb mm2, [ecx+ebx]
  ; packsswb mm2, [ecx+ebx]     ; <= use instead, for threshold
%endif
  movq [ecx+ebx], mm2
  lea ecx, [ecx+2*ebx]
%endmacro


align 16
Skl_Grad2_18x18_To_8x8_MMX:
  push ebx

  mov ecx, [esp+4  +4] ; Dst
  mov ebx, [esp+8  +4] ; Dst_BpS
  mov edx, [esp+12 +4] ; Src
  mov eax, [esp+16 +4] ; Src_BpS
  sub edx, eax

      ; process columns 0-3

  LOAD_G2 mm0,mm1, edx+8
  LOAD_G2 mm4,mm5, edx+eax+8
  lea edx, [edx+2*eax]
  ADD_TIMES mm0,mm1, mm4,mm5, [Cst3]

  STORE_G2 0, 8
  STORE_G2 0, 8
  STORE_G2 0, 8
  STORE_G2 0, 8

      ; process columns 4-7

  mov ecx, [esp+4  +4] ; Dst
  mov edx, [esp+12 +4] ; Src
  sub edx, eax

  LOAD_G2 mm0,mm1, edx
  LOAD_G2 mm4,mm5, edx+eax
  lea edx, [edx+2*eax]
  ADD_TIMES mm0,mm1, mm4,mm5, [Cst3]

  STORE_G2 1, 0
  STORE_G2 1, 0
  STORE_G2 1, 0
  STORE_G2 1, 0

  pop ebx
  ret

;//////////////////////////////////////////////////////////////////////
;// for Gradx,Grady,Smooth, the scheme is different than 
;// the C-version (=>lower op count).

%macro LOAD_S 2  ;%1: Dst, %2:src
  movq mm6, [Mask_ff]
  movq %1,  [%2+1]    ; 12345678  
  movq mm4, [%2]      ; 01234567
  movq mm5, %1
  pand mm4, mm6       ; 0|2|4|6  
  pand mm5, mm6       ; 1|3|5|7
  psrlq %1, 8
  pand  %1, mm6       ; 2|4|6|8
  pand mm6, [%2-1]    ;-1|1|3|5
  paddusw mm5, mm4
  paddusw %1, mm6
  pmullw mm5,  mm7  ; x[Cst3]
  paddusw %1, mm5
%endmacro

%macro MIX_S 4    ; %1-%4: regs.  output:%1
  paddusw %1, [Cst32]
  paddusw %2, %3
  paddusw %1, %4
  pmullw %2, mm7  ; x[Cst3]
  paddusw %1, %2
  psraw %1, 6
%endmacro

%macro STORE_S 2  ; %1:op type (0:tmp store, 1:final pack), %2:src offset
  LOAD_S mm2, edx+%2
  LOAD_S mm3, edx+eax+%2
  lea edx, [edx+2*eax]

  MIX_S mm0,mm1,mm2,mm3
%if (%1==1)
  packuswb mm0, [ecx]
%endif
  movq [ecx], mm0

  LOAD_S mm0, edx+%2
  LOAD_S mm1, edx+eax+%2
  lea edx, [edx+2*eax]
  MIX_S mm2,mm3, mm0,mm1
%if (%1==1)
  packuswb mm2, [ecx+ebx]
%endif
  movq [ecx+ebx], mm2
  lea ecx, [ecx+2*ebx]
%endmacro


align 16
Skl_Smooth_18x18_To_8x8_MMX:
  push ebx

  mov ecx, [esp+4  +4] ; Dst
  mov ebx, [esp+8  +4] ; Dst_BpS
  mov edx, [esp+12 +4] ; Src
  mov eax, [esp+16 +4] ; Src_BpS

  movq mm7, [Cst3]
  sub edx, eax

      ; process columns 0-3

  LOAD_S mm0, edx+8
  LOAD_S mm1, edx+eax+8
  lea edx, [edx+2*eax]

  STORE_S 0, 8
  STORE_S 0, 8
  STORE_S 0, 8
  STORE_S 0, 8

      ; process columns 4-7

  mov ecx, [esp+4  +4] ; Dst
  mov edx, [esp+12 +4] ; Src
  sub edx, eax

  LOAD_S mm0, edx
  LOAD_S mm1, edx+eax
  lea edx, [edx+2*eax]

  STORE_S 1, 0
  STORE_S 1, 0
  STORE_S 1, 0
  STORE_S 1, 0

  pop ebx
  ret

;//////////////////////////////////////////////////////////////////////

%macro LOAD_GX 2  ;%1: Dst, %2:src
  movq mm6, [Mask_ff]
  movq mm4,  [%2]      ; 01234567
  movq %1,   [%2+1]    ; 12345678
  movq mm5, %1
  psrlq %1, 8  
  pand mm4, mm6       ; 0|2|4|6  
  pand mm5, mm6       ; 1|3|5|7
  pand  %1, mm6       ; 2|4|6|8  
  pand mm6, [%2-1]    ;-1|1|3|5
  psubsw mm5, mm4  
  psubsw %1, mm6
  pmullw mm5,  mm7  ; x[Cst3]
  paddsw %1, mm5  
%endmacro

%macro MIX_GX 4    ; %1-%4: regs.  output:%1
  paddsw %1, [Cst64]
  paddsw %2, %3
  paddsw %1, %4
  pmullw %2, mm7  ; x[Cst3]
  paddsw %1, %2
  psraw %1, 7
%endmacro

%macro STORE_GX 2  ; %1:op type (0:tmp store, 1:final pack), %2:src offset
  LOAD_GX mm2, edx+%2
  LOAD_GX mm3, edx+eax+%2
  lea edx, [edx+2*eax]

  MIX_GX mm0,mm1, mm2,mm3
%if (%1==1)
  packsswb mm0, [ecx]
%endif
  movq [ecx], mm0

  LOAD_GX mm0, edx+%2
  LOAD_GX mm1, edx+eax+%2
  lea edx, [edx+2*eax]

  MIX_GX mm2,mm3, mm0,mm1
%if (%1==1)
  packsswb mm2, [ecx+ebx]
%endif
  movq [ecx+ebx], mm2
  lea ecx, [ecx+2*ebx]
%endmacro


align 16
Skl_Gradx_18x18_To_8x8_MMX:
  push ebx

  mov ecx, [esp+4  +4] ; Dst
  mov ebx, [esp+8  +4] ; Dst_BpS
  mov edx, [esp+12 +4] ; Src
  mov eax, [esp+16 +4] ; Src_BpS

  movq mm7, [Cst3]
  sub edx, eax

      ; process columns 0-3
  LOAD_GX mm0, edx+8
  LOAD_GX mm1, edx+eax+8
  lea edx, [edx+2*eax]

  STORE_GX 0, 8
  STORE_GX 0, 8
  STORE_GX 0, 8
  STORE_GX 0, 8

      ; process columns 4-7

  mov ecx, [esp+4  +4] ; Dst
  mov edx, [esp+12 +4] ; Src
  sub edx, eax

  LOAD_GX mm0, edx
  LOAD_GX mm1, edx+eax
  lea edx, [edx+2*eax]

  STORE_GX 1, 0
  STORE_GX 1, 0
  STORE_GX 1, 0
  STORE_GX 1, 0

  pop ebx
  ret

;//////////////////////////////////////////////////////////////////////

%macro MIX_GY 4    ; %1-%4: regs.  output:mm5
  movq mm5, [Cst64]  
  psubsw %2, %3
  psubsw %1, %4
  pmullw %2, mm7  ; x[Cst3]
  psubsw mm5, %1   
  psubsw mm5, %2
  psraw mm5, 7
%endmacro

%macro STORE_GY 2  ; %1:op type (0:tmp store, 1:final pack), %2:src offset
  LOAD_S mm2, edx+%2
  LOAD_S mm3, edx+eax+%2
  lea edx, [edx+2*eax]

  MIX_GY mm0,mm1, mm2,mm3
%if (%1==1)
  packsswb mm5, [ecx]
%endif
  movq [ecx], mm5

  LOAD_S mm0, edx+%2
  LOAD_S mm1, edx+eax+%2
  lea edx, [edx+2*eax]

  MIX_GY mm2,mm3, mm0,mm1
%if (%1==1)
  packsswb mm5, [ecx+ebx]
%endif
  movq [ecx+ebx], mm5
  lea ecx, [ecx+2*ebx]
%endmacro

align 16
Skl_Grady_18x18_To_8x8_MMX:
  push ebx

  mov ecx, [esp+4  +4] ; Dst
  mov ebx, [esp+8  +4] ; Dst_BpS
  mov edx, [esp+12 +4] ; Src
  mov eax, [esp+16 +4] ; Src_BpS

  movq mm7, [Cst3]
  sub edx, eax

      ; process columns 0-3

  LOAD_S mm0, edx+8
  LOAD_S mm1, edx+eax+8
  lea edx, [edx+2*eax]

  STORE_GY 0, 8
  STORE_GY 0, 8
  STORE_GY 0, 8
  STORE_GY 0, 8

      ; process columns 4-7

  mov ecx, [esp+4  +4] ; Dst
  mov edx, [esp+12 +4] ; Src
  sub edx, eax

  LOAD_S mm0, edx
  LOAD_S mm1, edx+eax
  lea edx, [edx+2*eax]

  STORE_GY 1, 0
  STORE_GY 1, 0
  STORE_GY 1, 0
  STORE_GY 1, 0

  pop ebx
  ret

;//////////////////////////////////////////////////////////////////////
