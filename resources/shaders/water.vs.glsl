#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 VertexNormal;
layout(location = 1) in vec3 VertexPosition;
layout(location = 2) in vec2 VertexTextureCoords;

layout(location = 0) out vec4 ClipSpace;
layout(location = 1) out vec4 FragmentPosition;
layout(location = 2) out vec2 FragmentTextureCoords;

layout(binding = 0) uniform MatrixBuffer {
	mat4 Normal;
	mat4 Model;
	mat4 View;
	mat4 Projection;
	mat4 MVP;
} mb;

void main()
{
    FragmentTextureCoords = VertexTextureCoords;
    FragmentPosition      = (mb.Model * vec4(VertexPosition, 1.0));
    ClipSpace             = (mb.MVP   * vec4(VertexPosition, 1.0));

    gl_Position = ClipSpace;
}
