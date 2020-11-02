#include "msys.h"

#ifndef PI
#define PI 3.141592653589794
#endif

// RGB struct (custom palette's component).
typedef struct _tagRGB {
	BYTE r, g, b;
}RGB;


//A class for 320x240 256 colour rendering
//Renders to a texture
class PixelBuffer  
{
public:
	GLuint texture;
	// constructor
	PixelBuffer ()
	{
		m_width=320;
		m_height=240;
		m_data=new unsigned char[m_width*m_height];
	    texture_data=new RGB[m_width*m_height];
		PixTab = (unsigned int *)malloc(sizeof(unsigned int) * 240);
		for (int i = 0; i < 240; i++)
		PixTab[i] = i * 320;
		// check its validity.
		ZeroMemory(m_data,m_width*m_height); 
		ZeroMemory(texture_data,m_width*m_height*3);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Upload the texture
		glTexImage2D(GL_TEXTURE_2D, 0, 3, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

	}
	// destructor
	virtual ~PixelBuffer ()
	{
		if (m_data)delete []m_data;
		if (texture_data)delete []texture_data;
		glDeleteTextures(1, &texture);
	}

	//set pallete
	void SetPallete(int n, int r, int g, int b)
	{
		pallete[n].r = r;
		pallete[n].g = g;
		pallete[n].b = b;
	}
	//Clear render buffer
	void Clear()
	{
		memset(m_data, 0, m_width*m_height);
		memset(texture_data,0,320*240*3);
	}

	//get pointer to memory
	unsigned char* GetVideoRAM()
	{

		return m_data;
	}

	void PutPixel(int x, int y, unsigned char col)
	{
		m_data[PixTab[y] + x] = col;
	}

	unsigned char GetPixel(int x, int y)
	{
		return (m_data[PixTab[y] + x]);
	}

	unsigned int *GetPixTable(void)
	{
		//Return a pointer to a Y offset table

		return (PixTab);
	}

	// render pixel buffer to texture
	void RenderBuffer()
	{
	    for (int x = 0; x < m_width; ++x){
		 for (int y = 0; y < m_height; ++y){
			BYTE col = m_data[y * m_width + x ];	
            //transparency
			texture_data[y * m_width + x ] = pallete[col];
			}
		}
		glBindTexture(GL_TEXTURE_2D,texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE,texture_data);
		glBindTexture(GL_TEXTURE_2D,0);
	}
protected:
	int  m_width, m_height;
	unsigned char *m_data;
	unsigned int *PixTab;				
	RGB *texture_data;
	RGB pallete[256];
	
};

#define IMG_OK 0x1
#define IMG_ERR_NO_FILE 0x2
#define IMG_ERR_MEM_FAIL 0x4
#define IMG_ERR_BAD_FORMAT_ID 0x8
#define IMG_ERR_BAD_FORMAT_BPP 0x10
#define IMG_ERR_BAD_FORMAT_DIM 0x20
#define IMG_ERR_BAD_FORMAT_PAL 0x40

#define IMG_ERR_UNSUPPORTED 0x80

class PCXTexture
{
private:
	short int iWidth,iHeight,iBPP,iPlanes,iBPL;
	long lImageSize;
	char bEnc;
	unsigned char *pImage, *pData;
	RGB* pPalette;

public:

	PCXTexture(unsigned char* data, int sizeOfData)
	{
		//using namespace std;
		//ifstream fIn;
		unsigned long ulSize = sizeOfData;
		int iRet;

		// Clear out any existing image and palette
		if(pImage)
		{
			pImage=NULL;
		}

		if(pPalette)
		{
			pPalette=NULL;
		}

		pData = data;
		iRet=ReadHeader();

		if(iRet!=IMG_OK)
			return ;
		iRet=LoadRLEData();
		iRet=LoadPalette(ulSize);
		pData=NULL;
		iBPP*=iPlanes;
	}
	~PCXTexture(void)
	{
		

	}//Destructor


