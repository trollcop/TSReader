;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
;//////////////////////////////////////////////////////////////////////
; [BITS 32]

%include "../../include/skl_syst/skl_nasm.h"

;%define FAST_H263

globl Skl_Dequant_Intra_H263_MMX
globl Skl_Dequant_Inter_H263_MMX
globl Skl_Quant_Intra_H263_MMX
globl Skl_Quant_Inter_H263_MMX
globl Skl_Dequant_Intra_MPEG4_MMX
globl Skl_Dequant_Inter_MPEG4_MMX
globl Skl_Quant_Intra_MPEG4_MMX
globl Skl_Quant_Inter_MPEG4_MMX
globl Skl_Quant_Zero_MMX
globl Skl_Quant_Zero16_MMX

globl Skl_Dequant_Intra_H263_SSE
globl Skl_Dequant_Inter_H263_SSE

globl Skl_Dequant_Intra_H263_SSE2
globl Skl_Dequant_Inter_H263_SSE2
globl Skl_Dequant_Intra_MPEG4_SSE2
globl Skl_Dequant_Inter_MPEG4_SSE2

globl Skl_Quant_Zero_SSE2
globl Skl_Quant_Zero16_SSE2

DATA

;//////////////////////////////////////////////////////////////////////

align 16

MMX_SAT_p2047 times 8 dw (32767-2047)
MMX_SAT_m2048 times 8 dw (32768-2048)
MMX_2047      times 8 dw  2047
MMX_m2048     times 8 dw -2048
Zero          times 8 dw 0
One           times 8 dw 1

TEXT

;//////////////////////////////////////////////////////////////////////
; 
;         H263 part
;
;//////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////
; Skl_Dequant_Intra_H263_MMX(SKL_INT16 *Out,
;                            const SKL_INT16 *In,
;                            const SKL_QUANTIZER Q,
;                            SKL_INT32 q, SKL_INT32 DC_q)
;//////////////////////////////////////////////////////////////////////

  ; same as MPEG4 dequant with constant intra matrix equal to (1<<4)

align 16
Skl_Dequant_Intra_H263_MMX: ; 160c
  mov eax, [esp+16] ; q
  mov ecx, [esp+12] ; Q
  shl eax, 8        ; q*256 (256=64*2*2)
  movq mm6, [ecx + eax +0x1e80]  ; Bias
  movq mm7, [ecx + eax +0x1e00]  ; Mult    (0x1e00=(31-1)*2*64*2)

  mov edx, [esp+4]  ; Out
  mov ecx, [esp+8]  ; In

  mov eax, -16

align 16
.Loop
  movq mm0, [ecx+8*eax+8*16 + 0]  ; c  = In[i]
  movq mm1, [ecx+8*eax+8*16 + 8]
  add eax,2         ; preserve z-flag until bottom

  movq  mm2, mm0
  movq  mm3, mm1
  psraw mm2, 16      ; extract sign mask
  pxor  mm4, mm4
  psraw mm3, 16

  pcmpeqw mm4, mm0  ; c==0?
  pxor  mm5, mm5
  pandn mm4, mm6    ; Bias = (c==0) ? 0 : Bias
  pcmpeqw mm5, mm1
  pxor  mm0, mm2     ; negate if negative
  pandn mm5, mm6
  pxor  mm1, mm3
  psubw mm0, mm2
  psubw mm1, mm3

  pmullw mm0, mm7   ; *= Mult
  pmullw mm1, mm7
  paddw  mm0, mm4   ; += Bias
  paddw  mm1, mm5
  paddw  mm0, mm2   ; negate back
  paddw  mm1, mm3

  movq mm5, [MMX_SAT_p2047]
  paddsw mm0, mm5
  paddsw mm1, mm5
  psubsw mm0, mm5
  psubsw mm1, mm5

  pxor mm0, mm2
  pxor mm1, mm3

  movq [edx+8*eax+8*16-2*8   ], mm0
  movq [edx+8*eax+8*16-2*8 +8], mm1
  jnz	near .Loop

  movd   mm0, [esp+20]    ; DC_q
  pmullw mm0, [ecx]
  movq   mm2, [MMX_SAT_m2048]
  paddsw mm0, mm5     ; mm5 = MMX_SAT_p2047
  psubsw mm0, mm5
  psubsw mm0, mm2
  paddsw mm0, mm2
  movd eax, mm0
  mov [edx], ax

  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Dequant_Intra_H263_SSE
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Dequant_Intra_H263_SSE:   ; 150c

  mov eax, [esp+16] ; q
  mov ecx, [esp+12] ; Q
  shl eax, 8        ; q*256 (256=64*2*2)
  movq mm6, [ecx + eax +0x1e80]  ; Bias
  movq mm7, [ecx + eax +0x1e00]  ; Mult

  mov edx, [esp+4]  ; Out
  mov ecx, [esp+8]  ; In

  mov eax, -16

