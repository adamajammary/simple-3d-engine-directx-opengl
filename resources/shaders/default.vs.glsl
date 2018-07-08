#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 VertexNormal;
layout(location = 1) in vec3 VertexPosition;
layout(location = 2) in vec2 VertexTextureCoords;

layout(location = 0) out vec3 FragmentNormal;
layout(location = 1) out vec4 FragmentPosition;
layout(location = 2) out vec2 FragmentTextureCoords;

layout(binding = 0) uniform MatrixBuffer {
	mat4 MatrixModel;
	mat4 MatrixView;
	mat4 MatrixProjection;
	mat4 MatrixMVP;
} mb;

/*
attribute vec3 VertexNormal;
attribute vec3 VertexPosition;
attribute vec2 VertexTextureCoords;

varying vec3 FragmentNormal;
varying vec4 FragmentPosition;
varying vec2 FragmentTextureCoords;

uniform mat4 MatrixModel;
//uniform mat4 MatrixView;
//uniform mat4 MatrixProjection;
uniform mat4 MatrixMVP;
*/

void main()
{
    //FragmentNormal        = ((mat3)mb.MatrixModel * VertexNormal);
    //FragmentNormal        = vec3(mb.MatrixModel * vec4(VertexNormal, 0.0));

    FragmentNormal        = (mb.MatrixModel * vec4(VertexNormal, 0.0)).xyz;
    FragmentTextureCoords = VertexTextureCoords;
    FragmentPosition      = (mb.MatrixModel * vec4(VertexPosition, 1.0));
    gl_Position           = (mb.MatrixMVP   * vec4(VertexPosition, 1.0));

    /*gl_Position = vec4(VertexPosition, 1.0);*/

}
