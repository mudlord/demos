/*
 * Just the class to load Quake 2 .MD2 and PCX files in OpenGL/GLUT.
 *
 *
 * Needs A LOT OF refactoring and cleaning! 
 * 
 *
 */

#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>



class Model_MD2 
{
  public: 
    void LoadPCX(char* textureFilename);
    int Load(char *filename, char* textureFilename);
	void Draw();
	void Play(int animation);

	void Stand();
	void Run();
	void Attack();
	void Pain1();
	void Pain2();
	void Pain3();
	void Jump();
	void Flip();
	void Salute(); // 10
	void Taunt(); // 16
	void Wave(); // 10
	void Point(); //11
	void Crstnd(); // 18 
	void Crwalk(); // 5
	void Crattack(); // 8
	void Crpain(); // 3
	void Crdeath(); // 4
	void Death1(); // 5
	void Death2(); // 5
	void Death3(); // 7

	Model_MD2();
    
	float Points[1000000];
    float Faces_Triangles[512][14096];
	float Normals[1000000];
    float Faces_Textures[512][14096];

	float TextureCoords[120048];

	int TotalConnectedPoints;
	int TotalConnectedTriangles;	

	int Scale;
	int AngleX, AngleY, AngleZ;

  private:

	typedef struct _MD2_Header
	{
		int ident;        // identifies as quake II header  (IDP2)
		int version;      // mine is 8
		int skinwidth;    // width of texture
		int skinheight;   // height of texture
		int framesize;    // number of bytes per frame
		int numSkins;     // number of textures
		int numXYZ;       // number of points
		int numST;        // number of texture
		int numTris;      // number of triangles
		int numGLcmds;
		int numFrames;    // total number of frames
		int offsetSkins;  // offset to skin names (64 bytes each)
		int offsetST;     // offset of texture s-t values
		int offsetTris;   // offset of triangle mesh
		int offsetFrames; // offset of frame data (points)
		int offsetGLcmds;
		int offsetEnd;    // end of file
	} MD2_Header;
		
	typedef struct _framePoint
	{
		 unsigned char v[3];             // the vertex
		 unsigned char lightNormalIndex;
	} framePoint;

	typedef struct _frame
	{
	 float scale[3];                 // vetex scaling
	 float translate[3];             // vertex translation
	 char name[16];                  // name of this model
	 framePoint fp[1];               // start of a list of framePoints
	} frame;

	typedef struct _mesh
  {
     unsigned short meshIndex[3];     // indices to triangle vertices
     unsigned short stIndex[3];       // indices to texture coordinates
  } mesh;

	int framenr;
	int updatecounter;
	frame *frm;

	int frame_length[3];
	int frame_start[3];
	int animation;

	int texWidth;
	int texHeight;
	int imgWidth;
	int imgHeight;
	unsigned char* texture;			
	GLuint	texturen[1];
};


typedef struct _pcxHeader
{
   short id[2];
   short offset[2];
   short size[2];
} pcxHeader;

struct Mesh_UV
 {
  unsigned short s;
  unsigned short t;
 };


float* calculateNormal( float *coord1, float *coord2, float *coord3 )
{
   /* calculate Vector1 and Vector2 */
   float va[3], vb[3], vr[3], val;
   va[0] = coord1[0] - coord2[0];
   va[1] = coord1[1] - coord2[1];
   va[2] = coord1[2] - coord2[2];

   vb[0] = coord1[0] - coord3[0];
   vb[1] = coord1[1] - coord3[1];
   vb[2] = coord1[2] - coord3[2];

   /* cross product */
   vr[0] = va[1] * vb[2] - vb[1] * va[2];
   vr[1] = vb[0] * va[2] - va[0] * vb[2];
   vr[2] = va[0] * vb[1] - vb[0] * va[1];

   /* normalization factor */
   val = sqrt( vr[0]*vr[0] + vr[1]*vr[1] + vr[2]*vr[2] );

 //  glNormal3f( vr[0]/val, vr[1]/val, vr[2]/val );
	float norm[3];
	norm[0] = vr[0]/val;
	norm[1] = vr[1]/val;
	norm[2] = vr[2]/val;


	return norm;
}