align 16
.Loop
  movq mm0, [ecx+8*eax+8*16 + 0]  ; c  = In[i]
  movq mm1, [ecx+8*eax+8*16 + 8]
  add eax, 2

  movq  mm2, mm0
  movq  mm3, mm1
  psraw mm2, 16      ; extract sign mask
  pxor  mm4, mm4
  psraw mm3, 16

  pcmpeqw mm4, mm0  ; c==0?
  pxor  mm5, mm5
  pandn mm4, mm6    ; Bias = (c==0) ? 0 : Bias
  pcmpeqw mm5, mm1
  pxor  mm0, mm2     ; negate if negative
  pandn mm5, mm6
  pxor  mm1, mm3
  psubw mm0, mm2 
  psubw mm1, mm3

  pmullw mm0, mm7   ; *= Mult
  pmullw mm1, mm7
  paddw  mm0, mm4   ; += Bias
  paddw  mm1, mm5
  paddw  mm0, mm2   ; negate back
  paddw  mm1, mm3

  movq mm5, [MMX_2047]
  pminsw mm0, mm5
  pminsw mm1, mm5

  pxor mm0, mm2
  pxor mm1, mm3

  movq [edx+8*eax+8*16-2*8   ], mm0
  movq [edx+8*eax+8*16-2*8 +8], mm1
  jnz	near .Loop

  movd   mm0, [esp+20]    ; DC_q
  pmullw mm0, [ecx]
  pminsw mm0, mm5       ; mm5 = 2047
  pmaxsw mm0, [MMX_m2048]
  movd eax, mm0
  mov [edx], ax

  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Dequant_Intra_H263_SSE2
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Dequant_Intra_H263_SSE2:

  mov eax, [esp+16] ; q
  mov ecx, [esp+12] ; Q
  shl eax, 8        ; q*256 (256=64*2*2)
  movdqa xmm6, [ecx + eax +0x1e80]  ; Bias
  movdqa xmm7, [ecx + eax +0x1e00]  ; Mult

  mov edx, [esp+4]  ; Out
  mov ecx, [esp+8]  ; In

  mov eax, -16

align 16
.Loop
  movdqa xmm0, [ecx+8*eax+8*16+ 0]  ; c  = In[i]
  movdqa xmm1, [ecx+8*eax+8*16+16]
  add eax, 4

  movdqa xmm2, xmm0
  movdqa xmm3, xmm1
  psraw  xmm2, 16      ; extract sign mask
  pxor   xmm4, xmm4
  psraw  xmm3, 16

  pcmpeqw xmm4, xmm0  ; c==0?
  pxor  xmm5, xmm5
  pandn xmm4, xmm6    ; Bias = (c==0) ? 0 : Bias
  pcmpeqw xmm5, xmm1
  pxor  xmm0, xmm2     ; negate if negative
  pandn xmm5, xmm6
  pxor  xmm1, xmm3
  psubw xmm0, xmm2 
  psubw xmm1, xmm3

  pmullw xmm0, xmm7   ; *= Mult
  pmullw xmm1, xmm7
  paddw  xmm0, xmm4   ; += Bias
  paddw  xmm1, xmm5
  paddw  xmm0, xmm2   ; negate back
  paddw  xmm1, xmm3

  movdqa xmm5, [MMX_2047]
  pminsw xmm0, xmm5
  pminsw xmm1, xmm5

  pxor   xmm0, xmm2
  pxor   xmm1, xmm3

  movdqa [edx+8*eax+8*16-4*8+ 0], xmm0
  movdqa [edx+8*eax+8*16-4*8+16], xmm1
  jnz	near .Loop

  movd   xmm0, [esp+20]    ; DC_q
  pmullw xmm0, [ecx]
  pminsw xmm0, xmm5       ; xmm5 = 2047
  pmaxsw xmm0, [MMX_m2048]
  pextrw eax,xmm0, 0
  mov [edx], ax

  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Dequant_Inter_H263_MMX
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Dequant_Inter_H263_MMX:   ; 148c

;SKL_RDTSC_IN

  mov eax, [esp+16] ; q
  mov ecx, [esp+12] ; Q
  shl eax, 8        ; q*256 (256=64*2*2)
  movq mm6, [ecx + eax +0x1e80]  ; Bias
  movq mm7, [ecx + eax +0x1e00]  ; Mult

  mov edx, [esp+4]  ; Out
  mov ecx, [esp+8]  ; In

  mov eax, [esp+20] ; Rows

  push ebx
  mov ebx, -16

.Loop
  shr eax, 1
  jnc near .Zero2

  movq mm0, [ecx+8*ebx+8*16 + 0]  ; c  = In[i]
  movq mm1, [ecx+8*ebx+8*16 + 8]

  add ebx,2         ; preserve z-flag until bottom
  
  movq  mm2, mm0
  movq  mm3, mm1
  psraw mm2, 16     ; extract sign mask
  pxor  mm4, mm4
  psraw mm3, 16

  pcmpeqw mm4, mm0  ; c==0?
  pxor   mm5, mm5
  pandn  mm4, mm6   ; Bias = (c==0) ? 0 : Bias
  pcmpeqw mm5, mm1
  pxor   mm0, mm2   ; negate if negative
  pandn  mm5, mm6
  pxor   mm1, mm3
  psubw  mm0, mm2
  psubw  mm1, mm3

  pmullw mm0, mm7   ; *= Mult
  pmullw mm1, mm7
  paddw  mm0, mm4   ; += Bias
  paddw  mm1, mm5
  paddw  mm0, mm2   ; negate back
  paddw  mm1, mm3

  movq mm5, [MMX_SAT_p2047]
  paddsw mm0, mm5
  paddsw mm1, mm5
  psubsw mm0, mm5
  psubsw mm1, mm5

  pxor   mm0, mm2
  pxor   mm1, mm3

.Zero
  movq [edx+8*ebx+8*16-2*8   ], mm0
  movq [edx+8*ebx+8*16-2*8 +8], mm1
  jnz	near .Loop
;SKL_RDTSC_OUT

.End
  pop ebx
  ret

.Zero2:
  add ebx,2
  pxor mm0,mm0
  pxor mm1,mm1
  jmp .Zero

;//////////////////////////////////////////////////////////////////////
; Skl_Dequant_Inter_H263_SSE
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Dequant_Inter_H263_SSE:     ; 137c

  mov eax, [esp+16] ; q
  mov ecx, [esp+12] ; Q
  shl eax, 8        ; q*256 (256=64*2*2)
  movq mm6, [ecx + eax +0x1e80]  ; Bias
  movq mm7, [ecx + eax +0x1e00]  ; Mult

  mov edx, [esp+4]  ; Out
  mov ecx, [esp+8]  ; In

  mov eax, [esp+20] ; Rows

  push ebx
  mov ebx, -16

