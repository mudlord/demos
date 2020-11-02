#include "sys/msys.h"
#include "intro.h"
#include "jpegenc.h"
#include "dct.h"
#include "arch.h"

typedef unsigned char color;

typedef struct {
	color Red;
	color Green;
	color Blue;
	color Alpha;
} RGBA;

typedef struct {
	color Blue;
	color Green;
	color Red;
	color Alpha;
} BGRA;

typedef struct {
	color Blue;
	color Green;
	color Red;
} BGR;

typedef struct {
	unsigned short int Blue:5;
	unsigned short int Green:5;
	unsigned short int Red:5;
	unsigned short int Reserved:1;
} BGR16;

// RGB <-> YCbCr Conversion:

#define CHECK(n) (((n) < 0)? 0: (((n) > 255)? 255: (n)))

inline color YCbCr2R(const conv y, const conv cb, const conv cr)
{
	conv ret = y + 1.402*(cr-128);

#ifdef CHK
	if (ret > 255 || ret < 0)
		printf("Error YCbCr2R %d YCbCr=%d,%d,%d\n", ret, y, cb, cr);
#endif

	return CHECK(ret);
}

inline color YCbCr2G(const conv y, const conv cb, const conv cr)
{
	conv ret = y - 0.34414*(cb-128) - 0.71414*(cr-128);

#ifdef CHK
	if (ret > 255 || ret < 0)
		printf("Error YCbCr2G %d YCbCr=%d,%d,%d\n", ret, y, cb, cr);
#endif

	return CHECK(ret);
}

inline color YCbCr2B(const conv y, const conv cb, const conv cr)
{
	conv ret = y + 1.772*(cb-128);

#ifdef CHK
	if (ret > 255 || ret < 0)
		printf("Error YCbCr2B %d YCbCr=%d,%d,%d\n", ret, y, cb, cr);
#endif

	return CHECK(ret);
}

void correct_color(conv data[8][8])
{
	unsigned r, c;

	for (r = 0; r < 8; r++)
		for (c = 0; c < 8; c++)
		{
			conv p = data[r][c];

			if (p < 0) p = 0;
			if (p > 255) p = 255;

			data[r][c] = p;
		}
}

extern "C" {
	extern const unsigned char qtable_paint_lum[8][8];
	extern const unsigned char qtable_paint_chrom[8][8];
}

