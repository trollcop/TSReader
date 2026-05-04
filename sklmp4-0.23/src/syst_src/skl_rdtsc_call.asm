;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
;
; calls for RDTSC timing
;
;//////////////////////////////////////////////////////////////////////////

%include "../../include/skl_syst/skl_nasm.h"

globl SKL_RDTSC_0_ASM
globl SKL_RDTSC_1_ASM
globl SKL_RDTSC_2_ASM
globl SKL_RDTSC_Get

DATA

extrn Skl_RCount_
extrn Skl_Tics_
extrn Skl_Cur_Count_
extrn Skl_EAX_In_
extrn Skl_EBX_In_
extrn Skl_ECX_In_
extrn Skl_EDX_In_
extrn Skl_EDI_In_
extrn Skl_ESI_In_
extrn Skl_EBP_In_
extrn Skl_ESP_In_
extrn Skl_EAX_Out_
extrn Skl_EBX_Out_
extrn Skl_ECX_Out_
extrn Skl_EDX_Out_
extrn Skl_EDI_Out_
extrn Skl_ESI_Out_
extrn Skl_EBP_Out_
extrn Skl_ESP_Out_
extrn Skl_f_In_
extrn Skl_f_Out_

SKL_RDTSC_0_ASM:

   mov [Skl_EAX_In_],eax
   mov [Skl_EBX_In_],ebx
   mov [Skl_ECX_In_],ecx
   mov [Skl_EDX_In_],edx
   mov [Skl_EDI_In_],edi
   mov [Skl_ESI_In_],esi
   mov [Skl_EBP_In_],ebp
   mov dword [Skl_ESP_In_],0x00000000
   fstp dword [Skl_f_In_ + 0*4]
   fstp dword [Skl_f_In_ + 1*4]
   fstp dword [Skl_f_In_ + 2*4]
   fstp dword [Skl_f_In_ + 3*4]
   fstp dword [Skl_f_In_ + 4*4]
   fstp dword [Skl_f_In_ + 5*4]
   fstp dword [Skl_f_In_ + 6*4]
   fstp dword [Skl_f_In_ + 7*4]
   mov dword [Skl_Cur_Count_], SKL_RDTSC_OFFSET
   ret

SKL_RDTSC_1_ASM:
   mov ebx, [Skl_EBX_In_]
   mov ecx, [Skl_ECX_In_]
   mov edi, [Skl_EDI_In_]
   mov esi, [Skl_ESI_In_]
   mov ebp, [Skl_EBP_In_]
   fcompp
   fcompp
   fcompp
   fcompp
   fld dword [Skl_f_In_ + 7*4]
   fld dword [Skl_f_In_ + 6*4]
   fld dword [Skl_f_In_ + 5*4]
   fld dword [Skl_f_In_ + 4*4]
   fld dword [Skl_f_In_ + 3*4]
   fld dword [Skl_f_In_ + 2*4]
   fld dword [Skl_f_In_ + 1*4]
   fld dword [Skl_f_In_ + 0*4]
;   xor eax,eax
   db 0x0f,0x31
   mov [Skl_Tics_], eax
   mov eax, [Skl_EAX_In_]
   mov edx, [Skl_EDX_In_]
   cld
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   nop
   ret

SKL_RDTSC_2_ASM:
   mov [Skl_EAX_Out_],eax
   mov [Skl_EDX_Out_],edx
   clc
   db 0x0f,0x31
   sub eax,[Skl_Tics_]
   mov edx,[Skl_Cur_Count_]
   mov [Skl_RCount_+edx*4],eax
   fstp dword [Skl_f_Out_ + 0*4]
   fstp dword [Skl_f_Out_ + 1*4]
   fstp dword [Skl_f_Out_ + 2*4]
   fstp dword [Skl_f_Out_ + 3*4]
   fstp dword [Skl_f_Out_ + 4*4]
   fstp dword [Skl_f_Out_ + 5*4]
   fstp dword [Skl_f_Out_ + 6*4]
   fstp dword [Skl_f_Out_ + 7*4]
   mov [Skl_EBX_Out_],ebx
   mov [Skl_ECX_Out_],ecx
   mov [Skl_EDI_Out_],edi
   mov [Skl_ESI_Out_],esi
   mov [Skl_EBP_Out_],ebp
   mov [Skl_ESP_Out_],esp
   ret

;//////////////////////////////////////////////////////////////////////////
;SKL_RDTSC_Get:
;  push eax
;  push edx
;  db 0x0f,0x31
;  mov [Skl_EAX_In_], eax
;  mov [Skl_EDX_In_], edx
;  pop edx
;  pop eax
;  ret