.Loop
  shr eax, 1
  jnc near .Zero2

  movq   mm0, [ecx+8*ebx+8*16 + 0] ; c  = In[i]
  movq   mm1, [ecx+8*ebx+8*16 + 8]

  movq   mm2, mm0
  movq   mm3, mm1
  psraw  mm2, 16     ; extract sign mask
  psraw  mm3, 16

  pxor   mm4, mm4
  pxor   mm5, mm5

  pcmpeqw mm4, mm0   ; c==0?
  pcmpeqw mm5, mm1

  pandn  mm4, mm6    ; Bias = (c==0) ? 0 : Bias
  pandn  mm5, mm6

  pxor   mm0, mm2    ; negate if negative
  pxor   mm1, mm3
  psubw  mm0, mm2
  psubw  mm1, mm3

  pmullw mm0, mm7    ; *= Mult
  pmullw mm1, mm7
  paddw  mm0, mm4    ; + Bias
  paddw  mm1, mm5
  paddw  mm0, mm2    ; start negating back
  paddw  mm1, mm3

  movq mm5, [MMX_2047]
  pminsw mm0, mm5
  add ebx, 2
  pminsw mm1, mm5

  pxor   mm0, mm2     ; finish negating back
  pxor   mm1, mm3

.Zero:
  movq [edx+8*ebx+8*16-8*2   ], mm0
  movq [edx+8*ebx+8*16-8*2 +8], mm1
  jnz	near .Loop

  pop ebx
  ret

.Zero2:
  add ebx,2
  pxor mm0,mm0
  pxor mm1,mm1
  jmp .Zero

;//////////////////////////////////////////////////////////////////////
; Skl_Dequant_Inter_H263_SSE2
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Dequant_Inter_H263_SSE2:     ; 137c

;SKL_RDTSC_IN

  mov eax, [esp+16] ; q
  mov ecx, [esp+12] ; Q
  shl eax, 8        ; q*256 (256=64*2*2)
  movdqa xmm6, [ecx + eax +0x1e80]  ; Bias
  movdqa xmm7, [ecx + eax +0x1e00]  ; Mult

  mov edx, [esp+4]  ; Out
  mov ecx, [esp+8]  ; In

  mov eax, [esp+20] ; Rows

  push ebx
  mov ebx, -16

.Loop
  test eax, eax
  jz near .End2
  shr eax, 2

  movdqa xmm0, [ecx+8*ebx+8*16 + 0] ; c  = In[i]
  movdqa xmm1, [ecx+8*ebx+8*16 +16]

  movdqa xmm2, xmm0
  movdqa xmm3, xmm1
  psraw  xmm2, 16     ; extract sign mask
  psraw  xmm3, 16

  pxor   xmm4, xmm4
  pxor   xmm5, xmm5

  pcmpeqw xmm4, xmm0  ; c==0?
  pcmpeqw xmm5, xmm1

  pandn  xmm4, xmm6   ; Bias = (c==0) ? 0 : Bias
  pandn  xmm5, xmm6

  pxor   xmm0, xmm2   ; negate if negative
  pxor   xmm1, xmm3
  psubw  xmm0, xmm2
  psubw  xmm1, xmm3

  pmullw xmm0, xmm7   ; *= Mult
  pmullw xmm1, xmm7
  paddw  xmm0, xmm4   ; + Bias
  paddw  xmm1, xmm5
  paddw  xmm0, xmm2   ; start negating back
  paddw  xmm1, xmm3

  movdqa xmm5, [MMX_2047]
  pminsw xmm0, xmm5
  add ebx, 4
  pminsw xmm1, xmm5

  pxor   xmm0, xmm2   ; finish negating back
  pxor   xmm1, xmm3

  movdqa [edx+8*ebx+8*16-4*8+ 0], xmm0
  movdqa [edx+8*ebx+8*16-4*8+16], xmm1
  jnz	near .Loop

;SKL_RDTSC_OUT

  pop ebx
  ret

.End2:
  pxor    xmm0,xmm0
.Loop_End2
  movdqa  [edx + 8*ebx + 8*16   ], xmm0   ; Out[i]  
  movdqa  [edx + 8*ebx + 8*16+16], xmm0
  add ebx,4
  jnz .Loop_End2
  pop ebx
  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Quant_Intra_H263_MMX
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Quant_Intra_H263_MMX:   ; 136c     FAST_H263: 108c
;SKL_RDTSC_IN

  mov eax, [esp+ 8]     ; In
  mov edx, [esp+20]     ; DC_q

  mov ecx, edx
  shr edx, 1            ; Bias = Mult/2 (only for DC)

  movsx eax, word [eax] ; DC
  jge   .Pos
  neg   edx
.Pos:
  add   eax, edx
  cdq

  idiv  ecx             ; launch the idiv

      ; and now, for something completely different...

  push edi
  mov edi, [esp+4 + 4]  ; Out
  mov edx, [esp+4 + 8]  ; In
  mov ecx, [esp+4 +16]  ; q

  cmp ecx, 1
  je near .Special_q1

%ifndef FAST_H263

  shl ecx, 8
  add ecx, [esp+4 +12]   ; ecx = Q[0][q-1][][]
  push ebx

  mov ebx, -16
