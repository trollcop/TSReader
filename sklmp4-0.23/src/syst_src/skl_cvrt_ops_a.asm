;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *       
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
;//////////////////////////////////////////////////////////////////////////
;// export globals
;//////////////////////////////////////////////////////////////////////////

%include "../../include/skl_syst/skl_nasm.h"

globl Skl_RGB8_To_32_ASM
globl Skl_RGB8_To_24_ASM
globl Skl_RGB8_To_16_ASM
globl Skl_RGB8_To_8_ASM

globl Skl_RGB16_To_32_ASM
globl Skl_RGB16_To_24_ASM
globl Skl_RGB16_To_16_ASM
globl Skl_RGB16_To_8_ASM

globl Skl_RGB24_To_32_ASM
globl Skl_RGB24_To_24_ASM
globl Skl_RGB24_To_16_ASM
globl Skl_RGB24_To_8_ASM

globl Skl_RGB32_To_32_ASM
globl Skl_RGB32_To_24_ASM
globl Skl_RGB32_To_16_ASM
globl Skl_RGB32_To_8_ASM

TEXT

%macro PROLOG 2   ; %1: Dst mult, %2: Src Mult
  push ebp
  push edi
  push esi
  push ebx

	mov edi, [esp+16+4 + 0]   ; Dst
	mov esi, [esp+16+4 + 8]   ; Src
	mov edx, [esp+16+4 +16]   ; W

%if (%1==3)
  lea eax, [edx + 2*edx]
  add edi, eax
%elif (%1!=0)
	lea edi, [edi + %1*edx]
%endif

%if (%2==3)
  lea eax, [edx + 2*edx]
  add esi , eax
%elif (%2!=0)
	lea esi, [esi + %2*edx]
%endif

  neg edx
	mov ebp, [esp+16+4 +24]   ; Map
	mov ecx, [esp+16+4 +20]   ; H

  push edx

.Loop_H
  push edi
  push esi
  push ecx
  mov ecx, [esp+12]    ; ecx = -W

%endmacro

%macro EPILOG 0
.Next
  pop ecx
  pop esi
  pop edi
  dec ecx   ; H-->0
  jle .End 
  add edi, [esp+16+8 +4]  ; Dst+=Dst_BpS
  add esi, [esp+16+8 +12] ; Src+=Src_BpS
  jmp .Loop_H

.End
  pop edx   ; pops -W
	pop ebx
  pop esi
	pop edi
	pop ebp
	ret
%endmacro

%define T1 ebp+0*1024
%define T2 ebp+1*1024
%define T3 ebp+2*1024
%define T4 ebp+3*1024

;//////////////////////////////////////////////////////////////////////////
;// Skl_RGB8_To_*
;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB8_To_8_ASM:

  PROLOG 1,1
.Loop:
   movzx eax, byte [esi+ecx]
   inc ecx
   mov ebx, [T1+4*eax]
   mov [edi+ecx-1],bl
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB8_To_16_ASM:

  PROLOG 2,1
  
.Loop
   movzx eax, byte [esi+ecx]
   inc ecx
   mov ebx, [T1+4*eax]
   mov [edi+2*ecx-2],bx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB8_To_24_ASM:

  PROLOG 0,1

.Loop:
   add edi, 3
   movzx eax, byte [esi+ecx]
   mov ebx, [T1+4*eax]
   mov [edi-3], bx
   shr ebx, 16
   mov [edi-1], bl
   inc ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB8_To_32_ASM:

  PROLOG 4,1

.Loop:
   movzx eax, byte [esi+ecx]
   inc ecx
   mov ebx, [T1+4*eax]
   mov [edi+4*ecx-4],ebx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////
;// Skl_RGB16_To_*
;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB16_To_8_ASM:

  PROLOG 1,2

.Loop:
   movzx eax, byte [esi+2*ecx+0]
   movzx edx, byte [esi+2*ecx+1]
   mov ebx, [T1+4*eax]
   inc ecx
   or ebx, [T2+4*edx]
   mov [edi+ecx-1],bl
   test ecx,ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB16_To_16_ASM:

  PROLOG 2,2

.Loop
   movzx eax, byte [esi+2*ecx+0]
   movzx edx, byte [esi+2*ecx+1]
   mov ebx, [T1+4*eax]
   or ebx, [T2+4*edx]
   mov [edi+2*ecx],bx
   inc ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB16_To_24_ASM:

  PROLOG 0,2

