#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_LIGHT_SOURCES = 13;
const int MAX_TEXTURES      = 6;

struct CBLight
{
    vec4 Active;
    vec4 Ambient;
    vec4 Angles;
    vec4 Attenuation;
    vec4 Diffuse;
    vec4 Direction;
    vec4 Position;
    vec4 Specular;
	mat4 ViewProjection;
};

struct CBDefault
{
    CBLight LightSources[MAX_LIGHT_SOURCES];

    vec4 IsTextured[MAX_TEXTURES];
    vec4 TextureScales[MAX_TEXTURES];

	vec4 CameraPosition;

    vec4 MeshSpecular;
    vec4 MeshDiffuse;

    vec4 ClipMax;
    vec4 ClipMin;
	vec4 EnableClipping;

    vec4 EnableSRGB;
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
	vec3  fromSurfaceToCamera = (wb.DB.CameraPosition.xyz - FragmentPosition.xyz);
	vec3  viewVector          = normalize(fromSurfaceToCamera);
	float refractionFactor    = clamp(pow(dot(viewVector, normal), 10.0), 0.0, 1.0);	// higher power => less reflective (transparent)
	GL_FragColor              = vec4(0.0, 0.0, 0.0, 1.0);

	// SPECULAR HIGHLIGHTS
	for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
        if ((wb.DB.LightSources[i].Active.x < 0.1) || (wb.DB.LightSources[i].Angles.x > 0.1) || (wb.DB.LightSources[i].Attenuation.r > 0.1))
			continue;

		//vec3  fromLightToSurface = (FragmentPosition.xyz - vec3(10.0f, 50.0f, 100.0f));
		vec3  fromLightToSurface = (FragmentPosition.xyz - (wb.DB.LightSources[i].Position.xyz * 10.0));

		vec3  reflectedLight     = reflect(normalize(fromLightToSurface), normal);
		float specularFactor     = pow(max(0.0, dot(reflectedLight, viewVector)), wb.DB.LightSources[i].Specular.r);
		vec3  specularHighlights = (wb.DB.LightSources[i].Diffuse.rgb * specularFactor * wb.DB.LightSources[i].Specular.a);

		GL_FragColor.rgb += (mix(reflectionColor, refractionColor, refractionFactor).rgb + specularHighlights);
	}

	// sRGB GAMMA CORRECTION
    if (wb.DB.EnableSRGB.x > 0.1) {
		float sRGB = (1.0 / 2.2);
		GL_FragColor.rgb = pow(GL_FragColor.rgb, vec3(sRGB, sRGB, sRGB));
	}

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
