unit CsaTestData;
// Unit converted from: FFdecsa_test_testcases.h (FFDecsa-1.0.0)

interface


// TEST DATA

////////// used as a wrong key
const test_invalid_key:Array[0..$08-1] of Byte = (
    $0f, $1e, $2d, $3c, $4b, $5a, $69, $78
);


////////// test 1: odd key

const test_1_key: Array[0..7] of Byte = (
    $07, $e0, $1b, $02, $c9, $e0, $45, $ee
);
const test_1_encrypted: Array[0..187] of Byte = (
    $47, $00, $00, $d0,
    $de, $cf, $0a, $0d, $b2, $d7, $c4, $40, $de, $5d, $63, $18, $5a, $98, $17, $aa,
    $c9, $bc, $27, $c6, $cb, $49, $40, $48, $fd, $20, $b7, $05, $5b, $27, $cb, $eb,
    $9a, $f0, $ac, $45, $6d, $56, $f4, $7b, $6f, $a0, $57, $f3, $9b, $f7, $a2, $c7,
    $d4, $68, $24, $00, $2f, $28, $13, $96, $94, $a8, $7c, $f4, $6f, $07, $2a, $0e,
    $e8, $a1, $eb, $c7, $80, $ac, $1f, $79, $bf, $5d, $b6, $10, $7c, $2e, $52, $e9,
    $34, $2c, $a8, $39, $01, $73, $04, $24, $a8, $1e, $db, $5b, $cb, $24, $f6, $31,
    $ab, $02, $6b, $f9, $f6, $f7, $e9, $52, $ad, $cf, $62, $0f, $42, $f6, $66, $5d,
    $c0, $86, $f2, $7b, $40, $20, $a9, $bd, $1f, $fd, $16, $ad, $2e, $75, $a6, $a0,
    $85, $f3, $9c, $31, $20, $4e, $fb, $95, $61, $78, $ce, $10, $c1, $48, $5f, $d3,
    $61, $05, $12, $f4, $e2, $04, $ae, $e0, $86, $01, $56, $55, $b1, $0f, $a6, $33,
    $95, $20, $92, $f0, $be, $39, $31, $e1, $2a, $f7, $93, $b4, $f7, $e4, $f1, $85,
    $ae, $50, $f1, $63, $d4, $5d, $9c, $6c
);

