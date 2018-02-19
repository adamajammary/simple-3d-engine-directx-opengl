#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_TEXTURES = 6;

//varying vec3 FragmentNormal;
//varying vec4 FragmentPosition;
varying vec3 FragmentTextureCoords;

uniform samplerCube Textures[MAX_TEXTURES];
uniform vec2        TextureScales[MAX_TEXTURES];	// tx = [ [x, y], [x, y], ... ];

void main()
{
	gl_FragColor = textureCube(Textures[0], FragmentTextureCoords);
}
