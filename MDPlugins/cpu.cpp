/*
 * Softcam plugin to VDR (C++)
 *
 * This code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include <ctype.h>
#include "stdafx.h"
//#include <vdr/tools.h>

#include "common.h"
#include "data.h"

#include "debug.h"
#include "cpu.h"

#include "xnprintf.h"

#include "main.h"
extern class DllClass m_DLLInstance;

static bool pc80flag=false;

// -- cMapMem ------------------------------------------------------------------

cMapMem::cMapMem(unsigned short Offset, int Size)
{
	offset=Offset; size=Size;
	if((mem=(unsigned char *)malloc(size)))
		memset(mem,0,size);
	dn(printf("mapmem: new map off=%04x size=%04x\n",offset,size))
}

cMapMem::~cMapMem()
{
	free(mem);
}

bool cMapMem::IsFine(void)
{
	return (mem!=0);
}

unsigned char cMapMem::Get(unsigned short ea)
{
	return (ea>=offset && ea<offset+size) ? mem[ea-offset] : 0;
}

void cMapMem::Set(unsigned short ea, unsigned char val)
{
	if(ea>=offset && ea<offset+size)
		mem[ea-offset]=val;
}

// -- cMapRom ------------------------------------------------------------------

cMapRom::cMapRom(unsigned short Offset, const char *Filename, int InFileOffset)
{
	offset=Offset; addr=0;
	fm=filemaps.GetFileMap(Filename,FILEMAP_DOMAIN,false);
	if(fm && fm->Map()) {
		addr=fm->Addr()+InFileOffset;
		size=fm->Size()-InFileOffset;
		dn(printf("maprom: new map off=%04x size=%04x\n",offset,size))
		}
}

cMapRom::~cMapRom()
{
	if(fm) fm->Unmap();
}

bool cMapRom::IsFine(void)
{
	return (addr!=0);
}

unsigned char cMapRom::Get(unsigned short ea)
{
	return (ea>=offset && ea<offset+size) ? addr[ea-offset] : 0;
}

void cMapRom::Set(unsigned short ea, unsigned char val)
{
	Show_Diss = m_DLLInstance.m_show_diss;
	if(ea>=offset && ea<offset+size) if (Show_Diss==1) de(printf("[ROM] "))
	// this is a ROM!
}

// -- cMapEeprom ---------------------------------------------------------------

cMapEeprom::cMapEeprom(unsigned short Offset, const char *Filename, int OtpSize, int InFileOffset)
{
	offset=Offset; otpSize=OtpSize; addr=0;
	fm=filemaps.GetFileMap(Filename,FILEMAP_DOMAIN,true);
	if(fm && fm->Map()) {
		addr=fm->Addr()+InFileOffset;
		size=fm->Size()-InFileOffset;
		dn(printf("mapeeprom: new map off=%04x size=%04x otp=%04x\n",offset,size,otpSize))
		}
}

cMapEeprom::~cMapEeprom()
{
	if(fm) fm->Unmap();
}

bool cMapEeprom::IsFine(void)
{
	return (addr!=0);
}

unsigned char cMapEeprom::Get(unsigned short ea)
{
	return (ea>=offset && ea<offset+size) ? addr[ea-offset] : 0;
}

void cMapEeprom::Set(unsigned short ea, unsigned char val)
{
	Show_Diss = m_DLLInstance.m_show_diss;
	if(ea>=offset && ea<offset+otpSize) {
		if(addr[ea-offset]==0) {
			addr[ea-offset]=val;
			if (Show_Diss==1) de(printf("[OTP-SET] "))
			}
		if (Show_Diss==1) de(printf("[OTP] "))
		}
	if(ea>=offset+otpSize && ea<offset+size) {
		addr[ea-offset]=val;
		if (Show_Diss==1) de(printf("[EEP] "))
		}
}

// -- c6805 --------------------------------------------------------------------

#define HILO(ea)      ((Get(ea)<<8)+Get((ea)+1))
#define HILOS(s,ea)   ((Get((ea),(s))<<8)+Get((ea)+1,(s)))
#define PAGEOFF(ea,s) (((ea)&0x8000) ? pageMap[s]:0)

static char * __paddr(unsigned char s, unsigned short ea)
{
	static char addr[32];
	_snprintf(addr,sizeof(addr),((ea&0x8000) && s>0)?"%2$x:%1$04x":"%04x",ea,s);
	return addr;
}
#define PADDR(s,ea)   __paddr(s,ea)

c6805::c6805(void) {
	cc.c=0; cc.z=0; cc.n=0; cc.i=0; cc.h=0; cc.v=1;
	pc=0; a=0; x=0; y=0; cr=dr=0; sp=spHi=0x100; spLow=0xC0;
	fAttributeRead = false;
	ClearBreakpoints();
	InitMapper();
#ifdef DEBUG_STAT
	memset(stats,0,sizeof(stats));
#endif
}

c6805::~c6805()
{
#ifdef DEBUG_STAT
	int i, j, sort[256];
	bool done=false;
	for(i=0 ; i<256 ; i++) sort[i]=i;
	for(i=0 ; i<256 && !done ; i++)
		for(j=0 ; j<255 ; j++)
			if(stats[sort[j]]<stats[sort[j+1]]) {
				int x=sort[j];
				sort[j]=sort[j+1];
				sort[j+1]=x;
				done=false;
				}
	printf("6805: opcode statistics\n");
	for(i=0 ; i<256 ; i++)
		if((j=stats[sort[i]])) printf("6805: opcode %02x: %d\n",sort[i],j);
#endif
	ClearMapper();
}

bool c6805::AddMapper(cMap *map, unsigned short start, int size, unsigned char seg)
{
	if(map && map->IsFine()) {
		if(nextMapper<MAX_MAPPER) {
			mapper[nextMapper]=map;
			memset(&mapMap[start+PAGEOFF(start,seg)],nextMapper,size);
			nextMapper++;
			return true;
			}
		else if (Show_Diss==1) dee(printf("6805: too many mappers\n"))
		}
	else if (Show_Diss==1) dee(printf("6805: mapper not ready\n"))
	delete map;
	return false;
}

void c6805::ResetMapper(void)
{
	ClearMapper(); InitMapper();
}

void c6805::InitMapper(void)
{
	memset(mapMap,0,sizeof(mapMap));
	memset(mapper,0,sizeof(mapper));
	mapper[0]=new cMapMem(0,PAGE_SIZE*2);
	nextMapper=1;
	memset(pageMap,0,sizeof(pageMap));
	pageMap[0]=PAGE_SIZE*0;
	pageMap[1]=PAGE_SIZE*1;
	pageMap[2]=PAGE_SIZE*2;
	pageMap[0x80]=PAGE_SIZE*3;
}

void c6805::ClearMapper(void)
{
	for(int i=0; i<MAX_MAPPER; i++) delete mapper[i];
}

unsigned char c6805::Get(unsigned short ea) const
{
	return mapper[mapMap[ea+PAGEOFF(ea,cr)]&0x7f]->Get(ea);
}

unsigned char c6805::Get(unsigned char seg, unsigned short ea) const
{
	return mapper[mapMap[ea+PAGEOFF(ea,seg)]&0x7f]->Get(ea);
}

void c6805::Set(unsigned short ea, unsigned char val)
{
	unsigned char mapId=mapMap[ea+PAGEOFF(ea,cr)];
	if(!(mapId&0x80)) mapper[mapId&0x7f]->Set(ea,val);
}

void c6805::Set(unsigned char seg, unsigned short ea, unsigned char val)
{
	unsigned char mapId=mapMap[ea+PAGEOFF(ea,seg)];
	if(!(mapId&0x80)) mapper[mapId&0x7f]->Set(ea,val);
}

void c6805::ForceSet(unsigned short ea, unsigned char val, bool ro)
{
	mapMap[ea]=0;     		// reset to RAM map
	Set(0,ea,val);      		// set value
	if(ro) mapMap[ea]|=0x80; 	// protect byte
}

void c6805::SetMem(unsigned short addr, const unsigned char *data, int len, unsigned char seg)
{
	while(len>0) { Set(seg,addr++,*data++); len--; }
}

void c6805::GetMem(unsigned short addr, unsigned char *data, int len, unsigned char seg) const
{
	while(len>0) { *data++=Get(seg,addr++); len--; }
}

void c6805::SetSp(unsigned short SpHi, unsigned short SpLow)
{
	spHi =sp=SpHi;
	spLow   =SpLow;
}

void c6805::SetPc(unsigned short addr)
{
	pc=addr;
}

void c6805::AddBreakpoint(unsigned short addr)
{
	Show_Diss = m_DLLInstance.m_show_diss;
	if(numBp<MAX_BREAKPOINTS) {
		bp[numBp++]=addr;
		if (Show_Diss==1) dee(printf("6805: setting breakpoint at 0x%04x\n",addr))
		}
	else if (Show_Diss==1) dee(printf("6805: too many breakpoints\n"))
}

void c6805::ClearBreakpoints(void)
{
	numBp=0;
	memset(bp,0,sizeof(bp));
}

static const char * const ops[] = {
//         0x00    0x01    0x02    0x03    0x04    0x05    0x06    0x07    0x08    0x09    0x0a    0x0b    0x0c    0x0d    0x0e    0x0f
/* 0x00 */ "BRSET","BRCLR","BRSET","BRCLR","BRSET","BRCLR","BRSET","BRCLR","BRSET","BRCLR","BRSET","BRCLR","BRSET","BRCLR","BRSET","BRCLR",
/* 0x10 */ "BSET", "BCLR", "BSET", "BCLR", "BSET", "BCLR", "BSET", "BCLR", "BSET", "BCLR", "BSET", "BCLR", "BSET", "BCLR", "BSET", "BCLR",
/* 0x20 */ "BRA",  "BRN",  "BHI",  "BLS",  "BCC",  "BCS",  "BNE",  "BEQ",  "BHCC", "BHCS", "BPL",  "BMI",  "BMC",  "BMS",  "BIL",  "BIH",
/* 0x30 */ "NEG",  "pre31","pre32","COM",  "LSR",  "op35", "ROR",  "ASR",  "ASL",  "ROL",  "DEC",  "op3b", "INC",  "TST",  "SWAP", "CLR",
/* 0x40 */ "NEG",  "op41", "MUL",  "COM",  "LSR",  "op45", "ROR",  "ASR",  "ASL",  "ROL",  "DEC",  "op4b", "INC",  "TST",  "SWAP", "CLR",
/* 0x50 */ "NEG",  "op51", "MUL",  "COM",  "LSR",  "op55", "ROR",  "ASR",  "ASL",  "ROL",  "DEC",  "op5b", "INC",  "TST",  "SWAP", "CLR",
/* 0x60 */ "NEG",  "op61", "op62", "COM",  "LSR",  "op65", "ROR",  "ASR",  "ASL",  "ROL",  "DEC",  "op6b", "INC",  "TST",  "SWAP", "CLR",
/* 0x70 */ "NEG",  "LDD",  "LDD",  "COM",  "LSR",  "LDD",  "ROR",  "ASR",  "ASL",  "ROL",  "DEC",  "TAD",  "INC",  "TST",  "SWAP", "CLR",
/* 0x80 */ "RTI",  "RTS",  "op82", "SWI",  "POPA", "POP%c","POPC", "PRTS", "PUSHA","PUSH%c","PUSHC","TDA", "TCA",  "PJSR", "STOP", "WAIT",
/* 0x90 */ "pre90","pre91","pre92","T%2$c%1$c","T%cS","TAS","TS%c","TA%c", "CLC",  "SEC",  "CLI",  "SEI",  "RSP",  "NOP",  "TSA",  "T%cA",
/* 0xa0 */ "SUB",  "CMP",  "SBC",  "CP%c", "AND",  "BIT",  "LDA",  "opa7", "EOR",  "ADC",  "ORA",  "ADD",  "PUSHD","BSR",  "LD%c", "POPD",
/* 0xb0 */ "SUB",  "CMP",  "SBC",  "CP%c", "AND",  "BIT",  "LDA",  "STA",  "EOR",  "ADC",  "ORA",  "ADD",  "JMP",  "JSR",  "LD%c", "ST%c",
/* 0xc0 */ "SUB",  "CMP",  "SBC",  "CP%c", "AND",  "BIT",  "LDA",  "STA",  "EOR",  "ADC",  "ORA",  "ADD",  "JMP",  "JSR",  "LD%c", "ST%c",
/* 0xd0 */ "SUB",  "CMP",  "SBC",  "CP%c", "AND",  "BIT",  "LDA",  "STA",  "EOR",  "ADC",  "ORA",  "ADD",  "JMP",  "JSR",  "LD%c", "ST%c",
/* 0xe0 */ "SUB",  "CMP",  "SBC",  "CP%c", "AND",  "BIT",  "LDA",  "STA",  "EOR",  "ADC",  "ORA",  "ADD",  "JMP",  "JSR",  "LD%c", "ST%c",
/* 0xf0 */ "SUB",  "CMP",  "SBC",  "CP%c", "AND",  "BIT",  "LDA",  "STA",  "EOR",  "ADC",  "ORA",  "ADD",  "JMP",  "JSR",  "LD%c", "ST%c",
	};