void subsample2(const color R[16][16], const color G[16][16], const color B[16][16],
	conv Y[2][2][8][8], conv cb[8][8], conv cr[8][8])
{
	SIMD_ALIGN static const unsigned short vecRY[8] = {19595, 19595, 19595, 19595, 19595, 19595, 19595, 19595};
	SIMD_ALIGN static const unsigned short vecGY[8] = {38470, 38470, 38470, 38470, 38470, 38470, 38470, 38470};
	SIMD_ALIGN static const unsigned short vecBY[8] = { 7471,  7471,  7471,  7471,  7471,  7471,  7471,  7471};
	SIMD_ALIGN static const unsigned short vecKY[8] = {  128,   128,   128,   128,   128,   128,   128,   128};
	SIMD_ALIGN static const short vecRCb[8] = {-11058,-11058,-11058,-11058,-11058,-11058,-11058,-11058};
	SIMD_ALIGN static const short vecGCb[8] = {-21709,-21709,-21709,-21709,-21709,-21709,-21709,-21709};
	SIMD_ALIGN static const short vecBCb[8] = { 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767};
	SIMD_ALIGN static const short vecRCr[8] = { 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767};
	SIMD_ALIGN static const short vecGCr[8] = {-27438,-27438,-27438,-27438,-27438,-27438,-27438,-27438};
	SIMD_ALIGN static const short vecBCr[8] = { -5329, -5329, -5329, -5329, -5329, -5329, -5329, -5329};

	__m128i ry = _mm_load_si128 ((__m128i*)vecRY);
	__m128i gy = _mm_load_si128 ((__m128i*)vecGY);
	__m128i by = _mm_load_si128 ((__m128i*)vecBY);
	__m128i ky = _mm_load_si128 ((__m128i*)vecKY);
	__m128i rcb = _mm_load_si128 ((__m128i*)vecRCb);
	__m128i gcb = _mm_load_si128 ((__m128i*)vecGCb);
	__m128i bcb = _mm_load_si128 ((__m128i*)vecBCb);
	__m128i rcr = _mm_load_si128 ((__m128i*)vecRCr);
	__m128i gcr = _mm_load_si128 ((__m128i*)vecGCr);
	__m128i bcr = _mm_load_si128 ((__m128i*)vecBCr);
	__m128i r0 = _mm_setzero_si128();
	__m128i r,g,b, r1,r2, g1,g2, b1,b2, y1,y2, cb1,cb2, cr1,cr2, t1,t2;
	unsigned i;

	for (i = 0; i < 16; i += 2)
	{
		
		unsigned l = i >> 3;
		unsigned k = i & 7;

		// Y - 1
		r = _mm_load_si128 ((__m128i*)R[i]);
		r1 = _mm_unpacklo_epi8 (r, r0);
		r2 = _mm_unpackhi_epi8 (r, r0);
		y1 = _mm_mulhi_epu16 (r1, ry);
		y2 = _mm_mulhi_epu16 (r2, ry);

		g = _mm_load_si128 ((__m128i*)G[i]);
		g1 = _mm_unpacklo_epi8 (g, r0);
		g2 = _mm_unpackhi_epi8 (g, r0);
		t1 = _mm_mulhi_epu16 (g1, gy);
		t2 = _mm_mulhi_epu16 (g2, gy);
		y1 = _mm_adds_epu16 (y1, t1);
		y2 = _mm_adds_epu16 (y2, t2);

		b = _mm_load_si128 ((__m128i*)B[i]);
		b1 = _mm_unpacklo_epi8 (b, r0);
		b2 = _mm_unpackhi_epi8 (b, r0);
		t1 = _mm_mulhi_epu16 (b1, by);
		t2 = _mm_mulhi_epu16 (b2, by);
		y1 = _mm_adds_epu16 (y1, t1);
		y2 = _mm_adds_epu16 (y2, t2);

		y1 = _mm_sub_epi16 (y1, ky);
		y2 = _mm_sub_epi16 (y2, ky);

		_mm_store_si128 ((__m128i*)Y[l][0][k], y1);
		_mm_store_si128 ((__m128i*)Y[l][1][k], y2);

		// Cb - 1
		cb1 = _mm_madd_epi16 (r1, rcb);
		cb2 = _mm_madd_epi16 (r2, rcb);

		t1 = _mm_madd_epi16 (g1, gcb);
		t2 = _mm_madd_epi16 (g2, gcb);
		cb1 = _mm_add_epi32 (cb1, t1);
		cb2 = _mm_add_epi32 (cb2, t2);

		t1 = _mm_madd_epi16 (b1, bcb);
		t2 = _mm_madd_epi16 (b2, bcb);
		cb1 = _mm_add_epi32 (cb1, t1);
		cb2 = _mm_add_epi32 (cb2, t2);

		// Cr - 1
		cr1 = _mm_madd_epi16 (r1, rcr);
		cr2 = _mm_madd_epi16 (r2, rcr);

		t1 = _mm_madd_epi16 (g1, gcr);
		t2 = _mm_madd_epi16 (g2, gcr);
		cr1 = _mm_add_epi32 (cr1, t1);
		cr2 = _mm_add_epi32 (cr2, t2);

		t1 = _mm_madd_epi16 (b1, bcr);
		t2 = _mm_madd_epi16 (b2, bcr);
		cr1 = _mm_add_epi32 (cr1, t1);
		cr2 = _mm_add_epi32 (cr2, t2);

		// Y - 2
		r = _mm_load_si128 ((__m128i*)R[i+1]);
		r1 = _mm_unpacklo_epi8 (r, r0);
		r2 = _mm_unpackhi_epi8 (r, r0);
		y1 = _mm_mulhi_epu16 (r1, ry);
		y2 = _mm_mulhi_epu16 (r2, ry);

		g = _mm_load_si128 ((__m128i*)G[i+1]);
		g1 = _mm_unpacklo_epi8 (g, r0);
		g2 = _mm_unpackhi_epi8 (g, r0);
		t1 = _mm_mulhi_epu16 (g1, gy);
		t2 = _mm_mulhi_epu16 (g2, gy);
		y1 = _mm_adds_epu16 (y1, t1);
		y2 = _mm_adds_epu16 (y2, t2);

		b = _mm_load_si128 ((__m128i*)B[i+1]);
		b1 = _mm_unpacklo_epi8 (b, r0);
		b2 = _mm_unpackhi_epi8 (b, r0);
		t1 = _mm_mulhi_epu16 (b1, by);
		t2 = _mm_mulhi_epu16 (b2, by);
		y1 = _mm_adds_epu16 (y1, t1);
		y2 = _mm_adds_epu16 (y2, t2);

		y1 = _mm_sub_epi16 (y1, ky);
		y2 = _mm_sub_epi16 (y2, ky);

		_mm_store_si128 ((__m128i*)Y[l][0][k+1], y1);
		_mm_store_si128 ((__m128i*)Y[l][1][k+1], y2);

		// Cb - 2
		t1 = _mm_madd_epi16 (r1, rcb);
		t2 = _mm_madd_epi16 (r2, rcb);
		cb1 = _mm_add_epi32 (cb1, t1);
		cb2 = _mm_add_epi32 (cb2, t2);

		t1 = _mm_madd_epi16 (g1, gcb);
		t2 = _mm_madd_epi16 (g2, gcb);
		cb1 = _mm_add_epi32 (cb1, t1);
		cb2 = _mm_add_epi32 (cb2, t2);

		t1 = _mm_madd_epi16 (b1, bcb);
		t2 = _mm_madd_epi16 (b2, bcb);
		cb1 = _mm_add_epi32 (cb1, t1);
		cb2 = _mm_add_epi32 (cb2, t2);

		cb1 = _mm_srai_epi32 (cb1, 18);
		cb2 = _mm_srai_epi32 (cb2, 18);
		cb1 = _mm_packs_epi32 (cb1, cb2);
		_mm_store_si128 ((__m128i*)cb[i>>1], cb1);

		// Cr - 2
		t1 = _mm_madd_epi16 (r1, rcr);
		t2 = _mm_madd_epi16 (r2, rcr);
		cr1 = _mm_add_epi32 (cr1, t1);
		cr2 = _mm_add_epi32 (cr2, t2);

		t1 = _mm_madd_epi16 (g1, gcr);
		t2 = _mm_madd_epi16 (g2, gcr);
		cr1 = _mm_add_epi32 (cr1, t1);
		cr2 = _mm_add_epi32 (cr2, t2);

		t1 = _mm_madd_epi16 (b1, bcr);
		t2 = _mm_madd_epi16 (b2, bcr);
		cr1 = _mm_add_epi32 (cr1, t1);
		cr2 = _mm_add_epi32 (cr2, t2);

		cr1 = _mm_srai_epi32 (cr1, 18);
		cr2 = _mm_srai_epi32 (cr2, 18);
		cr1 = _mm_packs_epi32 (cr1, cr2);
		_mm_store_si128 ((__m128i*)cr[i>>1], cr1);
	}
}



