#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_TEXTURES = 6;

layout(location = 0) in  vec3 FragmentTextureCoords;
layout(location = 0) out vec4 GL_FragColor;

layout(binding = 1) uniform SkyboxBuffer {
	vec4 EnableSRGB;
} sb;

layout(binding = 2) uniform samplerCube Textures[MAX_TEXTURES];

// sRGB GAMMA CORRECTION
vec3 GetFragColorSRGB(vec3 colorRGB)
{
	if (sb.EnableSRGB.x > 0.1) {
		float sRGB = (1.0 / 2.2);
		colorRGB.rgb = pow(colorRGB.rgb, vec3(sRGB, sRGB, sRGB));
	}

	return colorRGB;
}

void main()
{
	GL_FragColor     = texture(Textures[0], FragmentTextureCoords);
	GL_FragColor.rgb = GetFragColorSRGB(GL_FragColor.rgb);
}