const test_1_expected: Array[0..187] of Byte = (
    $47, $00, $00, $d0,
    $af, $be, $fb, $ef, $be, $fb, $ef, $be, $fb, $ef, $be, $fb, $e6, $b5, $ad, $7c,
    $f9, $f3, $e5, $b1, $6c, $7c, $f9, $f3, $e6, $b5, $ad, $6b, $5f, $3e, $7c, $f9,
    $6c, $5b, $1f, $3e, $7c, $f9, $ad, $6b, $5a, $d7, $cf, $9f, $3e, $5b, $16, $c7,
    $cf, $9f, $3e, $6b, $5a, $d6, $b5, $f3, $e7, $cf, $96, $c5, $b1, $f3, $e7, $cf,
    $9a, $d6, $b5, $ad, $7c, $f9, $f3, $e5, $b1, $6c, $7c, $f9, $f3, $e6, $b5, $ad,
    $6b, $5f, $3e, $7c, $f9, $6c, $5b, $1f, $3e, $7c, $f9, $ad, $6b, $5a, $d7, $cf,
    $9f, $3e, $5b, $16, $c7, $cf, $9f, $3e, $6b, $5a, $d6, $b5, $f3, $e7, $cf, $96,
    $c5, $b1, $f3, $e7, $cf, $9a, $d6, $b5, $ad, $7c, $f9, $f3, $e5, $b1, $6c, $7c,
    $f9, $f3, $e6, $b5, $ad, $6b, $5f, $3e, $7c, $f9, $6c, $5b, $1f, $3e, $7c, $f9,
    $ad, $6b, $5a, $d7, $cf, $9f, $3e, $5b, $16, $c7, $cf, $9f, $3e, $6b, $5a, $d6,
    $b5, $f3, $e7, $cf, $96, $c5, $b1, $f3, $e7, $cf, $9a, $d0, $00, $00, $00, $00,
    $ff, $fc, $44, $00, $66, $b1, $11, $11
);
const test_1_expected_stream: Array[0..175] of Byte = (
    $dc, $15, $de, $f1, $4a, $f1, $f8, $2c,
    $75, $c8, $3a, $1f, $bf, $67, $19, $e1,
    $f4, $6c, $78, $99, $48, $af, $ef, $94,
    $71, $6b, $23, $9e, $29, $69, $2d, $a1,
    $8a, $bb, $f4, $16, $68, $a5, $7f, $14,
    $a9, $37, $24, $05, $5e, $dd, $ec, $4b,
    $b5, $cb, $7f, $1d, $a7, $09, $2a, $ce,
    $c4, $30, $83, $fd, $d9, $88, $a9, $f3,
    $85, $9c, $38, $31, $88, $ac, $74, $02,
    $44, $dc, $b7, $81, $07, $c8, $1b, $03,
    $9c, $76, $be, $e9, $4d, $3e, $19, $ad,
    $e1, $f1, $a5, $13, $e8, $c0, $12, $57,
    $68, $b1, $9c, $6c, $9f, $58, $78, $ee,
    $4f, $5b, $33, $1e, $c6, $29, $fc, $40,
    $58, $22, $a2, $d8, $32, $dd, $29, $4f,
    $2b, $e1, $ef, $e4, $bb, $f2, $60, $94,
    $6c, $c5, $51, $ec, $35, $4c, $27, $c6,
    $9d, $73, $e0, $f4, $2b, $fa, $62, $12,
    $cd, $44, $be, $57, $fe, $80, $e7, $a9,
    $3c, $49, $42, $b6, $ed, $05, $57, $00,
    $d2, $25, $90, $b3, $e4, $65, $8f, $d6,
    $4e, $0c, $73, $30, $3b, $68, $48, $dd
// stream ^ sb
//    $02, $48, $bd, $e9, $10, $69, $ef, $86,
//    $bc, $74, $1d, $d9, $74, $2e, $59, $a9,
//    $09, $4c, $cf, $9c, $13, $88, $24, $7f,
//    $eb, $9b, $8f, $db, $44, $3f, $d9, $da,
);
const test_1_expected_block: Array[0..183] of Byte = (
    $ad, $f6, $46, $06, $ae, $92, $00, $38,
    $47, $9b, $a3, $22, $92, $9b, $f4, $d5,
    $f0, $bf, $2a, $2d, $7f, $f4, $dd, $8c,
    $0d, $2e, $22, $b0, $1b, $01, $a5, $23,
    $89, $40, $bc, $db, $8f, $ab, $70, $b8,
    $27, $88, $cf, $9a, $4f, $ae, $e9, $1a,
    $ee, $fc, $3d, $82, $92, $d8, $b5, $33,
    $cb, $5e, $fe, $ff, $e8, $d7, $51, $45,
    $a0, $17, $3b, $8c, $88, $7b, $d5, $0e,
    $c1, $9c, $63, $41, $f5, $5d, $aa, $8a,
    $5f, $37, $5b, $ce, $7f, $76, $b4, $83,
    $74, $8f, $37, $47, $75, $6d, $2c, $ca,
    $5a, $40, $a5, $75, $1a, $61, $81, $8d,
    $e4, $87, $17, $d0, $75, $ee, $9a, $6b,
    $82, $6e, $47, $92, $d3, $32, $59, $5a,
    $03, $6e, $8a, $26, $7e, $0d, $f7, $7d,
    $f4, $4e, $79, $49, $59, $6f, $27, $2b,
    $80, $8f, $9e, $5b, $d6, $c0, $b0, $0b,
    $e6, $2e, $b2, $d5, $80, $10, $7f, $c1,
    $bf, $ae, $1f, $d9, $6d, $57, $3c, $37,
    $4d, $21, $e4, $c8, $85, $44, $cf, $a0,
    $07, $93, $18, $83, $ef, $35, $d4, $b1,
    $ff, $fc, $44, $00, $66, $b1, $11, $11
);

const test_1_expected_kb:Array [0..55] of Byte = (
    $EE, $45, $E0, $C9, $02, $1B, $E0, $07,
    $46, $A4, $1C, $26, $7B, $0C, $01, $ED,
    $93, $99, $C3, $14, $C4, $4A, $8D, $54,
    $19, $82, $39, $D1, $33, $B0, $33, $52,
    $75, $62, $80, $3A, $C8, $83, $5E, $23,
    $A2, $57, $0C, $C4, $2C, $2D, $D2, $98,
    $A0, $6C, $77, $29, $11, $42, $49, $CE
);
const test_1_expected_kk:Array[0..55] of Byte = (
    $5e, $9d, $ff, $2e, $bb, $aa, $a8, $e9,
    $f6, $0e, $ff, $7c, $da, $ce, $55, $03,
    $d9, $de, $79, $f5, $2c, $af, $06, $f8,
    $b2, $c9, $f8, $78, $54, $f9, $d1, $e7,
    $eb, $be, $d7, $eb, $25, $e9, $17, $99,
    $bf, $24, $ce, $2a, $73, $fe, $f9, $bc,
    $d9, $55, $91, $cf, $e0, $c9, $df, $88
);


