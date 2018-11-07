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

	vec2 tiledCoordinates = vec2(FragmentTextureCoords.x * db.TextureScales[0].x, FragmentTextureCoords.y * db.TextureScales[0].y);

	// LIGHT COLOR (PHONG REFLECTION)
	vec3 lightColor = (db.Ambient + (db.SunLight.Color.rgb * dot(normalize(FragmentNormal), normalize(-db.SunLight.Direction))));

	// TEXTURE
	if (db.IsTextured) {
		vec4 texelColor = texture(Textures[0], tiledCoordinates);
		GL_FragColor    = vec4((texelColor.rgb * lightColor), texelColor.a);
	} else {
		GL_FragColor = vec4((db.MaterialColor.rgb * lightColor), db.MaterialColor.a);
	}
}
