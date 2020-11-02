#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "arch.h"
#include "dct.h"

#define PI  3.1415926535897932384626433832795
#define IDCTI_AMP 512

// DCT basis functions coeficients
int dct_tbl_i[8][8];
float dct_tbl[8][8];

CACHE_ALIGN short dct_tbl_s[8][8] =
{
	{362, 362, 362, 362, 362, 362, 362, 362},
	{502, 425, 284,  99, -99,-284,-425,-502},
	{473, 195,-195,-473,-473,-195, 195, 473},
	{425, -99,-502,-284, 284, 502,  99,-425},
	{362,-362,-362, 362, 362,-362,-362, 362},
	{284,-502, 99,  425,-425, -99, 502,-284},
	{195,-473, 473,-195,-195, 473,-473, 195},
	{ 99,-284, 425,-502, 502,-425, 284, -99}
};

CACHE_ALIGN short idct_tbl_s[8][8] =
{
	{362, 502, 473, 425, 362, 284, 195,  99},
	{362, 425, 195, -99,-362,-502,-473,-284},
	{362, 284,-195,-502,-362,  99, 473, 425},
	{362,  99,-473,-284, 362, 425,-195,-502},
	{362, -99,-473, 284, 362,-425,-195, 502},
	{362,-284,-195, 502,-362, -99, 473,-425},
	{362,-425, 195,  99,-362, 502,-473, 284},
	{362,-502, 473,-425, 362,-284, 195, -99}
};

/******************************************************************************
**  dct
**  --------------------------------------------------------------------------
**  Fast DCT - Discrete Cosine Transform.
**  This function converts 8x8 pixel block into frequencies.
**  Lowest frequencies are at the upper-left corner.
**  The input and output could point at the same array, in this case the data
**  will be overwritten.
**  9 multiplications ans 33 additions for 1D DCT.
**  
**  ARGUMENTS:
**      pixels  - 8x8 pixel array;
**      data    - 8x8 freq block;
**
**  RETURN: -
******************************************************************************/
void dct(conv pixels[8][8], conv data[8][8])
{
	short rows[8][8];
	unsigned i;

	static const int
				c1 = 1004,  /* cos(pi/16) << 10 */
				s1 = 200,   /* sin(pi/16) */
				c3 = 851,   /* cos(3pi/16) << 10 */
				s3 = 569,   /* sin(3pi/16) << 10 */
				r2c6 = 554, /* sqrt(2)*cos(6pi/16) << 10 */
				r2s6 = 1337,/* sqrt(2)*sin(6pi/16) << 10 */
				r2 = 181;   /* sqrt(2) << 7*/

	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		int x0,x1,x2,x3,x4,x5,x6,x7,x8;

		x0 = pixels[i][0];
		x1 = pixels[i][1];
		x2 = pixels[i][2];
		x3 = pixels[i][3];
		x4 = pixels[i][4];
		x5 = pixels[i][5];
		x6 = pixels[i][6];
		x7 = pixels[i][7];

		/* Stage 1 */
		x8=x7+x0;
		x0-=x7;
		x7=x1+x6;
		x1-=x6;
		x6=x2+x5;
		x2-=x5;
		x5=x3+x4;
		x3-=x4;

		/* Stage 2 */
		x4=x8+x5;
		x8-=x5;
		x5=x7+x6;
		x7-=x6;
		x6=c1*(x1+x2);
		x2=(-s1-c1)*x2+x6;
		x1=(s1-c1)*x1+x6;
		x6=c3*(x0+x3);
		x3=(-s3-c3)*x3+x6;
		x0=(s3-c3)*x0+x6;

		/* Stage 3 */
		x6=x4+x5;
		x4-=x5;
		x5=r2c6*(x7+x8);
		x7=(-r2s6-r2c6)*x7+x5;
		x8=(r2s6-r2c6)*x8+x5;
		x5=x0+x2;
		x0-=x2;
		x2=x3+x1;
		x3-=x1;

		/* Stage 4 and output */
		rows[i][0]=x6;
		rows[i][4]=x4;
		rows[i][2]=x8>>10;
		rows[i][6]=x7>>10;
		rows[i][7]=(x2-x5)>>10;
		rows[i][1]=(x2+x5)>>10;
		rows[i][3]=(x3*r2)>>17;
		rows[i][5]=(x0*r2)>>17;
	}

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		int x0,x1,x2,x3,x4,x5,x6,x7,x8;

		x0 = rows[0][i];
		x1 = rows[1][i];
		x2 = rows[2][i];
		x3 = rows[3][i];
		x4 = rows[4][i];
		x5 = rows[5][i];
		x6 = rows[6][i];
		x7 = rows[7][i];

		/* Stage 1 */
		x8=x7+x0;
		x0-=x7;
		x7=x1+x6;
		x1-=x6;
		x6=x2+x5;
		x2-=x5;
		x5=x3+x4;
		x3-=x4;

		/* Stage 2 */
		x4=x8+x5;
		x8-=x5;
		x5=x7+x6;
		x7-=x6;
		x6=c1*(x1+x2);
		x2=(-s1-c1)*x2+x6;
		x1=(s1-c1)*x1+x6;
		x6=c3*(x0+x3);
		x3=(-s3-c3)*x3+x6;
		x0=(s3-c3)*x0+x6;

		/* Stage 3 */
		x6=x4+x5;
		x4-=x5;
		x5=r2c6*(x7+x8);
		x7=(-r2s6-r2c6)*x7+x5;
		x8=(r2s6-r2c6)*x8+x5;
		x5=x0+x2;
		x0-=x2;
		x2=x3+x1;
		x3-=x1;

		/* Stage 4 and output */
		data[0][i]=((x6+16)>>3);
		data[4][i]=((x4+16)>>3);
		data[2][i]=((x8+16384)>>13);
		data[6][i]=((x7+16384)>>13);
		data[7][i]=((x2-x5+16384)>>13);
		data[1][i]=((x2+x5+16384)>>13);
		data[3][i]=(((x3>>8)*r2+8192)>>12);
		data[5][i]=(((x0>>8)*r2+8192)>>12);
	}
}
//
void dct_fill_tab()
{
	unsigned u,x;

	for (u = 0; u < 8; u++)
	{
		printf("%d: ", u);
		for (x = 0; x < 8; x++)
		{
			double Cu = (u==0)? 1.0/sqrt(2.0): 1.0;

			double K = Cu * cos((double)(2*x+1) * (double)u * PI/16.0);
			dct_tbl_i[u][x] = K * IDCTI_AMP;
			dct_tbl[u][x] = K;
			//dct_tbl_s[u][x] = K * IDCTI_AMP;
			//idct_tbl_s[x][u] = K * IDCTI_AMP; // different order

			printf("%f(%d),", K, idct_tbl_s[u][x]);
		}
		printf("\n");
	}
} 

/* real DCT
void dct2(conv pixel[8][8], conv data[8][8])
{
	unsigned x, y, n;
	float tmp[8][8];

	for (y = 0; y < 8; y++)
	for (x = 0; x < 8; x++)
	{
		float q = 0.0f;

		for (n = 0; n < 8; n++)
			q += pixel[y][n] * dct_tbl[x][n];

		tmp[y][x] = q/2;
	}
		
	for (x = 0; x < 8; x++)
	for (y = 0; y < 8; y++)
	{
		float q = 0.0f;

		for (n = 0; n < 8; n++)
			q += tmp[n][x] * dct_tbl[y][n];

		data[y][x] = q/2;
	}
}*/

// integer DCT 
void dct2_i(conv pixel[8][8], conv data[8][8])
{
	unsigned x, y, n;
	conv tmp[8][8];

	// process rows
	for (y = 0; y < 8; y++)
	for (x = 0; x < 8; x++)
	{
		int q = 0;

		for (n = 0; n < 8; n++)
			q += pixel[y][n] * dct_tbl_i[x][n];

		tmp[y][x] = (q + ((q<0)? -IDCTI_AMP: IDCTI_AMP))/(IDCTI_AMP*2);
	}
		
	// process columns
	for (x = 0; x < 8; x++)
	for (y = 0; y < 8; y++)
	{
		int q = 0;

		for (n = 0; n < 8; n++)
			q += tmp[n][x] * dct_tbl_i[y][n];

		data[y][x] = (q + ((q<0)? -IDCTI_AMP: IDCTI_AMP))/(IDCTI_AMP*2);
	}
} 

#ifdef _MSC_VER

void dct2_s(conv pixel[8][8], conv data[8][8])
{
	CACHE_ALIGN conv tmp[8][8];
	unsigned x, y;

	// process rows
	for (y = 0; y < 8; y++) {

		__m128i a = _mm_loadu_si128 ((__m128i*)pixel[y]);

		for (x = 0; x < 8; x++) {
			__m128i b, c;

			b = _mm_load_si128 ((__m128i*)dct_tbl_s[x]);
			b = _mm_madd_epi16 (a, b);
			c = _mm_shuffle_epi32 (b, 0xB1);
			c = _mm_add_epi32 (b, c);
			b = _mm_shuffle_epi32 (c, 0x27);
			c = _mm_add_epi32 (b, c);
			tmp[x][y] = _mm_cvtsi128_si32(c)/(IDCTI_AMP*2);
		}
	}

	// process columns
	for (y = 0; y < 8; y++) {

		__m128i a = _mm_loadu_si128 ((__m128i*)tmp[y]);

		for (x = 0; x < 8; x++) {
			__m128i b, c;

			b = _mm_load_si128 ((__m128i*)dct_tbl_s[x]);
			b = _mm_madd_epi16 (a, b);
			c = _mm_shuffle_epi32 (b, 0xB1);
			c = _mm_add_epi32 (b, c);
			b = _mm_shuffle_epi32 (c, 0x27);
			c = _mm_add_epi32 (b, c);

			data[x][y] = _mm_cvtsi128_si32(c)/(IDCTI_AMP*2);
		}
	}
}

#endif//_MSC_VER


__inline static int sdiv(const int data, const int quant, const unsigned mag)
{
	return data >> mag;
	//return (data + quant) >> mag;
	//return (data + ((data<0)? -quant: quant))/(1<<mag);
}

