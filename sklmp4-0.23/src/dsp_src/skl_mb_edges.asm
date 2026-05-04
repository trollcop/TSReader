;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
;//////////////////////////////////////////////////////////////////////
;// MMX/x86 versions of
;//  void Skl_Make_Edges(SKL_BYTE **YUV, const int Width,
;//                      const int Height, const int BpS)
;//////////////////////////////////////////////////////////////////////
; [BITS 32]

%include "../../include/skl_syst/skl_nasm.h"

globl Skl_Make_Edges_MMX
globl Skl_Make_Edges_SSE
globl Skl_Make_Edges_x86

;//////////////////////////////////////////////////////////////////////

TEXT

%macro B_TO_Q_MMX 4   ; %1:SrcLeft, %2:Width, %3:mm#0, %4:mm#1
    ; replicate left and right bytes into quardwords
  movq %3, [%1]
  punpcklbw %3,%3
  movq %4, [%1+%2-8]    ; <- -8 offset: we align Right read
  punpcklwd %3,%3
  punpckhbw %4,%4
  punpckldq %3,%3
  punpckhwd %4,%4
  punpckhdq %4,%4
%endmacro

%macro B_TO_Q_SSE 4   ; %1:SrcLeft, %2:Width, %3:mm#0, %4:mm#1
    ; replicate left and right bytes into quardwords
  movq %4, [%1+%2-8]    ; <- -8 offset: we align Right read
  movq %3, [%1]
  punpckhbw %4,%4
  punpcklbw %3,%3
  pshufw %4,%4,0xff
  pshufw %3,%3,0x00
%endmacro

%macro MAIN_MMX_SSE_LOOP 1    ; %1: B_TO_Q macro to use

  push ebx
  push esi
  push edi
  push ebp
  mov esi, [esp+ 4 +16]  ; YVU
  mov ebp, [esp+16 +16]  ; BpS

    ; //// Y border loops ////

      ; 1) Left/Right      

  mov edx, [esp+ 8 +16]  ; Width
  mov edi, [esp+12 +16]  ; Height
  mov ebx, [esi +0]      ; Y
  jmp .Loop_LR_Y

align 16
.Loop_LR_Y
  %1 ebx, edx, mm0, mm1 ; B_TO_Q
  dec edi
  movq [ebx-16], mm0
  movq [ebx+edx], mm1
  movq [ebx- 8], mm0
  movq [ebx+edx+8], mm1
  jle .Out_LR_Y
  lea  ebx, [ebx+ebp]
  jmp .Loop_LR_Y

.Out_LR_Y

      ; 2) Bottom Y

  ; ebx is correctly positioned

  lea ebx, [ebx+edx]
  mov eax, 16
  mov ecx, ebx
  mov edi, -16
  sub edi, edx    ; edi=-16-Width

  mov edx, edi
  lea ebx, [ebx+ebp]
  jmp .Loop_B_Y

align 16
.Loop_B_Y
  add edx, 16
  movq mm0, [ecx+edx-16  ]
  movq mm1, [ecx+edx-16+8]
  movq [ebx+edx-16  ], mm0
  movq [ebx+edx-16+8], mm1
  jle .Loop_B_Y
  dec eax
  mov edx, edi
  jle .Out_B_Y
  lea ebx, [ebx+ebp]
  jmp .Loop_B_Y

.Out_B_Y:

    ; 3) Top Y

  mov eax, [esp+ 8 +16]
  xor ebp, -1
  mov ebx, [esi+0]
  inc ebp       ; ebp = -BpS

  add ebx, eax  ; Y+Width
  mov eax, 16
  lea ecx, [ebx+ebp]
  jmp .Loop_T_Y

align 16
.Loop_T_Y
  add edx, 16
  movq mm0, [ebx+edx-16  ]
  movq mm1, [ebx+edx-16+8]
  movq [ecx+edx-16  ], mm0
  movq [ecx+edx-16+8], mm1
  jle .Loop_T_Y
  dec eax  
  jle .Out_T_Y
  mov edx, edi  
  lea ecx, [ecx+ebp]
  jmp .Loop_T_Y

.Out_T_Y


    ; //// UV border loops ////

      ; 1) left/right  

  mov esi, [esp+ 4 +16]  ; YVU
  mov edx, [esp+ 8 +16]  ; Width
  mov edi, [esp+12 +16]  ; Height
  mov ebp, [esp+16 +16]  ; BpS
  shr edx, 1             ; width/=2
  shr edi, 1             ; Height/2
  mov ebx, [esi +4]      ; U
  mov ecx, [esi +8]      ; V
  jmp .Skip_LR_UV