GLuint jpeg_shader,jpeg_texture,jpeg_vao; 
const int PBO_COUNT = 3;
GLuint jpeg_pbo[PBO_COUNT];           // IDs of PBOs
BGR output_data[XRES*YRES] = {0};

void init_jpeg()
{

		const char vertex_source [] =
			"#version 330\n"
			"layout(location = 0) in vec4 vposition;\n"
			"layout(location = 1) in vec2 vtexcoord;\n"
			"out vec2 ftexcoord;\n"
			"void main() {\n"
			"   ftexcoord = vtexcoord;\n"
			"   gl_Position =vposition;\n"
			"}\n";

		const char  fragment_source [] =
			"#version 330\n"
			"in vec2 ftexcoord;\n"
			"layout(location = 0) out vec4 FragColor;\n"
			"uniform sampler2D tex;\n"
			"uniform float time;\n"
			"uniform vec2 resolution;\n"
			"void main()"
			"{"
			"FragColor=texture2D(tex,ftexcoord);"
			"}";


		initShader(  (int*)&jpeg_shader, vertex_source, (const char*)fragment_source);

		// get texture uniform location
		

		// vao and vbo handle
		GLuint vbo;

		// generate and bind the vao
		glGenVertexArrays(1, &jpeg_vao);
		glBindVertexArray(jpeg_vao);

		// generate and bind the vertex buffer object
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		// data for a fullscreen quad (this time with texture coords)
		GLfloat vertexData[] = {
			//  X     Y     Z           U     V     
			1.0f, 1.0f, 0.0f,       1.0f, 1.0f, // vertex 0
			-1.0f, 1.0f, 0.0f,       0.0f, 1.0f, // vertex 1
			1.0f,-1.0f, 0.0f,       1.0f, 0.0f, // vertex 2
			1.0f,-1.0f, 0.0f,       1.0f, 0.0f, // vertex 2
			-1.0f,-1.0f, 0.0f,       0.0f, 0.0f, // vertex 3
			-1.0f, 1.0f, 0.0f,       0.0f, 1.0f, // vertex 1
		}; // 6 vertices with 5 components (floats) each

		// fill with data
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*6*5, vertexData, GL_STATIC_DRAW);
		// set up generic attrib pointers
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (char*)0 + 3*sizeof(GLfloat));
		// "unbind" voa
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenTextures( 1, &jpeg_texture );
		glBindTexture( GL_TEXTURE_2D,jpeg_texture );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, XRES, YRES, 0,
			GL_RGB, GL_UNSIGNED_BYTE,output_data );
		glBindTexture(GL_TEXTURE_2D,0);

		glGenBuffers(PBO_COUNT, jpeg_pbo);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, jpeg_pbo[0]);
		glBufferData(GL_PIXEL_PACK_BUFFER, XRES*YRES*3, 0, GL_STREAM_READ);
		glBindBuffer(GL_PIXEL_PACK_BUFFER,jpeg_pbo[1]);
		glBufferData(GL_PIXEL_PACK_BUFFER, XRES*YRES*3, 0, GL_STREAM_READ);
		glBindBuffer(GL_PIXEL_PACK_BUFFER,jpeg_pbo[2]);
		glBufferData(GL_PIXEL_PACK_BUFFER, XRES*YRES*3, 0, GL_STREAM_READ);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);



}