// simple but fast DCT - 22 multiplication and 28 additions. 
void dct3(conv pixels[8][8], conv data[8][8])
{
	CACHE_ALIGN short rows[8][8];
	unsigned          i;

	static const short // Ci = cos(i*PI/16)*(1<<14);
		C1 = 16070,
		C2 = 15137,
		C3 = 13623,
		C4 = 11586,
		C5 = 9103,
		C6 = 6270,
		C7 = 3197;

	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		short s07,s16,s25,s34,s0734,s1625;
		short d07,d16,d25,d34,d0734,d1625;

		s07 = pixels[i][0] + pixels[i][7];
		d07 = pixels[i][0] - pixels[i][7];
		s16 = pixels[i][1] + pixels[i][6];
		d16 = pixels[i][1] - pixels[i][6];
		s25 = pixels[i][2] + pixels[i][5];
		d25 = pixels[i][2] - pixels[i][5];
		s34 = pixels[i][3] + pixels[i][4];
		d34 = pixels[i][3] - pixels[i][4];

		rows[i][1] = (C1*d07 + C3*d16 + C5*d25 + C7*d34) >> 14;
		rows[i][3] = (C3*d07 - C7*d16 - C1*d25 - C5*d34) >> 14;
		rows[i][5] = (C5*d07 - C1*d16 + C7*d25 + C3*d34) >> 14;
		rows[i][7] = (C7*d07 - C5*d16 + C3*d25 - C1*d34) >> 14;

		s0734 = s07 + s34;
		d0734 = s07 - s34;
		s1625 = s16 + s25;
		d1625 = s16 - s25;

		rows[i][0] = (C4*(s0734 + s1625)) >> 14;
		rows[i][4] = (C4*(s0734 - s1625)) >> 14;

		rows[i][2] = (C2*d0734 + C6*d1625) >> 14;
		rows[i][6] = (C6*d0734 - C2*d1625) >> 14;
	}

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		short s07,s16,s25,s34,s0734,s1625;
		short d07,d16,d25,d34,d0734,d1625;

		s07 = rows[0][i] + rows[7][i];
		d07 = rows[0][i] - rows[7][i];
		s16 = rows[1][i] + rows[6][i];
		d16 = rows[1][i] - rows[6][i];
		s25 = rows[2][i] + rows[5][i];
		d25 = rows[2][i] - rows[5][i];
		s34 = rows[3][i] + rows[4][i];
		d34 = rows[3][i] - rows[4][i];

		data[1][i] = (C1*d07 + C3*d16 + C5*d25 + C7*d34) >> 16;
		data[3][i] = (C3*d07 - C7*d16 - C1*d25 - C5*d34) >> 16;
		data[5][i] = (C5*d07 - C1*d16 + C7*d25 + C3*d34) >> 16;
		data[7][i] = (C7*d07 - C5*d16 + C3*d25 - C1*d34) >> 16;

		s0734 = s07 + s34;
		d0734 = s07 - s34;
		s1625 = s16 + s25;
		d1625 = s16 - s25;

		data[0][i] = (C4*(s0734 + s1625)) >> 16;
		data[4][i] = (C4*(s0734 - s1625)) >> 16;

		data[2][i] = (C2*d0734 + C6*d1625) >> 16;
		data[6][i] = (C6*d0734 - C2*d1625) >> 16;
	}
}

// fast DCT, Vetterli & Ligtenberg - 16 multiplications and 26 additions.
void dct4(conv pixels[8][8], conv data[8][8])
{
	unsigned          i;
	CACHE_ALIGN short rows[8][8];

	static const short
		C1 = 16070,// cos(1*Pi/16) = 0.9808 * 16384
		S6 = 15137,// sin(6*Pi/16) = 0.9239
		C3 = 13623,// cos(3*Pi/16) = 0.8315
		C4 = 11586,// cos(4*Pi/16) = 0.7071
		S3 = 9102, // sin(3*Pi/16) = 0.5556
		C6 = 6270, // cos(6*Pi/16) = 0.3827
		S1 = 3196; // sin(1*Pi/16) = 0.1951

	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		short s07,s12,s34,s56;
		short d07,d12,d34,d56;
		short x, y;

		s07 = pixels[i][0] + pixels[i][7];
		d07 = pixels[i][0] - pixels[i][7];
		s12 = pixels[i][1] + pixels[i][2];
		d12 = pixels[i][1] - pixels[i][2];
		s34 = pixels[i][3] + pixels[i][4];
		d34 = pixels[i][3] - pixels[i][4];
		s56 = pixels[i][5] + pixels[i][6];
		d56 = pixels[i][5] - pixels[i][6];

		x = s07 + s34;
		y = s12 + s56;
		rows[i][0] = C4*(x + y) >> 15;
		rows[i][4] = C4*(x - y) >> 15;

		x = d12 - d56;
		y = s07 - s34;
		rows[i][2] = (C6*x + S6*y) >> 15;
		rows[i][6] = (C6*y - S6*x) >> 15;

		x = d07 - (C4*(s12 - s56) >> 14);
		y = d34 - (C4*(d12 + d56) >> 14);
		rows[i][3] = (C3*x - S3*y) >> 15;
		rows[i][5] = (S3*x + C3*y) >> 15;

		x = (d07 << 1) - x;
		y = (d34 << 1) - y;
		rows[i][1] = (C1*x + S1*y) >> 15;
		rows[i][7] = (S1*x - C1*y) >> 15;
	}

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		short s07,s12,s34,s56;
		short d07,d12,d34,d56;
		short x, y;

		s07 = rows[0][i] + rows[7][i];
		d07 = rows[0][i] - rows[7][i];
		s12 = rows[1][i] + rows[2][i];
		d12 = rows[1][i] - rows[2][i];
		s34 = rows[3][i] + rows[4][i];
		d34 = rows[3][i] - rows[4][i];
		s56 = rows[5][i] + rows[6][i];
		d56 = rows[5][i] - rows[6][i];

		x = s07 + s34;
		y = s12 + s56;
		data[0][i] = C4*(x + y) >> 15;
		data[4][i] = C4*(x - y) >> 15;

		x = d12 - d56;
		y = s07 - s34;
		data[2][i] = (C6*x + S6*y) >> 15;
		data[6][i] = (C6*y - S6*x) >> 15;

		x = d07 - (C4*(s12 - s56) >> 14);
		y = d34 - (C4*(d12 + d56) >> 14);
		data[3][i] = (C3*x - S3*y) >> 15;
		data[5][i] = (S3*x + C3*y) >> 15;

		x = (d07 << 1) - x;
		y = (d34 << 1) - y;
		data[1][i] = (C1*x + S1*y) >> 15;
		data[7][i] = (S1*x - C1*y) >> 15;
	}
}

/* inverse real DCT
void idct2(conv data[8][8], conv pixel[8][8])
{
	unsigned x, y, n;
	float tmp[8][8];

	uint64_t a = __rdtsc();

	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 8; x++)
		{
			float q = 0.0f;

			for (n = 0; n < 8; n++)
				q += data[y][n] * dct_tbl[n][x];

			tmp[y][x] = q/2;
		}
	}
		
	for (x = 0; x < 8; x++)
	{
		for (y = 0; y < 8; y++)
		{
			float q = 0.0f;

			for (n = 0; n < 8; n++)
				q += tmp[n][x] * dct_tbl[n][y];

			pixel[y][x] = q/2;
		}
	}

	idctclk += __rdtsc() - a;
}
*/

/* inverse integer DCT 
void idct2_i(conv data[8][8], conv pixel[8][8])
{
	unsigned x, y, n;
	conv tmp[8][8];

	uint64_t a = __rdtsc();

	// process rows
	for (y = 0; y < 8; y++)
	for (x = 0; x < 8; x++)
	{
		int q = 0;

		for (n = 0; n < 8; n++)
			q += data[y][n] * dct_tbl_i[n][x];

		tmp[y][x] = q/(IDCTI_AMP*2);
	}
		
	// process columns
	for (x = 0; x < 8; x++)
	for (y = 0; y < 8; y++)
	{
		int q = 0;

		for (n = 0; n < 8; n++)
			q += tmp[n][x] * dct_tbl_i[n][y];

		pixel[y][x] = q/(IDCTI_AMP*2);
	}

	idctclk += __rdtsc() - a;
} 
*/

#ifdef _MSC_VER

/*
void test_idct2_s()
{
	int data[4] = {1,2,3,4};
	int res;

	// process rows
	__m128i b = _mm_loadu_si128 ((__m128i*)data);

	__m128i c = _mm_shuffle_epi32 (b, 0xB1);
	c = _mm_add_epi32 (b, c);

	b = _mm_shuffle_epi32 (c, 0x27);
	c = _mm_add_epi32 (b, c);

	res = _mm_cvtsi128_si32 (c);
}
*/

void idct2_s(conv data[8][8], conv pixel[8][8])
{
	CACHE_ALIGN conv tmp[8][8];
	unsigned x, y;

	// process rows
	for (y = 0; y < 8; y++) {

		__m128i r0 = _mm_loadu_si128 ((__m128i*)data[y]);

		for (x = 0; x < 8; x++)
		{
			__m128i r1, r2, r3, r4;

			r1 = _mm_load_si128 ((__m128i*)idct_tbl_s[x]);
			r2 = _mm_madd_epi16 (r0, r1);
			r3 = _mm_shuffle_epi32 (r2, 0xB1);
			r4 = _mm_add_epi32 (r2, r3);
			r1 = _mm_shuffle_epi32 (r4, 0x27);
			r2 = _mm_add_epi32 (r1, r4);
			tmp[x][y] = _mm_cvtsi128_si32(r2)/(IDCTI_AMP*2);
		}
	}

	// process columns
	for (y = 0; y < 8; y++) {

		__m128i r0 = _mm_loadu_si128 ((__m128i*)tmp[y]);

		for (x = 0; x < 8; x += 2)
		{
			__m128i r1, r2, r3, r4, r5, r6;

			r1 = _mm_load_si128 ((__m128i*)idct_tbl_s[x]);
			r2 = _mm_madd_epi16 (r0, r1);
			r4 = _mm_load_si128 ((__m128i*)idct_tbl_s[x+1]);
			r1 = _mm_shuffle_epi32 (r2, 0xB1);
			r5 = _mm_madd_epi16 (r0, r4);
			r3 = _mm_add_epi32 (r1, r2);
			r4 = _mm_shuffle_epi32 (r5, 0xB1);
			r2 = _mm_shuffle_epi32 (r3, 0x27);
			r6 = _mm_add_epi32 (r4, r5);
			r1 = _mm_add_epi32 (r2, r3);
			r5 = _mm_shuffle_epi32 (r6, 0x27);
			pixel[x][y] = _mm_cvtsi128_si32(r1)/(IDCTI_AMP*2);
			r4 = _mm_add_epi32 (r5, r6);
			pixel[x+1][y] = _mm_cvtsi128_si32(r4)/(IDCTI_AMP*2);
		}
	}
}

