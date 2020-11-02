//-----------------------------------------------------------------------------
//           Name: sprite.h
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: 
//-----------------------------------------------------------------------------

#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <windows.h>
#include <sys/timeb.h>
#include <GL/gl.h>

class sprite2
{
public:

    sprite2();
    virtual ~sprite2() {};

    void render(int frame,float rcol, float bcol,float gcol, float acol);

    void setPosition(float fPositionX, float fPositionY) 
    {
        m_fPositionX = fPositionX; 
        m_fPositionY = fPositionY; 
    }

    void setSize(float fWidth, float fHeight) 
    { 
        m_fWidth  = fWidth;
        m_fHeight = fHeight;
    }

    void  setPositionX(float fPositionX) { m_fPositionX = fPositionX; }
    float getPositionX(void) { return m_fPositionX; }

    void  setPositionY(float fPositionY) { m_fPositionY = fPositionY; }
    float getPositionY(void) { return m_fPositionY; }
   
    void  setWidth(float fWidth) { m_fWidth = fWidth; }
    float getWidth(void) { return m_fWidth; }
    
    void  setHeight(float fHeight) { m_fHeight = fHeight; }
    float getHeight(void) { return m_fHeight; }

    void  setRotation(float fRotation) { m_fRotation = m_fRotation; }
    float getRotation(void) { return m_fRotation; }

    void  setAlpha(float fAlpha) { m_fAlpha = fAlpha; }
    float getAlpha(void) { return m_fAlpha; }
    
    void  setTextureID(GLuint textureID) { m_textureID = textureID; }
    int   getTextureID(void) { return m_textureID; }

    void  setTextureWidth(int nTextureWidth) { m_nTextureWidth = nTextureWidth; }
    int   getTextureWidth() { return m_nTextureWidth; }

    void  setTextureHeight(int nTextureHeight) { m_nTextureHeight = nTextureHeight; }
    int   getTextureHeight() { return m_nTextureHeight; }

    void  setFrameWidth(int nFrameWidth) { m_nFrameWidth = nFrameWidth; }
    int   getFrameWidth() { return m_nFrameWidth; }

    void  setFrameHeight(int nFrameHeight) { m_nFrameHeight = nFrameHeight; }
    int   getFrameHeight() { return m_nFrameHeight; }

    void  setNumFrameColumns(int nNumFrameColumns) { m_nNumFrameColumns = nNumFrameColumns; }
    int   getNumFrameColumns() { return m_nNumFrameColumns; }

    void  setNumFrameRows (int nNumFrameRows) { m_nNumFrameRows = nNumFrameRows; }
    int   getNumFrameRows () { return m_nNumFrameRows; }

    void  setTotalFrames(int nTotalFrames) { m_nTotalFrames = nTotalFrames; }
    int   getTotalFrames() { return m_nTotalFrames; }

    void  setOffsetX(int nOffsetX) { m_nOffsetX = nOffsetX; }
    int   getOffsetX() { return m_nOffsetX; }

    void  setOffsetY(int nOffsetY) { m_nOffsetY = nOffsetY; }
    int   getOffsetY() { return m_nOffsetY; }

    void  setFrameDelay( float fFrameDelay ) { m_fFrameDelay = fFrameDelay; }
    float getFrameDelay(void) { return m_fFrameDelay; }

    void  setTextureAnimeInfo( int nTextureWidth, int nTextureHeight, 
                               int nFrameWidth,   int nFrameHeight,
                               int nNumFrameColumns, int nNumFrameRows, 
                               int nTotalFrames,
                               int nOffsetX = 0, int nOffsetY = 0 ) 
    {
        m_nTextureWidth    = nTextureWidth;
        m_nTextureHeight   = nTextureHeight;
        m_nFrameWidth      = nFrameWidth;
        m_nFrameHeight     = nFrameHeight;
        m_nNumFrameColumns = nNumFrameColumns;
        m_nNumFrameRows    = nNumFrameRows;
        m_nTotalFrames     = nTotalFrames;
        m_nOffsetX         = nOffsetX;
        m_nOffsetY         = nOffsetY;

    }

private:

    // Sprite Controls...

    float  m_fPositionX;
    float  m_fPositionY;
    float  m_fWidth;
    float  m_fHeight;
    float  m_fRotation;
    float  m_fAlpha;

    // Texture Animation Settings...

    GLuint m_textureID;
    float  m_fFrameDelay;
    int    m_nNumFrameColumns;
    int    m_nNumFrameRows;
    int    m_nTotalFrames;
    int    m_nTextureWidth;
    int    m_nTextureHeight;
    int    m_nFrameWidth;
    int    m_nFrameHeight;
    int    m_nOffsetX;
    int    m_nOffsetY;

    // Frame tracking variables...

    int    m_nFrameNumber;
    int    m_nCurrentRow;
    int    m_nCurrentColumn;
	timeb  m_lastTime;
    bool   m_bFirstRendering;
};

#endif /* _SPRITE_H_ */