.Loop:
   add edi, 3
   movzx eax, byte [esi+2*ecx+0]
   mov ebx, [T1+4*eax]
   movzx eax, byte [esi+2*ecx+1]
   or ebx, [T2+4*eax]
   mov [edi-3], bx
   shr ebx, 16
   mov [edi-1], bl
   inc ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB16_To_32_ASM:

  PROLOG 4,2

.Loop:
   movzx eax, byte [esi+2*ecx+0]
   movzx edx, byte [esi+2*ecx+1]
   mov ebx, [T1+4*eax]
   inc ecx
   or ebx, [T2+4*edx]
   mov [edi+4*ecx-4],ebx
   test ecx,ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////
;// Skl_RGB24_To_*
;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB24_To_8_ASM:

  PROLOG 1,0

.Loop:
   movzx eax, byte [esi]
   movzx edx, byte [esi+1]
   mov ebx, [T1+4*eax]
   movzx eax, byte [esi+2]
   or ebx, [T2+4*edx]
   add esi, 3
   or ebx, [T3+4*eax]
   mov [edi+ecx],bl
   inc ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB24_To_16_ASM:

  PROLOG 2,0

.Loop
   movzx eax, byte [esi]
   movzx edx, byte [esi+1]
   mov ebx, [T1+4*eax]
   movzx eax, byte [esi+2]
   or ebx, [T2+4*edx]
   add esi, 3
   or ebx, [T3+4*eax]
   mov [edi+2*ecx],bx
   inc ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB24_To_24_ASM:

  PROLOG 0,0

.Loop:
   add edi, 3
   movzx eax, byte [esi]
   movzx edx, byte [esi+1]
   mov ebx, [T1+4*eax]
   movzx eax, byte [esi+2]
   or ebx, [T2+4*edx]
   add esi, 3
   or ebx, [T3+4*eax]
   mov [edi-3], bx
   shr ebx, 16
   mov [edi-1], bl
   inc ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB24_To_32_ASM:

  PROLOG 4,0

.Loop:
   movzx eax, byte [esi]
   movzx edx, byte [esi+1]
   mov ebx, [T1+4*eax]
   movzx eax, byte [esi+2]
   add esi, 3
   or ebx, [T2+4*edx]
   or ebx, [T3+4*eax]
   mov [edi+4*ecx],ebx
   inc ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////
;// Skl_RGB32_To_*
;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB32_To_8_ASM:

  PROLOG 1,4

.Loop:
   movzx eax, byte [esi+4*ecx+0]
   movzx edx, byte [esi+4*ecx+1]
   mov ebx, [T1+4*eax]
   movzx eax, byte [esi+4*ecx+2]
   or ebx, [T2+4*edx]
   movzx edx, byte [esi+4*ecx+3]
   or ebx, [T3+4*eax]
   or ebx, [T4+4*edx]
   mov [edi+ecx],bl
   inc ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB32_To_16_ASM:

  PROLOG 2,4
.Loop
   movzx eax, byte [esi+4*ecx+0]
   movzx edx, byte [esi+4*ecx+1]
   mov ebx, [T1+4*eax]
   movzx eax, byte [esi+4*ecx+2]
   or ebx, [T2+4*edx]
   movzx edx, byte [esi+4*ecx+3]
   or ebx, [T3+4*eax]
   or ebx, [T4+4*edx]
   mov [edi+2*ecx], bx
   inc ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB32_To_24_ASM:

  PROLOG 0,4

.Loop:
   add edi, 3
   movzx eax, byte [esi+4*ecx+0]
   movzx edx, byte [esi+4*ecx+1]
   mov ebx, [T1+4*eax]
   movzx eax, byte [esi+4*ecx+2]
   or ebx, [T2+4*edx]
   movzx edx, byte [esi+4*ecx+3]
   or ebx, [T3+4*eax]
   inc ecx
   or ebx, [T4+4*edx]   
   mov [edi-3], bx
   shr ebx, 16
   mov [edi-1], bl
   test ecx, ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////

align 16
Skl_RGB32_To_32_ASM:

  PROLOG 4,4

.Loop:
   movzx eax, byte [esi+4*ecx+0]
   movzx edx, byte [esi+4*ecx+1]
   mov ebx, [T1+4*eax]
   movzx eax, byte [esi+4*ecx+2]
   or ebx, [T2+4*edx]
   movzx edx, byte [esi+4*ecx+3] 
   or ebx, [T3+4*eax]
   inc ecx
   or ebx, [T4+4*edx]
   mov [edi+4*ecx-4],ebx
   test ecx, ecx
   jl .Loop

  EPILOG

;//////////////////////////////////////////////////////////////////////////
