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

layout(location = 0) in vec3 FragmentNormal;
layout(location = 1) in vec4 FragmentPosition;
layout(location = 2) in vec2 FragmentTextureCoords;
layout(location = 3) in vec4 ClipSpace;

layout(location = 0) out vec4 GL_FragColor;

layout(binding = 1) uniform DefaultBuffer
{
    CBLight LightSources[MAX_LIGHT_SOURCES];

    vec4 IsTextured[MAX_TEXTURES];
    vec4 TextureScales[MAX_TEXTURES];

    vec4 MeshSpecular;
    vec4 MeshDiffuse;

    vec4 ClipMax;
    vec4 ClipMin;
	vec4 EnableClipping;

	vec4 CameraPosition;
	vec4 ComponentType;
    vec4 EnableSRGB;
	vec4 WaterProps;
} db;

layout(binding = 2) uniform sampler2D        Textures[MAX_TEXTURES];
layout(binding = 3) uniform sampler2DArray   DepthMapTextures2D;
layout(binding = 4) uniform samplerCubeArray DepthMapTexturesCube;

const int NR_OF_POINT_OFFSETS = 20;

const vec3 ShadowPointSampleOffsets[NR_OF_POINT_OFFSETS] =
{
	vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
	vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
	vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
	vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
};

bool ClipFragment()
{
	vec4 fp = FragmentPosition;

	return ((db.EnableClipping.x > 0.1) && (
		(fp.x > db.ClipMax.x) || (fp.y > db.ClipMax.y) || (fp.z > db.ClipMax.z) ||
		(fp.x < db.ClipMin.x) || (fp.y < db.ClipMin.y) || (fp.z < db.ClipMin.z)
	));
}

// Attenuation = (1 / (c + (l * d) + (q * d^2))
float GetAttenuationFactor(vec3 lightPosition, vec3 attenuation)
{
    float distanceToLight = length(lightPosition - FragmentPosition.xyz); // Distance from fragment surface to the light source
	float constantFactor  = attenuation.x;
    float linearFactor    = (attenuation.y * distanceToLight);
	
    // WITHOUT SRGB - LINEAR
    if (db.EnableSRGB.x < 0.1)
        return (1.0f / (constantFactor + linearFactor + 0.0001));

    // WITH SRGB - QUADRATIC
    float quadratic = (attenuation.z * distanceToLight * distanceToLight);

    return (1.0f / (constantFactor + linearFactor + quadratic + 0.0001));
}

// Diffuse (color) - the impact of the light on the the fragment surface (angle difference)
float GetDiffuseFactor(vec3 normal, vec3 lightDirection)
{
	return max(dot(normal, lightDirection), 0.0);
}

vec2 GetTiledTexCoords(vec4 textureScale)
{
	return vec2((FragmentTextureCoords.x * textureScale.x), (FragmentTextureCoords.y * textureScale.y));
}

// MESH DIFFUSE (COLOR)
vec4 GetMaterialColor()
{
	if (db.IsTextured[0].x > 0.1)
		return texture(Textures[0], GetTiledTexCoords(db.TextureScales[0]));

	return db.MeshDiffuse;
}

vec4 GetMaterialColorTerrain()
{
	vec4  blendMapColor       = texture(Textures[4], FragmentTextureCoords);
	float backgroundTexAmount = (1.0 - (blendMapColor.r + blendMapColor.g + blendMapColor.b));
	vec2  tiledCoords         = GetTiledTexCoords(db.TextureScales[0]);
	vec4  backgroundTexColor  = (texture(Textures[0], tiledCoords) * backgroundTexAmount);
	vec4  rTextureColor       = (texture(Textures[1], tiledCoords) * blendMapColor.r);
	vec4  gTextureColor       = (texture(Textures[2], tiledCoords) * blendMapColor.g);
	vec4  bTextureColor       = (texture(Textures[3], tiledCoords) * blendMapColor.b);

	return (backgroundTexColor + rTextureColor + gTextureColor + bTextureColor);
}