Model_MD2::Model_MD2()
{
	this->Scale = 1;
	framenr = 0;
	updatecounter = 0;

	frame_length[0] = 39;	// stand
	frame_length[1] = 5;	// run
	frame_length[2] = 7;	// attack
	frame_length[3] = 3;	//	"pain1" 
	frame_length[4] = 3;	//	"pain2" 3
	frame_length[5]	= 3;	// "pain3" 3
	frame_length[6] = 5;	// "jump" 5
	frame_length[7] = 11;	// "flip" 11
	frame_length[8] = 10;	// "salute" 10
	frame_length[9] = 16;	// "taunt" 16
	frame_length[10] = 10;	// "wave" 10


	frame_start[0] = 0;		// stand
	frame_start[1] = 39;	// run
	frame_start[2] = 46;	// shoot
	frame_start[3] = 50;	// pain1
	frame_start[4] = 39 + 5+ 7 + 3;
	frame_start[5] = frame_length[0] + frame_length[1] + frame_length[2] + frame_length[3] + frame_length[4];


	animation = 0;
}



void Model_MD2::LoadPCX(char* textureFilename)
{
	// Load texture
	FILE* texFile = fopen(textureFilename,"rb");
	
	if (texFile)
	{
      int imgWidth, imgHeight, texFileLen, imgBufferPtr, i;
               pcxHeader *pcxPtr;
               unsigned char *imgBuffer, *texBuffer, *pcxBufferPtr, *paletteBuffer;

               /* find length of file */
               fseek( texFile, 0, SEEK_END );
               texFileLen = ftell( texFile );
               fseek( texFile, 0, SEEK_SET );

               /* read in file */
               texBuffer = (unsigned char*) malloc( texFileLen+1 );
               fread( texBuffer, sizeof( char ), texFileLen, texFile );

               /* get the image dimensions */
               pcxPtr = (pcxHeader *)texBuffer;
               imgWidth = pcxPtr->size[0] - pcxPtr->offset[0] + 1;
               imgHeight = pcxPtr->size[1] - pcxPtr->offset[1] + 1;

			   this->imgWidth = imgWidth;
			   this->imgHeight = imgHeight;

               /* image starts at 128 from the beginning of the buffer */
               imgBuffer = (unsigned char*) malloc( imgWidth * imgHeight );
               imgBufferPtr = 0;
               pcxBufferPtr = &texBuffer[128];
               /* decode the pcx image */
               while( imgBufferPtr < (imgWidth * imgHeight) )
               {
                  if( *pcxBufferPtr > 0xbf )
                  {
                     int repeat = *pcxBufferPtr++ & 0x3f;
                     for( i=0; i<repeat; i++ )
                        imgBuffer[imgBufferPtr++] = *pcxBufferPtr;
                  } else {
                     imgBuffer[imgBufferPtr++] = *pcxBufferPtr;
                  }
                  pcxBufferPtr++;
               }
               /* read in the image palette */
               paletteBuffer = (unsigned char*) malloc( 768 );
               for( i=0; i<768; i++ )
                  paletteBuffer[i] = texBuffer[ texFileLen-768+i ];

               /* find the nearest greater power of 2 for each dimension */
               {
                  int imageWidth = imgWidth, imageHeight = imgHeight;
                  i = 0;
                  while( imageWidth )
                  {
                     imageWidth /= 2;
                     i++;
                  }
                  this->texWidth = pow( 2, (double) i );
                  i = 0;
                  while( imageHeight )
                  {
                     imageHeight /= 2;
                     i++;
                  }
                  this->texHeight = pow( 2, (double) i );
               }
               /* now create the OpenGL texture */
               {
                  int i, j;
                  this->texture = (unsigned char*) malloc( this->texWidth * this->texHeight * 3 );
                  for (j = 0; j < imgHeight; j++)
                  {
                     for (i = 0; i < imgWidth; i++)
                     {
                        this->texture[3*(j * this->texWidth + i)+0]
                              = paletteBuffer[ 3*imgBuffer[j*imgWidth+i]+0 ];
                        this->texture[3*(j * this->texWidth + i)+1]
                              = paletteBuffer[ 3*imgBuffer[j*imgWidth+i]+1 ];
                        this->texture[3*(j * this->texWidth + i)+2]
                              = paletteBuffer[ 3*imgBuffer[j*imgWidth+i]+2 ];
                     }
                  }
               }

   
       /* cleanup */
       free( paletteBuffer );
       free( imgBuffer );




	    glGenTextures( 1, &texturen[0] );
	    glBindTexture( GL_TEXTURE_2D, texturen[0] );

	    /* Generate The Texture */
	    glTexImage2D( GL_TEXTURE_2D, 0, 3, this->texWidth,
			  this->texHeight, 0, GL_RGB,
			  GL_UNSIGNED_BYTE, this->texture );

		printf(" %i %i ", this->texWidth, this->texHeight);
	    /* Linear Filtering */
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}

}