#endif//_MSC_VER

// simple but fast IDCT
void idct3(short data[8][8], short pixel[8][8])
{
	CACHE_ALIGN short rows[8][8];
	unsigned i;

	static const short // Ci = cos(i*PI/16)*(1<<14);
        C1 = 16070,
        C2 = 15137,
        C3 = 13623,
        C4 = 11586,
        C5 = 9103,
        C6 = 6270,
        C7 = 3196;

	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		const short x0 = data[i][0];
		const short x4 = data[i][4];
		const short t0 = C4*(x0 + x4) >> 15;
		const short t4 = C4*(x0 - x4) >> 15;

		const short x2 = data[i][2];
		const short x6 = data[i][6];
		const short t2 = (C2*x2 + C6*x6) >> 15; 
		const short t6 = (C6*x2 - C2*x6) >> 15; 

		const short e0 = t0 + t2; 
		const short e3 = t0 - t2; 
		const short e1 = t4 + t6; 
		const short e2 = t4 - t6; 

		const short x1 = data[i][1];
		const short x3 = data[i][3];
		const short x5 = data[i][5];
		const short x7 = data[i][7];
		const short o0 = (C1*x1 + C5*x5 + C3*x3 + C7*x7) >> 15;
		const short o1 = (C3*x1 - C1*x5 - C7*x3 - C5*x7) >> 15;
		const short o2 = (C5*x1 + C7*x5 - C1*x3 + C3*x7) >> 15;
		const short o3 = (C7*x1 + C3*x5 - C5*x3 - C1*x7) >> 15;

		rows[i][0] = e0 + o0;
		rows[i][7] = e0 - o0;
		rows[i][1] = e1 + o1;
		rows[i][6] = e1 - o1;
		rows[i][2] = e2 + o2;
		rows[i][5] = e2 - o2;
		rows[i][3] = e3 + o3;
		rows[i][4] = e3 - o3;
	}

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		const short x0 = rows[0][i];
		const short x4 = rows[4][i];
		const short t0 = C4*(x0 + x4) >> 15;
		const short t4 = C4*(x0 - x4) >> 15;

		const short x2 = rows[2][i];
		const short x6 = rows[6][i];
		const short t2 = (C2*x2 + C6*x6) >> 15; 
		const short t6 = (C6*x2 - C2*x6) >> 15; 

		const short e0 = t0 + t2; 
		const short e3 = t0 - t2; 
		const short e1 = t4 + t6; 
		const short e2 = t4 - t6; 

		const short x1 = rows[1][i];
		const short x3 = rows[3][i];
		const short x5 = rows[5][i];
		const short x7 = rows[7][i];
		const short o0 = (C1*x1 + C5*x5 + C3*x3 + C7*x7) >> 15;
		const short o1 = (C3*x1 - C1*x5 - C7*x3 - C5*x7) >> 15;
		const short o2 = (C5*x1 + C7*x5 - C1*x3 + C3*x7) >> 15;
		const short o3 = (C7*x1 + C3*x5 - C5*x3 - C1*x7) >> 15;

		pixel[0][i] = e0 + o0;
		pixel[7][i] = e0 - o0;
		pixel[1][i] = e1 + o1;
		pixel[6][i] = e1 - o1;
		pixel[2][i] = e2 + o2;
		pixel[5][i] = e2 - o2;
		pixel[3][i] = e3 + o3;
		pixel[4][i] = e3 - o3;
	}
}

// Ci = cos(i*PI/16)*(1<<16);
#define C1	(16070*2+16)
#define C2	(15137*2+15)
#define C3	(13623*2+14)
#define C4	(11586*2+11)
#define C5	(9103*2+9)
#define C6	(6270*2+6)
#define C7	(3196*2+3)

// simple but fast DCT+SSE2 - 5 multiplications, 5 shuffles, 4 additions, 2 shifts, 2 unpacks (1D).
// only 2 times faster than the non-simd version :(
void dct3_sse_0(short pixels[8][8], short data[8][8])
{
	CACHE_ALIGN short rows[8][8];
	SIMD_ALIGN  int   tmp[4];
	unsigned          i;

	SIMD_ALIGN static const short vec0[8] = { -1,   1,  -1,   1,  -1,   1,  -1,   1};
	SIMD_ALIGN static const short vec1[8] = {-C4,  C1, -C2,  C3, -C4,  C5, -C6,  C7};
	SIMD_ALIGN static const short vec2[8] = {-C4,  C3, -C6, -C7,  C4, -C1,  C2, -C5};
	SIMD_ALIGN static const short vec3[8] = {-C4,  C5,  C6, -C1,  C4,  C7, -C2,  C3};
	SIMD_ALIGN static const short vec4[8] = {-C4,  C7,  C2, -C5, -C4,  C3,  C6, -C1};

	const __m128i v0 = _mm_load_si128 ((__m128i*)vec0);
	const __m128i v1 = _mm_load_si128 ((__m128i*)vec1);
	const __m128i v2 = _mm_load_si128 ((__m128i*)vec2);
	const __m128i v3 = _mm_load_si128 ((__m128i*)vec3);
	const __m128i v4 = _mm_load_si128 ((__m128i*)vec4);
 
	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		__m128i r0, r1, r2, r3;

		r1 = _mm_load_si128 ((__m128i*)pixels[i]);
		r1 = _mm_slli_epi16 (r1, 4); // to increase mulhi precision

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r3 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r3, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)tmp, r3);
		rows[0][i] = tmp[0];
		rows[1][i] = tmp[0] >> 16;
		rows[2][i] = tmp[1];
		rows[3][i] = tmp[1] >> 16;
		rows[4][i] = tmp[2];
		rows[5][i] = tmp[2] >> 16;
		rows[6][i] = tmp[3];
		rows[7][i] = tmp[3] >> 16;
	}

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		__m128i r0, r1, r2, r3;

		r1 = _mm_load_si128 ((__m128i*)rows[i]);
		r1 = _mm_slli_epi16 (r1, 4); // to increase mulhi precision

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r3 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r3, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)tmp, r3);
		data[0][i] = tmp[0];
		data[1][i] = tmp[0] >> 16;
		data[2][i] = tmp[1];
		data[3][i] = tmp[1] >> 16;
		data[4][i] = tmp[2];
		data[5][i] = tmp[2] >> 16;
		data[6][i] = tmp[3];
		data[7][i] = tmp[3] >> 16;
	}
}

void dct3_sse(short pixels[8][8], short data[8][8])
{
	CACHE_ALIGN short rows[8][8];
	SIMD_ALIGN  int   tmp[4];
	unsigned          i;

	SIMD_ALIGN static const short vec0[8] = { -1,   1,  -1,   1,  -1,   1,  -1,   1};
	SIMD_ALIGN static const short vec1[8] = {-C4,  C1, -C2,  C3, -C4,  C5, -C6,  C7};
	SIMD_ALIGN static const short vec2[8] = {-C4,  C3, -C6, -C7,  C4, -C1,  C2, -C5};
	SIMD_ALIGN static const short vec3[8] = {-C4,  C5,  C6, -C1,  C4,  C7, -C2,  C3};
	SIMD_ALIGN static const short vec4[8] = {-C4,  C7,  C2, -C5, -C4,  C3,  C6, -C1};

	const __m128i v0 = _mm_load_si128 ((__m128i*)vec0);
	const __m128i v1 = _mm_load_si128 ((__m128i*)vec1);
	const __m128i v2 = _mm_load_si128 ((__m128i*)vec2);
	const __m128i v3 = _mm_load_si128 ((__m128i*)vec3);
	const __m128i v4 = _mm_load_si128 ((__m128i*)vec4);

	/* transform rows */
	for (i = 0; i < 8; i += 2)
	{
		__m128i r0, r1, r2, r3;
 
		r1 = _mm_load_si128 ((__m128i*)pixels[i]);
		r1 = _mm_slli_epi16 (r1, 4); // to increase mulhi precision

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)tmp, r3);
		rows[0][i] = tmp[0];
		rows[1][i] = tmp[0] >> 16;
		rows[2][i] = tmp[1];
		rows[3][i] = tmp[1] >> 16;
		rows[4][i] = tmp[2];
		rows[5][i] = tmp[2] >> 16;
		rows[6][i] = tmp[3];
		rows[7][i] = tmp[3] >> 16;

		r1 = _mm_load_si128 ((__m128i*)pixels[i+1]);
		r1 = _mm_slli_epi16 (r1, 4);

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)tmp, r3);
		rows[0][i+1] = tmp[0];
		rows[1][i+1] = tmp[0] >> 16;
		rows[2][i+1] = tmp[1];
		rows[3][i+1] = tmp[1] >> 16;
		rows[4][i+1] = tmp[2];
		rows[5][i+1] = tmp[2] >> 16;
		rows[6][i+1] = tmp[3];
		rows[7][i+1] = tmp[3] >> 16;
	}

	/* transform columns */
	for (i = 0; i < 8; i += 2)
	{
		__m128i r0, r1, r2, r3;
 
		r1 = _mm_load_si128 ((__m128i*)rows[i]);
		r1 = _mm_slli_epi16 (r1, 4);

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)tmp, r3);
		data[0][i] = tmp[0];
		data[1][i] = tmp[0] >> 16;
		data[2][i] = tmp[1];
		data[3][i] = tmp[1] >> 16;
		data[4][i] = tmp[2];
		data[5][i] = tmp[2] >> 16;
		data[6][i] = tmp[3];
		data[7][i] = tmp[3] >> 16;
 
		r1 = _mm_load_si128 ((__m128i*)rows[i+1]);
		r1 = _mm_slli_epi16 (r1, 4);

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)tmp, r3);
		data[0][i+1] = tmp[0];
		data[1][i+1] = tmp[0] >> 16;
		data[2][i+1] = tmp[1];
		data[3][i+1] = tmp[1] >> 16;
		data[4][i+1] = tmp[2];
		data[5][i+1] = tmp[2] >> 16;
		data[6][i+1] = tmp[3];
		data[7][i+1] = tmp[3] >> 16;
	}
}

