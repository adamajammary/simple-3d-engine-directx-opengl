#version 450
#extension GL_ARB_separate_shader_objects : enable

const int MAX_TEXTURES = 6;

layout(location = 0) in vec3 VertexNormal;
layout(location = 1) in vec3 VertexPosition;
layout(location = 2) in vec2 VertexTextureCoords;

layout(location = 0) out vec2 FragmentTextureCoords;

layout(binding = 0) uniform MatrixBuffer {
	mat4 Normal;
	mat4 Model;
	mat4 VP[MAX_TEXTURES];
	mat4 MVP;
} mb;

void main()
{
	FragmentTextureCoords = vec2(((VertexPosition.x + 1.0) * 0.5), ((VertexPosition.y + 1.0) * 0.5));
    gl_Position           = (mb.Model * vec4(VertexPosition.xy, 0.0, 1.0));
}