bool GetBlock(unsigned x, unsigned y, unsigned sx, unsigned sy, BGR* buf, BGR* src,int width,int height)
{
	if ((y + sy) > height || (x + sx) > width) {
		return false;
	}

	for (unsigned r = 0; r < sy; r++) {
		unsigned offset = (height - (y+r+1))*width + x;

		for (unsigned c = 0; c < sx; c++) {
			unsigned i = offset + c;
			buf[sy*r + c] = src[i];
		}
	}

	return true;
}

int bmpGetBlock16x16(BGR* src,int width,int height, unsigned x, unsigned y, color R[16][16], color G[16][16], color B[16][16])
{
	unsigned r;

	if ((y + 16) > height || (x + 16) > width) {
		return 0;
	}

	for (r = 0; r < 16; r++)
	{
		unsigned offset = (height - (y+r+1))*width + x;
		unsigned *ptr = (unsigned*)(src + offset);
		unsigned c, i;

		// optimization for BGR color format
		for (c = 0, i = 0; c < 16; c += 4, i += 3)
		{
			unsigned n1 = ptr[i+0];
			unsigned n2 = ptr[i+1];
			unsigned n3 = ptr[i+2];

			*(unsigned*)&B[r][c] = ((n1 >>  0) & 0xff) | ((n1 >> 16) & 0xff00) | ((n2 >>  0) & 0xff0000) | ((n3 << 16) & 0xff000000);
			*(unsigned*)&G[r][c] = ((n1 >>  8) & 0xff) | ((n2 <<  8) & 0xff00) | ((n2 >>  8) & 0xff0000) | ((n3 <<  8) & 0xff000000);
			*(unsigned*)&R[r][c] = ((n1 >> 16) & 0xff) | ((n2 >>  0) & 0xff00) | ((n3 << 16) & 0xff0000) | ((n3 >>  0) & 0xff000000);
		}
	}

	return 1;
}



bool SetBlock(unsigned x, unsigned y, unsigned sx, unsigned sy, BGR* buf,BGR *dst,int width,int height)
{
	if ((y + sy) > height || (x + sx) > width) {
		return false;
	}

	for (unsigned r = 0; r < sy; r++) {
		unsigned offset = (height - (y+r+1))*width + x;

		for (unsigned c = 0; c < sx; c++) {
			unsigned i = offset + c;
		    dst[i]= buf[sy*r + c];
		}
	}

	return true;
}