static const char * const vops[] = {
	"BVGT","BVLE","BVGE","BVLT","BVC","BVS"
	};

// Flags:
// 1 - read operant
// 2 - write operant
// 4 - use dr register
// 8 - special address mode in high nibble
static const char opFlags[] = {
//            00    01    02    03    04    05    06    07    08    09    0a    0b    0c    0d    0e    0f
/* 0x00 */  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x10 */  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
/* 0x20 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x30 */  0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x01, 0x03, 0x02,
/* 0x40 */  0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x01, 0x03, 0x02,
/* 0x50 */  0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x01, 0x03, 0x02,
/* 0x60 */  0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x01, 0x03, 0x02,
/* 0x70 */  0x03, 0xA9, 0xB9, 0x03, 0x03, 0xC9, 0x03, 0x03, 0x03, 0x03, 0x03, 0x88, 0x03, 0x01, 0x03, 0x02,
/* 0x80 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x90 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0xa0 */  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x88, 0x88, 0x01, 0x88,
/* 0xb0 */  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x02,
/* 0xc0 */  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x05, 0x05, 0x05, 0x05, 0x00, 0x00, 0x05, 0x06,
/* 0xd0 */  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x05, 0x05, 0x05, 0x05, 0x00, 0x00, 0x05, 0x06,
/* 0xe0 */  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x05, 0x05, 0x05, 0x05, 0x00, 0x00, 0x05, 0x06,
/* 0xf0 */  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x05, 0x05, 0x05, 0x05, 0x00, 0x00, 0x05, 0x06,
	};