////////// test 2: even key
const test_2_key: Array[0..7] of Byte = (
    $07, $06, $05, $04, $03, $02, $01, $00
);
const test_2_encrypted: Array[0..187] of Byte = (
    $47, $00, $00, $90,
    $00, $01, $02, $03, $04, $05, $06, $07, $08, $09, $0a, $0b, $0c, $0d, $0e, $0f,
    $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $1a, $1b, $1c, $1d, $1e, $1f,
    $20, $21, $22, $23, $24, $25, $26, $27, $28, $29, $2a, $2b, $2c, $2d, $2e, $2f,
    $30, $31, $32, $33, $34, $35, $36, $37, $38, $39, $3a, $3b, $3c, $3d, $3e, $3f,
    $40, $41, $42, $43, $44, $45, $46, $47, $48, $49, $4a, $4b, $4c, $4d, $4e, $4f,
    $50, $51, $52, $53, $54, $55, $56, $57, $58, $59, $5a, $5b, $5c, $5d, $5e, $5f,
    $60, $61, $62, $63, $64, $65, $66, $67, $68, $69, $6a, $6b, $6c, $6d, $6e, $6f,
    $70, $71, $72, $73, $74, $75, $76, $77, $78, $79, $7a, $7b, $7c, $7d, $7e, $7f,
    $80, $81, $82, $83, $84, $85, $86, $87, $88, $89, $8a, $8b, $8c, $8d, $8e, $8f,
    $90, $91, $92, $93, $94, $95, $96, $97, $98, $99, $9a, $9b, $9c, $9d, $9e, $9f,
    $a0, $a1, $a2, $a3, $a4, $a5, $a6, $a7, $a8, $a9, $aa, $ab, $ac, $ad, $ae, $af,
    $b0, $b1, $b2, $b3, $b4, $b5, $b6, $b7
);
const test_2_expected: Array[0..187] of Byte = (
    $47, $00, $00, $90,
    $2d, $0a, $47, $20, $18, $11, $9c, $8a, $d1, $2a, $65, $6b, $89, $e4, $35, $2b,
    $c2, $b5, $90, $61, $d1, $7e, $02, $e1, $3f, $46, $70, $cf, $77, $91, $2f, $22,
    $93, $c1, $6c, $fe, $49, $ad, $7c, $c2, $af, $86, $1b, $a3, $29, $be, $aa, $64,
    $f0, $22, $b9, $5e, $98, $aa, $60, $ef, $df, $d6, $44, $77, $e6, $bf, $bb, $94,
    $b2, $0a, $63, $0e, $5c, $f2, $ac, $b4, $49, $cc, $9e, $4f, $94, $4c, $30, $12,
    $e8, $55, $c2, $44, $a4, $52, $cb, $61, $81, $c9, $b6, $a6, $6b, $ef, $af, $a6,
    $71, $1d, $7b, $58, $2f, $fa, $d1, $0c, $07, $9d, $1f, $35, $87, $be, $02, $9f,
    $20, $c6, $60, $8f, $1c, $30, $0f, $96, $d0, $71, $d6, $51, $10, $df, $5b, $f6,
    $44, $2f, $80, $28, $b7, $ec, $23, $59, $4b, $94, $0b, $9a, $74, $a1, $1f, $f7,
    $9e, $76, $b4, $df, $bb, $3c, $8c, $88, $97, $22, $56, $73, $16, $05, $ac, $f9,
    $4f, $77, $9d, $38, $a0, $6b, $05, $d2, $e6, $15, $01, $b1, $5c, $c9, $62, $a9,
    $9b, $1a, $6a, $1a, $cf, $e6, $a8, $ba
);


