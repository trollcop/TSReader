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

globl Skl_Add_8x8_FF_x86
globl Skl_Add_8x8_FH_Rnd0_x86
globl Skl_Add_8x8_HF_Rnd0_x86
globl Skl_Add_8x8_HH_Rnd0_x86
globl Skl_Add_16x8_FF_x86
globl Skl_Add_16x8_FH_Rnd0_x86
globl Skl_Add_16x8_HF_Rnd0_x86
globl Skl_Add_16x8_HH_Rnd0_x86
globl Skl_Copy_8x8_FF_x86
globl Skl_Copy_8x8_FH_Rnd0_x86
globl Skl_Copy_8x8_HF_Rnd0_x86
globl Skl_Copy_8x8_HH_Rnd0_x86
globl Skl_Copy_8x8_FH_Rnd1_x86
globl Skl_Copy_8x8_HF_Rnd1_x86
globl Skl_Copy_8x8_HH_Rnd1_x86
globl Skl_Copy_16x8_FF_x86
globl Skl_Copy_16x8_FH_Rnd1_x86
globl Skl_Copy_16x8_HF_Rnd1_x86
globl Skl_Copy_16x8_HH_Rnd1_x86
globl Skl_Copy_16x8_FH_Rnd0_x86
globl Skl_Copy_16x8_HF_Rnd0_x86
globl Skl_Copy_16x8_HH_Rnd0_x86

TEXT

%macro PROLOG1 1
  push edi
  mov  edi, [esp+4 + 4]  ; *Dst
  push esi
  mov  esi, [esp+8 + 8]  ; *Src
  push ebp
  mov  ebp, [esp+12+12] ; BpS
  push ebx
  mov  edx, %1    ; Height
%endmacro
%macro LOOP1 0
  add esi, ebp  ; += BpS
  add edi, ebp  ; += BpS
  dec edx
%endmacro
%macro EPILOG1 0
  pop ebx
  pop ebp
  pop esi
  pop edi
%endmacro

%macro PROLOG2 1
  push edi
  mov  edi, [esp+4  +4]  ; *Dst
  push esi
  mov  esi, [esp+8  +8]  ; *Src
  mov  edx, %1    ; Height
%endmacro
%macro LOOP2 0
  add esi, [esp+8 +12]    ; +BpS
  add edi, [esp+8 +12]    ; +BpS
  dec edx
%endmacro
%macro EPILOG2 0
  pop esi
  pop edi
%endmacro

;//////////////////////////////////////////////////////////////////////
;// Block copy functions
;//////////////////////////////////////////////////////////////////////

%macro COPY_HH 2
  movzx eax, byte [esi+%1]
  movzx ecx, byte [esi+1+%1]
  lea eax, [eax+ecx]
  movzx ecx, byte [esi+ebp+%1]
  movzx ebx, byte [esi+ebp+%1+1]
  lea ecx, [ecx+ebx]
  lea eax, [eax+ecx+2 - %2]
  shr eax,2
  mov [edi+%1], al
%endmacro

align 16
Skl_Copy_8x8_HH_Rnd0_x86:
  PROLOG1 8
.Loop_y
  COPY_HH 0, 0
  COPY_HH 1, 0
  COPY_HH 2, 0
  COPY_HH 3, 0
  COPY_HH 4, 0
  COPY_HH 5, 0
  COPY_HH 6, 0
  COPY_HH 7, 0

  LOOP1
  jg .Loop_y
  EPILOG1
  ret

align 16
Skl_Copy_8x8_HH_Rnd1_x86:
  PROLOG1 8
.Loop_y1
  COPY_HH 0, 1
  COPY_HH 1, 1
  COPY_HH 2, 1
  COPY_HH 3, 1
  COPY_HH 4, 1
  COPY_HH 5, 1
  COPY_HH 6, 1
  COPY_HH 7, 1

  LOOP1
  jg .Loop_y1
  EPILOG1
  ret

align 16
Skl_Copy_16x8_HH_Rnd0_x86:
  PROLOG1 8
.Loop_y
  COPY_HH 0, 0
  COPY_HH 1, 0
  COPY_HH 2, 0
  COPY_HH 3, 0
  COPY_HH 4, 0
  COPY_HH 5, 0
  COPY_HH 6, 0
  COPY_HH 7, 0
  COPY_HH 8, 0
  COPY_HH 9, 0
  COPY_HH 10, 0
  COPY_HH 11, 0
  COPY_HH 12, 0
  COPY_HH 13, 0
  COPY_HH 14, 0
  COPY_HH 15, 0

  LOOP1
  jg .Loop_y
  EPILOG1
  ret

