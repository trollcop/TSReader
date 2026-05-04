;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
;//////////////////////////////////////////////////////////////////////
;// YUV <-> RGB. 
;//////////////////////////////////////////////////////////////////////
; [BITS 32]

%include "../../include/skl_syst/skl_nasm.h"

globl Skl_RGB565_To_YUV_MMX
globl Skl_YUV_To_RGB565_MMX
globl Skl_RGB32_To_YUV_MMX
globl Skl_YUV_To_RGB32_MMX
globl Skl_YUV_To_RGB24_MMX

extrn Skl_YUV_Tab32_MMX    ; 6k
extrn Skl_RGB32_To_565_MMX ; 6k

;  Signatures:
;RGB -> YUV:
;    SKL_BYTE *Y, SKL_BYTE *U, SKL_BYTE *V,
;    const int Dst_BpS,
;    const SKL_BYTE *Src, const int Src_BpS,
;    const int Width, const int Height
;YUV -> RGB:
;    SKL_BYTE *RGB, const int Dst_BpS,
;    const SKL_BYTE *Y, const SKL_BYTE *U, const SKL_BYTE *V,
;    const int Src_BpS,
;    const int Width, const int Height

;//////////////////////////////////////////////////////////////////////

TEXT

Skl_RGB565_To_YUV_MMX:
  mov eax,[0] ; abort
  ret

;//////////////////////////////////////////////////////////////////////

Skl_YUV_To_RGB565_MMX:

  ; eax: *U
  ; ebx: *V
  ; esi: *Y
  ; edx: Src_BpS
  ; edi: *Dst
  ; ebx: Dst_BpS
  ; ecx: Counter

%define RGBp  esp+20
%define Yp    esp+16
%define Up    esp+12
%define Vp    esp+8
%define xCnt  esp+4
%define yCnt  esp+0

%macro TO_565 2  ; %1:src, %2:dst
  movd ebx, %1
  psrlq %1, 32
  movzx eax, bl
  movd %2, [Skl_RGB32_To_565_MMX+0*258*8 + eax*8+4]
  movzx eax, bh
  por %2, [Skl_RGB32_To_565_MMX+1*258*8 + eax*8+4]
  shr ebx,16
  por %2, [Skl_RGB32_To_565_MMX+2*258*8 + ebx*8+4]
  movd ebx, %1
  movzx eax, bl
  por %2, [Skl_RGB32_To_565_MMX+0*258*8 + eax*8+0]
  movzx eax, bh
  por %2, [Skl_RGB32_To_565_MMX+1*258*8 + eax*8+0]
  shr ebx,16
  por %2, [Skl_RGB32_To_565_MMX+2*258*8 + ebx*8+0]
%endmacro

  push ebx
  push esi
  push edi
  push ebp

  mov edi, [esp+4  +16] ; RGB
  mov ebp, [esp+12 +16] ; Y
  mov eax, [esp+16 +16] ; U
  mov ebx, [esp+20 +16] ; V
  mov edx, [esp+24 +16] ; Src_BpS
  mov ecx, [esp+28 +16] ; Width

  lea edi, [edi+2*ecx]  ; RGB += Width*sizeof(16b)
  lea ebp, [ebp+ecx]    ; ebp: Y1 = Y + Width
  add edx, ebp          ; edx: Y2 = Y1+ BpS
  push edi
  push ebp
  shr ecx, 1            ; Width/=2
  lea eax, [eax+ecx]    ; U += W/2
  lea ebx, [ebx+ecx]    ; V += W/2
  push eax
  push ebx

  neg ecx               ; ecx = -Width/2
  push ecx              ; save [xCnt]
  push eax              ; fake
  
  mov ecx, [esp+32 +40] ; Height
  shr ecx, 1            ; /2

  mov esi, [Up]
  mov edi, [Vp]

  jmp .Go

align 16
.Loop_y
  dec ecx
  jg .Add

  add esp, 24   ; rid of all tmp
  pop ebp
  pop edi
  pop esi
  pop ebx

  ret

align 16
.Add
  mov edi, [esp+8 +40]  ; Dst_BpS
  mov esi, [esp+24 +40] ; Src_BpS
  mov edx, [RGBp]
  mov ebp, [Yp]
  lea edx, [edx+2*edi]  ; RGB += 2*Dst_BpS
  lea ebp, [ebp+2*esi]  ; Y   += 2*Src_BpS
  mov [RGBp], edx
  mov edi, [Vp]
  mov [Yp], ebp         ; Y1
  lea edx, [ebp+esi]    ; Y2

  lea edi, [edi+esi]    ; V += Src_BpS
  add esi, [Up]         ; U += Src_BpS
  mov [Vp], edi
  mov [Up], esi

