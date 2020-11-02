
// Vertex buffer class
// Wrapper for OpenGL arrays

class VertexBuffer
{
public:
    int mIndices;
    int mVertices;
    int mPrimitiveType;
    GLfloat * mVertex;
    GLfloat * mNormal;
    GLfloat * mTexcoord[4];
    GLuint * mIndex;
    void activate();
    void render();
    void render_normals(float aScale = 0.2f);
    VertexBuffer();
    ~VertexBuffer();
};