/* FFdecsa -- fast decsa algorithm
 *
 * Copyright (C) 2003-2004  fatih89r
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifndef FFDECSA_H
#define FFDECSA_H

#ifdef MAKE_DLL
#define DOEXP __declspec(dllexport)
#else
#define DOEXP 
#endif


// ********* Parallelization stuff, large speed differences are possible

// possible choices
#define PARALLEL_32_4CHAR     320
#define PARALLEL_32_4CHARA    321
#define PARALLEL_32_INT       322
#define PARALLEL_64_8CHAR     640
#define PARALLEL_64_8CHARA    641
#define PARALLEL_64_2INT      642
#define PARALLEL_64_LONG      643
#define PARALLEL_64_MMX       644 //* Best ?
#define PARALLEL_128_16CHAR  1280
#define PARALLEL_128_16CHARA 1281
#define PARALLEL_128_4INT    1282
#define PARALLEL_128_2LONG   1283
#define PARALLEL_128_2MMX    1284
#define PARALLEL_128_SSE     1285 // * Exception

//////// our choice //////////////// our choice //////////////// our choice //////////////// our choice ////////

#define PARALLEL_MODE PARALLEL_64_MMX

//////// our choice //////////////// our choice //////////////// our choice //////////////// our choice ////////

#include "parallel_generic.h"
//// conditionals
#if PARALLEL_MODE==PARALLEL_32_4CHAR
#include "parallel_032_4char.h"
#elif PARALLEL_MODE==PARALLEL_32_4CHARA
#include "parallel_032_4charA.h"
#elif PARALLEL_MODE==PARALLEL_32_INT
#include "parallel_032_int.h"
#elif PARALLEL_MODE==PARALLEL_64_8CHAR
#include "parallel_064_8char.h"
#elif PARALLEL_MODE==PARALLEL_64_8CHARA
#include "parallel_064_8charA.h"
#elif PARALLEL_MODE==PARALLEL_64_2INT
#include "parallel_064_2int.h"
#elif PARALLEL_MODE==PARALLEL_64_LONG
#include "parallel_064_long.h"
#elif PARALLEL_MODE==PARALLEL_64_MMX
#include "parallel_064_mmx.h"
#elif PARALLEL_MODE==PARALLEL_128_16CHAR
#include "parallel_128_16char.h"
#elif PARALLEL_MODE==PARALLEL_128_16CHARA
#include "parallel_128_16charA.h"
#elif PARALLEL_MODE==PARALLEL_128_4INT
#include "parallel_128_4int.h"
#elif PARALLEL_MODE==PARALLEL_128_2LONG
#include "parallel_128_2long.h"
#elif PARALLEL_MODE==PARALLEL_128_2MMX
#include "parallel_128_2mmx.h"
#elif PARALLEL_MODE==PARALLEL_128_SSE
#include "parallel_128_sse.h"
#else
#error "unknown/undefined parallel mode"
#endif

// ******************** KEYS SET ******************************

struct csa_key_t{
	unsigned char ck[8];
// used by stream
        int iA[8];  // iA[0] is for A1, iA[7] is for A8
        int iB[8];  // iB[0] is for B1, iB[7] is for B8
// used by stream (group)
        group ck_g[8][8]; // [byte][bit:0=LSB,7=MSB]
        group iA_g[8][4]; // [0 for A1][0 for LSB]
        group iB_g[8][4]; // [0 for B1][0 for LSB]
// used by block
	unsigned char kk[56];
// used by block (group)
	__attribute__((aligned(16))) batch kkmulti[56]; // many times the same byte in every batch
};


struct csa_keys_t{
  struct csa_key_t even;
  struct csa_key_t odd;
};

typedef struct csa_keys_t csa_keys_set;

// ******************** External Functions *************************

int __stdcall DOEXP get_keyset_size(void);

//----- public interface

// -- how many packets can be decrypted at the same time
// This is an info about internal decryption parallelism.
// You should try to call decrypt_packets with more packets than the number
// returned here for performance reasons (use get_suggested_cluster_size to know
// how many).
int __stdcall DOEXP get_internal_parallelism(void);

// -- how many packets you should have in a cluster when calling decrypt_packets
// This is a suggestion to achieve optimal performance; typically a little
// higher than what get_internal_parallelism returns.
// Passing less packets could slow down the decryption.
// Passing more packets is never bad (if you don't spend a lot of time building
// the list).
// !!! DELETED !!!
//int __stdcall DOEXP get_suggested_cluster_size(void);

// -- set control words, 8 bytes each
void __stdcall DOEXP set_control_words(unsigned char *ev, unsigned char *od,csa_keys_set *myKeys);

// -- decrypt many TS packets
// This interface is a bit complicated because it is designed for maximum speed.
// Please read doc/how_to_use.txt.
int __stdcall DOEXP decrypt_packets(unsigned char **cluster,csa_keys_set *myKeys);

#endif