.Go
  mov [yCnt], ecx
  mov ecx, [xCnt]
  

    ; 12664c@640x480

;align 4
.Loop_x   ; edi,esi: U,V;  ebp,edx: Y1, Y2;  ecx: xCnt

    ; R = Y +       a.U
    ; G = Y + c.V + b.U
    ; B = Y + d.V

  movzx eax, byte [edi+ecx+0]
  movzx ebx, byte [esi+ecx+0]
  movq  mm0, [Skl_YUV_Tab32_MMX+0*2048 + eax*8]
  movzx eax, byte [edi+ecx+1]
  paddw mm0, [Skl_YUV_Tab32_MMX+1*2048 + ebx*8]
  movzx ebx, byte [esi+ecx+1]
  movq  mm4, [Skl_YUV_Tab32_MMX+0*2048 + eax*8]
  movzx eax, byte [ebp + 2*ecx+0]
  paddw mm4, [Skl_YUV_Tab32_MMX+1*2048 + ebx*8]
  movzx ebx, byte [ebp + 2*ecx+1]

  movq mm1, mm0
  movq mm2, mm0
  movq mm3, mm0
  movq mm5, mm4
  movq mm6, mm4
  movq mm7, mm4

  paddw mm0, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]  
  movzx eax, byte [ebp + 2*ecx+2]
  paddw mm1, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]
  movzx ebx, byte [ebp + 2*ecx+3]  
  packuswb mm1, mm0
  paddw mm4, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]
  mov esi, [RGBp]
  paddw mm5, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]


  packuswb mm5, mm4
  TO_565 mm1, mm0
  movd [esi+4*ecx+0], mm0   ; 2x16b
  TO_565 mm5, mm4
  movd [esi+4*ecx+4], mm4   ; 2x16b
  movzx eax, byte [edx + 2*ecx+0]
  movzx ebx, byte [edx + 2*ecx+1]

  paddw mm2, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]
  movzx eax, byte [edx + 2*ecx+2]  
  paddw mm3, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]
  movzx ebx, byte [edx + 2*ecx+3]
  packuswb mm3, mm2
  paddw mm6, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]
  add esi, [esp+8  +40]
  paddw mm7, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]

  mov edi, [Vp]
  packuswb mm7, mm6
  TO_565 mm3, mm2
  movd [esi+4*ecx+0], mm2   ; 2x16b
  TO_565 mm7, mm6
  movd [esi+4*ecx+4], mm6   ; 2x16b

  add ecx, 2
  mov esi, [Up]

  jl near .Loop_x

  mov ecx, [yCnt]
  jmp .Loop_y


%undef RGBp
%undef Yp
%undef Up
%undef Vp
%undef xCnt
%undef yCnt

;//////////////////////////////////////////////////////////////////////

Skl_RGB32_To_YUV_MMX:
  mov eax,[0] ; abort
  ret

;//////////////////////////////////////////////////////////////////////

  ; eax: *U
  ; ebx: *V
  ; esi: *Y
  ; edx: Src_BpS
  ; edi: *Dst
  ; ebx: Dst_BpS
  ; ecx: Counter

%define RGBp  esp+20
%define Yp    esp+16
%define Up    esp+12
%define Vp    esp+8
%define xCnt  esp+4
%define yCnt  esp+0

Skl_YUV_To_RGB32_MMX:

  push ebx
  push esi
  push edi
  push ebp

  mov edi, [esp+4  +16] ; RGB
  mov ebp, [esp+12 +16] ; Y
  mov eax, [esp+16 +16] ; U
  mov ebx, [esp+20 +16] ; V
  mov edx, [esp+24 +16] ; Src_BpS
  mov ecx, [esp+28 +16] ; Width

  lea edi, [edi+4*ecx]  ; RGB += Width*sizeof(32b)
  lea ebp, [ebp+ecx]    ; ebp: Y1 = Y + Width
  add edx, ebp          ; edx: Y2 = Y1+ BpS
  push edi              ; [RGBp]
  push ebp              ; [Yp]
  shr ecx, 1            ; Width/=2
  lea eax, [eax+ecx]    ; U += W/2
  lea ebx, [ebx+ecx]    ; V += W/2
  push eax              ; [Up]
  push ebx              ; [Vp]

  neg ecx               ; ecx = -Width/2
  push ecx              ; save [xCnt]
  push eax              ; fake ([yCnt])
  
  mov ecx, [esp+32 +40] ; Height
  shr ecx, 1            ; /2

  mov esi, [Up]
  mov edi, [Vp]

  jmp .Go