vec4 GetMaterialColorWater(vec3 cameraView, out vec3 normal)
{
	// PROJECTIVE TEXTURE COORDS - NORMALIZED DEVICE SPACE
	vec2 ndcReflectionTexCoords = ((vec2(ClipSpace.x, -ClipSpace.y) / ClipSpace.w) * 0.5 + 0.5); // [-1,1] => [0,1]
	vec2 ndcRefractionTexCoords = ((vec2(ClipSpace.x, ClipSpace.y)  / ClipSpace.w) * 0.5 + 0.5); // [-1,1] => [0,1]

	// DU/DV MAP - DISTORTION
	float moveFactor   = db.WaterProps.x;
	float waveStrength = db.WaterProps.y;
	vec2  tiledCoords  = GetTiledTexCoords(db.TextureScales[0]);

	vec2 distortedTexCoords = (texture(Textures[2], vec2((tiledCoords.x + moveFactor), tiledCoords.y)).rg * 0.1);
	distortedTexCoords      = (tiledCoords + vec2(distortedTexCoords.x, (distortedTexCoords.y + moveFactor)));

	vec2 totalDistortion = ((texture(Textures[2], distortedTexCoords).rg * 2.0 - 1.0) * waveStrength);

	ndcReflectionTexCoords += totalDistortion;
	ndcRefractionTexCoords += totalDistortion;
		
	vec4 reflectionColor = texture(Textures[0], ndcReflectionTexCoords);
	vec4 refractionColor = texture(Textures[1], ndcRefractionTexCoords);

	// NORMAL MAP
	vec4 normalColor = texture(Textures[3], distortedTexCoords);
	normal = normalize(vec3((normalColor.r * 2.0 - 1.0), normalColor.b, (normalColor.g * 2.0 - 1.0)));

	// FRESNEL EFFECT - higher power => more refractive (transparent)
	//float refractionFactor = clamp(pow(dot(cameraView, normal), 10.0), 0.0, 1.0);
	float refractionFactor = clamp(dot(cameraView, normal), 0.0, 1.0);

	// MATERIAL COLOR
	return mix(reflectionColor, refractionColor, refractionFactor);
}

// MESH SPECULAR HIGHLIGHTS
vec4 GetMaterialSpecular()
{
	if (db.IsTextured[1].x > 0.1)
		return texture(Textures[1], GetTiledTexCoords(db.TextureScales[1]));

	return db.MeshSpecular;
}

// Shadow - the impact of the light on the the fragment from the perspective of the directional/spot light
float GetShadowFactor(int depthLayer, vec3 lightDirection, vec3 normal, vec4 positionLightSpace)
{
	// Convert shadow map UV and Depth coordinates
	vec3 shadowMapCoordinates = vec3(positionLightSpace.xyz / positionLightSpace.w); // Perspective projection divide
	shadowMapCoordinates      = ((shadowMapCoordinates * 0.5) + 0.5); // Convert coordinate range from [-1,1] to [0,1]

	// Get shadow map depth coordinates
	//float closestDepth = texture(DepthMapTextures2D, vec3(shadowMapCoordinates.xy, i)).r;	// Closest depth value from light's perspective
	float currentDepth = shadowMapCoordinates.z; // Depth of current fragment from light's perspective

	// POBLEM:   Large dark region at the far end of the light source's frustum.
	// REASON:   Coordinates outside the far plane of the light's orthographic frustum.
	// SOLUTION: Force shadow values larger than 1 to 0 (not in shadow).
	if (currentDepth > 1.0)
		return 0.0;
	
	// PROBLEM:  Shadow acne.
	// REASON:   Because the shadow map is limited by resolution,
	//           multiple fragments can sample the same value from the depth map
	//         	 when they're relatively far away from the light source.
	// SOLUTION: Offset depth.
	const float MIN_OFFSET = 0.005;
	const float MAX_OFFSET = 0.05;
	float       offsetBias = max(MAX_OFFSET * (1.0 - dot(normal, lightDirection)), MIN_OFFSET);
	
	// PCF - Percentage-Closer Filtering
	// Produces softer shadows, making them appear less blocky or hard.
	const float NR_OF_SAMPLES = 9.0;
	const float SAMPLE_OFFSET = 1.0;
	float       shadowFactor  = 0.0;

	// Pixel size of each texel in the sampler texture at mipmap level 0
	vec2 texelSize = (1.0 / textureSize(DepthMapTextures2D, 0).xy);
	
	// Sample surrounding texels (9 samples)
	for (float x = -SAMPLE_OFFSET; x <= SAMPLE_OFFSET; x += 1.0)
	{
		for (float y = -SAMPLE_OFFSET; y <= SAMPLE_OFFSET; y += 1.0)
		{
			vec2  texel        = vec2(shadowMapCoordinates.xy + (vec2(x, y) * texelSize));
			float closestDepth = texture(DepthMapTextures2D, vec3(texel, depthLayer)).r;

			// Check if the fragment is in shadow (0 = not in shadow, 1 = in shadow).
			// Compare current fragment with the surrounding texel's depth.
			if ((currentDepth - offsetBias) > closestDepth)
				shadowFactor += 1.0;
		}
	}
	
	// Average the results
	shadowFactor /= NR_OF_SAMPLES;
	
	// Reduce the shadow intensity (max 75%) by adding some ambience
	const float MAX_INTENSITY = 0.75;
	
	return min(shadowFactor, MAX_INTENSITY);
}

