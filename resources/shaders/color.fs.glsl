#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

layout(location = 0) out vec4 GL_FragColor;

layout(binding = 1) uniform ColorBuffer {
	vec4 Color;
} cb;

void main()
{
	GL_FragColor = cb.Color;
}