void dct3_sse_1(short pixels[8][8], short data[8][8])
{
	CACHE_ALIGN short rows[8][8];
	SIMD_ALIGN  short tmp[8];
	unsigned          i;

	SIMD_ALIGN static const short vec0[8] = { -1,   1,  -1,   1,  -1,   1,  -1,   1};
	SIMD_ALIGN static const short vec1[8] = {-C4,  C1, -C2,  C3, -C4,  C5, -C6,  C7};
	SIMD_ALIGN static const short vec2[8] = {-C4,  C3, -C6, -C7,  C4, -C1,  C2, -C5};
	SIMD_ALIGN static const short vec3[8] = {-C4,  C5,  C6, -C1,  C4,  C7, -C2,  C3};
	SIMD_ALIGN static const short vec4[8] = {-C4,  C7,  C2, -C5, -C4,  C3,  C6, -C1};

	const __m128i v0 = _mm_load_si128 ((__m128i*)vec0);
	const __m128i v1 = _mm_load_si128 ((__m128i*)vec1);
	const __m128i v2 = _mm_load_si128 ((__m128i*)vec2);
	const __m128i v3 = _mm_load_si128 ((__m128i*)vec3);
	const __m128i v4 = _mm_load_si128 ((__m128i*)vec4);

	__m128i r0, r1, r2, r3;
 
	/* transform rows */
	r1 = _mm_load_si128 ((__m128i*)pixels[0]);

	r0 = _mm_unpacklo_epi16 (r1, r1);
	r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
	r0 = _mm_mullo_epi16 (r0, v0);
	r1 = _mm_unpackhi_epi16 (r1, r1);

	r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

	r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
	r3 = _mm_mulhi_epi16 (r2, v1);

	r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
	r1 = _mm_mulhi_epi16 (r1, v2);
	r3 = _mm_add_epi16 (r3, r1);

	r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
	r2 = _mm_mulhi_epi16 (r2, v3);
	r3 = _mm_add_epi16 (r3, r2);

	r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
	r1 = _mm_mulhi_epi16 (r1, v4);
	r3 = _mm_add_epi16 (r3, r1);

	_mm_store_si128 ((__m128i*)tmp, r3);

	for (i = 1; i < 8; i++)
	{
		r1 = _mm_load_si128 ((__m128i*)pixels[i]);

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		rows[0][i-1] = tmp[0];
		rows[1][i-1] = tmp[1];
		rows[2][i-1] = tmp[2];
		rows[3][i-1] = tmp[3];
		rows[4][i-1] = tmp[4];
		rows[5][i-1] = tmp[5];
		rows[6][i-1] = tmp[6];
		rows[7][i-1] = tmp[7];
		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		_mm_store_si128 ((__m128i*)tmp, r3);
	}

	rows[0][7] = tmp[0];
	rows[1][7] = tmp[1];
	rows[2][7] = tmp[2];
	rows[3][7] = tmp[3];
	rows[4][7] = tmp[4];
	rows[5][7] = tmp[5];
	rows[6][7] = tmp[6];
	rows[7][7] = tmp[7];

	/* transform columns */
	r1 = _mm_load_si128 ((__m128i*)rows[0]);

	r0 = _mm_unpacklo_epi16 (r1, r1);
	r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
	r0 = _mm_mullo_epi16 (r0, v0);
	r1 = _mm_unpackhi_epi16 (r1, r1);

	r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

	r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
	r3 = _mm_mulhi_epi16 (r2, v1);

	r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
	r1 = _mm_mulhi_epi16 (r1, v2);
	r3 = _mm_add_epi16 (r3, r1);

	r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
	r2 = _mm_mulhi_epi16 (r2, v3);
	r3 = _mm_add_epi16 (r3, r2);

	r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
	r1 = _mm_mulhi_epi16 (r1, v4);
	r3 = _mm_add_epi16 (r3, r1);

	_mm_store_si128 ((__m128i*)tmp, r3);

	for (i = 1; i < 8; i++)
	{
		r1 = _mm_load_si128 ((__m128i*)rows[i]);

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		data[0][i-1] = tmp[0];
		data[1][i-1] = tmp[1];
		data[2][i-1] = tmp[2];
		data[3][i-1] = tmp[3];
		data[4][i-1] = tmp[4];
		data[5][i-1] = tmp[5];
		data[6][i-1] = tmp[6];
		data[7][i-1] = tmp[7];
		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		_mm_store_si128 ((__m128i*)tmp, r3);
	}

	data[0][7] = tmp[0];
	data[1][7] = tmp[1];
	data[2][7] = tmp[2];
	data[3][7] = tmp[3];
	data[4][7] = tmp[4];
	data[5][7] = tmp[5];
	data[6][7] = tmp[6];
	data[7][7] = tmp[7];
}

static void transpose1(short data[8][8])
{
	__m128i r0, r1, r2, r3, r4, r5, r6, r7;

	r0 = _mm_load_si128 ((__m128i*)data[0]);
	r1 = _mm_load_si128 ((__m128i*)data[1]);
	r2 = _mm_load_si128 ((__m128i*)data[2]);
	r3 = _mm_load_si128 ((__m128i*)data[3]);
	r4 = _mm_load_si128 ((__m128i*)data[4]);
	r5 = _mm_load_si128 ((__m128i*)data[5]);
	r6 = _mm_load_si128 ((__m128i*)data[6]);
	r7 = _mm_load_si128 ((__m128i*)data[7]);

	data[0][0] = _mm_extract_epi16 (r0, 0);
	data[1][0] = _mm_extract_epi16 (r0, 1);
	data[2][0] = _mm_extract_epi16 (r0, 2);
	data[3][0] = _mm_extract_epi16 (r0, 3);
	data[4][0] = _mm_extract_epi16 (r0, 4);
	data[5][0] = _mm_extract_epi16 (r0, 5);
	data[6][0] = _mm_extract_epi16 (r0, 6);
	data[7][0] = _mm_extract_epi16 (r0, 7);

	data[0][1] = _mm_extract_epi16 (r1, 0);
	data[1][1] = _mm_extract_epi16 (r1, 1);
	data[2][1] = _mm_extract_epi16 (r1, 2);
	data[3][1] = _mm_extract_epi16 (r1, 3);
	data[4][1] = _mm_extract_epi16 (r1, 4);
	data[5][1] = _mm_extract_epi16 (r1, 5);
	data[6][1] = _mm_extract_epi16 (r1, 6);
	data[7][1] = _mm_extract_epi16 (r1, 7);

	data[0][2] = _mm_extract_epi16 (r2, 0);
	data[1][2] = _mm_extract_epi16 (r2, 1);
	data[2][2] = _mm_extract_epi16 (r2, 2);
	data[3][2] = _mm_extract_epi16 (r2, 3);
	data[4][2] = _mm_extract_epi16 (r2, 4);
	data[5][2] = _mm_extract_epi16 (r2, 5);
	data[6][2] = _mm_extract_epi16 (r2, 6);
	data[7][2] = _mm_extract_epi16 (r2, 7);

	data[0][3] = _mm_extract_epi16 (r3, 0);
	data[1][3] = _mm_extract_epi16 (r3, 1);
	data[2][3] = _mm_extract_epi16 (r3, 2);
	data[3][3] = _mm_extract_epi16 (r3, 3);
	data[4][3] = _mm_extract_epi16 (r3, 4);
	data[5][3] = _mm_extract_epi16 (r3, 5);
	data[6][3] = _mm_extract_epi16 (r3, 6);
	data[7][3] = _mm_extract_epi16 (r3, 7);

	data[0][4] = _mm_extract_epi16 (r4, 0);
	data[1][4] = _mm_extract_epi16 (r4, 1);
	data[2][4] = _mm_extract_epi16 (r4, 2);
	data[3][4] = _mm_extract_epi16 (r4, 3);
	data[4][4] = _mm_extract_epi16 (r4, 4);
	data[5][4] = _mm_extract_epi16 (r4, 5);
	data[6][4] = _mm_extract_epi16 (r4, 6);
	data[7][4] = _mm_extract_epi16 (r4, 7);

	data[0][5] = _mm_extract_epi16 (r5, 0);
	data[1][5] = _mm_extract_epi16 (r5, 1);
	data[2][5] = _mm_extract_epi16 (r5, 2);
	data[3][5] = _mm_extract_epi16 (r5, 3);
	data[4][5] = _mm_extract_epi16 (r5, 4);
	data[5][5] = _mm_extract_epi16 (r5, 5);
	data[6][5] = _mm_extract_epi16 (r5, 6);
	data[7][5] = _mm_extract_epi16 (r5, 7);

	data[0][6] = _mm_extract_epi16 (r6, 0);
	data[1][6] = _mm_extract_epi16 (r6, 1);
	data[2][6] = _mm_extract_epi16 (r6, 2);
	data[3][6] = _mm_extract_epi16 (r6, 3);
	data[4][6] = _mm_extract_epi16 (r6, 4);
	data[5][6] = _mm_extract_epi16 (r6, 5);
	data[6][6] = _mm_extract_epi16 (r6, 6);
	data[7][6] = _mm_extract_epi16 (r6, 7);

	data[0][7] = _mm_extract_epi16 (r7, 0);
	data[1][7] = _mm_extract_epi16 (r7, 1);
	data[2][7] = _mm_extract_epi16 (r7, 2);
	data[3][7] = _mm_extract_epi16 (r7, 3);
	data[4][7] = _mm_extract_epi16 (r7, 4);
	data[5][7] = _mm_extract_epi16 (r7, 5);
	data[6][7] = _mm_extract_epi16 (r7, 6);
	data[7][7] = _mm_extract_epi16 (r7, 7);
}

