//--------------------------------------------------------------------------//
// iq / rgba  .  tiny codes  .  2008                                        //
//--------------------------------------------------------------------------//

static const char raymarch_vert[] = \
"uniform float aspect;"
"void main(void)"
"{"
"gl_Position=gl_Vertex;"
"gl_TexCoord[0].xy = gl_Vertex.xy * vec2(1.0,aspect);"  //correct aspect
"}";


static const char vhs_vert[] = \
"varying vec4 vpos;"
"void main(){"
"gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex, gl_TexCoord[0] = gl_MultiTexCoord0, vpos = gl_Position;"
"}";
