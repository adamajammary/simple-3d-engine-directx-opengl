#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_TEXTURES = 6;

//varying vec3 FragmentNormal;
//varying vec4 FragmentPosition;
varying vec2 FragmentTextureCoords;

uniform vec4      MaterialColor;
//uniform bool      IsTextured;
uniform bool      IsTransparent;
uniform sampler2D Textures[MAX_TEXTURES];
//uniform vec2      TextureScales[MAX_TEXTURES];	// tx = [ [x, y], [x, y], ... ];

void main()
{
	vec4 sampledColor = texture2D(Textures[5], FragmentTextureCoords);

	if (IsTransparent)
		gl_FragColor = sampledColor;
	else
		gl_FragColor = vec4(sampledColor.rgb, MaterialColor.a);
}