// Shadow - the impact of the light on the the fragment from the perspective of the point light
float GetShadowFactorOmni(int depthLayer, vec3 lightPosition)
{
	// PROBLEM:  Shadow acne.
	// REASON:   Because the shadow map is limited by resolution,
	//           multiple fragments can sample the same value from the depth map
	//         	 when they're relatively far away from the light source.
	// SOLUTION: Offset depth.
    float offsetBias = 0.15;

	// PCF - Percentage-Closer Filtering
	// Produces softer shadows, making them appear less blocky or hard.
	vec3  fragToLight  = (FragmentPosition.xyz - lightPosition);
	float currentDepth = length(fragToLight);
    float viewDistance = length(db.CameraPosition.xyz - FragmentPosition.xyz);
    float offsetRadius = ((1.0 + (viewDistance / 25.0)) / 25.0);
	float shadowFactor = 0.0;

	// Add offsets to a radius around the original fragToLight direction vector to sample from the cubemap
	for(int s = 0; s < NR_OF_POINT_OFFSETS; s++)
    {
		vec3  texel        = (fragToLight + ShadowPointSampleOffsets[s] * offsetRadius);
        float closestDepth = (texture(DepthMapTexturesCube, vec4(texel, depthLayer)).r * 25.0);

		// Check if the fragment is in shadow (0 = not in shadow, 1 = in shadow).
		// Compare current fragment with the surrounding texel's depth.
		if ((currentDepth - offsetBias) > closestDepth)
            shadowFactor += 1.0;
    }

	// Average the results
    shadowFactor /= float(NR_OF_POINT_OFFSETS);

    return shadowFactor;
}

// Specular - reflects/mirrors the light direction over the normal
float GetSpecularFactor(vec3 lightDirection, vec3 normal, vec3 cameraView, float shininess)
{
	if (shininess < 0.1)
		return 0.0;

	//return pow(max(dot(viewDirection, reflect(-lightDirection, normal)), 0.0), shininess); // Phong
	return pow(max(dot(normal, normalize(lightDirection + cameraView)), 0.0), shininess); // Blinn-Phong
}

// Spot Light - the impact of light inside the cone
// I = ((theta - outer) / epsilon)
float GetSpotLightFactor(CBLight light, vec3 lightDirection)
{
	float theta   = dot(lightDirection, normalize(-light.Direction.xyz));	// Angle between light and spot directions
    float epsilon = (light.Angles.x - light.Angles.y);						// Angle between inner and outer cones
	
	return clamp(((theta - light.Angles.y) / epsilon), 0.0, 1.0);
}

