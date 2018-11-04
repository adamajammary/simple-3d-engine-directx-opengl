#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_TEXTURES = 6;

layout(location = 0) in  vec2 FragmentTextureCoords;
layout(location = 0) out vec4 GL_FragColor;

layout(binding = 1) uniform HUDBuffer
{
	vec4 MaterialColor;
	vec3 Padding1;
	bool IsTransparent;
} hb;

layout(binding = 2) uniform sampler2D Textures[MAX_TEXTURES];

void main()
{
	vec4 sampledColor = texture(Textures[5], FragmentTextureCoords);

	if (hb.IsTransparent)
		GL_FragColor = sampledColor;
	else
		GL_FragColor = vec4(sampledColor.rgb, hb.MaterialColor.a);
}
