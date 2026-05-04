/****************************************************************************
    FUNCTION:  EpgDescText

    PURPOSE:   Returns a description of the 

    NOTE:      Any 0x0d byte found in the decompressed string should be 
	           replaced with an appropriate new line character.

*******************************************************************************/

int EpgDescText (FILE *Outfp, unsigned char *Chars, int Index) {
	char decompressed[256];

	switch(Chars[Index]) {

	case 0x54: // Content descriptor (does not follow the standard coding)
		fprintf (Outfp, "      Descriptor 0x54: Content=%01x,%01x User=%01x,%01x\n",
			     (Chars[Index+2] & 0xF0) >> 4, Chars[Index+2] & 0x0F,
				 (Chars[Index+3] & 0xF0) >> 4, Chars[Index+3] & 0x0F);
		break;

	case 0x89: // ???  data is always 0x18, 0x00
		fprintf (Outfp, "      Descriptor 0x89: 0x%02x%02x\n", Chars[Index+2], Chars[Index+3]);
		break;

	case 0x91: // Title
		Decompress(&Chars[Index+3], ((int) Chars[Index+1])-1, 1, decompressed);
		fprintf (Outfp, "      Descriptor 0x91: %s\n", decompressed);
		break;

	case 0x92: // Description
		if ((Chars[Index+3] & 0xF8) == 0x80) { // This is a movie
			Decompress(&Chars[Index+4], ((int) Chars[Index+1])-2, 1, decompressed);
			fprintf (Outfp, "      Descriptor 0x92: %s\n", decompressed);
		}
		else {
			Decompress(&Chars[Index+3], ((int) Chars[Index+1])-1, 1, decompressed);
			fprintf (Outfp, "      Descriptor 0x92: %s\n", decompressed);
		}
		break;

	case 0x94: // ???
		if ((Chars[Index+4] & 0xF8) == 0x80) { // This is a movie
			Decompress(&Chars[Index+5], ((int) Chars[Index+1])-3, 1, decompressed);
			fprintf (Outfp, "      Descriptor 0x94: %s\n", decompressed);
		}
		else {
			Decompress(&Chars[Index+4], ((int) Chars[Index+1])-2, 1, decompressed);
			fprintf (Outfp, "      Descriptor 0x94: %s\n", decompressed);
		}
		break;

	default:
		fprintf (Outfp, "      Unknown elementary stream descriptor 0x%02x\n", Chars[Index]);
		break;
	}

	return(SUCCESS);
}
