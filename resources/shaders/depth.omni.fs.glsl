#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

layout(location = 0) in vec4 FragmentPosition;

layout(binding = 1) uniform DepthBuffer {
	vec4 lightPosition;
} db;

void main()
{
    gl_FragDepth = (length(FragmentPosition.xyz - db.lightPosition.xyz) / 25.0f);
}