.Loop

  movq    mm0, [edx+ 8*ebx + 8*16   ] ; c  = In[i]
  movq    mm1, [edx+ 8*ebx + 8*16+ 8]

  movq    mm4, mm0
  movq    mm2, [edx+ 8*ebx + 8*16+16]
  movq    mm5, mm1
  movq    mm3, [edx+ 8*ebx + 8*16+24]

  movq    mm6, mm2
  psraw   mm4, 16      ; extract sign mask
  movq    mm7, mm3
  psraw   mm5, 16

  pxor    mm0, mm4
  pxor    mm1, mm5
  psubsw  mm0, mm4
  
  psubusw mm0, [ecx-0x080+8*16 +8*ebx   ]  ; -Bias  (Bias is assumed >=0)
  psubsw  mm1, mm5

  psubusw mm1, [ecx-0x080+8*16 +8*ebx+ 8]
  psraw   mm6, 16      ; extract sign mask
  pmulhw  mm0, [ecx-0x100+8*16 +8*ebx   ]  ; *= Mult
  pxor    mm2, mm6
  pmulhw  mm1, [ecx-0x100+8*16 +8*ebx+ 8]

  psraw   mm7, 16
  pxor    mm3, mm7
  psubsw  mm2, mm6
  psubusw mm2, [ecx-0x080+8*16 +8*ebx+16]
  psubsw  mm3, mm7

  psubusw mm3, [ecx-0x080+8*16 +8*ebx+24]
  pxor    mm0, mm4
  pmulhw  mm2, [ecx-0x100+8*16 +8*ebx+16]
  pxor    mm1, mm5
  pmulhw  mm3, [ecx-0x100+8*16 +8*ebx+24]

  add ebx, 4

  psubsw mm0, mm4
  pxor   mm2, mm6
  
  movq   [edi+8*ebx+8*16-4*8   ], mm0
  psubsw mm1, mm5
  pxor   mm3, mm7
  movq   [edi+8*ebx+8*16-4*8+ 8], mm1


  psubsw mm2, mm6
  movq   [edi+8*ebx+8*16-4*8+16], mm2
  psubsw mm3, mm7
  movq   [edi+8*ebx+8*16-4*8+24], mm3

  jl .Loop 

  pop ebx
%else

  shl ecx, 8
  add ecx, [esp+4 +12]   ; ecx = Q[0][q-1][][]
  movq mm7, [ecx-0x100]  ; mm7 = Mult

  mov ecx, -16

.Loop

  movq  mm0, [edx+ 8*ecx + 8*16   ] ; c  = In[i]
  movq  mm1, [edx+ 8*ecx + 8*16 +8]

  movq  mm2, mm0
  movq  mm3, mm1
  psraw mm2, 16      ; extract sign mask
  psraw mm3, 16

      ; we don't use the Bias, but we correct the shift by 16 with
      ; the sign. E.g:      x/8 = (x>>3) + (x<0)
      ; to have the correct rounding (toward zero)

  pmulhw mm0, mm7  ; *= Mult
  pmulhw mm1, mm7

  add ecx, 2

  psubw mm0, mm2    ; correct the rounding
  movq  [edi+8*ecx+8*16-2*8   ], mm0
  psubw mm1, mm3
  movq  [edi+8*ecx+8*16-2*8+ 8], mm1

  jl .Loop 

%endif

.End:
  mov  [edi], ax    ; DC

  pop edi
;SKL_RDTSC_OUT
  ret

.Special_q1:      ; 91c
  mov ecx, -16

.Loop1

  movq  mm4, [edx+ 8*ecx + 8*16   ] ; c  = In[i]
  movq  mm0, mm4
  movq  mm5, [edx+ 8*ecx + 8*16+ 8]

  movq  mm1, mm5
  psraw mm4, 16      ; extract sign mask
  psraw mm5, 16

  movq  mm6, [edx+ 8*ecx + 8*16+16] ; c  = In[i]
  movq  mm2, mm6
  
  movq  mm7, [edx+ 8*ecx + 8*16+24]
  add ecx, 4
  movq  mm3, mm7
  psraw mm6, 16
  psraw mm7, 16

  psraw mm0, 1       ; >>1
  psraw mm1, 1
  
  psraw mm2, 1       ; >>1
  psraw mm3, 1

  psubw mm0, mm4    ; correct the rounding
  psubw mm1, mm5

  movq  [edi+8*ecx+8*16-4*8   ], mm0
  psubw mm2, mm6
  movq  [edi+8*ecx+8*16-4*8+ 8], mm1
  psubw mm3, mm7
  movq  [edi+8*ecx+8*16-4*8+16], mm2
  movq  [edi+8*ecx+8*16-4*8+24], mm3

  jl .Loop1
  jmp .End


;//////////////////////////////////////////////////////////////////////
; Skl_Quant_Inter_H263_MMX
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Quant_Inter_H263_MMX:   ; 120c      FAST_H263: 106c

  mov eax, [esp + 4]  ; Out
  mov edx, [esp + 8]  ; In
  mov ecx, [esp +16]  ; q

  cmp ecx, 1
  pxor mm7, mm7          ; SAD

  je near .Special_q1

%ifndef FAST_H263

  shl ecx, 8
  add ecx, [esp +12]   ; ecx = Q[0][q-1][][]
  push ebx

  mov ebx, -16
.Loop

  movq    mm0, [edx+ 8*ebx + 8*16   ] ; c  = In[i]
  movq    mm1, [edx+ 8*ebx + 8*16+ 8]

  movq    mm4, mm0
  movq    mm2, [edx+ 8*ebx + 8*16+16]
  movq    mm5, mm1
  movq    mm3, [edx+ 8*ebx + 8*16+24]

  movq    mm6, mm2
  psraw   mm4, 16      ; extract sign mask

  psraw   mm5, 16

  pxor    mm0, mm4
  pxor    mm1, mm5
  psubsw  mm0, mm4
  
  psubusw mm0, [ecx-0x080+8*16 +8*ebx   ]  ; -Bias  (Bias is assumed >=0)
  psubsw  mm1, mm5

  psubusw mm1, [ecx-0x080+8*16 +8*ebx+ 8]
  psraw   mm6, 16      ; extract sign mask
  pmulhw  mm0, [ecx-0x100+8*16 +8*ebx   ]  ; *= Mult
  pxor    mm2, mm6
  pmulhw  mm1, [ecx-0x100+8*16 +8*ebx+ 8]
  psubsw  mm2, mm6
  psubusw mm2, [ecx-0x080+8*16 +8*ebx+16]
  paddsw  mm7, mm0    ; collect SAD
  pxor    mm0, mm4
  pmulhw  mm2, [ecx-0x100+8*16 +8*ebx+16]
  paddsw  mm7, mm1    ; collect SAD
  pxor    mm1, mm5


  add ebx, 4

  paddsw  mm7, mm2    ; collect SAD
  pxor    mm2, mm6
  psubsw  mm0, mm4
  movq    [eax+8*ebx+8*16-4*8   ], mm0
  psubsw  mm1, mm5
  movq    [eax+8*ebx+8*16-4*8+ 8], mm1
  psubsw  mm2, mm6
  movq    [eax+8*ebx+8*16-4*8+16], mm2


  movq    mm0, mm3
  psraw   mm3, 16
  pxor    mm0, mm3
  psubsw  mm0, mm3
  psubusw mm0, [ecx-0x080+8*16-4*8 +8*ebx+24]
  pmulhw  mm0, [ecx-0x100+8*16-4*8 +8*ebx+24]
  paddsw  mm7, mm0    ; collect SAD
  pxor    mm0, mm3
  psubsw  mm0, mm3
  movq   [eax+8*ebx+8*16-4*8+24], mm0
  
  jl .Loop 

  pop ebx