align 16
Skl_Copy_16x8_HH_Rnd1_x86:
  PROLOG1 8
.Loop_y1
  COPY_HH  0, 1
  COPY_HH  1, 1
  COPY_HH  2, 1
  COPY_HH  3, 1
  COPY_HH  4, 1
  COPY_HH  5, 1
  COPY_HH  6, 1
  COPY_HH  7, 1
  COPY_HH  8, 1
  COPY_HH  9, 1
  COPY_HH 10, 1
  COPY_HH 11, 1
  COPY_HH 12, 1
  COPY_HH 13, 1
  COPY_HH 14, 1
  COPY_HH 15, 1
  LOOP1
  jg .Loop_y1
  EPILOG1
  ret

;//////////////////////////////////////////////////////////////////////

%macro COPY_FH 2
  movzx ecx, byte [esi+%1+1]
  lea eax, [eax+ecx+1 - %2]
  shr eax,1
  mov [edi+%1], al

  movzx eax, byte [esi+%1+1 +1]
  lea ecx, [eax+ecx+1 - %2]
  shr ecx,1
  mov [edi+%1 +1], cl
%endmacro

align 16
Skl_Copy_8x8_FH_Rnd0_x86:
  PROLOG2 8
.Loop_y
  movzx eax, byte [esi]
  COPY_FH 0, 0
  COPY_FH 2, 0
  COPY_FH 4, 0
  COPY_FH 6, 0

  LOOP2
  jg .Loop_y
  EPILOG2
  ret

align 16
Skl_Copy_8x8_FH_Rnd1_x86:
  PROLOG2 8
.Loop_y1
  movzx eax, byte [esi]
  COPY_FH 0, 1
  COPY_FH 2, 1
  COPY_FH 4, 1
  COPY_FH 6, 1

  LOOP2
  jg .Loop_y1
  EPILOG2
  ret

align 16
Skl_Copy_16x8_FH_Rnd0_x86:
  PROLOG2 8
.Loop_y
  movzx eax, byte [esi]
  COPY_FH 0, 0
  COPY_FH 2, 0
  COPY_FH 4, 0
  COPY_FH 6, 0
  COPY_FH 8, 0
  COPY_FH 10, 0
  COPY_FH 12, 0
  COPY_FH 14, 0

  LOOP2
  jg .Loop_y
  EPILOG2
  ret


align 16
Skl_Copy_16x8_FH_Rnd1_x86:
  PROLOG2 8
.Loop_y1
  movzx eax, byte [esi]
  COPY_FH 0, 1
  COPY_FH 2, 1
  COPY_FH 4, 1
  COPY_FH 6, 1
  COPY_FH 8, 1
  COPY_FH 10, 1
  COPY_FH 12, 1
  COPY_FH 14, 1

  LOOP2
  jg .Loop_y1

  EPILOG2
  ret

;//////////////////////////////////////////////////////////////////////

%macro COPY_HF 2
  movzx eax, byte [esi+%1]
  movzx ecx, byte [esi+ebp+%1]
  lea eax, [eax+ecx+1 - %2]
  shr eax,1
  mov [edi+%1], al
%endmacro

align 16
Skl_Copy_8x8_HF_Rnd0_x86:
  PROLOG1 8
.Loop_y
  COPY_HF 0, 0
  COPY_HF 1, 0
  COPY_HF 2, 0
  COPY_HF 3, 0
  COPY_HF 4, 0
  COPY_HF 5, 0
  COPY_HF 6, 0
  COPY_HF 7, 0

  LOOP1
  jg .Loop_y

  EPILOG1
  ret

align 16
Skl_Copy_8x8_HF_Rnd1_x86:
  PROLOG1 8
.Loop_y1
  COPY_HF 0, 1
  COPY_HF 1, 1
  COPY_HF 2, 1
  COPY_HF 3, 1
  COPY_HF 4, 1
  COPY_HF 5, 1
  COPY_HF 6, 1
  COPY_HF 7, 1

  LOOP1
  jg .Loop_y1

  EPILOG1
  ret

align 16 
Skl_Copy_16x8_HF_Rnd0_x86:
  PROLOG1 8
