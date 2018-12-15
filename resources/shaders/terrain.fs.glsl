#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_LIGHT_SOURCES = 16;
const int MAX_TEXTURES      = 6;

struct CBLight
{
    vec4 Active;
    vec4 Ambient;
    vec4 Angles;
    vec4 Attenuation;
    vec4 Diffuse;
    vec4 Direction;
    vec4 Position;
    vec4 Specular;
};

layout(location = 0) in vec3 FragmentNormal;
layout(location = 1) in vec4 FragmentPosition;
layout(location = 2) in vec2 FragmentTextureCoords;

layout(location = 0) out vec4 GL_FragColor;

layout(binding = 1) uniform TerrainBuffer
{
    CBLight LightSources[MAX_LIGHT_SOURCES];

    vec4 IsTextured[MAX_TEXTURES];
    vec4 TextureScales[MAX_TEXTURES];

	vec4 CameraPosition;

    vec4 MeshSpecular;
    vec4 MeshDiffuse;

    vec4 ClipMax;
    vec4 ClipMin;
	vec4 EnableClipping;
} tb;

layout(binding = 2) uniform sampler2D Textures[MAX_TEXTURES];

void main()
{
	if (tb.EnableClipping.x > 0.1)
	{
		vec4 p = FragmentPosition;

		if ((p.x > tb.ClipMax.x) || (p.y > tb.ClipMax.y) || (p.z > tb.ClipMax.z) || (p.x < tb.ClipMin.x) || (p.y < tb.ClipMin.y) || (p.z < tb.ClipMin.z))
			discard;
	}

	vec4  blendMapColor           = texture(Textures[4], FragmentTextureCoords);
	float backgroundTextureAmount = (1.0 - (blendMapColor.r + blendMapColor.g + blendMapColor.b));
	//vec2  tiledCoordinates        = (FragmentTextureCoords * tb.TextureScales[0]);
	vec2  tiledCoordinates        = vec2(FragmentTextureCoords.x * tb.TextureScales[0].x, FragmentTextureCoords.y * tb.TextureScales[0].y);
	vec4  backgroundTextureColor  = (texture(Textures[0], tiledCoordinates) * backgroundTextureAmount);
	vec4  rTextureColor           = (texture(Textures[1], tiledCoordinates) * blendMapColor.r);
	vec4  gTextureColor           = (texture(Textures[2], tiledCoordinates) * blendMapColor.g);
	vec4  bTextureColor           = (texture(Textures[3], tiledCoordinates) * blendMapColor.b);
	vec4  totalColor              = (backgroundTextureColor + rTextureColor + gTextureColor + bTextureColor);

	// LIGHT COLOR (PHONG REFLECTION)
	GL_FragColor = vec4(0);

	for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
        if ((tb.LightSources[i].Active.x < 0.1) || (tb.LightSources[i].Angles.x > 0.1) || (tb.LightSources[i].Attenuation.r > 0.1))
			continue;

		vec3 lightColor = (tb.LightSources[i].Ambient.rgb + (tb.LightSources[i].Diffuse.rgb * dot(normalize(FragmentNormal), normalize(-tb.LightSources[i].Direction.xyz))));

		GL_FragColor += vec4((totalColor.rgb * lightColor), totalColor.a);
	}

	/*
	// DEEP WATER
	if (fragmentPosition.y < 0.1)
		GL_FragColor = vec4((vec3(0.0, 0.0, rand(vec2(0.1, 0.5))) * lightColor), 1.0);
	// WATER
	else if (fragmentPosition.y < 0.15)
		GL_FragColor = vec4((vec3(0.0, 0.0, rand(vec2(0.6, 1.0))) * lightColor), 1.0);
	// SAND
	else if (fragmentPosition.y < 0.3)
		GL_FragColor = vec4((vec3(0.8, 0.7, 0.5) * lightColor), 1.0);
	// GRASS
	else if (fragmentPosition.y < 0.5)
		GL_FragColor = vec4((vec3(0.0, rand(vec2(0.2, 1.0)), 0.0) * lightColor), 1.0);
	// ROCK
	else if (fragmentPosition.y < 0.7)
		GL_FragColor = vec4((vec3(rand(vec2(0.4, 0.6)), rand(vec2(0.4, 0.6)), rand(vec2(0.4, 0.6))) * lightColor), 1.0);
	// SNOW
	else if (fragmentPosition.y < 0.95)
		GL_FragColor = vec4((vec3(1.0, 1.0, 1.0) * lightColor), 1.0);
	*/
}