static void transpose2(short data[8][8])
{
	__m128i r0, r1, r2, r3, r4, r5, r6, r7;

	r0 = _mm_insert_epi16 (r0, data[0][0], 0);
	r0 = _mm_insert_epi16 (r0, data[1][0], 1);
	r0 = _mm_insert_epi16 (r0, data[2][0], 2);
	r0 = _mm_insert_epi16 (r0, data[3][0], 3);
	r0 = _mm_insert_epi16 (r0, data[4][0], 4);
	r0 = _mm_insert_epi16 (r0, data[5][0], 5);
	r0 = _mm_insert_epi16 (r0, data[6][0], 6);
	r0 = _mm_insert_epi16 (r0, data[7][0], 7);

	r1 = _mm_insert_epi16 (r1, data[0][1], 0);
	r1 = _mm_insert_epi16 (r1, data[1][1], 1);
	r1 = _mm_insert_epi16 (r1, data[2][1], 2);
	r1 = _mm_insert_epi16 (r1, data[3][1], 3);
	r1 = _mm_insert_epi16 (r1, data[4][1], 4);
	r1 = _mm_insert_epi16 (r1, data[5][1], 5);
	r1 = _mm_insert_epi16 (r1, data[6][1], 6);
	r1 = _mm_insert_epi16 (r1, data[7][1], 7);

	r2 = _mm_insert_epi16 (r2, data[0][2], 0);
	r2 = _mm_insert_epi16 (r2, data[1][2], 1);
	r2 = _mm_insert_epi16 (r2, data[2][2], 2);
	r2 = _mm_insert_epi16 (r2, data[3][2], 3);
	r2 = _mm_insert_epi16 (r2, data[4][2], 4);
	r2 = _mm_insert_epi16 (r2, data[5][2], 5);
	r2 = _mm_insert_epi16 (r2, data[6][2], 6);
	r2 = _mm_insert_epi16 (r2, data[7][2], 7);

	r3 = _mm_insert_epi16 (r3, data[0][3], 0);
	r3 = _mm_insert_epi16 (r3, data[1][3], 1);
	r3 = _mm_insert_epi16 (r3, data[2][3], 2);
	r3 = _mm_insert_epi16 (r3, data[3][3], 3);
	r3 = _mm_insert_epi16 (r3, data[4][3], 4);
	r3 = _mm_insert_epi16 (r3, data[5][3], 5);
	r3 = _mm_insert_epi16 (r3, data[6][3], 6);
	r3 = _mm_insert_epi16 (r3, data[7][3], 7);

	r4 = _mm_insert_epi16 (r4, data[0][4], 0);
	r4 = _mm_insert_epi16 (r4, data[1][4], 1);
	r4 = _mm_insert_epi16 (r4, data[2][4], 2);
	r4 = _mm_insert_epi16 (r4, data[3][4], 3);
	r4 = _mm_insert_epi16 (r4, data[4][4], 4);
	r4 = _mm_insert_epi16 (r4, data[5][4], 5);
	r4 = _mm_insert_epi16 (r4, data[6][4], 6);
	r4 = _mm_insert_epi16 (r4, data[7][4], 7);

	r5 = _mm_insert_epi16 (r5, data[0][5], 0);
	r5 = _mm_insert_epi16 (r5, data[1][5], 1);
	r5 = _mm_insert_epi16 (r5, data[2][5], 2);
	r5 = _mm_insert_epi16 (r5, data[3][5], 3);
	r5 = _mm_insert_epi16 (r5, data[4][5], 4);
	r5 = _mm_insert_epi16 (r5, data[5][5], 5);
	r5 = _mm_insert_epi16 (r5, data[6][5], 6);
	r5 = _mm_insert_epi16 (r5, data[7][5], 7);

	r6 = _mm_insert_epi16 (r6, data[0][6], 0);
	r6 = _mm_insert_epi16 (r6, data[1][6], 1);
	r6 = _mm_insert_epi16 (r6, data[2][6], 2);
	r6 = _mm_insert_epi16 (r6, data[3][6], 3);
	r6 = _mm_insert_epi16 (r6, data[4][6], 4);
	r6 = _mm_insert_epi16 (r6, data[5][6], 5);
	r6 = _mm_insert_epi16 (r6, data[6][6], 6);
	r6 = _mm_insert_epi16 (r6, data[7][6], 7);

	r7 = _mm_insert_epi16 (r7, data[0][7], 0);
	r7 = _mm_insert_epi16 (r7, data[1][7], 1);
	r7 = _mm_insert_epi16 (r7, data[2][7], 2);
	r7 = _mm_insert_epi16 (r7, data[3][7], 3);
	r7 = _mm_insert_epi16 (r7, data[4][7], 4);
	r7 = _mm_insert_epi16 (r7, data[5][7], 5);
	r7 = _mm_insert_epi16 (r7, data[6][7], 6);
	r7 = _mm_insert_epi16 (r7, data[7][7], 7);

	_mm_store_si128 ((__m128i*)data[0], r0);
	_mm_store_si128 ((__m128i*)data[1], r1);
	_mm_store_si128 ((__m128i*)data[2], r2);
	_mm_store_si128 ((__m128i*)data[3], r3);
	_mm_store_si128 ((__m128i*)data[4], r4);
	_mm_store_si128 ((__m128i*)data[5], r5);
	_mm_store_si128 ((__m128i*)data[6], r6);
	_mm_store_si128 ((__m128i*)data[7], r7);
}

static void transpose3(unsigned short data[8][8])
{
	unsigned c0, c1, c2, c3;
	unsigned r0, r1, r2, r3;

	c0 = data[0][0] | (data[1][0] << 16);
	c1 = data[2][0] | (data[3][0] << 16);
	r0 = ((unsigned*)data[0])[0];
	r1 = ((unsigned*)data[0])[1];
	((unsigned*)data[0])[0] = c0;
	((unsigned*)data[0])[1] = c1;
	data[0][0] = r0;
	data[1][0] = r0 >> 16;
	data[2][0] = r1;
	data[3][0] = r1 >> 16;

	c2 = data[4][0] | (data[5][0] << 16);
	c3 = data[6][0] | (data[7][0] << 16);
	r2 = ((unsigned*)data[0])[2];
	r3 = ((unsigned*)data[0])[3];
	((unsigned*)data[0])[2] = c2;
	((unsigned*)data[0])[3] = c3;
	data[4][0] = r2;
	data[5][0] = r2 >> 16;
	data[6][0] = r3;
	data[7][0] = r3 >> 16;

	c1 = data[2][1] | (data[3][1] << 16);
	c2 = data[4][1] | (data[5][1] << 16);
	c3 = data[6][1] | (data[7][1] << 16);
	r1 = ((unsigned*)data[1])[1];
	r2 = ((unsigned*)data[1])[2];
	r3 = ((unsigned*)data[1])[3];
	((unsigned*)data[1])[1] = c1;
	((unsigned*)data[1])[2] = c2;
	((unsigned*)data[1])[3] = c3;
	data[2][1] = r1;
	data[3][1] = r1 >> 16;
	data[4][1] = r2;
	data[5][1] = r2 >> 16;
	data[6][1] = r3;
	data[7][1] = r3 >> 16;

	c1 = data[2][2] | (data[3][2] << 16);
	c2 = data[4][2] | (data[5][2] << 16);
	c3 = data[6][2] | (data[7][2] << 16);
	r1 = ((unsigned*)data[2])[1];
	r2 = ((unsigned*)data[2])[2];
	r3 = ((unsigned*)data[2])[3];
	((unsigned*)data[2])[1] = c1;
	((unsigned*)data[2])[2] = c2;
	((unsigned*)data[2])[3] = c3;
	data[2][2] = r1;
	data[3][2] = r1 >> 16;
	data[4][2] = r2;
	data[5][2] = r2 >> 16;
	data[6][2] = r3;
	data[7][2] = r3 >> 16;

	c2 = data[4][3] | (data[5][3] << 16);
	c3 = data[6][3] | (data[7][3] << 16);
	r2 = ((unsigned*)data[3])[2];
	r3 = ((unsigned*)data[3])[3];
	((unsigned*)data[3])[2] = c2;
	((unsigned*)data[3])[3] = c3;
	data[4][3] = r2;
	data[5][3] = r2 >> 16;
	data[6][3] = r3;
	data[7][3] = r3 >> 16;

	c2 = data[4][4] | (data[5][4] << 16);
	c3 = data[6][4] | (data[7][4] << 16);
	r2 = ((unsigned*)data[4])[2];
	r3 = ((unsigned*)data[4])[3];
	((unsigned*)data[4])[2] = c2;
	((unsigned*)data[4])[3] = c3;
	data[4][4] = r2;
	data[5][4] = r2 >> 16;
	data[6][4] = r3;
	data[7][4] = r3 >> 16;

	c3 = data[6][5] | (data[7][5] << 16);
	r3 = ((unsigned*)data[5])[3];
	((unsigned*)data[5])[3] = c3;
	data[6][5] = r3;
	data[7][5] = r3 >> 16;

	c3 = data[6][6] | (data[7][6] << 16);
	r3 = ((unsigned*)data[6])[3];
	((unsigned*)data[6])[3] = c3;
	data[6][6] = r3;
	data[7][6] = r3 >> 16;
}

