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
	//vec3  Position;
	//float Reflection;
	//float Shine;
};

varying vec3 FragmentNormal;
varying vec4 FragmentPosition;
varying vec2 FragmentTextureCoords;

uniform vec3      Ambient;
uniform bool      EnableClipping;
uniform vec3      ClipMax;
uniform vec3      ClipMin;
uniform bool      IsTextured;
uniform vec4      MaterialColor;
uniform Light     SunLight;
uniform sampler2D Textures[MAX_TEXTURES];
uniform vec2      TextureScales[MAX_TEXTURES];	// tx = [ [x, y], [x, y], ... ];

void main()
{
	if (EnableClipping) {
		vec4 p = FragmentPosition;
		if ((p.x > ClipMax.x) || (p.y > ClipMax.y) || (p.z > ClipMax.z) || (p.x < ClipMin.x) || (p.y < ClipMin.y) || (p.z < ClipMin.z))
			discard;
	}

	vec2 tiledCoordinates = vec2(FragmentTextureCoords.x * TextureScales[0].x, FragmentTextureCoords.y * TextureScales[0].y);

	// LIGHT COLOR (PHONG REFLECTION)
	vec3 lightColor = (Ambient + (SunLight.Color.rgb * dot(normalize(FragmentNormal), normalize(-SunLight.Direction))));

	// TEXTURE
	if (IsTextured) {
		vec4 texelColor = texture2D(Textures[0], tiledCoordinates);
		gl_FragColor    = vec4((texelColor.rgb * lightColor), texelColor.a);
	} else {
		gl_FragColor = vec4((MaterialColor.rgb * lightColor), MaterialColor.a);
	}
}
