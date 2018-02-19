#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_TEXTURES = 6;

struct Camera
{
	vec3  Position;
	float Near;
	float Far;
};

struct Light
{
	vec4  Color;
	vec3  Direction;
	vec3  Position;
	float Reflection;
	float Shine;
};

varying vec3 FragmentNormal;
varying vec4 FragmentPosition;
varying vec2 FragmentTextureCoords;

uniform vec3      Ambient;
uniform bool      EnableClipping;
uniform vec3      ClipMax;
uniform vec3      ClipMin;
uniform Light     SunLight;
uniform sampler2D Textures[MAX_TEXTURES];
uniform vec2      TextureScales[MAX_TEXTURES];	// tx = [ [x, y], [x, y], ... ];

// uniform vec4             MaterialColor;
// uniform bool             IsTextured;
// uniform sampler2D        backgroundTexture;	// 0
// uniform sampler2D        rTexture;			// 1
// uniform sampler2D        gTexture;			// 2
// uniform sampler2D        bTexture;			// 3
// uniform sampler2D        blendMap;			// 4
//uniform float     textureScales[MAX_TEXTURES];

// highp float rand(vec2 co)
// {
// 	highp float a  = 12.9898;
// 	highp float b  = 78.233;
// 	highp float c  = 43758.5453;
// 	highp float dt = dot(co.xy, vec2(a,b));
// 	highp float sn = mod(dt, 3.14);

// 	return fract(sin(sn) * c);
// }

void main()
{
	if (EnableClipping) {
		vec4 p = FragmentPosition;
		if ((p.x > ClipMax.x) || (p.y > ClipMax.y) || (p.z > ClipMax.z) || (p.x < ClipMin.x) || (p.y < ClipMin.y) || (p.z < ClipMin.z))
			discard;
	}

	vec4  blendMapColor           = texture2D(Textures[4], FragmentTextureCoords);
	float backgroundTextureAmount = (1.0 - (blendMapColor.r + blendMapColor.g + blendMapColor.b));
	//vec2  tiledCoordinates        = (FragmentTextureCoords * TextureScales[0]);
	vec2  tiledCoordinates        = vec2(FragmentTextureCoords.x * TextureScales[0].x, FragmentTextureCoords.y * TextureScales[0].y);
	vec4  backgroundTextureColor  = (texture2D(Textures[0], tiledCoordinates) * backgroundTextureAmount);
	vec4  rTextureColor           = (texture2D(Textures[1], tiledCoordinates) * blendMapColor.r);
	vec4  gTextureColor           = (texture2D(Textures[2], tiledCoordinates) * blendMapColor.g);
	vec4  bTextureColor           = (texture2D(Textures[3], tiledCoordinates) * blendMapColor.b);
	vec4  totalColor              = (backgroundTextureColor + rTextureColor + gTextureColor + bTextureColor);

	// LIGHT COLOR (PHONG REFLECTION)
	//vec3 lightColor = max((Ambient + (SunLight.Color.rgb * dot(normalize(FragmentNormal), normalize(-SunLight.Direction)))), 0.0);
	vec3 lightColor = (Ambient + (SunLight.Color.rgb * dot(normalize(FragmentNormal), normalize(-SunLight.Direction))));
	gl_FragColor    = vec4((totalColor.rgb * lightColor), totalColor.a);

	// // TEXTURE
	// if (isTextured) {
	// 	texelColor   = texture2D(textureSampler, fragmentTextureCoords);
	// 	gl_FragColor = vec4((texelColor.rgb * lightColor), texelColor.a);
	// } else {
	// 	gl_FragColor = vec4((materialColor.rgb * lightColor), materialColor.a);
	// }

	// // DEEP WATER
	// if (fragmentPosition.y < 0.1) {
	// 	gl_FragColor = vec4((vec3(0.0, 0.0, rand(vec2(0.1, 0.5))) * lightColor), 1.0);
	// // WATER
	// } else if (fragmentPosition.y < 0.15) {
	// 	gl_FragColor = vec4((vec3(0.0, 0.0, rand(vec2(0.6, 1.0))) * lightColor), 1.0);
	// // SAND
	// } else if (fragmentPosition.y < 0.3) {
	// 	gl_FragColor = vec4((vec3(0.8, 0.7, 0.5) * lightColor), 1.0);
	// // GRASS
	// } else if (fragmentPosition.y < 0.5) {
	// 	gl_FragColor = vec4((vec3(0.0, rand(vec2(0.2, 1.0)), 0.0) * lightColor), 1.0);
	// // ROCK
	// } else if (fragmentPosition.y < 0.7) {
	// 	gl_FragColor = vec4((vec3(rand(vec2(0.4, 0.6)), rand(vec2(0.4, 0.6)), rand(vec2(0.4, 0.6))) * lightColor), 1.0);
	// // SNOW
	// } else if (fragmentPosition.y < 0.95) {
	// 	gl_FragColor = vec4((vec3(1.0, 1.0, 1.0) * lightColor), 1.0);
	// }

	//gl_FragColor = vec4(fragmentColor, 1.0);
	//gl_FragColor = vec4(fragmentNormal, 1.0);
	//gl_FragColor = texture2D(textureSampler, fragmentTextureCoords);
	//gl_FragColor = vec4((texelColor.rgb * lightIntensity), texelColor.a);
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	//gl_FragColor = vec4(fragmentTextureCoords, 1.0, 1.0);
}