.Loop_y
  COPY_HF 0, 0
  COPY_HF 1, 0
  COPY_HF 2, 0
  COPY_HF 3, 0
  COPY_HF 4, 0
  COPY_HF 5, 0
  COPY_HF 6, 0
  COPY_HF 7, 0
  COPY_HF 8, 0
  COPY_HF 9, 0
  COPY_HF 10, 0
  COPY_HF 11, 0
  COPY_HF 12, 0
  COPY_HF 13, 0
  COPY_HF 14, 0
  COPY_HF 15, 0

  LOOP1
  jg .Loop_y
  EPILOG1
  ret

align 16
Skl_Copy_16x8_HF_Rnd1_x86:
  PROLOG1 8
.Loop_y1
  COPY_HF 0, 1
  COPY_HF 1, 1
  COPY_HF 2, 1
  COPY_HF 3, 1
  COPY_HF 4, 1
  COPY_HF 5, 1
  COPY_HF 6, 1
  COPY_HF 7, 1
  COPY_HF 8, 1
  COPY_HF 9, 1
  COPY_HF 10, 1
  COPY_HF 11, 1
  COPY_HF 12, 1
  COPY_HF 13, 1
  COPY_HF 14, 1
  COPY_HF 15, 1

  LOOP1
  jg .Loop_y1

  EPILOG1
  ret

;//////////////////////////////////////////////////////////////////////

%macro COPY_FF 0
  fld qword [eax]
  fstp qword [edx]
  lea eax, [eax+ecx]
  fld qword [eax]
  lea eax, [eax+ecx]
  fstp qword [edx+ecx]
%endmacro

align 16
Skl_Copy_8x8_FF_x86:
  mov edx, [esp+4]  ; *Dst
  mov eax, [esp+8]  ; *Src
  mov ecx, [esp+12] ; BpS

  COPY_FF
  lea edx,[edx+2*ecx]
  COPY_FF
  lea edx,[edx+2*ecx]
  COPY_FF
  lea edx,[edx+2*ecx]
  COPY_FF

  ret


%macro COPY_FF2 0
  fld qword [eax]
  fstp qword [edx]
  fld qword [eax+8]
  fstp qword [edx+8]
  lea eax, [eax+ecx]
  fld qword [eax]
  fstp qword [edx+ecx]
  fld qword [eax+8]
  lea eax, [eax+ecx]
  fstp qword [edx+ecx+8]
%endmacro

align 16
Skl_Copy_16x8_FF_x86:
  mov edx, [esp+4]  ; *Dst
  mov eax, [esp+8]  ; *Src
  mov ecx, [esp+12] ; BpS

  COPY_FF2
  lea edx,[edx+2*ecx]
  COPY_FF2
  lea edx,[edx+2*ecx]
  COPY_FF2
  lea edx,[edx+2*ecx]
  COPY_FF2

  ret

;//////////////////////////////////////////////////////////////////////
;// Block addition functions
;//////////////////////////////////////////////////////////////////////

%macro ADD_HH 2   ; %1:offset, %2:rounding
  movzx eax, byte [esi+%1]
  movzx ecx, byte [esi+%1+1]
  lea eax, [eax+ecx]
  movzx ecx, byte [esi+ebp+%1]
  movzx ebx, byte [esi+ebp+%1+1]
  lea ecx, [ecx+ebx]
  lea eax, [eax+ecx+2 - %2]
  shr eax, 2
  movzx ebx, byte [edi+%1]
  lea eax, [eax+ebx+1]
  shr eax, 1
  mov [edi+%1], al
%endmacro

align 16
Skl_Add_8x8_HH_Rnd0_x86:
  PROLOG1 8
.Loop_y
  ADD_HH 0, 0
  ADD_HH 1, 0
  ADD_HH 2, 0
  ADD_HH 3, 0
  ADD_HH 4, 0
  ADD_HH 5, 0
  ADD_HH 6, 0
  ADD_HH 7, 0

  LOOP1
  jg .Loop_y

  EPILOG1
  ret

align 16
Skl_Add_16x8_HH_Rnd0_x86:
  PROLOG1 8
.Loop_y
  ADD_HH 0, 0
  ADD_HH 1, 0
  ADD_HH 2, 0
  ADD_HH 3, 0
  ADD_HH 4, 0
  ADD_HH 5, 0
  ADD_HH 6, 0
  ADD_HH 7, 0
  ADD_HH 8, 0
  ADD_HH 9, 0
  ADD_HH 10, 0
  ADD_HH 11, 0
  ADD_HH 12, 0
  ADD_HH 13, 0
  ADD_HH 14, 0
  ADD_HH 15, 0

  LOOP1
  jg .Loop_y
  EPILOG1
  ret