void jpeg_process(BGR* src, int width, int height, BGR* dst)
{
	CACHE_ALIGN BGR RGB16x16[16][16];
	CACHE_ALIGN conv Y8x8[2][2][8][8]; // four 8x8 blocks - 16x16
	CACHE_ALIGN conv Cb8x8[8][8];
	CACHE_ALIGN conv Cr8x8[8][8];
	CACHE_ALIGN color R[16][16], G[16][16], B[16][16];

	for (unsigned y = 0; y < height-15; y += 16) {
		for (unsigned x = 0; x < width-15; x += 16)
		{
			//GetBlock(x, y, 16, 16, (BGR*)RGB16x16,);
			bmpGetBlock16x16(src,width,height, x, y, R, G, B);
			subsample2(R, G, B, Y8x8, Cb8x8, Cr8x8);

			dct3_sse_4(Y8x8[0][0], Y8x8[0][0]);	// 1 Y-transform
			dct3_sse_4(Y8x8[0][1], Y8x8[0][1]);	// 2 Y-transform
			dct3_sse_4(Y8x8[1][0], Y8x8[1][0]);	// 3 Y-transform
			dct3_sse_4(Y8x8[1][1], Y8x8[1][1]);	// 4 Y-transform
			dct3_sse_4(Cb8x8, Cb8x8);				// Cb-transform
			dct3_sse_4(Cr8x8, Cr8x8);				// Cr-transform


			quantization_lum((conv*)Y8x8[0][0]);
			quantization_lum((conv*)Y8x8[0][1]);
			quantization_lum((conv*)Y8x8[1][0]);
			quantization_lum((conv*)Y8x8[1][1]);
			quantization_chrom((conv*)Cb8x8);
			quantization_chrom((conv*)Cr8x8);


			iquantization_lum((conv*)Y8x8[0][0]);
			idct3(Y8x8[0][0], Y8x8[0][0]);
			iquantization_lum((conv*)Y8x8[0][1]);
			idct3(Y8x8[0][1], Y8x8[0][1]);
			iquantization_lum((conv*)Y8x8[1][0]);
			idct3(Y8x8[1][0], Y8x8[1][0]);
			iquantization_lum((conv*)Y8x8[1][1]);
			idct3(Y8x8[1][1], Y8x8[1][1]);
			iquantization_chrom((conv*)Cb8x8);
			idct3(Cb8x8, Cb8x8);
			iquantization_chrom((conv*)Cr8x8);
			idct3(Cr8x8, Cr8x8);

			for (unsigned i = 0; i < 2; i++)
				for (unsigned j = 0; j < 2; j++)
				{
					for (unsigned r = 0; r < 8; r += 2)
						for (unsigned c = 0; c < 8; c += 2)
						{
							const unsigned rr = (i<<3) + r;
							const unsigned cc = (j<<3) + c;
							// convert pixels back into RGB
							const conv Cb = Cb8x8[rr>>1][cc>>1] + 128;
							const conv Cr = Cr8x8[rr>>1][cc>>1] + 128;
							conv Y;

							Y = Y8x8[i][j][r][c] + 128;
							RGB16x16[rr][cc].Red   = YCbCr2R(Y, Cb, Cr);
							RGB16x16[rr][cc].Green = YCbCr2G(Y, Cb, Cr);
							RGB16x16[rr][cc].Blue  = YCbCr2B(Y, Cb, Cr);

							Y = Y8x8[i][j][r][c+1] + 128;
							RGB16x16[rr][cc+1].Red   = YCbCr2R(Y, Cb, Cr);
							RGB16x16[rr][cc+1].Green = YCbCr2G(Y, Cb, Cr);
							RGB16x16[rr][cc+1].Blue  = YCbCr2B(Y, Cb, Cr);

							Y = Y8x8[i][j][r+1][c] + 128;
							RGB16x16[rr+1][cc].Red   = YCbCr2R(Y, Cb, Cr);
							RGB16x16[rr+1][cc].Green = YCbCr2G(Y, Cb, Cr);
							RGB16x16[rr+1][cc].Blue  = YCbCr2B(Y, Cb, Cr);

							Y = Y8x8[i][j][r+1][c+1] + 128;
							RGB16x16[rr+1][cc+1].Red   = YCbCr2R(Y, Cb, Cr);
							RGB16x16[rr+1][cc+1].Green = YCbCr2G(Y, Cb, Cr);
							RGB16x16[rr+1][cc+1].Blue  = YCbCr2B(Y, Cb, Cr);
						}
				}



			SetBlock(x, y, 16, 16, (BGR*)RGB16x16,dst,width,height);
			
		}
	}
}
void do_jpeg()
{
	static int shift = 0;
	// brightness shift amount
	shift = ++shift % 200;

	static int index = 0;
	int nextIndex = 0;                  // pbo index used for next frame
	index = (index + 1) % PBO_COUNT;
	nextIndex = (index + 1) % PBO_COUNT;




	glReadBuffer(GL_BACK);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, jpeg_pbo[index]);
	glReadPixels(0, 0, XRES, YRES, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, jpeg_pbo[nextIndex]);
	GLubyte* src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	if(src)
	{
		jpeg_process((BGR*)src,XRES,YRES,output_data);
		// change brightness
		//add(src, XRES, YRES, shift, (unsigned char*)input_data);
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);     // release pointer to the mapped buffer
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	

	glUseProgram(jpeg_shader);
	GLint texture_location = glGetUniformLocation(jpeg_shader, "tex");
	glm::mat4 Projection =  glm::mat4(1.0); 
	glm::mat4 ViewProjection = Projection;
	// set the uniform
	// bind texture to texture unit 0
	glActiveTexture(GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_2D, jpeg_texture);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, XRES, YRES, 0,
		GL_RGB, GL_UNSIGNED_BYTE,output_data );
	// set texture uniform
	glUniform1i(texture_location, 0);
	// bind the vao
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(jpeg_vao);
	// draw
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D,0);
	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_BLEND);




}