// OP Code - Instruction Set Length
	static const unsigned int ISlen[] = {
//         0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
/* 0x00 */ 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
/* 0x10 */ 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
/* 0x20 */ 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
/* 0x30 */ 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
/* 0x40 */ 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x50 */ 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x60 */ 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
/* 0x70 */ 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x80 */ 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0x90 */ 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
/* 0xa0 */ 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x02, 0x02, 0x01,
/* 0xb0 */ 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
/* 0xc0 */ 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
/* 0xd0 */ 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
/* 0xe0 */ 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
/* 0xf0 */ 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};

int c6805::Run(int max_count)
{
// returns:
// 0 - breakpoint
// 1 - stack overflow
// 2 - instruction counter exeeded
// 3 - unsupported instruction

	Show_Diss = m_DLLInstance.m_show_diss;
	pc80flag=false;
	LoopCount=0;
	if (Show_Diss==1) dee(printf("6805: cr:-pc- aa xx yy dr -sp- VHINZC -mem@pc- -mem@sp-\n"))
	while (1) {
		if (sp<spLow) {
			printf("*** stack overflow (count=%d)\n",LoopCount);
			return 1;
			}
		if (spHi>spLow && sp>spHi) {
			printf("*** stack underflow (count=%d)\n",LoopCount);
			return 1;
			}
		LoopCount++;

		bool flag=(pc>=0x80 && pc<0xE0);
		if (pc80flag && !flag && Show_Diss==1) printf("6805: [...]\n");
		pc80flag=flag;

		sprintf(displaystr1,"  %.4X: ", pc);
	    sprintf(displaystr2,"");
		sprintf(txt,"%.2X", Get(pc));	strcat(displaystr1, txt);
		if (Show_Diss==1) de(printf("6805: %02x:%04x %02x %02x %02x %02x %04x %c%c%c%c%c%c %02x%02x%02x%02x %02x%02x%02x%02x ",
				       cr,pc,a,x,y,dr,sp,
				       cc.v?'V':'.',cc.h?'H':'.',cc.i?'I':'.',cc.n?'N':'.',cc.z?'Z':'.',cc.c?'C':'.',
				       Get(pc),Get(pc+1),Get(pc+2),Get(pc+3),Get(sp+1),Get(sp+2),Get(sp+3),Get(sp+4)))

		Stepper();
		unsigned char *ex=&x;
		unsigned short idx=*ex;
		indirect=false;
		bool paged=false, vbra=false;
		unsigned char ins=Get(pc++);
		char xs='X', xi='X';
 
	  // check pre-bytes
		switch (ins) {
			case 0x31: // use SP indexed or indirect paged mode (ST19)
				ins=Get(pc++);
				switch(ins) {
					case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: 
						vbra=true; 
						if (Show_Diss==1) printf("WARN: V-flag not yet calculated ");
						break;
					case 0x75:
					case 0x8D:
					case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7: case 0xC8: case 0xC9: case 0xCA: case 0xCB:
					case 0xCE: case 0xCF:
					case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7: case 0xD8: case 0xD9: case 0xDA: case 0xDB:
					case 0xDE: case 0xDF: 
						paged=true; 
						indirect=true; 
						break;
					case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7: case 0xE8: case 0xE9: case 0xEA: case 0xEB:
					case 0xEE: case 0xEF: 
						idx=sp; 
						de(xi='S') 
							break;
				}
				break;
			case 0x32: // use indirect SP indexed or indirect paged Y indexed mode (ST19)
				ins=Get(pc++);
				switch(ins) {
					case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: 
						vbra=true; 
						if (Show_Diss==1) printf("WARN: V-flag not yet calculated ");
						indirect=true; 
						break;
					case 0xC3:
					case 0xCE: case 0xCF:
					case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7: case 0xD8: case 0xD9: case 0xDA: case 0xDB:
					case 0xDE: case 0xDF: 
						paged=true; 
						indirect=true; 
						ex=&y; idx=*ex; 
						de(xs='Y') 
						de(xi='Y') 
						break;
					case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7: case 0xE8: case 0xE9: case 0xEA: case 0xEB:
					case 0xEE: case 0xEF: indirect=true; idx=sp; de(xi='S') break;
				}
				break;
			case 0x91: // use Y register with indirect addr mode (ST7)
				indirect=true;
				// fall through
			case 0x90: // use Y register (ST7)
				ex=&y; idx=*ex; de(xs='Y') de(xi='Y')
				ins=Get(pc++);
				break;
			case 0x92: // use indirect addr mode (ST7)
				indirect=true;
				ins=Get(pc++);
				break;
		}

		char str[8];
		if (!vbra) _snprintf(str,sizeof(str),ops[ins],xs,xs^1);
		else       _snprintf(str,sizeof(str),"%s",vops[ins-0x22]);
		sprintf(txt,"%-5s ", str); strcat(displaystr2, txt);
		if (pc80flag && Show_Diss==1) printf(txt);

	  // address decoding
		unsigned short ea=0;
		unsigned char flags=opFlags[ins];
		unsigned char pr=(flags&4) ? dr:cr;
		switch (((flags&8) ? flags:ins)>>4) {
			case 0x2:			// no or special address mode
			case 0x8:
			case 0x9:
				if (ISlen[ins] == 2) {
					sprintf(txt,"%.2X", Get(pc)); strcat(displaystr1, txt);
				}
				break;
			case 0xA:			// immediate
				if (ISlen[ins] == 2) {
					sprintf(txt,"%.2X", Get(pc)); strcat(displaystr1, txt);
				}
				sprintf(txt,"#%02x ",Get(pc)); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				ea=pc++; break;
			case 0x3:			// short
			case 0xB:
				sprintf(txt,"%.2X", Get(pc)); strcat(displaystr1, txt);
				ea=Get(pc++);
				
				if (ea==5) 
					//Special Attribute bit
					ea = 5;
				else {
					if(!indirect) {		// short direct
				  }
					else {			// short indirect
					  de(printf("[%02x] -> ",ea))
					  ea=Get(ea);
					}
				}
				sprintf(txt,"%02x ",ea); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				break;
			case 0xC:			// long
				if(!indirect) {		// long direct
					sprintf(txt,"%.2X%.2X", Get(pc),Get(pc+1)); strcat(displaystr1, txt);
					ea=HILO(pc); pc+=2;
				} else {			// long indirect
					sprintf(txt,"%.2X", Get(pc)); strcat(displaystr1, txt);
					if (paged) {
						ea=HILO(pc); pc+=2;
						sprintf(txt,"[%s] -> ",PADDR(pr,ea)); strcat(displaystr2, txt);
						if (pc80flag && Show_Diss==1) printf(txt);
						unsigned char s=Get(pr,ea);
						ea=HILOS(pr,ea+1);
						pr=s;
				    } else {
						ea=Get(pc++);
						sprintf(txt,"[%02x] -> ",ea); strcat(displaystr2, txt);
						if (pc80flag && Show_Diss==1) printf(txt);
						ea=HILO(ea);
				    }
				}
				sprintf(txt,"%s ",PADDR(pr,ea)); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				break;
			case 0xD:			// long indexed
				if(!indirect) {		// long direct indexed
					sprintf(txt,"%.2X%.2X", Get(pc),Get(pc+1)); strcat(displaystr1, txt);
					ea=HILO(pc); pc+=2;
					sprintf(txt,"(%s",PADDR(cr,ea)); strcat(displaystr2, txt);
					if (pc80flag && Show_Diss==1) printf(txt);
				} else {			// long indirect indexed
					sprintf(txt,"%.2X", Get(pc)); strcat(displaystr1, txt);
					if(paged) {
						ea=HILO(pc); pc+=2;
						sprintf(txt,"([%s]",PADDR(pr,ea)); strcat(displaystr2, txt);
						if (pc80flag && Show_Diss==1) printf(txt);
						unsigned char s=Get(pr,ea++);
						ea=HILOS(pr,ea);
						pr=s;
				    } else {
						ea=Get(pc++);
						sprintf(txt,"([%02x]",ea); strcat(displaystr2, txt);
						if (pc80flag && Show_Diss==1) printf(txt);
						ea=HILO(ea);
				    }
					sprintf(txt,",%c) -> (%s",xi,PADDR(pr,ea)); strcat(displaystr2, txt);
					if (pc80flag && Show_Diss==1) printf(txt);
				}
				ea+=idx;
				sprintf(txt,",%c) -> %s ",xi,PADDR(pr,ea)); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				break;
			case 0x6:			// short indexed
			case 0xE:
				sprintf(txt,"%.2X", Get(pc)); strcat(displaystr1, txt);
				ea=Get(pc++);
				if(!indirect) {		// short direct indexed
					sprintf(txt,"(%02x",ea); strcat(displaystr2, txt);
					if (pc80flag && Show_Diss==1) printf(txt);
				} else {			// short indirect indexed
					sprintf(txt,"([%02x]",ea); strcat(displaystr2, txt);
					if (pc80flag && Show_Diss==1) printf(txt);
					ea=Get(ea);
					sprintf(txt,",%c) -> (%02x",xi,ea); strcat(displaystr2, txt);
					if (pc80flag && Show_Diss==1) printf(txt);
				}
				ea+=idx;
				sprintf(txt,",%c) -> %s ",xi,PADDR(pr,ea)); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				break;
			case 0x7:			// indexed
			case 0xF:
				ea=idx;
				sprintf(txt,"(%c) -> %s ",xi,PADDR(pr,ea)); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				break;
			case 0x4:			// inherent A
				sprintf(txt,"A "); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				break;
			case 0x5:			// inherent X/Y
				sprintf(txt,"%c ",xs); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				break;
			case 0x0:			// bit
			case 0x1:
				sprintf(txt,"%.2X", Get(pc)); strcat(displaystr1, txt);
				ea=Get(pc++);
				if(!indirect) {
				} else {
					sprintf(txt,"[%02x] -> ",ea); strcat(displaystr2, txt);
					if (pc80flag && Show_Diss==1) printf(txt);
					ea=Get(ea);
					indirect=false; // don't use indirect mode in case this is a bit branch
				}
				sprintf(txt,"%02x ",ea); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				break;
		}

		// read operant
		unsigned char op=0;
		if (flags & 1) {
			switch (((flags&8) ? flags:ins)>>4) {
				case 0x2:			// no or special address mode
				case 0x8:
				case 0x9:
				  break;
				case 0xA:			// immediate
				case 0x3:			// short
				case 0xB:
				case 0xC:			// long
				case 0xD:			// long indexed
				case 0x6:			// short indexed
				case 0xE:
				case 0x7:			// indexed
				case 0xF:
				case 0x0:			// bit
				case 0x1:
				  //op=Get(pr,ea); break;
					  if (fAttributeRead)
						  op = 0x01; //assume code space for now
					  else
						  op=Get(pr,ea); break;				 
				case 0x4:			// inherent A
				  op=a; break;
				case 0x5:			// inherent X/Y
				  op=*ex; break;
			}
		}

		// command decoding
#ifdef DEBUG_STAT
		stats[ins]++;
#endif
		switch (ins) {
			case 0xA6: // LDA
			case 0xB6:
			case 0xC6:
			case 0xD6:
			case 0xE6:
			case 0xF6:
				a=op; tst(op); break;
			case 0xAE: // LDX
			case 0xBE:
			case 0xCE:
			case 0xDE:
			case 0xEE:
			case 0xFE:
				*ex=op; tst(op); break;
			case 0xB7: // STA
			case 0xC7:
			case 0xD7:
			case 0xE7:
			case 0xF7:
				op=a; tst(op); break;
			case 0xBF: // STX
			case 0xCF:
			case 0xDF:
			case 0xEF:
			case 0xFF:
				op=*ex; tst(op); break;
			case 0x97: // TAX
				*ex=a; break;
			case 0x9F: // TXA
				a=*ex; break;
			case 0x93: // TYX (ST7)
				if(ex==&x) *ex=y; else *ex=x; break;
			case 0x3D: // TST
			case 0x4D:
			case 0x5D:
			case 0x6D:
			case 0x7D:
				tst(op); break;
			case 0x3F: // CLR
			case 0x4F:
			case 0x5F:
			case 0x6F:
			case 0x7F:
				op=0; tst(0); break;
			case 0x3C: // INC
			case 0x4C:
			case 0x5C:
			case 0x6C:
			case 0x7C:
				op++; tst(op); break;
			case 0x3A: // DEC
			case 0x4A:
			case 0x5A:
			case 0x6A:
			case 0x7A:
				op--; tst(op); break;
			case 0x33: // COM
			case 0x43:
			case 0x53:
			case 0x63:
			case 0x73:
				op=~op; cc.c=1; tst(op); break;
			case 0x30: // NEG
			case 0x40:
			case 0x50:
			case 0x60:
			case 0x70:
				op=~op+1; if(!op) cc.c=0; tst(op); break;
			case 0x42: // MUL
			case 0x52:
				{
				unsigned short res=*ex * a;
				*ex=(res>>8); a=res&0xff; cc.c=0; cc.h=0;
				break;
				}
			case 0xA9: // ADC
			case 0xB9:
			case 0xC9:
			case 0xD9:
			case 0xE9:
			case 0xF9:
				a=add(op,cc.c); break;
			case 0xAB: // ADD
			case 0xBB:
			case 0xCB:
			case 0xDB:
			case 0xEB:
			case 0xFB:
				a=add(op,0); break;
			case 0xA2: // SBC
			case 0xB2:
			case 0xC2:
			case 0xD2:
			case 0xE2:
			case 0xF2:
				a=sub(a,op,cc.c); break;
			case 0xA0: // SUB
			case 0xB0:
			case 0xC0:
			case 0xD0:
			case 0xE0:
			case 0xF0:
				a=sub(a,op,0); break;
			case 0xA1: // CMP
			case 0xB1:
			case 0xC1:
			case 0xD1:
			case 0xE1:
			case 0xF1:
				sub(a,op,0); break;
			case 0xA3: // CPX
			case 0xB3:
			case 0xC3:
			case 0xD3:
			case 0xE3:
			case 0xF3:
				sub(*ex,op,0); break;
			case 0xA4: // AND
			case 0xB4:
			case 0xC4:
			case 0xD4:
			case 0xE4:
			case 0xF4:
				a &= op; tst(a); break;
			case 0xAA: // ORA
			case 0xBA:
			case 0xCA:
			case 0xDA:
			case 0xEA:
			case 0xFA:
				a |= op; tst(a); break;
			case 0xA8: // EOR
			case 0xB8:
			case 0xC8:
			case 0xD8:
			case 0xE8:
			case 0xF8:
				a ^= op; tst(a); break;
			case 0xA5: // BIT
			case 0xB5:
			case 0xC5:
			case 0xD5:
			case 0xE5:
			case 0xF5:
				tst(a & op); break;
			case 0x38: // ASL
			case 0x48:
			case 0x58:
			case 0x68:
			case 0x78:
				op=rollL(op,0); break;
			case 0x39: // ROL
			case 0x49:
			case 0x59:
			case 0x69:
			case 0x79:
				op=rollL(op,cc.c); break;
			case 0x37: // ASR
			case 0x47:
			case 0x57:
			case 0x67:
			case 0x77:
				op=rollR(op,bitset(op,7)); break;
			case 0x34: // LSR
			case 0x44:
			case 0x54:
			case 0x64:
			case 0x74:
				op=rollR(op,0); break;
			case 0x36: // ROR
			case 0x46:
			case 0x56:
			case 0x66:
			case 0x76:
				op=rollR(op,cc.c); break;
			case 0x3E: // SWAP (ST7)
			case 0x4E:
			case 0x5E:
			case 0x6E:
			case 0x7E:
				op=(op<<4)|(op>>4); tst(op); break;
	  case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F: // BRSET BRCLR
				{
				int bit=(ins&0x0F)>>1;
				sprintf(txt,",#%x,",bit); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				cc.c=bitset(op,bit);
				branch((ins&0x01) ? cc.c:!cc.c);
				break;
				}
	  case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F: // BSET BCLR
				{
				int bit=(ins&0x0F)>>1;
				sprintf(txt,",#%x",bit); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				if(ins&0x01) op &= ~(1<<bit);
				else         op |=  (1<<bit);
					if (op==0x05 && bit==7)
					fAttributeRead = (ins==0x1E) ? true:false;
				break;
				}
			case 0x20: // BRA
				branch(true); break;
			case 0x21: // BRN
				branch(false); break;
			case 0x22: // BHI
				if(vbra) branch(!cc.z && ((cc.n && cc.v) || (!cc.n && !cc.v)));
				else branch(!cc.c && !cc.z);
				break;
			case 0x23: // BLS
				if(vbra) branch(cc.z || (cc.n && !cc.v) || (!cc.n && cc.v));
				else branch( cc.c ||  cc.z);
				break;
			case 0x24: // BCC BHS
				if(vbra) branch((cc.n && cc.v) || (!cc.n && !cc.v));
				else branch(!cc.c);
				break;
			case 0x25: // BCS BLO
				if(vbra) branch((cc.n && !cc.v) || (!cc.n && cc.v));
				else branch( cc.c);
				break;
			case 0x26: // BNE
				if(vbra) branch(!cc.v);
				else branch(!cc.z);
				break;
			case 0x27: // BEQ
				if(vbra) branch(cc.v);
				else branch( cc.z); break;
			case 0x28: // BHCC
				branch(!cc.h); break;
			case 0x29: // BHCS
				branch( cc.h); break;
			case 0x2A: // BPL
				branch(!cc.n); break;
			case 0x2B: // BMI
				branch( cc.n); break;
			case 0x2C: // BMC
				branch(!cc.i); break;
			case 0x2D: // BMS
				branch( cc.i); break;
			case 0xBC: // JMP
			case 0xCC:
			case 0xDC:
			case 0xEC:
			case 0xFC:
				pc=ea; break;
			case 0xAD: // BSR
				pc++; pushpc(); pc--; branch(true); break;
			case 0xBD: // JSR
			case 0xCD:
			case 0xDD:
			case 0xED:
			case 0xFD:
				pushpc(); pc=ea; break;
			case 0x81: // RTS
				poppc(); break;
			case 0x83: // SWI
				pushpc(); push(x); push(a); pushc();
				cc.i=1; pc=HILO(0x1ffc); break;
			case 0x80: // RTI
				popc(); a=pop(); x=pop(); poppc(); break;
			case 0x9C: // RSP
				sp=spHi; break;
			case 0x96: // TSX
				*ex=sp; break;
			case 0x94: // TXS (ST7)
				sp=*ex; break;
			case 0x9E: // TSA
				a=sp; break;
			case 0x95: // TAS (ST7)
				sp=a; break;
			case 0x84: // POPA (ST7)
				a=pop(); break;
			case 0x85: // POPX (ST7)
				*ex=pop(); break;
			case 0x86: // POPC (ST7)
				popc(); break;
			case 0x88: // PUSHA (ST7)
				push(a); break;
			case 0x89: // PUSHX (ST7)
				push(*ex); break;
			case 0x8A: // PUSHC (ST7)
				pushc(); break;
			case 0x98: // CLC
				cc.c=0; break;
			case 0x99: // SEC
				cc.c=1; break;
			case 0x9A: // CLI
				cc.i=0; break;
			case 0x9B: // SEI
				cc.i=1; break;
			case 0x9D: // NOP
				break;

			case 0x71: // LDD (ST19)
			case 0x72:
			case 0x75:
				dr=op; break;
			case 0x7B: // TAD (ST19)
				dr=a; break;
			case 0x8B: // TDA (ST19)
				a=dr; break;
			case 0x8C: // TCA (ST19)
				a=cr; break;
			case 0xAC: // PUSHD (ST19)
				push(dr); break;
			case 0xAF: // POPD (ST19)
				dr=pop(); break;
			case 0x87: // PRTS (ST19)
				cr=pop(); poppc(); break;
			case 0x8D: // PJSR (ST19)
				if (paged) {
					ea=HILO(pc); pc+=2;
					sprintf(txt,"[%s] -> ",PADDR(cr,ea)); strcat(displaystr2, txt);
					if (pc80flag && Show_Diss==1) printf(txt);
				} else {
					ea=pc; pc+=3;
				}
				sprintf(txt,"%s ",PADDR(pr,ea)); strcat(displaystr2, txt);
				if (pc80flag && Show_Diss==1) printf(txt);
				pr=Get(ea++);
				ea=HILO(ea);
				pushpc(); push(cr); cr=pr; pc=ea; break;

			case 0x90: // pre-bytes
			case 0x91:
			case 0x92:
			case 0x31:
			case 0x32:
				printf("*** pre-byte %02x in command decoding (count=%d)\n",ins,LoopCount);
				return 3;
			default:
				printf("*** unsupported instruction 0x%02x (count=%d)\n",ins,LoopCount);
				return 3;
		}

		// write operant
		if (flags & 2) {
			switch(((flags&8) ? flags:ins)>>4) {
				case 0x2:			// no or special address mode
				case 0x8:
				case 0x9:
				  break;
				case 0xA:			// immediate
				case 0x3:			// short
				case 0xB:
				case 0xC:			// long
				case 0xD:			// long indexed
				case 0x6:			// short indexed
				case 0xE:
				case 0x7:			// indexed
				case 0xF:
				case 0x0:			// bit
				case 0x1:
				  Set(ea,op); break;
				case 0x4:			// inherent A
				  a=op; break;
				case 0x5:			// inherent X/Y
				  *ex=op; break;
			}
		}

		for (int i=numBp-1 ; i>=0 ; i--) {
			if (bp[i] == pc) {
				       if (bp[i]==0x9569 && Show_Diss==0 ) {
				//	printf("*** %s {next EMM_CMD}\n\n", displaystr2);
				} else if (bp[i]==0x9569 && Show_Diss==1 ) {
					printf(" {next EMM_CMD}\n\n");
				} else if (bp[i]==0x9569 && Show_Diss==2 ) {
					printf("%-18s ;%s {next EMM_CMD}\n\n", displaystr1, displaystr2);
				} else {
					printf("*** Breakpoint at: %04x (count=%d)\n\n",pc,LoopCount);
				}
				return 0;
			}
		}
		if (pc80flag && Show_Diss==1) printf("\n");

		if (LoopCount >= max_count) {
			printf("*** Max. instruction counter exceeded (count=%d)\n",LoopCount);
			return 2;
		}
		if (Show_Diss==2) printf("%-18s ;%s\n", displaystr1, displaystr2);
	}
}

