#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

//layout(location = 0) in float FragmentPosition;

void main()
{
	//gl_FragDepth = 0.1;
	//gl_FragDepth = FragmentPosition;
	//gl_FragDepth = gl_FragCoord.z;
}