int Model_MD2::Load(char* filename, char* textureFilename)
{

	LoadPCX(textureFilename);
	// Load model
    MD2_Header *mdh;

	mesh* m;

	for (int i = 0; i < TotalConnectedTriangles; i++){  Faces_Triangles[0][i] = 0x00; }
	this->AngleX = 0;
	this->AngleY = 0;
	this->AngleZ = 0;
	this->Scale = 1;
    this->TotalConnectedTriangles = 0; 

    char* pch = strstr(filename,".md2");
     
    if (pch != NULL)
    {
	   FILE* file = fopen(filename,"r");
    
       if (file)
       {

		   // get size of file
           fseek( file, 0, SEEK_END );
		   size_t  fileSize = ftell( file );
		   fseek( file, 0, SEEK_SET );

		   // read in entire file
		   char* buffer;
 		   buffer = (char*) malloc( fileSize+1 );
		   fread( buffer, sizeof( char ), fileSize, file );


		   // start analyzing the buffer
	       mdh = (MD2_Header *)buffer;

		   printf("mdh framesize %i \n", mdh->framesize);

		   for (int z = 0; z< mdh->numFrames; z++)
		   {
			   frm = (frame *)&buffer[ mdh->offsetFrames + z*mdh->framesize  ];
			  m = (mesh *)&buffer[mdh->offsetTris];
			  int ti = 0;
			  int point_index = 0;
	
			  /* offset to points of frame */
			  for(int i=0; i<mdh->numXYZ; i++ )
			  {
				this->Points[point_index]   = frm->scale[0] * frm->fp[i].v[0] + frm->translate[0]; // X
				this->Points[point_index+1] = frm->scale[1] * frm->fp[i].v[1] + frm->translate[1]; // Y
				this->Points[point_index+2] = frm->scale[2] * frm->fp[i].v[2] + frm->translate[2]; // Z
				point_index += 3;
			  }
			  this->TotalConnectedTriangles = mdh->numTris * 3;
			  int n = 0;
		
			  //-------------------------------------------------------------
			  //-- create texture coordinate list ---------------------------

			  ti = 0;
				  		
			  for(int i=0; i<mdh->numST; i++ )
			  {
				 Mesh_UV* mUV = (Mesh_UV *)&buffer[mdh->offsetST + i*4 ];
			
				  this->TextureCoords[ti] = (float) mUV->s / this->texWidth;	
				  this->TextureCoords[ti+1] = (float) mUV->t / this->texHeight;	
				// printf(" (%i %i) ", mUV->s, mUV->t);
				// printf(" %f %f \n ", this->TextureCoords[ti], this->TextureCoords[ti+1]);
				ti+=2;
			  }


			  //---
			  m = (mesh *)&buffer[mdh->offsetTris];

			  n = 0;
			  ti = 0;


			  for(int i=0; i<mdh->numTris; i++ )
			  {

			
				 this->Faces_Triangles[z][n] = Points[ 3*m[i].meshIndex[0] ];
				 this->Faces_Triangles[z][n+1] = Points[ 3*m[i].meshIndex[0]+1 ];
				 this->Faces_Triangles[z][n+2] = Points[ 3*m[i].meshIndex[0]+2 ];		
						 
				 this->Faces_Textures[z][ti] = this->TextureCoords[ 2*m[i].stIndex[0] ];
				 this->Faces_Textures[z][ti+1] = this->TextureCoords[ 2*m[i].stIndex[0]+1 ];

  				n+=3;
				ti += 2;

				 this->Faces_Triangles[z][n] = Points[ 3*m[i].meshIndex[1] ];
				 this->Faces_Triangles[z][n+1] = Points[3* m[i].meshIndex[1]+1 ];
				 this->Faces_Triangles[z][n+2] = Points[3* m[i].meshIndex[1]+2 ];

				 this->Faces_Textures[z][ti] = TextureCoords[ 2*m[i].stIndex[1] ];
				 this->Faces_Textures[z][ti+1] = TextureCoords[ 2*m[i].stIndex[1] + 1 ];

				 n+=3;
				ti += 2;
	
				 this->Faces_Triangles[z][n] = Points[3* m[i].meshIndex[2] ];
				 this->Faces_Triangles[z][n+1] = Points[3* m[i].meshIndex[2]+1 ];
				 this->Faces_Triangles[z][n+2] = Points[3* m[i].meshIndex[2]+2 ];

				 this->Faces_Textures[z][ti] = TextureCoords[ 2*m[i].stIndex[2] ];
				 this->Faces_Textures[z][ti+1] = TextureCoords[ 2*m[i].stIndex[2] + 1 ];

				 n+=3;
				ti += 2;
			


	//			 m = (mesh *)&buffer[mdh->offsetTris + 12*i ];
			  }
		   }
		   //mdh->offsetST

		   printf("\n");

	  mdh = (MD2_Header *)buffer;

	for (int i = 0; i< 10; i+=2)
	{
		printf(" %f %f \n", this->Faces_Textures[0][i], this->Faces_Textures[0][i+1]);
	}

		int normal_index = 0;
	for (int triangle_index = 0; triangle_index < TotalConnectedTriangles*9; triangle_index += 9)
	{
		float coord1[3] = { Faces_Triangles[0][triangle_index], Faces_Triangles[0][triangle_index+1],Faces_Triangles[0][triangle_index+2]};
		float coord2[3] = {Faces_Triangles[0][triangle_index+3],Faces_Triangles[0][triangle_index+4],Faces_Triangles[0][triangle_index+5]};
		float coord3[3] = {Faces_Triangles[0][triangle_index+6],Faces_Triangles[0][triangle_index+7],Faces_Triangles[0][triangle_index+8]};
		float *norm = calculateNormal( coord1, coord2, coord3 );

		printf("\nnormal %f %f %f \n", norm[0],norm[1],norm[2]);

		Normals[normal_index] = norm[0];
		Normals[normal_index+1] = norm[1];
		Normals[normal_index+2] = norm[2];

		Normals[normal_index+3] = norm[0];
		Normals[normal_index+4] = norm[1];
		Normals[normal_index+5] = norm[2];

		Normals[normal_index+6] = norm[0];
		Normals[normal_index+7] = norm[1];
		Normals[normal_index+8] = norm[2];

		normal_index += 9;
	}
	

	   }
      else { printf("File can't be opened\n"); }
    } else {
      printf("File does not have a .MD2 extension. ");    
    }   

	return 0;
}