align 16
.Loop_y
  dec ecx
  jg .Add

  add esp, 24   ; rid of all tmp
  pop ebp
  pop edi
  pop esi
  pop ebx

  ret

align 16
.Add
  mov edi, [esp+8 +40]  ; Dst_BpS
  mov esi, [esp+24 +40] ; Src_BpS
  mov edx, [RGBp]
  mov ebp, [Yp]
  lea edx, [edx+2*edi]  ; RGB += 2*Dst_BpS
  lea ebp, [ebp+2*esi]  ; Y   += 2*Src_BpS
  mov [RGBp], edx
  mov edi, [Vp]
  mov [Yp], ebp         ; Y1
  lea edx, [ebp+esi]    ; Y2

  lea edi, [edi+esi]    ; V += Src_BpS
  add esi, [Up]         ; U += Src_BpS
  mov [Vp], edi
  mov [Up], esi

.Go
  mov [yCnt], ecx
  mov ecx, [xCnt]
  
        ; 5210c@640x480

.Loop_x   ; edi,esi: U,V;  ebp,edx: Y1, Y2;  ecx: xCnt

    ; R = Y +       a.U
    ; G = Y + c.V + b.U
    ; B = Y + d.V

  movzx eax, byte [edi+ecx+0]
  movzx ebx, byte [esi+ecx+0]
  movq  mm0, [Skl_YUV_Tab32_MMX+0*2048 + eax*8]
  movzx eax, byte [edi+ecx+1]
  paddw mm0, [Skl_YUV_Tab32_MMX+1*2048 + ebx*8]
  movzx ebx, byte [esi+ecx+1]
  movq  mm4, [Skl_YUV_Tab32_MMX+0*2048 + eax*8]
  movzx eax, byte [ebp + 2*ecx+0]
  paddw mm4, [Skl_YUV_Tab32_MMX+1*2048 + ebx*8]
  movzx ebx, byte [ebp + 2*ecx+1]

  movq mm1, mm0
  movq mm2, mm0
  movq mm3, mm0
  movq mm5, mm4
  movq mm6, mm4
  movq mm7, mm4

  paddw mm0, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]  
  movzx eax, byte [ebp + 2*ecx+2]
  paddw mm1, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]
  movzx ebx, byte [ebp + 2*ecx+3]  
  packuswb mm0, mm1  
  paddw mm4, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]
  movzx eax, byte [edx + 2*ecx+0]
  paddw mm5, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]

  packuswb mm4, mm5
  mov esi, [RGBp]  
  movzx ebx, byte [edx + 2*ecx+1]  
  movq [esi+8*ecx+0], mm0   ; 2x32b  
  movq [esi+8*ecx+8], mm4   ; 2x32b

  paddw mm2, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]
  movzx eax, byte [edx + 2*ecx+2]  
  paddw mm3, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]
  movzx ebx, byte [edx + 2*ecx+3]
  packuswb mm2, mm3
  paddw mm6, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]
  add esi, [esp+8  +40]
  paddw mm7, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]

  mov edi, [Vp]
  packuswb mm6, mm7
  movq [esi+8*ecx+0], mm2   ; 2x32b  
  movq [esi+8*ecx+8], mm6   ; 2x32b

  add ecx, 2
  mov esi, [Up]

  jl near .Loop_x

  mov ecx, [yCnt]
  jmp .Loop_y

;//////////////////////////////////////////////////////////////////////

%macro STORE_24 2
  lea ebx, [esi+4*ecx]
  lea ebx, [ebx+2*ecx]

  movd eax, %1
  psrlq %1,32
  mov [ebx+ 2], al
  mov [ebx+ 1], ah
  shr eax, 16
  mov [ebx+ 0], al

  movd eax, %1
  mov [ebx+ 5], al
  mov [ebx+ 4], ah
  shr eax, 16
  mov [ebx+ 3], al


  movd eax, %2
  psrlq %2,32
  mov [ebx+ 8], al
  mov [ebx+ 7], ah
  shr eax, 16
  mov [ebx+ 6], al

  movd eax, %2
  mov [ebx+11], al
  mov [ebx+10], ah
  shr eax, 16
  mov [ebx+ 9], al
%endmacro