static void transpose4(short data[8][8])
{
	__m128i t0, t1, t2, t3, t4, t5, t6, t7;

	// left,upper
	t0 = _mm_loadl_epi64 ((__m128i*)&data[0][0]);
	t1 = _mm_loadl_epi64 ((__m128i*)&data[1][0]);
	t2 = _mm_loadl_epi64 ((__m128i*)&data[2][0]);
	t3 = _mm_loadl_epi64 ((__m128i*)&data[3][0]);

	t0 = _mm_unpacklo_epi16 (t0, t1);
	t2 = _mm_unpacklo_epi16 (t2, t3);
	t1 = _mm_unpacklo_epi32 (t0, t2);
	t3 = _mm_unpackhi_epi32 (t0, t2);

	t0 = _mm_srli_si128 (t1, 8);
	t2 = _mm_srli_si128 (t3, 8);

	_mm_storel_epi64((__m128i*)&data[0][0], t1);
	_mm_storel_epi64((__m128i*)&data[1][0], t0);
	_mm_storel_epi64((__m128i*)&data[2][0], t3);
	_mm_storel_epi64((__m128i*)&data[3][0], t2);

	// right,upper <-> left,down
	t0 = _mm_loadl_epi64 ((__m128i*)&data[0][4]);
	t1 = _mm_loadl_epi64 ((__m128i*)&data[1][4]);
	t2 = _mm_loadl_epi64 ((__m128i*)&data[2][4]);
	t3 = _mm_loadl_epi64 ((__m128i*)&data[3][4]);

	t0 = _mm_unpacklo_epi16 (t0, t1);
	t2 = _mm_unpacklo_epi16 (t2, t3);
	t1 = _mm_unpacklo_epi32 (t0, t2);
	t3 = _mm_unpackhi_epi32 (t0, t2);

	t0 = _mm_srli_si128 (t1, 8);
	t2 = _mm_srli_si128 (t3, 8);

	t4 = _mm_loadl_epi64 ((__m128i*)&data[4][0]);
	t5 = _mm_loadl_epi64 ((__m128i*)&data[5][0]);
	t6 = _mm_loadl_epi64 ((__m128i*)&data[6][0]);
	t7 = _mm_loadl_epi64 ((__m128i*)&data[7][0]);
	_mm_storel_epi64((__m128i*)&data[4][0], t1);
	_mm_storel_epi64((__m128i*)&data[5][0], t0);
	_mm_storel_epi64((__m128i*)&data[6][0], t3);
	_mm_storel_epi64((__m128i*)&data[7][0], t2);

	t4 = _mm_unpacklo_epi16 (t4, t5);
	t6 = _mm_unpacklo_epi16 (t6, t7);
	t5 = _mm_unpacklo_epi32 (t4, t6);
	t7 = _mm_unpackhi_epi32 (t4, t6);

	t4 = _mm_srli_si128 (t5, 8);
	t6 = _mm_srli_si128 (t7, 8);
	_mm_storel_epi64((__m128i*)&data[0][4], t5);
	_mm_storel_epi64((__m128i*)&data[1][4], t4);
	_mm_storel_epi64((__m128i*)&data[2][4], t7);
	_mm_storel_epi64((__m128i*)&data[3][4], t6);

	// right,down
	t0 = _mm_loadl_epi64 ((__m128i*)&data[4][4]);
	t1 = _mm_loadl_epi64 ((__m128i*)&data[5][4]);
	t2 = _mm_loadl_epi64 ((__m128i*)&data[6][4]);
	t3 = _mm_loadl_epi64 ((__m128i*)&data[7][4]);

	t0 = _mm_unpacklo_epi16 (t0, t1);
	t2 = _mm_unpacklo_epi16 (t2, t3);
	t1 = _mm_unpacklo_epi32 (t0, t2);
	t3 = _mm_unpackhi_epi32 (t0, t2);

	t0 = _mm_srli_si128 (t1, 8);
	t2 = _mm_srli_si128 (t3, 8);

	_mm_storel_epi64((__m128i*)&data[4][4], t1);
	_mm_storel_epi64((__m128i*)&data[5][4], t0);
	_mm_storel_epi64((__m128i*)&data[6][4], t3);
	_mm_storel_epi64((__m128i*)&data[7][4], t2);
}

void dct3_sse_2(short pixels[8][8], short data[8][8])
{
	SIMD_ALIGN static const short vec0[8] = { -1,   1,  -1,   1,  -1,   1,  -1,   1};
	SIMD_ALIGN static const short vec1[8] = {-C4,  C1, -C2,  C3, -C4,  C5, -C6,  C7};
	SIMD_ALIGN static const short vec2[8] = {-C4,  C3, -C6, -C7,  C4, -C1,  C2, -C5};
	SIMD_ALIGN static const short vec3[8] = {-C4,  C5,  C6, -C1,  C4,  C7, -C2,  C3};
	SIMD_ALIGN static const short vec4[8] = {-C4,  C7,  C2, -C5, -C4,  C3,  C6, -C1};

	const __m128i v0 = _mm_load_si128 ((__m128i*)vec0);
	const __m128i v1 = _mm_load_si128 ((__m128i*)vec1);
	const __m128i v2 = _mm_load_si128 ((__m128i*)vec2);
	const __m128i v3 = _mm_load_si128 ((__m128i*)vec3);
	const __m128i v4 = _mm_load_si128 ((__m128i*)vec4);
 
	unsigned i;

	/* transform rows */
	for (i = 0; i < 8; i++)
	{
		__m128i r0, r1, r2, r3;

		r1 = _mm_load_si128 ((__m128i*)pixels[i]);
		r1 = _mm_slli_epi16 (r1, 4); // to increase mulhi precision

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r3 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r3, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)data[i], r3);
	}

	transpose3(data);

	/* transform columns */
	for (i = 0; i < 8; i++)
	{
		__m128i r0, r1, r2, r3;

		r1 = _mm_load_si128 ((__m128i*)data[i]);
		r1 = _mm_slli_epi16 (r1, 4); // to increase mulhi precision

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r3 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r3, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)data[i], r3);
	}

	transpose3(data);
}

// This DCT leaves data matrix transposed to skip some data movement.
// The huffman coding then applies transposed zig-zap lookup table to compensate this -
// it costs nothing.
void dct3_sse_3(short pixels[8][8], short data[8][8])
{
	CACHE_ALIGN short rows[8][8];
	SIMD_ALIGN  int   tmp[4];
	unsigned          i;

	SIMD_ALIGN static const short vec0[8] = { -1,   1,  -1,   1,  -1,   1,  -1,   1};
	SIMD_ALIGN static const short vec1[8] = {-C4,  C1, -C2,  C3, -C4,  C5, -C6,  C7};
	SIMD_ALIGN static const short vec2[8] = {-C4,  C3, -C6, -C7,  C4, -C1,  C2, -C5};
	SIMD_ALIGN static const short vec3[8] = {-C4,  C5,  C6, -C1,  C4,  C7, -C2,  C3};
	SIMD_ALIGN static const short vec4[8] = {-C4,  C7,  C2, -C5, -C4,  C3,  C6, -C1};

	const __m128i v0 = _mm_load_si128 ((__m128i*)vec0);
	const __m128i v1 = _mm_load_si128 ((__m128i*)vec1);
	const __m128i v2 = _mm_load_si128 ((__m128i*)vec2);
	const __m128i v3 = _mm_load_si128 ((__m128i*)vec3);
	const __m128i v4 = _mm_load_si128 ((__m128i*)vec4);

	/* transform rows */
	for (i = 0; i < 8; i += 2)
	{
		__m128i r0, r1, r2, r3;
 
		r1 = _mm_load_si128 ((__m128i*)pixels[i]);
		r1 = _mm_slli_epi16 (r1, 4); // to increase mulhi precision

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)tmp, r3);
		rows[0][i] = tmp[0];
		rows[1][i] = tmp[0] >> 16;
		rows[2][i] = tmp[1];
		rows[3][i] = tmp[1] >> 16;
		rows[4][i] = tmp[2];
		rows[5][i] = tmp[2] >> 16;
		rows[6][i] = tmp[3];
		rows[7][i] = tmp[3] >> 16;

		r1 = _mm_load_si128 ((__m128i*)pixels[i+1]);
		r1 = _mm_slli_epi16 (r1, 4);

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)tmp, r3);
		rows[0][i+1] = tmp[0];
		rows[1][i+1] = tmp[0] >> 16;
		rows[2][i+1] = tmp[1];
		rows[3][i+1] = tmp[1] >> 16;
		rows[4][i+1] = tmp[2];
		rows[5][i+1] = tmp[2] >> 16;
		rows[6][i+1] = tmp[3];
		rows[7][i+1] = tmp[3] >> 16;
	}

	/* transform columns */
	for (i = 0; i < 8; i += 2)
	{
		__m128i r0, r1, r2, r3;
 
		r1 = _mm_load_si128 ((__m128i*)rows[i]);
		r1 = _mm_slli_epi16 (r1, 4);

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)data[i], r3);
 
		r1 = _mm_load_si128 ((__m128i*)rows[i+1]);
		r1 = _mm_slli_epi16 (r1, 4);

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)data[i+1], r3);
	}
}

