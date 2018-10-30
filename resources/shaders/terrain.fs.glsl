#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_TEXTURES = 6;

struct Light
{
	vec4  Color;
	vec3  Direction;
	float Reflection;
	vec3  Position;
	float Shine;
};

layout(location = 0) in vec3 FragmentNormal;
layout(location = 1) in vec4 FragmentPosition;
layout(location = 2) in vec2 FragmentTextureCoords;

layout(location = 0) out vec4 GL_FragColor;

layout(binding = 1) uniform DefaultBuffer {
	vec3  Ambient;
	bool  EnableClipping;
	vec3  ClipMax;
	bool  IsTextured;
	vec3  ClipMin;
	float Padding1;
	vec4  MaterialColor;
	Light SunLight;
	vec2  TextureScales[MAX_TEXTURES];	// tx = [ [x, y], [x, y], ... ];
} db;

layout(binding = 2) uniform sampler2D Textures[MAX_TEXTURES];

void main()
{
	if (db.EnableClipping)
	{
		vec4 p = FragmentPosition;

		if ((p.x > db.ClipMax.x) || (p.y > db.ClipMax.y) || (p.z > db.ClipMax.z) || (p.x < db.ClipMin.x) || (p.y < db.ClipMin.y) || (p.z < db.ClipMin.z))
			discard;
	}

	vec4  blendMapColor           = texture(Textures[4], FragmentTextureCoords);
	float backgroundTextureAmount = (1.0 - (blendMapColor.r + blendMapColor.g + blendMapColor.b));
	//vec2  tiledCoordinates        = (FragmentTextureCoords * db.TextureScales[0]);
	vec2  tiledCoordinates        = vec2(FragmentTextureCoords.x * db.TextureScales[0].x, FragmentTextureCoords.y * db.TextureScales[0].y);
	vec4  backgroundTextureColor  = (texture(Textures[0], tiledCoordinates) * backgroundTextureAmount);
	vec4  rTextureColor           = (texture(Textures[1], tiledCoordinates) * blendMapColor.r);
	vec4  gTextureColor           = (texture(Textures[2], tiledCoordinates) * blendMapColor.g);
	vec4  bTextureColor           = (texture(Textures[3], tiledCoordinates) * blendMapColor.b);
	vec4  totalColor              = (backgroundTextureColor + rTextureColor + gTextureColor + bTextureColor);

	// LIGHT COLOR (PHONG REFLECTION)
	//vec3 lightColor = max((db.Ambient + (db.SunLight.Color.rgb * dot(normalize(FragmentNormal), normalize(-db.SunLight.Direction)))), 0.0);
	vec3 lightColor = (db.Ambient + (db.SunLight.Color.rgb * dot(normalize(FragmentNormal), normalize(-db.SunLight.Direction))));
	GL_FragColor    = vec4((totalColor.rgb * lightColor), totalColor.a);

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

