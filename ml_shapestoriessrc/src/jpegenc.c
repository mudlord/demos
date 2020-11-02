#include "arch.h"
#include "jpegenc.h"


#define QTAB_SCALE	10

// as you can see I use Paint tables
static const unsigned char qtable_0_lum[64] =
{
	 8,  6,  5,  8, 12, 20, 26, 31, 
	 6,  6,  7, 10, 13, 29, 30, 28,
	 7,  7,  8, 12, 20, 29, 35, 28,
	 7,  9, 11, 15, 26, 44, 40, 31,
	 9, 11, 19, 28, 34, 55, 52, 39,
	12, 18, 28, 32, 41, 52, 57, 46,
	25, 32, 39, 44, 52, 61, 60, 51,
	36, 46, 48, 49, 56, 50, 52, 50
};

static const unsigned char qtable_0_chrom[64] =
{
	 9,  9, 12, 24, 50, 50, 50, 50,
	 9, 11, 13, 33, 50, 50, 50, 50,
	12, 13, 28, 50, 50, 50, 50, 50,
	24, 33, 50, 50, 50, 50, 50, 50,
	50, 50, 50, 50, 50, 50, 50, 50,
	50, 50, 50, 50, 50, 50, 50, 50,
	50, 50, 50, 50, 50, 50, 50, 50,
	50, 50, 50, 50, 50, 50, 50, 50
};

// (1 << QTAB_SCALE)/qtable_0_lum[][]
static const unsigned char qtable_lum[64] =
{
	128,171,205,128, 85, 51, 39, 33,
	171,171,146,102, 79, 35, 34, 37,
	146,146,128, 85, 51, 35, 29, 37,
	146,114, 93, 68, 39, 23, 26, 33,
	114, 93, 54, 37, 30, 19, 20, 26,
	 85, 57, 37, 32, 25, 20, 18, 22,
	 41, 32, 26, 23, 20, 17, 17, 20,
	 28, 22, 21, 21, 18, 20, 20, 20
};

// (1 << QTAB_SCALE)/qtable_0_chrom[][]
static const unsigned char qtable_chrom[64] =
{
	114,114, 85, 43, 20, 20, 20, 20,
	114, 93, 79, 31, 20, 20, 20, 20,
	 85, 79, 37, 20, 20, 20, 20, 20,
	 43, 31, 20, 20, 20, 20, 20, 20,
	 20, 20, 20, 20, 20, 20, 20, 20,
	 20, 20, 20, 20, 20, 20, 20, 20,
	 20, 20, 20, 20, 20, 20, 20, 20,
	 20, 20, 20, 20, 20, 20, 20, 20
};

/******************************************************************************
**  quantize
**  --------------------------------------------------------------------------
**  DCT coeficient quantization.
**  To avoid division function uses quantization coefs amplified by 2^QTAB_SCALE
**  and then shifts the product by QTAB_SCALE bits to the right.
**  To make this operation a bit faster some tricks are used but it is just
**  returns round(data[i]/qt0[i]).
**  
**  ARGUMENTS:
**      data    - DCT freq value;
**      qt      - quantization value ( (1 << QTAB_SCALE)/qt0 );
**
**  RETURN: quantized value.
******************************************************************************/
static short quantize(const short data, const unsigned char qt)
{
	return (data*qt - (data>>15) + ((1<<(QTAB_SCALE-1))-1)) >> QTAB_SCALE;
}

/******************************************************************************
**  quantization
**  --------------------------------------------------------------------------
**  DCT coeficients quantization to discard weak high frequencies.
**  This function processes 8x8 DCT blocks inplace.
**  
**  ARGUMENTS:
**      data    - 8x8 DCT freq block;
**      qt      - 8x8 quantization table;
**
**  RETURN: -
******************************************************************************/
static void quantization(short data[], const unsigned char qt[])
{
	unsigned i;

	for (i = 0; i < 64; i+=4)
	{
		data[i]   = quantize(data[i],   qt[i]);
		data[i+1] = quantize(data[i+1], qt[i+1]);
		data[i+2] = quantize(data[i+2], qt[i+2]);
		data[i+3] = quantize(data[i+3], qt[i+3]);
	}
}

void quantization_lum(short data[64])
{
	quantization((short*)data, qtable_lum);
}

void quantization_chrom(short data[64])
{
	quantization((short*)data, qtable_chrom);
}

static void iquantization(short data[64], const unsigned char qt[64])
{
	unsigned i;

	// de-quantization
	for (i = 0; i < 64; i++)
	{
		int p = data[i];
		unsigned char q = qt[i];

		data[i] = p*q;
	}
}

void iquantization_lum(short data[64])
{
	iquantization(data, qtable_0_lum);
}

void iquantization_chrom(short data[64])
{
	iquantization(data, qtable_0_chrom);
}
