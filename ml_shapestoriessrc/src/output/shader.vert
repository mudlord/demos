#version 330
uniform mat4 ViewProjection; // the projection matrix uniform
layout(location = 0) in vec4 vposition;
layout(location = 1) in vec4 vuv;
out vec2 vertexuv;
void main() {
	vertexuv = vuv.xy;
	gl_Position =ViewProjection*vposition;
}