align 16
.Loop_LR_UV
  lea  ebx, [ebx+ebp]
  lea  ecx, [ecx+ebp]
.Skip_LR_UV 
  %1 ebx, edx, mm0, mm1   ; <-B_TO_Q
  %1 ecx, edx, mm2, mm3   ; <-B_TO_Q
  dec edi  
  movq [ebx-8], mm0
  movq [ebx+edx+0], mm1
  movq [ecx-8], mm2
  movq [ecx+edx+0], mm3
  jg .Loop_LR_UV

    ; 2) Bottom UV 

  ; ebx/ecx are correctly positioned

  lea ebx, [ebx+edx]
  lea ecx, [ecx+edx]
  mov edi, ecx
  mov esi, ebx
  mov eax,-16
  sub eax, edx    ; eax = -16-Width/2
  mov edx, 8
  push eax
.Loop_B_UV_y
  mov eax, [esp]
  lea ebx, [ebx+ebp]
  lea ecx, [ecx+ebp]
  jmp .Loop_B_UV

align 16
.Loop_B_UV
  add eax, 8
  movq mm0, [esi+eax]
  movq mm1, [edi+eax]
  movq [ebx+eax], mm0
  movq [ecx+eax], mm1
  jl .Loop_B_UV
  dec edx
  jle .Out_B_UV
  jmp .Loop_B_UV_y
.Out_B_UV:

    ; 3) Top UV

  mov esi, [esp+ 4 +16+4]  ; YVU
  mov edx, [esp+ 8 +16+4]  ; Width
  mov edi, [esi+4]  ; U
  mov esi, [esi+8]  ; V
  shr edx, 1
  lea edi, [edi+edx]
  lea esi, [esi+edx]
  xor ebp, -1
  mov ebx, esi
  inc ebp       ; ebp = -BpS
  mov ecx, edi
  mov edx, 8

.Loop_T_UV_y
  mov eax, [esp]
  lea ebx, [ebx+ebp]
  lea ecx, [ecx+ebp]
  jmp .Loop_T_UV

align 16
.Loop_T_UV
  add eax, 8
  movq mm0, [esi+eax]
  movq mm1, [edi+eax]
  movq [ebx+eax], mm0
  movq [ecx+eax], mm1
  jl .Loop_T_UV
  dec edx
  jg .Loop_T_UV_y

  pop eax


  pop ebp
  pop edi
  pop esi
  pop ebx

%endmacro

;//////////////////////////////////////////////////////////////////////

align 16
Skl_Make_Edges_MMX: ; approx.: ~20200c@512x384, ~21500c@640x352, ~13460c@352x240
  MAIN_MMX_SSE_LOOP B_TO_Q_MMX
  ret

align 16
Skl_Make_Edges_SSE: ; approx.: ~20200c@512x384, ~21500c@640x352, ~13460c@352x240
  MAIN_MMX_SSE_LOOP B_TO_Q_SSE
  ret

;//////////////////////////////////////////////////////////////////////

%macro B_TO_DW_X86 3   ; %1:Src  %2/%3: tmp reg
    ; replicate byte into dword
  mov al, [%1]
  mov ah, al
  mov %2, eax
  shl eax,16
  mov ax, %3
%endmacro

align 16
Skl_Make_Edges_x86:

  push ebx
  push esi
  push edi
  push ebp
  mov esi, [esp+ 4 +16]  ; YVU
  mov ebp, [esp+16 +16]  ; BpS

    ; //// Y border loops ////

      ; 1) Left/Right      

  mov edx, [esp+ 8 +16]  ; Width
  mov edi, [esp+12 +16]  ; Height
  mov ebx, [esi +0]      ; Y
  jmp .Loop_LR_Y

align 16
.Loop_LR_Y
  B_TO_DW_X86 ebx, ecx, cx
  mov [ebx-16], eax
  mov [ebx-12], eax
  mov [ebx-8], eax
  mov [ebx-4], eax
  B_TO_DW_X86 ebx+edx-1, ecx, cx
  mov [ebx+edx], eax
  mov [ebx+edx+4], eax
  mov [ebx+edx+8], eax
  mov [ebx+edx+12], eax
  dec edi
  jle .Out_LR_Y
  lea  ebx, [ebx+ebp]
  jmp .Loop_LR_Y

.Out_LR_Y

      ; 2) Bottom Y

  ; ebx is correctly positioned

  lea ebx, [ebx+edx]
  mov eax, 16
  mov ecx, ebx
  mov edi, -16
  sub edi, edx    ; edi=-16-Width

  mov edx, edi
  lea ebx, [ebx+ebp]
  jmp .Loop_B_Y