%else

  shl ecx, 8
  add ecx, [esp +12]     ; ecx = Q[0][q-1][][]
  movq mm5, [ecx-0x100]  ; mm5 = Mult

  mov ecx, -16

.Loop

  movq    mm0, [edx+8*ecx+8*16 + 0] ; c  = In[i]
  movq    mm1, [edx+8*ecx+8*16 + 8]

  movq    mm2, mm0
  movq    mm3, mm1
  psraw   mm2, 16     ; extract sign mask
  psraw   mm3, 16

  pmulhw  mm0, mm5  ; *= Mult
  pmulhw  mm1, mm5

  add ecx,2

  psubw mm0, mm2    ; correct the rounding
  movq  [eax+8*ecx+8*16-2*8   ], mm0
  psubw mm1, mm3
  movq  [eax+8*ecx+8*16-2*8 +8], mm1

    ; We only compute abs(c) for the SAD...

  pxor   mm0, mm2     ; take abs value
  pxor   mm1, mm3
  psubw  mm0, mm2
  psubw  mm1, mm3

  paddsw mm7, mm0   ; update SAD
  paddsw mm7, mm1

  jnz .Loop 

%endif

.End:
  pmaddwd mm7, [One]
  movq    mm0, mm7
  psrlq   mm7, 32
  paddd   mm0, mm7
  movd  eax, mm0    ; return SAD

  ret

.Special_q1:

  mov ecx, -16

.Loop1    ; 98c

  movq  mm0, [edx+8*ecx+8*16 + 0] ; c  = In[i]
  movq  mm1, [edx+8*ecx+8*16 + 8]
  add ecx,2

  movq  mm2, mm0
  movq  mm3, mm1
  psraw mm2, 16     ; extract sign mask
  psraw mm3, 16
  psraw mm0, 1      ; >>1
  psraw mm1, 1

  psubw mm0, mm2    ; correct the rounding
  movq  [eax+8*ecx+8*16-2*8   ], mm0
  psubw mm1, mm3
  movq  [eax+8*ecx+8*16-2*8 +8], mm1

    ; We only compute abs(c) for the SAD...

  pxor mm0, mm2     ; take abs value
  pxor mm1, mm3
  psubw  mm0, mm2
  psubw  mm1, mm3

  paddsw mm7, mm0   ; update SAD
  paddsw mm7, mm1

  jnz .Loop1

  jmp .End

;//////////////////////////////////////////////////////////////////////
; 
;         MPEG4 part
;
;//////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////
; Skl_Dequant_Intra_MPEG4_MMX
;//////////////////////////////////////////////////////////////////////

  ;   Note: in order to saturate 'easily', we pre-shift the quantifier
  ; by 4. Then, the high-word of (c*M[i]*Q) is used to
  ; build a saturating mask. It is non-zero only when an overflow occured.
  ; We thus avoid packing/unpacking toward double-word.

align 16
Skl_Dequant_Intra_MPEG4_MMX:    ; 166c

  mov eax, [esp+16] ; q
  mov ecx, [esp+12] ; Q
  shl eax, 8        ; q*256 (256=64*2*2)
  push ebx
  lea ebx, [ecx+eax +0x1e00 + 8*16]  ; Mult = (Quant*M[i]*Mult)<<2
  mov edx, [esp+4 + 4]  ; Out
  mov ecx, [esp+4 + 8]  ; In

  mov eax, -16   ; to keep aligned, we regularly process coeff[0]
  pxor mm6, mm6   ; this is a NOP

align 16
.Loop
  movq    mm0, [ecx+8*eax + 8*16 +0]   ; c = In[i]
  movq    mm1, [ecx+8*eax + 8*16 +8]

  movq    mm2, mm0
  movq    mm3, mm1
  psraw   mm2, 16      ; extract sign mask
  psraw   mm3, 16

  pxor    mm0, mm2     ; negate if negative
  pxor    mm1, mm3

  psubw   mm0, mm2
  psubw   mm1, mm3

  movq    mm5, [ebx+8*eax  ] ; M[i]
  movq    mm6, [ebx+8*eax+8]
  pmullw  mm5, mm0           ; low  of c*M[i]*Q
  pmullw  mm6, mm1
  pmulhw  mm0, [ebx+8*eax  ] ; high of c*M[i]*Q
  pmulhw  mm1, [ebx+8*eax+8]

  pcmpgtw mm0, [Zero]
  paddusw mm5, mm0  
  pcmpgtw mm1, [Zero]
  paddusw mm6, mm1
  add eax,2   ; z-flag will be tested later

  psrlw   mm5, 5
  psrlw   mm6, 5

  pxor    mm5, mm2  ; start negating back
  pxor    mm6, mm3

  psubusw mm2, mm0
  psubusw mm3, mm1

  psubw   mm5, mm2 ; finish negating back  
  psubw   mm6, mm3

  movq [edx+8*eax+8*16-2*8   ], mm5   ; Out[i]
  movq [edx+8*eax+8*16-2*8 +8], mm6
  jnz .Loop

  movd    mm0, [esp+4+20]    ; DC_q
  movq    mm5, [MMX_SAT_p2047]
  pmullw  mm0, [ecx]
  movq    mm2, [MMX_SAT_m2048]
  paddsw  mm0, mm5
  psubsw  mm0, mm5
  psubsw  mm0, mm2
  paddsw  mm0, mm2
  movd eax, mm0
  pop ebx
  mov [edx], ax

  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Dequant_Intra_MPEG4_SSE2
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Dequant_Intra_MPEG4_SSE2:

  mov eax, [esp+16] ; q
  mov ecx, [esp+12] ; Q
  shl eax, 8        ; q*256 (256=64*2*2)
  push ebx
  lea ebx, [ecx+eax +0x1e00 + 8*16]  ; Mult = (Quant*M[i]*Mult)<<2
  mov edx, [esp+4 + 4]  ; Out
  mov ecx, [esp+4 + 8]  ; In

  mov eax, -16   ; to keep aligned, we regularly process coeff[0]
  pxor xmm6, xmm6   ; this is a NOP