void c6805::branch(bool branch)
{
	if (pc80flag) {
		unsigned char off=Get(pc);
		if (indirect) {
			sprintf(txt,"[%02x] -> ",off); strcat(displaystr2, txt);
			if (pc80flag && Show_Diss==1) printf(txt);
			off=Get(off);
		}
		unsigned short npc=pc+off+1;
		if (off&0x80) npc-=0x100; // gcc fixup. take care of sign
		sprintf(txt,"%s ",PADDR(cr,npc)); strcat(displaystr2, txt);
		if (pc80flag && Show_Diss==1) printf(txt);
		if (branch) {
			sprintf(txt,"(taken) "); strcat(displaystr2, txt);
			if (pc80flag && Show_Diss==1) printf(txt);
		}
	}
	pc++;
	if(branch) {
		unsigned char offset=Get(pc-1);
		if(indirect) offset=Get(offset);
		pc+=offset;
		if(offset&0x80) pc-=0x100; // gcc fixup. take care of sign
		}
}

void c6805::push(unsigned char c)
{
	Set(sp--,c);
}

unsigned char c6805::pop(void)
{
	return Get(++sp);
}

void c6805::pushpc(void)
{
	push(pc & 0xff);
	push(pc >> 8);
}

void c6805::poppc(void)
{
	pc=(pop()<<8) | pop();
}