align 16
.Loop_B_Y
  add edx, 16
  fld qword [ecx+edx-16  ]
  fld qword [ecx+edx-16+8]
  fxch st1
  fstp qword [ebx+edx-16  ]
  fstp qword [ebx+edx-16+8]
  jle .Loop_B_Y
  dec eax
  mov edx, edi
  jle .Out_B_Y
  lea ebx, [ebx+ebp]
  jmp .Loop_B_Y

.Out_B_Y:

    ; 3) Top Y

  mov eax, [esp+ 8 +16]
  xor ebp, -1
  mov ebx, [esi+0]
  inc ebp       ; ebp = -BpS

  add ebx, eax  ; Y+Width
  mov eax, 16
  lea ecx, [ebx+ebp]
  jmp .Loop_T_Y

align 16
.Loop_T_Y
  add edx, 16
  fld qword [ebx+edx-16  ]
  fld qword [ebx+edx-16+8]
  fxch st1
  fstp qword [ecx+edx-16  ]
  fstp qword [ecx+edx-16+8]
  jle .Loop_T_Y
  dec eax  
  jle .Out_T_Y
  mov edx, edi  
  lea ecx, [ecx+ebp]
  jmp .Loop_T_Y

.Out_T_Y

    ; //// UV border loops ////

      ; 1) left/right  

  mov esi, [esp+ 4 +16]  ; YVU
  mov edx, [esp+ 8 +16]  ; Width
  mov edi, [esp+12 +16]  ; Height
  mov ebp, [esp+16 +16]  ; BpS
  shr edx, 1             ; width/=2
  shr edi, 1             ; Height/2
  mov ebx, [esi +4]      ; U
  mov ecx, [esi +8]      ; V
  jmp .Skip_LR_UV

align 16
.Loop_LR_UV
  lea  ebx, [ebx+ebp]
  lea  ecx, [ecx+ebp]
.Skip_LR_UV 
  B_TO_DW_X86 ebx, esi, si
  mov [ebx-8], eax
  mov [ebx-4], eax
  B_TO_DW_X86 ebx+edx-1, esi, si
  mov [ebx+edx], eax
  mov [ebx+edx+4], eax
  B_TO_DW_X86 ecx, esi, si
  mov [ecx-8], eax
  mov [ecx-4], eax
  B_TO_DW_X86 ecx+edx-1, esi, si
  mov [ecx+edx], eax
  mov [ecx+edx+4], eax
  dec edi
  jg .Loop_LR_UV


    ; 2) Bottom UV 

  ; ebx/ecx are correctly positioned

  lea ebx, [ebx+edx]
  lea ecx, [ecx+edx]
  mov edi, ecx
  mov esi, ebx
  mov eax,-16
  sub eax, edx    ; eax = -16-Width/2
  mov edx, 8
  push eax
.Loop_B_UV_y
  mov eax, [esp]
  lea ebx, [ebx+ebp]
  lea ecx, [ecx+ebp]
  jmp .Loop_B_UV

align 16
.Loop_B_UV
  add eax, 8
  fld qword [esi+eax]
  fld qword [edi+eax]
  fxch st1
  fstp qword [ebx+eax]
  fstp qword [ecx+eax]
  jl .Loop_B_UV
  dec edx
  jle .Out_B_UV
  jmp .Loop_B_UV_y
.Out_B_UV:

    ; 3) Top UV

  mov esi, [esp+ 4 +16+4]  ; YVU
  mov edx, [esp+ 8 +16+4]  ; Width
  mov edi, [esi+4]  ; U
  mov esi, [esi+8]  ; V
  shr edx, 1
  lea edi, [edi+edx]
  lea esi, [esi+edx]
  xor ebp, -1
  mov ebx, esi
  inc ebp       ; ebp = -BpS
  mov ecx, edi
  mov edx, 8

.Loop_T_UV_y
  mov eax, [esp]
  lea ebx, [ebx+ebp]
  lea ecx, [ecx+ebp]
  jmp .Loop_T_UV

align 16
.Loop_T_UV
  add eax, 8
  fld qword [esi+eax]
  fld qword [edi+eax]
  fxch st1
  fstp qword [ebx+eax]
  fstp qword [ecx+eax]
  jl .Loop_T_UV
  dec edx
  jg .Loop_T_UV_y

  pop eax


  pop ebp
  pop edi
  pop esi
  pop ebx

  ret

;//////////////////////////////////////////////////////////////////////