align 16
.Loop
  movdqa  xmm0, [ecx+8*eax + 8*16+ 0]   ; c = In[i]
  movdqa  xmm1, [ecx+8*eax + 8*16+16]

  movdqa  xmm2, xmm0
  movdqa  xmm3, xmm1
  psraw   xmm2, 16      ; extract sign mask
  psraw   xmm3, 16

  pxor    xmm0, xmm2     ; negate if negative
  pxor    xmm1, xmm3

  psubw   xmm0, xmm2
  psubw   xmm1, xmm3

  movdqa  xmm5, [ebx+8*eax   ] ; M[i]
  movdqa  xmm6, [ebx+8*eax+16]
  pmullw  xmm5, xmm0           ; low  of c*M[i]*Q
  pmullw  xmm6, xmm1
  pmulhw  xmm0, [ebx+8*eax   ] ; high of c*M[i]*Q
  pmulhw  xmm1, [ebx+8*eax+16]

  pcmpgtw xmm0, [Zero]
  paddusw xmm5, xmm0  
  pcmpgtw xmm1, [Zero]
  paddusw xmm6, xmm1
  add eax,4   ; z-flag will be tested later

  psrlw   xmm5, 5
  psrlw   xmm6, 5

  pxor    xmm5, xmm2  ; start negating back
  pxor    xmm6, xmm3

  psubusw xmm2, xmm0
  psubusw xmm3, xmm1

  psubw   xmm5, xmm2 ; finish negating back  
  psubw   xmm6, xmm3

  movdqa [edx+8*eax+8*16-4*8   ], xmm5   ; Out[i]
  movdqa [edx+8*eax+8*16-4*8+16], xmm6
  jnz .Loop

  movd    mm0, [esp+4+20]    ; DC_q
  movq    mm5, [MMX_SAT_p2047]
  pmullw  mm0, [ecx]
  movq    mm2, [MMX_SAT_m2048]
  paddsw  mm0, mm5
  psubsw  mm0, mm5
  psubsw  mm0, mm2
  paddsw  mm0, mm2
  movd eax, mm0
  pop ebx
  mov [edx], ax

  ret

;//////////////////////////////////////////////////////////////////////
; Skl_Dequant_Inter_MPEG4_MMX
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Dequant_Inter_MPEG4_MMX:  ; 219c

;SKL_RDTSC_IN

  mov eax, [esp+16] ; q
  mov ecx, [esp+12] ; Q
  shl eax, 8        ; q*256 (256=64*2*2)
  push ebx
  lea ebx, [ecx+eax +0x1e00 + 8*16 -2*8]  ; Mult = (Quant*M[i]*Mult)<<1

  mov edx, [esp+4 + 4]  ; Out
  mov ecx, [esp+4 + 8]  ; In

  mov eax, -16
  pxor mm6, mm6 ; mismatch sum

  push ebp
  mov ebp, [esp+8 +20] ; Rows

align 4
.Loop
  shr ebp, 1
  jnc near .Zero2

  movq mm0, [ecx+8*eax + 8*16   ]   ; c = In[i]
  movq mm1, [ecx+8*eax + 8*16 +8]
  add eax,2

    ; We use (2*c + sgn(c) - sgn(-c)) as multiplier
    ; so we handle the 3 cases: c<0, c==0, and c>0 in one shot.
    ; sgn(x) is the result of 'pcmpgtw 0,x':  0 if x>=0, -1 if x<0.

  movq   mm2, mm0
  movq   mm3, mm1
  psraw  mm2, 16     ; extract sign mask
  psraw  mm3, 16

  paddsw mm0, mm2    ; c += sign(c)
  paddsw mm1, mm3
  paddw  mm0, mm0    ; c *= 2
  paddw  mm1, mm1

  pxor   mm4, mm4
  pxor   mm5, mm5
  psubw  mm4, mm0    ; -c
  psubw  mm5, mm1
  psraw  mm4, 16     ; mm4 = sgn(-c)
  psraw  mm5, 16

  psubsw mm0, mm4    ; c  -= sgn(-c)
  psubsw mm1, mm5

  pxor   mm0, mm2    ; negate if needed
  pxor   mm1, mm3

  movq    mm4, [ebx +8*eax  ] ; M[i]*Q
  movq    mm5, [ebx +8*eax+8]
  pmulhw  mm4, mm0            ; high of c*M[i]*Q
  pmulhw  mm5, mm1
  pmullw  mm0, [ebx +8*eax  ] ; low  of c*M[i]*Q
  pmullw  mm1, [ebx +8*eax+8]
  pcmpgtw mm4, [Zero]
  pcmpgtw mm5, [Zero]

  paddusw mm0, mm4
  paddusw mm1, mm5
  psrlw   mm0, 5
  psrlw   mm1, 5
  pxor    mm0, mm2  ; start negating back
  pxor    mm1, mm3
  psubusw mm2, mm4
  psubusw mm3, mm5

  psubw   mm0, mm2  ; finish negating back  
  psubw   mm1, mm3

  pxor    mm6, mm0     ; mismatch  
  pxor    mm6, mm1

