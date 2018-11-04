#version 450
#extension GL_ARB_separate_shader_objects : enable

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
	vec3  Padding1;
	float Far;
};

struct Light
{
	vec4  Color;
	vec3  Direction;
	float Reflection;
	vec3  Position;
	float Shine;
};

layout(location = 0) in vec4 ClipSpace;
layout(location = 1) in vec4 FragmentPosition;
layout(location = 2) in vec2 FragmentTextureCoords;

layout(location = 0) out vec4 GL_FragColor;

layout(binding = 1) uniform WaterBuffer
{
	Camera CameraMain;
	Light  SunLight;
	vec3   ClipMax;
	bool   EnableClipping;
	vec3   ClipMin;
	float  MoveFactor;
	vec3   Padding1;
	float  WaveStrength;
	vec2   TextureScales[MAX_TEXTURES];	// tx = [ [x, y], [x, y], ... ];
} wb;

layout(binding = 2) uniform sampler2D Textures[MAX_TEXTURES];

void main()
{
	// CLIPPING PLANE
	if (wb.EnableClipping)
	{
		vec4 p = FragmentPosition;

		if ((p.x > wb.ClipMax.x) || (p.y > wb.ClipMax.y) || (p.z > wb.ClipMax.z) || (p.x < wb.ClipMin.x) || (p.y < wb.ClipMin.y) || (p.z < wb.ClipMin.z))
			discard;
	}

	vec2 tiledCoordinates = vec2(FragmentTextureCoords.x * wb.TextureScales[0].x, FragmentTextureCoords.y * wb.TextureScales[0].y);

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
    vec3  fromSurfaceToCamera = (wb.CameraMain.Position - FragmentPosition.xyz);
	vec3  fromLightToSurface  = (FragmentPosition.xyz - wb.SunLight.Position);
	vec3  viewVector         = normalize(fromSurfaceToCamera);
	float refractionFactor   = clamp(pow(dot(viewVector, normal), 10.0), 0.0, 1.0);					// higher power => less reflective (transparent)
	vec3  reflectedLight     = reflect(normalize(fromLightToSurface), normal);
	float specular           = pow(max(0.0, dot(reflectedLight, viewVector)), wb.SunLight.Shine);
	vec3  specularHighlights = (wb.SunLight.Color.rgb * specular * wb.SunLight.Reflection);

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