// Simple but fast DCT - 22 multiplication and 28 additions. 
// This DCT leaves data matrix transposed to skip some data movement.
// The huffman coding then applies transposed zig-zap lookup table to compensate this.
void dct3_sse_4(short pixels[8][8], short data[8][8])
{
#	define SH	3
	SIMD_ALIGN static const short vec1[8] = { C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1};
	SIMD_ALIGN static const short vec2[8] = { C2,  C2,  C2,  C2,  C2,  C2,  C2,  C2};
	SIMD_ALIGN static const short vec3[8] = { C3,  C3,  C3,  C3,  C3,  C3,  C3,  C3};
	SIMD_ALIGN static const short vec4[8] = { C4,  C4,  C4,  C4,  C4,  C4,  C4,  C4};
	SIMD_ALIGN static const short vec5[8] = { C5,  C5,  C5,  C5,  C5,  C5,  C5,  C5};
	SIMD_ALIGN static const short vec6[8] = { C6,  C6,  C6,  C6,  C6,  C6,  C6,  C6};
	SIMD_ALIGN static const short vec7[8] = { C7,  C7,  C7,  C7,  C7,  C7,  C7,  C7};

	__m128i c1,c2,c3,c4,c5,c6,c7;
	__m128i s07,s16,s25,s34,s0734,s1625;
	__m128i d07,d16,d25,d34,d0734,d1625;
	register __m128i t1, t2, t3, t4;

	/* transform columns */
	t1 = _mm_load_si128((__m128i*)pixels[0]);
	t2 = _mm_load_si128((__m128i*)pixels[7]);
	s07 = _mm_add_epi16 (t1, t2);
	s07 = _mm_slli_epi16 (s07, SH);
	d07 = _mm_sub_epi16 (t1, t2);
	d07 = _mm_slli_epi16 (d07, SH);
	t1 = _mm_load_si128((__m128i*)pixels[1]);
	t2 = _mm_load_si128((__m128i*)pixels[6]);
	s16 = _mm_add_epi16 (t1, t2);
	s16 = _mm_slli_epi16 (s16, SH);
	d16 = _mm_sub_epi16 (t1, t2);
	d16 = _mm_slli_epi16 (d16, SH);
	t1 = _mm_load_si128((__m128i*)pixels[2]);
	t2 = _mm_load_si128((__m128i*)pixels[5]);
	s25 = _mm_add_epi16 (t1, t2);
	s25 = _mm_slli_epi16 (s25, SH);
	d25 = _mm_sub_epi16 (t1, t2);
	d25 = _mm_slli_epi16 (d25, SH);
	t1 = _mm_load_si128((__m128i*)pixels[3]);
	t2 = _mm_load_si128((__m128i*)pixels[4]);
	s34 = _mm_add_epi16 (t1, t2);
	s34 = _mm_slli_epi16 (s34, SH);
	d34 = _mm_sub_epi16 (t1, t2);
	d34 = _mm_slli_epi16 (d34, SH);

	c1 = _mm_load_si128 ((__m128i*)vec1);
	c3 = _mm_load_si128 ((__m128i*)vec3);
	c5 = _mm_load_si128 ((__m128i*)vec5);
	c7 = _mm_load_si128 ((__m128i*)vec7);

	//rows[i][1] = C1*d07 + C3*d16 + C5*d25 + C7*d34;
	t1 = _mm_mulhi_epi16 (c1, d07);
	t2 = _mm_mulhi_epi16 (c3, d16);
	t3 = _mm_mulhi_epi16 (c5, d25);
	t4 = _mm_mulhi_epi16 (c7, d34);
	t1 = _mm_add_epi16 (t1, t2);
	t3 = _mm_add_epi16 (t3, t4);
	t1 = _mm_add_epi16 (t1, t3);
	_mm_store_si128 ((__m128i*)data[1], t1);

	//rows[i][3] = C3*d07 - C7*d16 - C1*d25 - C5*d34;
	t1 = _mm_mulhi_epi16 (c3, d07);
	t2 = _mm_mulhi_epi16 (c7, d16);
	t3 = _mm_mulhi_epi16 (c1, d25);
	t4 = _mm_mulhi_epi16 (c5, d34);
	t1 = _mm_sub_epi16 (t1, t2);
	t3 = _mm_add_epi16 (t3, t4);
	t1 = _mm_sub_epi16 (t1, t3);
	_mm_store_si128 ((__m128i*)data[3], t1);

	//rows[i][5] = C5*d07 - C1*d16 + C7*d25 + C3*d34;
	t1 = _mm_mulhi_epi16 (c5, d07);
	t2 = _mm_mulhi_epi16 (c1, d16);
	t3 = _mm_mulhi_epi16 (c7, d25);
	t4 = _mm_mulhi_epi16 (c3, d34);
	t1 = _mm_sub_epi16 (t1, t2);
	t3 = _mm_add_epi16 (t3, t4);
	t1 = _mm_add_epi16 (t1, t3);
	_mm_store_si128 ((__m128i*)data[5], t1);

	//rows[i][7] = C7*d07 - C5*d16 + C3*d25 - C1*d34;
	t1 = _mm_mulhi_epi16 (c7, d07);
	t2 = _mm_mulhi_epi16 (c5, d16);
	t3 = _mm_mulhi_epi16 (c3, d25);
	t4 = _mm_mulhi_epi16 (c1, d34);
	t1 = _mm_sub_epi16 (t1, t2);
	t3 = _mm_sub_epi16 (t3, t4);
	t1 = _mm_add_epi16 (t1, t3);
	_mm_store_si128 ((__m128i*)data[7], t1);

	s0734 = _mm_add_epi16 (s07, s34);
	d0734 = _mm_sub_epi16 (s07, s34);
	s1625 = _mm_add_epi16 (s16, s25);
	d1625 = _mm_sub_epi16 (s16, s25);

	c4 = _mm_load_si128 ((__m128i*)vec4);

	//rows[i][0] = C4*(s0734 + s1625);
	//rows[i][4] = C4*(s0734 - s1625);
	t1 = _mm_add_epi16 (s0734, s1625);
	t2 = _mm_sub_epi16 (s0734, s1625);
	t1 = _mm_mulhi_epi16 (t1, c4);
	t2 = _mm_mulhi_epi16 (t2, c4);
	_mm_store_si128 ((__m128i*)data[0], t1);
	_mm_store_si128 ((__m128i*)data[4], t2);

	c2 = _mm_load_si128 ((__m128i*)vec2);
	c6 = _mm_load_si128 ((__m128i*)vec6);

	//rows[i][2] = C2*d0734 + C6*d1625;
	//rows[i][6] = C6*d0734 - C2*d1625;
	t1 = _mm_mulhi_epi16 (d0734, c2);
	t2 = _mm_mulhi_epi16 (d1625, c6);
	t3 = _mm_mulhi_epi16 (d0734, c6);
	t4 = _mm_mulhi_epi16 (d1625, c2);
	t1 = _mm_add_epi16 (t1, t2);
	t3 = _mm_sub_epi16 (t3, t4);
	_mm_store_si128 ((__m128i*)data[2], t1);
	_mm_store_si128 ((__m128i*)data[6], t3);

	/* transpose matrix */
	transpose4(data);

	/* transform rows */
	t1 = _mm_load_si128((__m128i*)data[0]);
	t2 = _mm_load_si128((__m128i*)data[7]);
	s07 = _mm_add_epi16 (t1, t2);
	d07 = _mm_sub_epi16 (t1, t2);
	t1 = _mm_load_si128((__m128i*)data[1]);
	t2 = _mm_load_si128((__m128i*)data[6]);
	s16 = _mm_add_epi16 (t1, t2);
	d16 = _mm_sub_epi16 (t1, t2);
	t1 = _mm_load_si128((__m128i*)data[2]);
	t2 = _mm_load_si128((__m128i*)data[5]);
	s25 = _mm_add_epi16 (t1, t2);
	d25 = _mm_sub_epi16 (t1, t2);
	t1 = _mm_load_si128((__m128i*)data[3]);
	t2 = _mm_load_si128((__m128i*)data[4]);
	s34 = _mm_add_epi16 (t1, t2);
	d34 = _mm_sub_epi16 (t1, t2);

	//data[i][1] = C1*d07 + C3*d16 + C5*d25 + C7*d34;
	t1 = _mm_mulhi_epi16 (c1, d07);
	t2 = _mm_mulhi_epi16 (c3, d16);
	t3 = _mm_mulhi_epi16 (c5, d25);
	t4 = _mm_mulhi_epi16 (c7, d34);
	t1 = _mm_add_epi16 (t1, t2);
	t3 = _mm_add_epi16 (t3, t4);
	t1 = _mm_add_epi16 (t1, t3);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[1], t1);

	//data[i][3] = C3*d07 - C7*d16 - C1*d25 - C5*d34;
	t1 = _mm_mulhi_epi16 (c3, d07);
	t2 = _mm_mulhi_epi16 (c7, d16);
	t3 = _mm_mulhi_epi16 (c1, d25);
	t4 = _mm_mulhi_epi16 (c5, d34);
	t1 = _mm_sub_epi16 (t1, t2);
	t3 = _mm_add_epi16 (t3, t4);
	t1 = _mm_sub_epi16 (t1, t3);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[3], t1);

	//data[i][5] = C5*d07 - C1*d16 + C7*d25 + C3*d34;
	t1 = _mm_mulhi_epi16 (c5, d07);
	t2 = _mm_mulhi_epi16 (c1, d16);
	t3 = _mm_mulhi_epi16 (c7, d25);
	t4 = _mm_mulhi_epi16 (c3, d34);
	t1 = _mm_sub_epi16 (t1, t2);
	t3 = _mm_add_epi16 (t3, t4);
	t1 = _mm_add_epi16 (t1, t3);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[5], t1);

	//data[i][7] = C7*d07 - C5*d16 + C3*d25 - C1*d34;
	t1 = _mm_mulhi_epi16 (c7, d07);
	t2 = _mm_mulhi_epi16 (c5, d16);
	t3 = _mm_mulhi_epi16 (c3, d25);
	t4 = _mm_mulhi_epi16 (c1, d34);
	t1 = _mm_sub_epi16 (t1, t2);
	t3 = _mm_sub_epi16 (t3, t4);
	t1 = _mm_add_epi16 (t1, t3);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[7], t1);

	s0734 = _mm_add_epi16 (s07, s34);
	d0734 = _mm_sub_epi16 (s07, s34);
	s1625 = _mm_add_epi16 (s16, s25);
	d1625 = _mm_sub_epi16 (s16, s25);

	//data[i][0] = C4*(s0734 + s1625);
	//data[i][4] = C4*(s0734 - s1625);
	t1 = _mm_add_epi16 (s0734, s1625);
	t2 = _mm_sub_epi16 (s0734, s1625);
	t1 = _mm_mulhi_epi16 (t1, c4);
	t2 = _mm_mulhi_epi16 (t2, c4);
	t1 = _mm_srai_epi16 (t1, SH);
	t2 = _mm_srai_epi16 (t2, SH);
	_mm_store_si128 ((__m128i*)data[0], t1);
	_mm_store_si128 ((__m128i*)data[4], t2);

	//data[i][2] = C2*d0734 + C6*d1625;
	//data[i][6] = C6*d0734 - C2*d1625;
	t1 = _mm_mulhi_epi16 (d0734, c2);
	t2 = _mm_mulhi_epi16 (d1625, c6);
	t3 = _mm_mulhi_epi16 (d0734, c6);
	t4 = _mm_mulhi_epi16 (d1625, c2);
	t1 = _mm_add_epi16 (t1, t2);
	t3 = _mm_sub_epi16 (t3, t4);
	t1 = _mm_srai_epi16 (t1, SH);
	t3 = _mm_srai_epi16 (t3, SH);
	_mm_store_si128 ((__m128i*)data[2], t1);
	_mm_store_si128 ((__m128i*)data[6], t3);
