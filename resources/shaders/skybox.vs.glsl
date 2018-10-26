#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 VertexNormal;
layout(location = 1) in vec3 VertexPosition;
layout(location = 2) in vec2 VertexTextureCoords;

layout(location = 0) out vec3 FragmentTextureCoords;

layout(binding = 0) uniform MatrixBuffer {
	mat4 MatrixModel;
	mat4 MatrixView;
	mat4 MatrixProjection;
	mat4 MatrixMVP;
} mb;

void main()
{
	FragmentTextureCoords = VertexPosition;
    gl_Position           = vec4(mb.MatrixMVP * vec4(VertexPosition, 1.0)).xyww;
}
