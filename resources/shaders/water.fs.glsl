#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_TEXTURES = 6;

struct CBDefault
{
    vec4 IsTextured[MAX_TEXTURES];
    vec4 TextureScales[MAX_TEXTURES];

	vec3  CameraPosition;
	float CameraPadding1;

    vec3  MeshSpecularIntensity;
	float MeshSpecularShininess;

    vec4  MeshDiffuse;

    vec3  SunLightSpecularIntensity;
	float SunLightSpecularShininess;

    vec3  SunLightAmbient;
	float SunLightPadding1;
    vec4  SunLightDiffuse;

    vec3  SunLightPosition;
	float SunLightPadding2;
    vec3  SunLightDirection;
	float SunLightPadding3;

    vec3  ClipMax;
	float EnableClipping;
    vec3  ClipMin;
    float ClipPadding1;
};

layout(location = 0) in vec4 ClipSpace;
layout(location = 1) in vec4 FragmentPosition;
layout(location = 2) in vec2 FragmentTextureCoords;

layout(location = 0) out vec4 GL_FragColor;

layout(binding = 1) uniform WaterBuffer
{
	float MoveFactor;
	float WaveStrength;
	vec2  Padding1;

	CBDefault DB;
} wb;

layout(binding = 2) uniform sampler2D Textures[MAX_TEXTURES];

void main()
{
	// CLIPPING PLANE
	if (wb.DB.EnableClipping.x > 0.1)
	{
		vec4 p = FragmentPosition;

		if ((p.x > wb.DB.ClipMax.x) || (p.y > wb.DB.ClipMax.y) || (p.z > wb.DB.ClipMax.z) || (p.x < wb.DB.ClipMin.x) || (p.y < wb.DB.ClipMin.y) || (p.z < wb.DB.ClipMin.z))
			discard;
	}

	vec2 tiledCoordinates = vec2(FragmentTextureCoords.x * wb.DB.TextureScales[0].x, FragmentTextureCoords.y * wb.DB.TextureScales[0].y);

	// PROJECTIVE TEXTURE
	vec2 normalizedDeviceSpace   = ((ClipSpace.xy / ClipSpace.w) * 0.5 + 0.5);				// [-1,1] => [0,1]
	vec2 reflectionTextureCoords = vec2(normalizedDeviceSpace.x, -normalizedDeviceSpace.y);
	vec2 refractionTextureCoords = vec2(normalizedDeviceSpace.x, normalizedDeviceSpace.y);

	// DU/DV MAP - DISTORTION
	vec2 distortedTextureCoords = (texture(Textures[2], vec2(tiledCoordinates.x + wb.MoveFactor, tiledCoordinates.y)).rg * 0.1);
	distortedTextureCoords      = (tiledCoordinates + vec2(distortedTextureCoords.x, distortedTextureCoords.y + wb.MoveFactor));
	vec2 totalDistortion        = ((texture(Textures[2], distortedTextureCoords).rg * 2.0 - 1.0) * wb.WaveStrength);

	reflectionTextureCoords += totalDistortion;
	refractionTextureCoords += totalDistortion;
	
	vec4 reflectionColor = texture(Textures[0], reflectionTextureCoords);
	vec4 refractionColor = texture(Textures[1], refractionTextureCoords);

	// NORMAL MAP
	vec4 normalMapColor = texture(Textures[3], distortedTextureCoords);
	vec3 normal         = normalize(vec3((normalMapColor.r * 2.0 - 1.0), (normalMapColor.b * 7.0), (normalMapColor.g * 2.0 - 1.0)));

	// FRESNEL EFFECT
    vec3  fromSurfaceToCamera = (wb.DB.CameraPosition - FragmentPosition.xyz);
	vec3  fromLightToSurface  = (FragmentPosition.xyz - wb.DB.SunLightPosition);
	vec3  viewVector         = normalize(fromSurfaceToCamera);
	float refractionFactor   = clamp(pow(dot(viewVector, normal), 10.0), 0.0, 1.0);	// higher power => less reflective (transparent)
	vec3  reflectedLight     = reflect(normalize(fromLightToSurface), normal);
	float specular           = pow(max(0.0, dot(reflectedLight, viewVector)), wb.DB.SunLightSpecularIntensity[0]);
	vec3  specularHighlights = (wb.DB.SunLightDiffuse.rgb * specular * wb.DB.SunLightSpecularShininess);

	GL_FragColor = (mix(reflectionColor, refractionColor, refractionFactor) + vec4(specularHighlights, 0.0));
	//GL_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	//GL_FragColor = vec4(texture(Textures[0], FragmentTextureCoords).rgb, 1.0);

	//vec2 tiledCoordinates = (fragmentTextureCoords * textureScales[0]);
	// // http://web.archive.org/web/20130416194336/http://olivers.posterous.com/linear-depth-in-glsl-for-real
	// // https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
	// // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/gl_FragCoord.xhtml
	// float depth1                  = texture2D(textures[4], refractionTextureCoords).r;
	// float depth2                  = gl_FragCoord.z;
	// float a                       = (2.0 * camera.Near * CameraMain.Far);
	// float b                       = (CameraMain.Far + CameraMain.Near);
	// float c                       = (CameraMain.Far - CameraMain.Near);
	// float distanceCameraToFloor   = (a / (b - (2.0 * depth1 - 1.0) * c));
	// float distanceCameraToSurface = (a / (b - (2.0 * depth2 - 1.0) * c));
	// float depth                   = (distanceCameraToSurface - distanceCameraToFloor);
	//vec2 totalDistortion        = ((texture2D(Textures[2], distortedTextureCoords).rg * 2.0 - 1.0) * WaveStrength * clamp((depth / 20.0), 0.0, 1.0));
	// reflectionTextureCoords.x = clamp(reflectionTextureCoords.x,  0.001,  0.999);
	// reflectionTextureCoords.y = clamp(reflectionTextureCoords.y, -0.999, -0.001);
	// refractionTextureCoords  = clamp(refractionTextureCoords,    0.001,  0.999);
	//vec3 normal         = normalize(vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b, normalMapColor.g * 2.0 - 1.0));
	//float refractionFactor = clamp(pow(dot(viewVector, vec3(0.0, 1.0, 0.0)), 10.0), 0.0, 1.0);	// higher power => less reflective (transparent)
	//vec3  specularHighlights = (SunLight.Color.rgb * specular * SunLight.Reflection * clamp((depth / 5.0), 0.0, 1.0));
	// gl_FragColor   = (mix(gl_FragColor, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + vec4(specularHighlights, 0.0));
	//gl_FragColor.a = clamp((depth / 5.0), 0.0, 1.0);
}