;//////////////////////////////////////////////////////////////////////

%macro ADD_FH 2 ; %1:offset, %2:rounding

  movzx ecx, byte [esi+%1+1]
  lea eax, [eax+ecx+1 - %2]
  movzx ebx, byte [edi+%1]
  shr eax,1
  lea ebx, [ebx+eax+1]
  shr ebx, 1
  mov [edi+%1], bl

  movzx eax, byte [esi+%1+1 +1]
  lea ecx, [eax+ecx+1 - %2]
  movzx ebx, byte [edi+%1 +1]
  shr ecx,1
  lea ebx, [ebx+ecx+1]
  shr ebx, 1
  mov [edi+%1 +1], bl

%endmacro

align 16
Skl_Add_8x8_FH_Rnd0_x86:
  PROLOG1 8
.Loop_y
  movzx eax, byte [esi]
  ADD_FH 0, 0
  ADD_FH 2, 0
  ADD_FH 4, 0
  ADD_FH 6, 0

  LOOP1
  jg .Loop_y
  EPILOG1
  ret

align 16
Skl_Add_16x8_FH_Rnd0_x86:
  PROLOG1 8
.Loop_y
  movzx eax, byte [esi]
  ADD_FH 0, 0
  ADD_FH 2, 0
  ADD_FH 4, 0
  ADD_FH 6, 0
  ADD_FH 8, 0
  ADD_FH 10, 0
  ADD_FH 12, 0
  ADD_FH 14, 0

  LOOP1
  jg .Loop_y
  EPILOG1
  ret

;//////////////////////////////////////////////////////////////////////

%macro ADD_HF 2 ; %1:offset, %2:rounding
  movzx eax, byte [esi+%1]
  movzx ebx, byte [esi+ebp+%1]
  lea eax,[eax+ebx+1 - %2]
  movzx ebx, byte [edi+%1]
  shr eax, 1
  lea ebx,[eax+ebx+1]
  shr ebx, 1
  mov [edi+%1], bl
%endmacro

align 16
Skl_Add_8x8_HF_Rnd0_x86:
  PROLOG1 8
.Loop_y
  ADD_HF 0, 0
  ADD_HF 1, 0
  ADD_HF 2, 0
  ADD_HF 3, 0
  ADD_HF 4, 0
  ADD_HF 5, 0
  ADD_HF 6, 0
  ADD_HF 7, 0

  LOOP1
  jg .Loop_y
  EPILOG1
  ret

align 16
Skl_Add_16x8_HF_Rnd0_x86:
  PROLOG1 8
.Loop_y
  ADD_HF 0, 0
  ADD_HF 1, 0
  ADD_HF 2, 0
  ADD_HF 3, 0
  ADD_HF 4, 0
  ADD_HF 5, 0
  ADD_HF 6, 0
  ADD_HF 7, 0
  ADD_HF 8, 0
  ADD_HF 9, 0
  ADD_HF 10, 0
  ADD_HF 11, 0
  ADD_HF 12, 0
  ADD_HF 13, 0
  ADD_HF 14, 0
  ADD_HF 15, 0

  LOOP1
  jg .Loop_y
  EPILOG1
  ret

;//////////////////////////////////////////////////////////////////////

%macro ADD_FF 1
  movzx eax, byte [esi+%1]
  movzx ecx, byte [edi+%1]
  lea eax, [eax+ecx+1]
  shr eax, 1
  mov [edi+%1], al
%endmacro

align 16
Skl_Add_8x8_FF_x86:

  PROLOG1 8

.Loop_y
  ADD_FF 0
  ADD_FF 1
  ADD_FF 2
  ADD_FF 3
  ADD_FF 4
  ADD_FF 5
  ADD_FF 6
  ADD_FF 7

  LOOP1
  jg .Loop_y

  EPILOG1
  ret

align 16
Skl_Add_16x8_FF_x86:

  PROLOG1 8

.Loop_y
  ADD_FF 0
  ADD_FF 1
  ADD_FF 2
  ADD_FF 3
  ADD_FF 4
  ADD_FF 5
  ADD_FF 6
  ADD_FF 7
  ADD_FF 8
  ADD_FF 9
  ADD_FF 10
  ADD_FF 11
  ADD_FF 12
  ADD_FF 13
  ADD_FF 14
  ADD_FF 15

  LOOP1
  jg .Loop_y

  EPILOG1
  ret

;//////////////////////////////////////////////////////////////////////
