#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 VertexNormal;
layout(location = 1) in vec3 VertexPosition;
layout(location = 2) in vec2 VertexTextureCoords;

layout(location = 0) out vec3 FragmentNormal;
layout(location = 1) out vec4 FragmentPosition;
layout(location = 2) out vec2 FragmentTextureCoords;

layout(binding = 0) uniform MatrixBuffer {
	mat4 Model;
	mat4 View;
	mat4 Projection;
	mat4 MVP;
} mb;

void main()
{
	// TODO: Calculate the normal matrix on the CPU and send it to the shaders via a uniform before drawing
	//FragmentNormal        = (transpose(inverse(mat3(mb.MatrixModel))) * VertexNormal).xyz;
    FragmentNormal        = (mb.Model * vec4(VertexNormal, 0.0)).xyz;
    FragmentTextureCoords = VertexTextureCoords;
    FragmentPosition      = (mb.Model * vec4(VertexPosition, 1.0));
    gl_Position           = (mb.MVP   * vec4(VertexPosition, 1.0));
}
