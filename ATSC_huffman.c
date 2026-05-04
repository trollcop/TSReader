#include <windows.h>

#include "ATSC_huffman.h"
#include "bcdmux.h"

void ATSCHuffmanDecode(int nBitBufferIndex, int type, int bytes, char * outtext)
{
	int root = 0;
	int offset = 0;
	int position = 0;
	int value = 1;
	int bits = bytes * 8;
	unsigned char previous_byte = 0;
	unsigned char string[4096];

	while (bits > 0 && value)
	{
		// The first 256 bytes of the table are just a jump table to a root node
		// The root node is dependent on the "previous_byte".
		if (type == 1)
			root = ATSC_C5[previous_byte * 2] * 256 + ATSC_C5[previous_byte * 2 + 1];
		else 
			root = ATSC_C7[previous_byte * 2] * 256 + ATSC_C7[previous_byte * 2 + 1];

		do
		{
			int right = get_bits(nBitBufferIndex, 1);
			//field int right:1;
			bits--;
			if (!bits)
				break;

			// Either get left or right node value (depends on bit)
			if (type == 1)
			{
				if (right)
					value = ATSC_C5[root + offset + 1];
				else
					value = ATSC_C5[root + offset];
			}
			else
			{
				if (right)
					value = ATSC_C7[root + offset + 1];
				else 
					value = ATSC_C7[root + offset];
			}

			// A value less than 128 means that we are in a branch and not at a leaf 
			if (value < 128)
				offset = value * 2;
		} while (value < 128);

		// Check for an escape sequence
		// If present then read next 8 bits and use them as the value
		value &= 0x7F;
		if (value == 27)
		{
			//field int fullByte:8;			
			value = get_bits(nBitBufferIndex, 8);
			bits -= 8;
		}
		previous_byte = string[position++] = value;
		offset = 0;
	}

	// IS THE FOLLOWING NECESSARY???????
	// The end of the string has come before finishing off the number of bits
	// Dump the rest of the bits not used.
	if (bits > 0)
	{
		int i;
		for (i = 0; i < bits; i++)
			get_bits(nBitBufferIndex, 1);
	}

	// We have the entire string - Print it out
	string[position] = 0;	// Just in case
	lstrcpy(outtext, string);
}