void c6805::pushc(void)
{
	unsigned char c=0xC0+(cc.v?32:0)+(cc.h?16:0)+(cc.i?8:0)+(cc.n?4:0)+(cc.z?2:0)+(cc.c?1:0);
	push(c);
}

void c6805::popc(void)
{
	unsigned char c=pop();
	cc.v=(c&32) ? 1:0;
	cc.h=(c&16) ? 1:0;
	cc.i=(c& 8) ? 1:0;
	cc.n=(c& 4) ? 1:0;
	cc.z=(c& 2) ? 1:0;
	cc.c=(c& 1) ? 1:0;
}

void c6805::tst(unsigned char c)
{
	cc.z=!c;
	cc.n=bitset(c,7);
}

unsigned char c6805::add(unsigned char op, unsigned char c)
{
	unsigned short res_half=(a&0x0f) + (op&0x0f) + c;
	unsigned short res=(unsigned short)a + (unsigned short)op + (unsigned short)c;
	cc.h=res_half > 0x0f;
	cc.c=res > 0xff;
	res&=0xff;
	tst(res);
	return res;
}

unsigned char c6805::sub(unsigned char op1, unsigned char op2, unsigned char c)
{
	short res=(short)op1 - (short)op2 - (short)c;
	cc.c=res < 0;
	res&=0xff;
	tst(res);
	return res;
}

unsigned char c6805::rollR(unsigned char op, unsigned char c)
{
	cc.c=bitset(op,0);
	op >>= 1;
	op |= c << 7;
	tst(op);
	return op;
}

unsigned char c6805::rollL(unsigned char op, unsigned char c)
{
	cc.c=bitset(op,7);
	op <<= 1;
	op |= c;
	tst(op);
	return op;
}