Skl_YUV_To_RGB24_MMX:

  push ebx
  push esi
  push edi
  push ebp

  mov edi, [esp+4  +16] ; RGB
  mov ebp, [esp+12 +16] ; Y
  mov eax, [esp+16 +16] ; U
  mov ebx, [esp+20 +16] ; V
  mov edx, [esp+24 +16] ; Src_BpS
  mov ecx, [esp+28 +16] ; Width

  lea edi, [edi+2*ecx]  ; RGB += Width*sizeof(24b)
  lea edi, [edi+  ecx]
  lea ebp, [ebp+ecx]    ; ebp: Y1 = Y + Width
  add edx, ebp          ; edx: Y2 = Y1+ BpS
  push edi              ; [RGBp]
  push ebp              ; [Yp]
  shr ecx, 1            ; Width/=2
  lea eax, [eax+ecx]    ; U += W/2
  lea ebx, [ebx+ecx]    ; V += W/2
  push eax              ; [Up]
  push ebx              ; [Vp]

  neg ecx               ; ecx = -Width/2
  push ecx              ; save [xCnt]
  push eax              ; fake ([yCnt])
  
  mov ecx, [esp+32 +40] ; Height
  shr ecx, 1            ; /2

  mov esi, [Up]
  mov edi, [Vp]

  jmp .Go

align 16
.Loop_y
  dec ecx
  jg .Add

  add esp, 24   ; rid of all tmp
  pop ebp
  pop edi
  pop esi
  pop ebx

  ret

align 16
.Add
  mov edi, [esp+8 +40]  ; Dst_BpS
  mov esi, [esp+24 +40] ; Src_BpS
  mov edx, [RGBp]
  mov ebp, [Yp]
  lea edx, [edx+2*edi]  ; RGB += 2*Dst_BpS
  lea ebp, [ebp+2*esi]  ; Y   += 2*Src_BpS
  mov [RGBp], edx
  mov edi, [Vp]
  mov [Yp], ebp         ; Y1
  lea edx, [ebp+esi]    ; Y2

  lea edi, [edi+esi]    ; V += Src_BpS
  add esi, [Up]         ; U += Src_BpS
  mov [Vp], edi
  mov [Up], esi

.Go
  mov [yCnt], ecx
  mov ecx, [xCnt]
  
.Loop_x   ; edi,esi: U,V;  ebp,edx: Y1, Y2;  ecx: xCnt

    ; R = Y +       a.U
    ; G = Y + c.V + b.U
    ; B = Y + d.V

  movzx eax, byte [edi+ecx+0]
  movzx ebx, byte [esi+ecx+0]
  movq  mm0, [Skl_YUV_Tab32_MMX+0*2048 + eax*8]
  movzx eax, byte [edi+ecx+1]
  paddw mm0, [Skl_YUV_Tab32_MMX+1*2048 + ebx*8]
  movzx ebx, byte [esi+ecx+1]
  movq  mm4, [Skl_YUV_Tab32_MMX+0*2048 + eax*8]
  movzx eax, byte [ebp + 2*ecx+0]
  paddw mm4, [Skl_YUV_Tab32_MMX+1*2048 + ebx*8]
  movzx ebx, byte [ebp + 2*ecx+1]

  movq mm1, mm0
  movq mm2, mm0
  movq mm3, mm0
  movq mm5, mm4
  movq mm6, mm4
  movq mm7, mm4

  paddw mm0, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]  
  movzx eax, byte [ebp + 2*ecx+2]
  paddw mm1, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]
  movzx ebx, byte [ebp + 2*ecx+3]  
  packuswb mm0, mm1  
  paddw mm4, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]

  paddw mm5, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]

  packuswb mm4, mm5
  mov esi, [RGBp]  
  STORE_24 mm0, mm4     ; 2x24b

  movzx eax, byte [edx + 2*ecx+0]
  movzx ebx, byte [edx + 2*ecx+1]

  paddw mm2, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]
  movzx eax, byte [edx + 2*ecx+2]  
  paddw mm3, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]
  movzx ebx, byte [edx + 2*ecx+3]
  packuswb mm2, mm3
  paddw mm6, [Skl_YUV_Tab32_MMX+2*2048 + eax*8]
  add esi, [esp+8  +40]
  paddw mm7, [Skl_YUV_Tab32_MMX+2*2048 + ebx*8]

  mov edi, [Vp]
  packuswb mm6, mm7
  STORE_24 mm2, mm6     ; 2x24b

  add ecx, 2
  mov esi, [Up]

  jl near .Loop_x

  mov ecx, [yCnt]
  jmp .Loop_y


%undef RGBp
%undef Yp
%undef Up
%undef Vp
%undef xCnt
%undef yCnt

;//////////////////////////////////////////////////////////////////////
