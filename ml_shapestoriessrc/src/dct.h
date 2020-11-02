#ifndef __DCT_H__
#define __DCT_H__

#ifdef __cplusplus
extern "C" {
#endif

void dct_fill_tab();

// integer DCT 
void dct(short pixel[8][8], short data[8][8]);
void dct2(short pixel[8][8], short data[8][8]);
void dct3(short pixel[8][8], short data[8][8]);
void dct4(short pixel[8][8], short data[8][8]);
void dct5(short pixel[8][8], short data[8][8]);
void dct2_i(short pixel[8][8], short data[8][8]);
void dct2_s(short pixel[8][8], short data[8][8]);
void dct3_sse(short pixel[8][8], short data[8][8]);
void dct3_sse_0(short pixel[8][8], short data[8][8]);
void dct3_sse_1(short pixel[8][8], short data[8][8]);
void dct3_sse_2(short pixel[8][8], short data[8][8]);
void dct3_sse_3(short pixel[8][8], short data[8][8]);
void dct3_sse_4(short pixel[8][8], short data[8][8]);
void dct3_sse_5(short pixel[8][8], short data[8][8]);
//void transpose4(short data[8][8]);

// inverse real DCT
void idct(short data[8][8], short pixel[8][8]);
void idct2(short data[8][8], short pixel[8][8]);
void idct3(short data[8][8], short pixel[8][8]);
// inverse integer DCT 
void idct2_i(short data[8][8], short pixel[8][8]);
void idct2_s(short data[8][8], short pixel[8][8]);

void test_idct2_s();

#ifdef __cplusplus
}
#endif

#endif//__DCT_H__
