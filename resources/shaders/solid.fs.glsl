#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

uniform vec4 SolidColor;

void main()
{
	gl_FragColor = SolidColor;
}