////////// test 3: even key
const test_3_key: Array[0..7] of Byte = (
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
);
const test_3_encrypted: Array[0..187] of Byte = (
    $47, $00, $00, $90,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
    $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
);
const test_3_expected: Array[0..187] of Byte = (
    $47, $00, $00, $90,
    $fe, $91, $a7, $2f, $bf, $b0, $6a, $54, $c1, $e4, $33, $27, $18, $d5, $9c, $43,
    $ea, $aa, $6b, $38, $5c, $e7, $ae, $c9, $ac, $ec, $ef, $c3, $51, $7d, $53, $47,
    $a0, $a7, $6d, $73, $8a, $9d, $16, $7d, $05, $2d, $d6, $6b, $f4, $8d, $4b, $81,
    $98, $2f, $46, $a5, $34, $84, $f3, $70, $a4, $e9, $04, $84, $7b, $87, $79, $3c,
    $01, $25, $b5, $fc, $3d, $d0, $25, $ea, $2f, $91, $f0, $3f, $7f, $d4, $8e, $1e,
    $36, $83, $22, $91, $57, $92, $36, $0b, $44, $a5, $cc, $5e, $ef, $44, $3e, $f8,
    $e9, $7b, $5e, $0c, $ea, $b2, $50, $39, $b7, $ea, $c4, $fb, $e4, $37, $f8, $85,
    $c2, $dc, $01, $98, $01, $2a, $44, $d3, $75, $10, $38, $f4, $85, $3e, $c9, $f7,
    $e7, $e4, $ec, $40, $3d, $8f, $a5, $d2, $8a, $ca, $62, $03, $3f, $65, $28, $8d,
    $f5, $56, $a7, $ea, $d1, $0d, $70, $82, $bc, $90, $59, $f8, $3e, $08, $c9, $e1,
    $97, $ef, $82, $43, $35, $41, $3e, $7f, $00, $96, $3f, $90, $e5, $1e, $96, $ba,
    $ce, $6d, $d2, $54, $ce, $84, $76, $3c
);


////////// odd key, only 80 ($50) bytes of payload (10 groups of 8 bytes + 0 byte residue)
const test_p_10_0_key: Array[0..7] of Byte = (
    $2d, $11, $5f, $9d, $29, $bf, $7f, $67
);
const test_p_10_0_encrypted: Array[0..187] of Byte = (
  $47, $00, $7a, $be,
  $67, $00, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $71, $a5, $7b, $8f, $f9, $87, $cb, $ac,
  $ea, $08, $0c, $02, $87, $7b, $ad, $10, $40, $28, $8e, $d4, $4e, $62, $c7, $74,
  $d6, $bb, $3a, $aa, $b0, $7b, $70, $be, $06, $c9, $dc, $07, $d2, $2d, $ab, $2d,
  $e2, $c6, $36, $a6, $da, $64, $61, $15, $d1, $6a, $40, $c0, $a9, $fb, $3f, $b2,
  $6d, $a5, $59, $ae, $57, $88, $6b, $0e, $00, $ae, $ce, $64, $ee, $fd, $b1, $7f,
  $78, $9c, $12, $42, $be, $30, $8a, $a3
);
const test_p_10_0_expected: Array[0..187] of Byte = (
  $47, $00, $7a, $be,
  $67, $00, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $a7, $ca, $32, $af, $2e, $6a, $ea, $05,
  $39, $33, $67, $5d, $a3, $61, $0f, $34, $40, $6c, $1a, $b3, $ee, $54, $64, $d5,
  $a3, $01, $95, $87, $9d, $3d, $38, $c5, $82, $8b, $8d, $ab, $ad, $93, $0f, $e8,
  $f9, $bd, $52, $98, $59, $b2, $41, $95, $cd, $ae, $9b, $3e, $df, $db, $14, $9b,
  $a9, $22, $0d, $2d, $61, $f5, $f2, $52, $83, $20, $ae, $b8, $83, $52, $02, $ee,
  $bd, $d2, $94, $6c, $27, $58, $55, $d0
);


////////// odd key, only 14 ($0e) bytes of payload (1 group of 8 bytes + 6 byte residue)
const test_p_1_6_key: Array[0..7] of Byte = (
    $2d, $11, $5f, $9d, $29, $bf, $7f, $67
);
const test_p_1_6_encrypted: Array[0..187] of Byte = (
  $47, $00, $7a, $b7,
  $a9, $00, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $c0, $5e, $fb, $c8, $4a, $63,
  $e3, $3c, $11, $d9, $e0, $75, $8e, $f2 
);
const test_p_1_6_expected: Array[0..187] of Byte = (
  $47, $00, $7a, $b7,
  $a9, $00, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff,
  $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $5a, $2c, $ee, $b3, $de, $92,
  $e7, $a6, $6c, $aa, $99, $84, $e4, $00 
);

implementation

end.