void Model_MD2::Play(int animation)
{
	updatecounter++;
	if (updatecounter > 10)
	{
		//if (framenr == frame_length[animation]) 
		//	framenr = 0;
		//else
		//{
			//if (framenr < frame_length[animation])	framenr++;
		//}
		framenr++;
		printf("%i \n", framenr);
		//framenr++;
		updatecounter = 0;
	}

	this->animation = animation;
}


void Model_MD2::Crstnd()
{
	updatecounter++;
	if (updatecounter > 10)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 147) framenr = 147;
		if (framenr < 147+5) framenr++; else framenr = 147;
		
		updatecounter = 0;
	}
}

void Model_MD2::Point()
{
	updatecounter++;
	if (updatecounter > 20)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 121) framenr = 121;
		if (framenr < 121+11) framenr++; else framenr = 121;
		
		updatecounter = 0;
	}
}

void Model_MD2::Wave()
{
	updatecounter++;
	if (updatecounter > 20)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 111) framenr = 111;
		if (framenr < 111+10) framenr++; else framenr = 111;
		
		updatecounter = 0;
	}
}

void Model_MD2::Taunt()
{
	updatecounter++;
	if (updatecounter > 20)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 91) framenr = 91;
		if (framenr < 91+20) framenr++; else framenr = 91;
		
		updatecounter = 0;
	}
}

