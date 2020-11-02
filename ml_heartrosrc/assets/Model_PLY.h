#ifndef MODEL_PLY_H
#define MODEL_PLY_H

class Model_PLY 
{
public:
	int Load(char *filename);
	void Draw();
	float* calculateNormal( float *coord1, float *coord2, float *coord3 );
	Model_PLY();
 
    float* Faces_Triangles;
    float* Faces_Quads;
	float* Vertex_Buffer;
	float* Normals;
 
	int TotalConnectedTriangles;	
	int TotalConnectedQuads;	
	int TotalConnectedPoints;
	int TotalFaces;
 
 
};


#endif