.Zero:
  movq [edx + 8*eax + 8*16 -2*8   ], mm0   ; Out[i]  
  movq [edx + 8*eax + 8*16 -2*8 +8], mm1

  jnz .Loop

.End:
    ; finish mismatch
  
  movq    mm0, mm6
  psrlq   mm0, 48
  movq    mm1, mm6
  movq    mm2, mm6
  psrlq   mm1, 32
  pxor    mm6, mm0
  psrlq   mm2, 16
  pxor    mm6, mm1
  pxor    mm6, mm2
  movd    ecx, mm6
  and     ecx, 1
  xor     ecx, 1
  xor     word [edx + 2*63], cx

  pop ebp
  pop ebx
;SKL_RDTSC_OUT
  ret

.Zero2:
  add eax,2
  pxor mm0,mm0
  pxor mm1,mm1
  jmp .Zero

;//////////////////////////////////////////////////////////////////////
; Skl_Dequant_Inter_MPEG4_SSE2
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Dequant_Inter_MPEG4_SSE2:

;SKL_RDTSC_IN

  mov eax, [esp+16] ; q
  mov ecx, [esp+12] ; Q
  shl eax, 8        ; q*256 (256=64*2*2)
  push ebx
  lea ebx, [ecx+eax +0x1e00 + 8*16 -4*8]  ; Mult = (Quant*M[i]*Mult)<<1

  mov edx, [esp+4 + 4]  ; Out
  mov ecx, [esp+4 + 8]  ; In

  mov eax, -16
  pxor    xmm6, xmm6 ; mismatch sum

  push ebp
  mov ebp, [esp+8 +20] ; Rows

align 4
.Loop
  test ebp, ebp
  jz near .End2
  shr ebp, 2

  movdqa  xmm0, [ecx+8*eax + 8*16   ]   ; c = In[i]
  movdqa  xmm1, [ecx+8*eax + 8*16+16]
  add eax,4

    ; We use (2*c + sgn(c) - sgn(-c)) as multiplier
    ; so we handle the 3 cases: c<0, c==0, and c>0 in one shot.
    ; sgn(x) is the result of 'pcmpgtw 0,x':  0 if x>=0, -1 if x<0.

  movdqa  xmm2, xmm0
  movdqa  xmm3, xmm1
  psraw   xmm2, 16      ; extract sign mask
  psraw   xmm3, 16

  paddsw  xmm0, xmm2    ; c += sign(c)
  paddsw  xmm1, xmm3
  paddw   xmm0, xmm0    ; c *= 2
  paddw   xmm1, xmm1

  pxor    xmm4, xmm4
  pxor    xmm5, xmm5
  psubw   xmm4, xmm0    ; -c
  psubw   xmm5, xmm1
  psraw   xmm4, 16      ; xmm4 = sgn(-c)
  psraw   xmm5, 16

  psubsw  xmm0, xmm4    ; c  -= sgn(-c)
  psubsw  xmm1, xmm5

  pxor    xmm0, xmm2    ; negate if needed
  pxor    xmm1, xmm3

  movdqa  xmm4, [ebx +8*eax   ]  ; M[i]*Q
  movdqa  xmm5, [ebx +8*eax+16]
  pmulhw  xmm4, xmm0             ; high of c*M[i]*Q
  pmulhw  xmm5, xmm1
  pmullw  xmm0, [ebx +8*eax   ]  ; low  of c*M[i]*Q
  pmullw  xmm1, [ebx +8*eax+16]
  pcmpgtw xmm4, [Zero]
  pcmpgtw xmm5, [Zero]

  paddusw xmm0, xmm4
  paddusw xmm1, xmm5
  psrlw   xmm0, 5
  psrlw   xmm1, 5
  pxor    xmm0, xmm2    ; start negating back
  pxor    xmm1, xmm3
  psubusw xmm2, xmm4
  psubusw xmm3, xmm5

  psubw   xmm0, xmm2    ; finish negating back  
  psubw   xmm1, xmm3

  pxor    xmm6, xmm0    ; mismatch  
  pxor    xmm6, xmm1

  movdqa  [edx + 8*eax + 8*16 -4*8   ], xmm0   ; Out[i]  
  movdqa  [edx + 8*eax + 8*16 -4*8+16], xmm1

  jnz .Loop

    ; finish mismatch
.End:
  pmaddwd xmm6, [One]
  pshufd  xmm7, xmm6, 1110b
  paddd   xmm6, xmm7
  pshufd  xmm7, xmm6, 01b
  paddd   xmm6, xmm7
  pextrw ecx, xmm6, 0
  and ecx, 1
  xor ecx, 1
  xor word [edx + 2*63], cx

  pop ebp
  pop ebx

;SKL_RDTSC_OUT
  ret

.End2:
  pxor    xmm0,xmm0
.Loop_End2
  movdqa  [edx + 8*eax + 8*16   ], xmm0   ; Out[i]  
  movdqa  [edx + 8*eax + 8*16+16], xmm0
  add eax,4
  jnz .Loop_End2
  jmp .End

;//////////////////////////////////////////////////////////////////////
; Skl_Quant_Intra_MPEG4_MMX
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Quant_Intra_MPEG4_MMX:  ; 153c

;SKL_RDTSC_IN
  mov eax, [esp+ 8]     ; In
  mov edx, [esp+20]     ; DC_q
  movsx eax, word [eax] ; DC
  mov ecx, edx
  shr edx, 1            ; Bias = Mult/2

  cmp eax, 0
  jge .Pos
  neg edx