#	undef SH
}

void dct3_sse_5(short pixels[8][8], short data[8][8])
{
	CACHE_ALIGN short rows[8][8];

#	define SH	3
	SIMD_ALIGN static const short vec1[8] = { C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1};
	SIMD_ALIGN static const short vec2[8] = { C2,  C2,  C2,  C2,  C2,  C2,  C2,  C2};
	SIMD_ALIGN static const short vec3[8] = { C3,  C3,  C3,  C3,  C3,  C3,  C3,  C3};
	SIMD_ALIGN static const short vec4[8] = { C4,  C4,  C4,  C4,  C4,  C4,  C4,  C4};
	SIMD_ALIGN static const short vec5[8] = { C5,  C5,  C5,  C5,  C5,  C5,  C5,  C5};
	SIMD_ALIGN static const short vec6[8] = { C6,  C6,  C6,  C6,  C6,  C6,  C6,  C6};
	SIMD_ALIGN static const short vec7[8] = { C7,  C7,  C7,  C7,  C7,  C7,  C7,  C7};

	SIMD_ALIGN static const short vecA[8] = { -1,   1,  -1,   1,  -1,   1,  -1,   1};
	SIMD_ALIGN static const short vecB[8] = {-C4,  C1, -C2,  C3, -C4,  C5, -C6,  C7};
	SIMD_ALIGN static const short vecC[8] = {-C4,  C3, -C6, -C7,  C4, -C1,  C2, -C5};
	SIMD_ALIGN static const short vecD[8] = {-C4,  C5,  C6, -C1,  C4,  C7, -C2,  C3};
	SIMD_ALIGN static const short vecE[8] = {-C4,  C7,  C2, -C5, -C4,  C3,  C6, -C1};

	__m128i c1,c2,c3,c4,c5,c6,c7;
	__m128i s07,s16,s25,s34,s0734,s1625;
	__m128i d07,d16,d25,d34,d0734,d1625;
	register __m128i t1, t2, t3, t4;

	/* transform rows */
	const __m128i v0 = _mm_load_si128 ((__m128i*)vecA);
	const __m128i v1 = _mm_load_si128 ((__m128i*)vecB);
	const __m128i v2 = _mm_load_si128 ((__m128i*)vecC);
	const __m128i v3 = _mm_load_si128 ((__m128i*)vecD);
	const __m128i v4 = _mm_load_si128 ((__m128i*)vecE);

	unsigned i;

	/* transform rows *
	for (i = 0; i < 8; i += 2)
	{
		__m128i r0, r1, r2, r3;
 
		r1 = _mm_load_si128 ((__m128i*)pixels[i]);
		r1 = _mm_slli_epi16 (r1, 4); // to increase mulhi precision

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)rows[i], r3);

		r1 = _mm_load_si128 ((__m128i*)pixels[i+1]);
		r1 = _mm_slli_epi16 (r1, 4);

		r0 = _mm_unpacklo_epi16 (r1, r1);
		r1 = _mm_shufflehi_epi16 (r1, _MM_SHUFFLE(0,1,2,3)); // reverse order
		r0 = _mm_mullo_epi16 (r0, v0);
		r1 = _mm_unpackhi_epi16 (r1, r1);

		r0 = _mm_sub_epi16 (r0, r1); // compute sums & diffs

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(0,0,0,0));
		r3 = _mm_mulhi_epi16 (r2, v1);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(1,1,1,1));
		r1 = _mm_mulhi_epi16 (r1, v2);
		r3 = _mm_add_epi16 (r3, r1);

		r2 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(2,2,2,2));
		r2 = _mm_mulhi_epi16 (r2, v3);
		r3 = _mm_add_epi16 (r3, r2);

		r1 = _mm_shuffle_epi32 (r0, _MM_SHUFFLE(3,3,3,3));
		r1 = _mm_mulhi_epi16 (r1, v4);
		r3 = _mm_add_epi16 (r3, r1);

		r3 = _mm_srai_epi16 (r3, 4);
		_mm_store_si128 ((__m128i*)rows[i+1], r3);
	}*/

	transpose3(rows);
	/* transform columns */
	t1 = _mm_load_si128((__m128i*)rows[0]);
	t2 = _mm_load_si128((__m128i*)rows[7]);
	s07 = _mm_add_epi16 (t1, t2);
	s07 = _mm_slli_epi16 (s07, SH);
	d07 = _mm_sub_epi16 (t1, t2);
	d07 = _mm_slli_epi16 (d07, SH);
	t1 = _mm_load_si128((__m128i*)rows[1]);
	t2 = _mm_load_si128((__m128i*)rows[6]);
	s16 = _mm_add_epi16 (t1, t2);
	s16 = _mm_slli_epi16 (s16, SH);
	d16 = _mm_sub_epi16 (t1, t2);
	d16 = _mm_slli_epi16 (d16, SH);
	t1 = _mm_load_si128((__m128i*)rows[2]);
	t2 = _mm_load_si128((__m128i*)rows[5]);
	s25 = _mm_add_epi16 (t1, t2);
	s25 = _mm_slli_epi16 (s25, SH);
	d25 = _mm_sub_epi16 (t1, t2);
	d25 = _mm_slli_epi16 (d25, SH);
	t1 = _mm_load_si128((__m128i*)rows[3]);
	t2 = _mm_load_si128((__m128i*)rows[4]);
	s34 = _mm_add_epi16 (t1, t2);
	s34 = _mm_slli_epi16 (s34, SH);
	d34 = _mm_sub_epi16 (t1, t2);
	d34 = _mm_slli_epi16 (d34, SH);

	c1 = _mm_load_si128 ((__m128i*)vec1);
	c3 = _mm_load_si128 ((__m128i*)vec3);
	c5 = _mm_load_si128 ((__m128i*)vec5);
	c7 = _mm_load_si128 ((__m128i*)vec7);

	//rows[i][1] = (C1*d07 + C3*d16 + C5*d25 + C7*d34) >> 14;
	t1 = _mm_mulhi_epi16 (c1, d07);
	t2 = _mm_mulhi_epi16 (c3, d16);
	t3 = _mm_mulhi_epi16 (c5, d25);
	t4 = _mm_mulhi_epi16 (c7, d34);
	t1 = _mm_add_epi16 (t1, t2);
	t3 = _mm_add_epi16 (t3, t4);
	t1 = _mm_add_epi16 (t1, t3);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[1], t1);

	//rows[i][3] = (C3*d07 - C7*d16 - C1*d25 - C5*d34) >> 14;
	t1 = _mm_mulhi_epi16 (c3, d07);
	t2 = _mm_mulhi_epi16 (c7, d16);
	t3 = _mm_mulhi_epi16 (c1, d25);
	t4 = _mm_mulhi_epi16 (c5, d34);
	t1 = _mm_sub_epi16 (t1, t2);
	t3 = _mm_add_epi16 (t3, t4);
	t1 = _mm_sub_epi16 (t1, t3);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[3], t1);

	//rows[i][5] = (C5*d07 - C1*d16 + C7*d25 + C3*d34) >> 14;
	t1 = _mm_mulhi_epi16 (c5, d07);
	t2 = _mm_mulhi_epi16 (c1, d16);
	t3 = _mm_mulhi_epi16 (c7, d25);
	t4 = _mm_mulhi_epi16 (c3, d34);
	t1 = _mm_sub_epi16 (t1, t2);
	t3 = _mm_add_epi16 (t3, t4);
	t1 = _mm_add_epi16 (t1, t3);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[5], t1);

	//rows[i][7] = (C7*d07 - C5*d16 + C3*d25 - C1*d34) >> 14;
	t1 = _mm_mulhi_epi16 (c7, d07);
	t2 = _mm_mulhi_epi16 (c5, d16);
	t3 = _mm_mulhi_epi16 (c3, d25);
	t4 = _mm_mulhi_epi16 (c1, d34);
	t1 = _mm_sub_epi16 (t1, t2);
	t3 = _mm_sub_epi16 (t3, t4);
	t1 = _mm_add_epi16 (t1, t3);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[7], t1);

	s0734 = _mm_add_epi16 (s07, s34);
	d0734 = _mm_sub_epi16 (s07, s34);
	s1625 = _mm_add_epi16 (s16, s25);
	d1625 = _mm_sub_epi16 (s16, s25);

	c4 = _mm_load_si128 ((__m128i*)vec4);

	//rows[i][0] = (C4*(s0734 + s1625)) >> 14;
	t1 = _mm_add_epi16 (s0734, s1625);
	t1 = _mm_mulhi_epi16 (t1, c4);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[0], t1);

	//rows[i][4] = (C4*(s0734 - s1625)) >> 14;
	t1 = _mm_sub_epi16 (s0734, s1625);
	t1 = _mm_mulhi_epi16 (t1, c4);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[4], t1);

	c2 = _mm_load_si128 ((__m128i*)vec2);
	c6 = _mm_load_si128 ((__m128i*)vec6);

	//rows[i][2] = (C2*d0734 + C6*d1625) >> 14;
	t1 = _mm_mulhi_epi16 (d0734, c2);
	t2 = _mm_mulhi_epi16 (d1625, c6);
	t1 = _mm_add_epi16 (t1, t2);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[2], t1);

	//rows[i][6] = (C6*d0734 - C2*d1625) >> 14;
	t1 = _mm_mulhi_epi16 (d0734, c6);
	t2 = _mm_mulhi_epi16 (d1625, c2);
	t1 = _mm_sub_epi16 (t1, t2);
	t1 = _mm_srai_epi16 (t1, SH);
	_mm_store_si128 ((__m128i*)data[6], t1);
#	undef SH
}