// Directional light - all light rays have the same direction, independent of the location of the light source. Ex: sun light
vec4 GetDirectionalLight(int i, vec3 normal, vec3 cameraView, vec4 materialColor, vec4 materialSpec)
{
	CBLight light = db.LightSources[i];

	// Direction of the light from the fragment surface
	vec3 lightDirection = normalize(-light.Direction.xyz);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, cameraView, materialSpec.a);
	
	// Shadow
	vec4  positionLightSpace = (light.ViewProjection * vec4(FragmentPosition.xyz, 1.0));
	float shadowFactor       = (1.0 - GetShadowFactor(i, lightDirection, normal, positionLightSpace));

    // Combine the light calculations
    vec3 ambientFinal  = (light.Ambient.rgb  * materialColor.rgb);
    vec3 diffuseFinal  = (light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    vec3 specularFinal = (light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    //return vec4((ambientFinal + diffuseFinal + specularFinal), materialColor.a);
    return vec4((ambientFinal + (shadowFactor * (diffuseFinal + specularFinal))), materialColor.a);
}

vec4 GetPointLight(int i, vec3 normal, vec3 cameraView, vec4 materialColor, vec4 materialSpec)
{
	CBLight light = db.LightSources[i];

	// Direction of the light from the fragment surface
	vec3 lightDirection = normalize(light.Position.xyz - FragmentPosition.xyz);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, cameraView, materialSpec.a);
	
	// Attenuation = 1 / (constant + linear * distanceToLight + quadratic * distanceToLight^2)
	float attenuationFactor = GetAttenuationFactor(light.Position.xyz, light.Attenuation.xyz);
	
	// Shadow
	float shadowFactor = (1.0 - GetShadowFactorOmni(i, light.Position.xyz));
	
    // Combine the light calculations
    vec3 ambient  = (attenuationFactor * light.Ambient.rgb  * materialColor.rgb);
    vec3 diffuse  = (attenuationFactor * light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    vec3 specular = (attenuationFactor * light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    //return vec4((ambient + diffuse + specular), materialColor.a);
    return vec4((ambient + (shadowFactor * (diffuse + specular))), materialColor.a);
}

vec4 GetSpotLight(int i, vec3 normal, vec3 cameraView, vec4 materialColor, vec4 materialSpec)
{
	CBLight light = db.LightSources[i];

	// Direction of the light from the fragment surface
	vec3 lightDirection = normalize(light.Position.xyz - FragmentPosition.xyz);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, cameraView, materialSpec.a);
	
	// Attenuation = 1 / (constant + linear * distanceToLight + quadratic * distanceToLight^2)
	float attenuationFactor = GetAttenuationFactor(light.Position.xyz, light.Attenuation.xyz);
	
	// Spotlight intensity
	float spotLightFactor = GetSpotLightFactor(light, lightDirection);
	
	// Shadow
	vec4  positionLightSpace = (light.ViewProjection * vec4(FragmentPosition.xyz, 1.0));
	float shadowFactor       = (1.0 - GetShadowFactor(i, lightDirection, normal, positionLightSpace));
	
    // Combine the light calculations
    vec3 ambient  = (spotLightFactor * attenuationFactor * light.Ambient.rgb  * materialColor.rgb);
    vec3 diffuse  = (spotLightFactor * attenuationFactor * light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    vec3 specular = (spotLightFactor * attenuationFactor * light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    //return vec4((ambient + diffuse + specular), materialColor.a);
    return vec4((ambient + (shadowFactor * (diffuse + specular))), materialColor.a);
}

// HDR (HIGH DYNAMIC RANGE) - TONE MAPPING (REINHARD)
vec3 GetFragColorHDR(vec3 colorRGB)
{
    return (colorRGB / (colorRGB + vec3(1.0)));
}

// sRGB GAMMA CORRECTION
vec3 GetFragColorSRGB(vec3 colorRGB)
{
	if (db.EnableSRGB.x > 0.1) {
		float sRGB = (1.0 / 2.2);
		colorRGB.rgb = pow(colorRGB.rgb, vec3(sRGB, sRGB, sRGB));
	}

	return colorRGB;
}

vec4 GetFragColorLight(vec4 materialColor, vec4 materialSpecular, vec3 cameraView, vec3 normal)
{
	vec4 fragColor = vec4(0);

    // LIGHT SOURCES
    for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if (db.LightSources[i].Active.x > 0.1)
		{
			// SPOT = 2
			if (db.LightSources[i].Active.y > 1.9)
				fragColor += GetSpotLight(i, normal, cameraView, materialColor, materialSpecular);
			// POINT = 1
			else if (db.LightSources[i].Active.y > 0.9)
				fragColor += GetPointLight(i, normal, cameraView, materialColor, materialSpecular);
			// DIRECTIONAL = 0
			else
				fragColor += GetDirectionalLight(i, normal, cameraView, materialColor, materialSpecular);
		}
    }

    fragColor.rgb = GetFragColorHDR(fragColor.rgb);
	fragColor.rgb = GetFragColorSRGB(fragColor.rgb);

	return fragColor;
}

void main()
{
	if (ClipFragment())
		discard;

	vec3 cameraView = normalize(db.CameraPosition.xyz - FragmentPosition.xyz);
	vec3 normal     = normalize(FragmentNormal);
	vec4 color      = vec4(0);
	vec4 specular   = vec4(0);

	// COMPONENT_WATER = 6
    if (db.ComponentType.x > 5.9) {
		color    = GetMaterialColorWater(cameraView, normal);
		specular = db.MeshSpecular;
	// COMPONENT_TERRAIN = 5
    } else if (db.ComponentType.x > 4.9) {
		color = GetMaterialColorTerrain();
		specular = db.MeshSpecular;
	// COMPONENT_MODEL = 3, COMPONENT_MESH = 2
	} else {
		color    = GetMaterialColor();
		specular = GetMaterialSpecular();
	}

	GL_FragColor = GetFragColorLight(color, specular, cameraView, normal);
}