.Pos:
  add eax, edx
  cdq

  push edi
  push ebx
  
  mov edi, [esp+8 + 4]  ; Out    
  idiv  ecx             ; DC // DC_q
  mov edx, [esp+8 + 8]  ; In
  nop
  mov ecx, [esp+8 +12]  ; ecx = Q[0][q-1][][]
  nop
  mov ebx, [esp+8 +16]  ; q
  nop

  add edx, 8*16
  shl ebx, 8
  add edi, 8*16

   ; Bias = [ebx-0x080], Mult = [ebx-0x100]
  lea ebx, [ecx+ebx+8*16] ; => Mult[ecx]
  mov ecx, -16*8

.Loop
  movq  mm0, [edx+ecx   ] ; c = In[]
  movq  mm1, [edx+ecx+ 8]

  movq mm2, mm0
  movq mm3, mm1

  psraw mm2, 16      ; extract sign mask
  psraw mm3, 16

  pxor  mm0, mm2     ; take abs value with it
  pxor  mm1, mm3
  psubw mm0, mm2
  psubw mm1, mm3

  psllw mm0, 3       ; c<<3
  psllw mm1, 3

  paddsw mm0,  [ebx-0x080+ecx  ]  ; += Bias
  paddsw mm1,  [ebx-0x080+ecx+8]

  pmulhw  mm0, [ebx-0x100+ecx  ]  ; *= Mult
  pmulhw  mm1, [ebx-0x100+ecx+8]

  pxor  mm0, mm2     ; put back sign
  pxor  mm1, mm3

  add ecx, 2*8

  psubw mm0, mm2
  movq  [edi+ecx-2*8   ], mm0
  psubw mm1, mm3
  movq  [edi+ecx-2*8+ 8], mm1

  jl .Loop 

  mov [edi-8*16], ax         ; Out[0] = ax
  pop ebx
  pop edi
;SKL_RDTSC_OUT

    ret


;//////////////////////////////////////////////////////////////////////
; Skl_Quant_Inter_MPEG4_MMX
;//////////////////////////////////////////////////////////////////////

align 16
Skl_Quant_Inter_MPEG4_MMX:  ; 126c

  push edi
  pxor mm7, mm7         ; SAD

  mov edi, [esp+4 + 4]  ; Out
  mov edx, [esp+4 + 8]  ; In
  mov eax, [esp+4 +16]  ; q

  mov ecx, [esp+4 +12]  ; ecx = Q[0][q-1][][]
  add edx, 8*16
  shl eax, 8
  add edi, 8*16

   ; Bias = [ecx-0x080], Mult = [ecx-0x100]
   ; No bias is used, so we lea[] now...
  lea eax, [ecx+eax-0x100+8*16] ; => Mult[ecx]
  mov ecx, -16

.Loop
  movq   mm0, [edx+8*ecx   ] ; c = In[]
  movq   mm1, [edx+8*ecx+ 8]

  movq  mm2, mm0
  movq  mm3, mm1
  psraw mm2, 16      ; extract sign mask
  psraw mm3, 16

  pxor   mm0, mm2    ; take abs value
  pxor   mm1, mm3
  psubw  mm0, mm2
  psubw  mm1, mm3

  psllw  mm0, 3      ; c<<3
  pmulhw mm0, [eax+ 8*ecx   ]  ; *= Mult
  psllw  mm1, 3
  pmulhw mm1, [eax+ 8*ecx +8]

  paddw  mm7, mm0    ; update SAD
  paddw  mm7, mm1
  pxor   mm0, mm2    ; put back sign
  pxor   mm1, mm3
  psubw  mm0, mm2
  psubw  mm1, mm3

  movq  [edi+ 8*ecx   ], mm0
  movq  [edi+ 8*ecx+ 8], mm1

  add ecx,2
  jl .Loop 

  pop edi
  pmaddwd mm7, [One]
  movq    mm0, mm7
  psrlq   mm7, 32
  paddd   mm0, mm7
  movd  eax, mm0    ; return SAD

  ret

;//////////////////////////////////////////////////////////////////////

Skl_Quant_Zero16_MMX:
  mov eax, [esp+4]
  pxor mm7, mm7
  movq [eax     ], mm7
  movq [eax+ 1*8], mm7
  movq [eax+ 2*8], mm7
  movq [eax+ 3*8], mm7
  ret

Skl_Quant_Zero_MMX:
  mov eax, [esp+4]
  pxor mm7, mm7
  movq [eax     ], mm7
  movq [eax+ 1*8], mm7
  movq [eax+ 2*8], mm7
  movq [eax+ 3*8], mm7
  movq [eax+ 4*8], mm7
  movq [eax+ 5*8], mm7
  movq [eax+ 6*8], mm7
  movq [eax+ 7*8], mm7
  movq [eax+ 8*8], mm7
  movq [eax+ 9*8], mm7
  movq [eax+10*8], mm7
  movq [eax+11*8], mm7
  movq [eax+12*8], mm7
  movq [eax+13*8], mm7
  movq [eax+14*8], mm7
  movq [eax+15*8], mm7
  ret

Skl_Quant_Zero_SSE2:
  mov eax, [esp+4]
  pxor xmm7, xmm7
  movdqa [eax     ], xmm7
  movdqa [eax+ 2*8], xmm7
  movdqa [eax+ 4*8], xmm7
  movdqa [eax+ 6*8], xmm7
  movdqa [eax+ 8*8], xmm7
  movdqa [eax+10*8], xmm7
  movdqa [eax+12*8], xmm7
  movdqa [eax+14*8], xmm7
  ret

Skl_Quant_Zero16_SSE2:
  mov eax, [esp+4]
  pxor xmm7, xmm7
  movdqa [eax     ], xmm7
  movdqa [eax+ 2*8], xmm7
  ret

;//////////////////////////////////////////////////////////////////////
