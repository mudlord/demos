#ifdef _MSC_VER
#include <windows.h> // needed to get GL stuff to work
#endif
#include "sys/msys.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "vertexbuffer.h"

void VertexBuffer::render_normals(float aScale)
{
    GLfloat *temp = new GLfloat[mVertices * 3 * 2];
    int i;
    for (i = 0; i < mVertices; i++)
    {
        temp[i * 6 + 0] = mVertex[i * 3 + 0];
        temp[i * 6 + 1] = mVertex[i * 3 + 1];
        temp[i * 6 + 2] = mVertex[i * 3 + 2];

        temp[i * 6 + 3] = mVertex[i * 3 + 0] + mNormal[i * 3 + 0] * aScale;
        temp[i * 6 + 4] = mVertex[i * 3 + 1] + mNormal[i * 3 + 1] * aScale;
        temp[i * 6 + 5] = mVertex[i * 3 + 2] + mNormal[i * 3 + 2] * aScale;
    }

    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glVertexPointer(3, GL_FLOAT, 0, temp);            
    glDrawArrays(GL_LINES,0,mVertices * 2);
    delete[] temp;
}


void VertexBuffer::activate()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, mVertex);            
    glNormalPointer(GL_FLOAT, 0, mNormal);

    int i;
    for (i = 0; i < 4; i++)
    {
        if (mTexcoord[i])
        {
            oglClientActiveTextureARB(GL_TEXTURE0_ARB + i);
            glTexCoordPointer(2, GL_FLOAT, 0, mTexcoord[i]);
        }
    }
   oglClientActiveTextureARB(GL_TEXTURE0_ARB);
}

void VertexBuffer::render()
{
    glDrawElements(mPrimitiveType,mIndices,GL_UNSIGNED_INT,mIndex);
}

VertexBuffer::VertexBuffer()
{
    mVertex = NULL;
    mNormal = NULL;
    mIndex = NULL;
    mIndices = 0;
    mPrimitiveType = 0;
    mTexcoord[0] = mTexcoord[1] = mTexcoord[2] = mTexcoord[3] = 0;
}

VertexBuffer::~VertexBuffer()
{
    delete[] mVertex;
    mVertex = NULL;

    delete[] mNormal;
    mNormal = NULL;

    delete[] mIndex;
    mIndex = NULL;

    mIndices = 0;
    mPrimitiveType = 0;

    int i,j;

    // clear tex coord arrays, guarding against double deletion
    for (i = 0; i < 4; i++)
    {
        delete[] mTexcoord[i];
        for (j = i + 1; j < 4; j++)
        {
            if (mTexcoord[j] == mTexcoord[i])
                mTexcoord[j] = NULL;
        }
        mTexcoord[i] = NULL;
    }
    

}