	int ReadHeader()
	{
		unsigned short x1,x2,y1,y2;

		if(pData==NULL)
			return IMG_ERR_NO_FILE;

		if(pData[0]!=0xA) // PCX ID Byte, should be 0xA
			return IMG_ERR_BAD_FORMAT_ID;

		if(pData[1]>5) // Version, we don't know about anything after v5
			return IMG_ERR_UNSUPPORTED;

		bEnc=pData[2]; // Encode flag 1 = RLE Compression

		if(pData[3]==1 || pData[3]==2 || pData[3]==4 || pData[3]==8) // BPP value
			iBPP=pData[3];
		else
			return IMG_ERR_BAD_FORMAT_BPP;

		// Get image window and produce width & height values
		memcpy(&x1,&pData[4],2);
		memcpy(&y1,&pData[6],2);
		memcpy(&x2,&pData[8],2);
		memcpy(&y2,&pData[10],2);

		iWidth=(x2-x1)+1;
		iHeight=(y2-y1)+1;

		if(iWidth<1 || iHeight<1)
			return IMG_ERR_BAD_FORMAT_DIM;

		// Planes byte. 1 = Indexed, 3 = RGB
		iPlanes=pData[65];

		// Bits per line for decoding purposes,
		memcpy(&iBPL,&pData[66],2);

		return IMG_OK;
	}

	int LoadRLEData()
	{
		int iLineCount,iBufferLineLen,iImageLineLen;
		long lLinePos=0;
		unsigned char bRunLen;
		unsigned char *pCur,*pLine,*pInterLine;

		// Set our pointer to the beginning of the image data
		pCur=&pData[128];

		// Calculate line lengths for image and buffer, Allocate the buffer scan line
		iBufferLineLen=iBPL*iPlanes;
		iImageLineLen =iWidth*iPlanes;
		pLine=new unsigned char[iBufferLineLen];

		if(pLine==NULL)
			return IMG_ERR_MEM_FAIL;

		// Allocate space for the image data
		if(pImage!=NULL)
			delete [] pImage;

		pImage=new unsigned char[(iImageLineLen * iHeight)+1];

		if(pImage==NULL)
			return IMG_ERR_MEM_FAIL;

		// Decode each scanline
		for(iLineCount=0;iLineCount<iHeight;++iLineCount)
		{
			lLinePos=0;
			while(lLinePos<iBufferLineLen)
			{
				if(*pCur > 0xC0) // First 2 bits indicate run of next byte value
				{
					bRunLen=*pCur & 0x3F; // Remaining 6 bits indicate run length
					++pCur; // Repeated value
					for( ;bRunLen!=0;bRunLen--,lLinePos++)
						pLine[lLinePos]=*pCur;

					++pCur;
				}
				else
				{
					pLine[lLinePos]=*pCur; // Other bytes are directly copied
					++lLinePos;
					++pCur;
				}
			}

			// Once we've decoded a line, copy it to the image.
			// This disregards any end-of-line padding inserted during the compression

			if(iPlanes==1) // 8 bit images, straight copy
			{
				memcpy(&pImage[iLineCount*iImageLineLen],pLine,iImageLineLen);
			}
			else if(iPlanes==3) // for 24 bit, We have to interleave the RGB values
			{
				pInterLine=&pImage[iLineCount*iImageLineLen];
				for(lLinePos=0;lLinePos!=iWidth;++lLinePos,pInterLine+=3)
				{
					pInterLine[0]=pLine[lLinePos];
					pInterLine[1]=pLine[lLinePos+iWidth];
					pInterLine[2]=pLine[lLinePos+(iWidth*2)];
				}
			}

		}

		return IMG_OK;
	}

	int LoadPalette(unsigned long ulDataSize)
	{
		// Load a 256 color palette

		if(pPalette)
		{
			delete [] pPalette;
			pPalette=NULL;
		}
		if(iPlanes==3) // NULL Palette for RGB images
			return IMG_OK;
		// Create space for palette
		pPalette=new RGB[256];
		if(pPalette==NULL)
			return IMG_ERR_MEM_FAIL;
		// Start of palette entries should be 769 bytes back from the end of the file
		// First byte is 0x0C
		if(pData[ulDataSize-769]!=0x0C)
			return IMG_ERR_BAD_FORMAT_PAL;
		memcpy(pPalette,&pData[ulDataSize-768],768);
		return IMG_OK;
	}



	
	RGB *GetPalette(void)
	{
		//Return pcx palette

		return (pPalette);
	}
	unsigned char *GetImage(void)
	{
		//Return Pcx image data

		return (pImage);
	}
	short GetXSize(void)
	{
		//Return Width of image

		return (iWidth);
	}
	short GetYSize(void)
	{
		//Return Height of image

		return (iHeight);
	}
};