void Model_MD2::Salute()
{
	updatecounter++;
	if (updatecounter > 20)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 81) framenr = 81;
		if (framenr < 81+10) framenr++; else framenr = 81;
		
		updatecounter = 0;
	}
}

void Model_MD2::Flip()
{
	updatecounter++;
	if (updatecounter > 20)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 69) framenr = 69;
		if (framenr < 69+11) framenr++; else framenr = 69;
		
		updatecounter = 0;
	}
}

void Model_MD2::Jump()
{
	updatecounter++;
	if (updatecounter > 20)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 64) framenr = 64;
		if (framenr < 64+5) framenr++; else framenr = 64;
		
		updatecounter = 0;
	}
}

void Model_MD2::Pain3()
{
	updatecounter++;
	if (updatecounter > 20)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 59) framenr = 59;
		if (framenr < 59+3) framenr++; else framenr = 59;
		
		updatecounter = 0;
	}
}

void Model_MD2::Pain2()
{
	updatecounter++;
	if (updatecounter > 20)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 56) framenr = 56;
		if (framenr < 56+3) framenr++; else framenr = 56;
		
		updatecounter = 0;
	}

}

void Model_MD2::Pain1()
{
	updatecounter++;
	if (updatecounter > 20)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 53) framenr = 53;
		if (framenr < 53+3) framenr++; else framenr = 53;
		
		updatecounter = 0;
	}

	this->animation = 1;
}

void Model_MD2::Attack()
{
	updatecounter++;
	if (updatecounter > 20)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 46) framenr = 46;
		if (framenr < 46+7) framenr++; else framenr = 46;
		
		updatecounter = 0;
	}

	this->animation = 1;
}

void Model_MD2::Run()
{
	updatecounter++;
	if (updatecounter > 1)
	{
		//if (framenr == (framenr+frame_length[animation]) ) framenr = 0; else framenr++;
		if (framenr < 40) framenr = 40;
		if (framenr < 40+5) framenr++; else framenr = 40;
		
		updatecounter = 0;
	}

	this->animation = 1;

}


void Model_MD2::Stand()
{
	updatecounter++;
	if (updatecounter > 1)
	{
		if (framenr > 28) framenr = 0; else framenr++;
		updatecounter = 0;
	}

	this->animation = 0;

}


void Model_MD2::Draw()
{
	/*
	glRasterPos2i(0,0);
	glDrawPixels(this->texWidth , this->texHeight, GL_RGB, GL_UNSIGNED_BYTE, this->texture);
*/



    glEnable(GL_TEXTURE_2D);	
	glBindTexture(GL_TEXTURE_2D, texturen[0]);

	
 	glEnableClientState(GL_VERTEX_ARRAY);	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, Normals);

	glTexCoordPointer(2,GL_FLOAT,0, this->Faces_Textures[ framenr] );
	glVertexPointer(3,GL_FLOAT,	0,Faces_Triangles[ framenr ]);	 
	glDrawArrays(GL_TRIANGLES, 0, TotalConnectedTriangles);	

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisable(GL_TEXTURE_2D);
}