;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
;
; CPU detection
;
;//////////////////////////////////////////////////////////////////////////

%include "../../include/skl_syst/skl_nasm.h"

globl Skl_Detect_CPUID
globl Skl_Detect_MMX
globl Skl_Detect_SSE
globl Skl_Detect_SSE2
globl Skl_Detect_3DNOW
globl Skl_Detect_3DNOWEXT
globl Skl_Get_CPUID
globl Skl_Get_ModelID

TEXT

;//////////////////////////////////////////////////////////////////////////

Skl_Detect_CPUID: ; tests if ID bit #21 is flippable
  push ebx
  xor eax,eax
  pushf
  pop ebx
  mov ecx, ebx
  xor ebx, (1<<21) ; reverse bit #21 (ID)
  push ebx
  popf
  xor ebx,ecx ; did the bit went back to its original value?
  jz .Ok      ; yep...
  inc eax
.Ok:
  push ecx
  popf
  pop ebx
  ret

;//////////////////////////////////////////////////////////////////////////

Detect_Error:
  xor eax,eax
Detect_Ok:
  pop ebx
  ret

;//////////////////////////////////////////////////////////////////////////

Skl_Detect_MMX:
  push ebx
  mov eax,1
  cpuid
  test edx, (1<<23) ; bit 23
  jz Detect_Error
  jnz Detect_Ok

;//////////////////////////////////////////////////////////////////////////

Skl_Detect_SSE:
  push ebx
  mov eax,1
  cpuid
  test edx, (1<<25) ; bit 25
  jmp SSE_Test

Skl_Detect_SSE2:
  push ebx
  mov eax,1
  cpuid
  test edx, (1<<26) ; bit 26
SSE_Test:
  jz Detect_Error
  xorps xmm0, xmm0	; this may throw an interrupt! (caught higher)
  jmp Detect_Ok


;//////////////////////////////////////////////////////////////////////////

Skl_Detect_3DNOW:
  push ebx
  mov eax, 0x80000001
  cpuid
  cmp eax, 0x80000000
  jbe Detect_Error
  mov eax, 0x80000001
  cpuid
  test edx, 0x80000000  ; bit 31
  jz Detect_Error
  jnz Detect_Ok

Skl_Detect_3DNOWEXT:
  push ebx
  mov eax, 0x80000001
  cpuid
  cmp eax, 0x80000000
  jbe Detect_Error
  mov eax, 0x80000001
  cpuid
  test edx, 0x40000000  ; bit 30
  jz Detect_Error
  jnz Detect_Ok

;//////////////////////////////////////////////////////////////////////////

Skl_Get_CPUID:
  push edx
  push ebx
  xor eax,eax
  cpuid
  mov eax,[esp+4+8]
  mov [eax+0], ebx
  mov [eax+4], edx
  mov [eax+8], ecx
  mov byte [eax+12], 0
  pop ebx
  pop edx
  ret

Skl_Get_ModelID:
  push edx
  push ebx
  push edi
  mov edi, [esp+4+12]

  mov eax,0x80000000
  cpuid
  cmp eax,0x80000004
  jb .Not_Ok
  mov eax,0x80000002
  cpuid
  mov [edi+ 0], eax
  mov [edi+ 4], ebx
  mov [edi+ 8], ecx
  mov [edi+12], edx
  add edi,16
  mov eax,0x80000003
  cpuid
  mov [edi+ 0], eax
  mov [edi+ 4], ebx
  mov [edi+ 8], ecx
  mov [edi+12], edx
  add edi,16
  mov eax,0x80000004
  cpuid
  mov [edi+ 0], eax
  mov [edi+ 4], ebx
  mov [edi+ 8], ecx
  mov [edi+12], edx
  add edi,16
.Not_Ok:
  mov byte [edi],0
.End:
  pop edi
  pop ebx
  pop edx
  ret

;//////////////////////////////////////////////////////////////////////////
