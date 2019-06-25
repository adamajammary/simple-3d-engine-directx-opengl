static const int MAX_LIGHT_SOURCES = 13;
static const int MAX_TEXTURES      = 6;

struct CBLight
{
    float4 Active;
    float4 Ambient;
    float4 Angles;
    float4 Attenuation;
    float4 Diffuse;
    float4 Direction;
    float4 Position;
    float4 Specular;
    matrix ViewProjection;
};

struct CBMatrix
{
    matrix Normal;
    matrix Model;
	matrix VP[MAX_TEXTURES];
	matrix MVP;
};

cbuffer DefaultBuffer : register(b0)
{
    CBMatrix MB;

    CBLight LightSources[MAX_LIGHT_SOURCES];

    float4 IsTextured[MAX_TEXTURES];
    float4 TextureScales[MAX_TEXTURES];

    float4 MeshSpecular;
    float4 MeshDiffuse;

    float4 ClipMax;
    float4 ClipMin;
    float4 EnableClipping;

	float4 CameraPosition;
	float4 ComponentType;
	float4 EnableSRGB;
	float4 WaterProps;
};

Texture2D    Textures[MAX_TEXTURES]        : register(t0);
SamplerState TextureSamplers[MAX_TEXTURES] : register(s0);

Texture2DArray DepthMapTextures2D        : register(t6);
SamplerState   DepthMapTextures2DSampler : register(s6);

TextureCubeArray DepthMapTexturesCube        : register(t7);
SamplerState     DepthMapTexturesCubeSampler : register(s7);

struct VS_INPUT
{
	float3 VertexNormal        : NORMAL;
	float3 VertexPosition      : POSITION;
	float2 VertexTextureCoords : TEXCOORD;
};

struct FS_INPUT
{
	float3 FragmentNormal        : NORMAL0;
	float2 FragmentTextureCoords : TEXCOORD0;
	float4 FragmentPosition      : POSITION0;
	float4 ClipSpace             : POSITION1;
	float4 GL_Position           : SV_POSITION;
};

static const int NR_OF_POINT_OFFSETS = 20;

static const float3 ShadowPointSampleOffsets[NR_OF_POINT_OFFSETS] = 
{
	float3(1, 1,  1), float3( 1, -1,  1), float3(-1, -1,  1), float3(-1, 1,  1),
	float3(1, 1, -1), float3( 1, -1, -1), float3(-1, -1, -1), float3(-1, 1, -1),
	float3(1, 1,  0), float3( 1, -1,  0), float3(-1, -1,  0), float3(-1, 1,  0),
	float3(1, 0,  1), float3(-1,  0,  1), float3( 1,  0, -1), float3(-1, 0, -1),
	float3(0, 1,  1), float3( 0, -1,  1), float3( 0, -1, -1), float3( 0, 1, -1)
};

bool ClipFragment(float3 fragPos)
{
	return ((EnableClipping.x > 0.1) && (
		(fragPos.x > ClipMax.x) || (fragPos.y > ClipMax.y) || (fragPos.z > ClipMax.z) ||
		(fragPos.x < ClipMin.x) || (fragPos.y < ClipMin.y) || (fragPos.z < ClipMin.z)
	));
}

// Attenuation = (1 / (c + (l * d) + (q * d^2))
float GetAttenuationFactor(float3 lightPosition, float3 fragmentPosition, float3 attenuation)
{
    float distanceToLight = length(lightPosition - fragmentPosition); // Distance from fragment surface to the light source
	float constantFactor  = attenuation.x;
    float linearFactor    = (attenuation.y * distanceToLight);
	
    // WITHOUT SRGB - LINEAR
    if (EnableSRGB.x < 0.1)
        return (1.0f / (constantFactor + linearFactor + 0.0001));

    // WITH SRGB - QUADRATIC
    float quadratic = (attenuation.z * distanceToLight * distanceToLight);

    return (1.0f / (constantFactor + linearFactor + quadratic + 0.0001));
}

// Diffuse (color) - the impact of the light on the the fragment surface (angle difference)
float GetDiffuseFactor(float3 normal, float3 lightDirection)
{
	return max(dot(normal, lightDirection), 0.0);
}

float2 GetTiledTexCoords(float2 fragTexCoords, float4 textureScale)
{
	return float2((fragTexCoords.x * textureScale.x), (fragTexCoords.y * textureScale.y));
}

// MESH DIFFUSE (COLOR)
float4 GetMaterialColor(float2 fragTexCoords)
{
	if (IsTextured[0].x > 0.1)
		return Textures[0].Sample(TextureSamplers[0], GetTiledTexCoords(fragTexCoords, TextureScales[0]));

	return MeshDiffuse;
}

// MESH SPECULAR HIGHLIGHTS
float4 GetMaterialSpecular(float2 fragTexCoords)
{
	if (IsTextured[1].x > 0.1)
		return Textures[1].Sample(TextureSamplers[1], GetTiledTexCoords(fragTexCoords, TextureScales[1]));

	return MeshSpecular;
}

float4 GetMaterialColorTerrain(float2 fragTexCoords)
{
	float4 blendMapColor       = Textures[4].Sample(TextureSamplers[4], fragTexCoords);
	float  backgroundTexAmount = (1.0 - (blendMapColor.r + blendMapColor.g + blendMapColor.b));
	float2 tiledCoords         = GetTiledTexCoords(fragTexCoords, TextureScales[0]);
	float4 backgroundTexColor  = (Textures[0].Sample(TextureSamplers[0], tiledCoords) * backgroundTexAmount);
	float4 rTextureColor       = (Textures[1].Sample(TextureSamplers[1], tiledCoords) * blendMapColor.r);
	float4 gTextureColor       = (Textures[2].Sample(TextureSamplers[2], tiledCoords) * blendMapColor.g);
	float4 bTextureColor       = (Textures[3].Sample(TextureSamplers[3], tiledCoords) * blendMapColor.b);

	return (backgroundTexColor + rTextureColor + gTextureColor + bTextureColor);
}

float4 GetMaterialColorWater(float2 fragTexCoords, float4 clipSpace, float3 cameraView, out float3 normal)
{
	// PROJECTIVE TEXTURE COORDS - NORMALIZED DEVICE SPACE
	float2 ndcReflectionTexCoords = ((float2(clipSpace.x, clipSpace.y)  / clipSpace.w) * 0.5 + 0.5); // [-1,1] => [0,1]
	float2 ndcRefractionTexCoords = ((float2(clipSpace.x, -clipSpace.y) / clipSpace.w) * 0.5 + 0.5); // [-1,1] => [0,1]

	// DU/DV MAP - DISTORTION
	float  moveFactor   = WaterProps.x;
	float  waveStrength = WaterProps.y;
	float2 tiledCoords  = GetTiledTexCoords(fragTexCoords, TextureScales[0]);

	float2 distortedTexCoords = (Textures[2].Sample(TextureSamplers[2], float2((tiledCoords.x + moveFactor), tiledCoords.y)).rg * 0.1);
	distortedTexCoords        = (tiledCoords + float2(distortedTexCoords.x, (distortedTexCoords.y + moveFactor)));

	float2 totalDistortion = ((Textures[2].Sample(TextureSamplers[2], distortedTexCoords).rg * 2.0 - 1.0) * waveStrength);

	ndcReflectionTexCoords += totalDistortion;
	ndcRefractionTexCoords += totalDistortion;
		
	float4 reflectionColor = Textures[0].Sample(TextureSamplers[0], ndcReflectionTexCoords);
	float4 refractionColor = Textures[1].Sample(TextureSamplers[1], ndcRefractionTexCoords);

	// NORMAL MAP
	float4 normalColor = Textures[3].Sample(TextureSamplers[3], distortedTexCoords);
	normal = normalize(float3((normalColor.r * 2.0 - 1.0), normalColor.b, (normalColor.g * 2.0 - 1.0)));

	// FRESNEL EFFECT - higher power => more refractive (transparent)
	float refractionFactor = clamp(pow(dot(cameraView, normal), 10.0), 0.0, 1.0);

	// MATERIAL COLOR
	return lerp(reflectionColor, refractionColor, refractionFactor);
}

// Shadow - the impact of the light on the the fragment from the perspective of the directional/spot light
float GetShadowFactor(int depthLayer, float3 lightDirection, float3 normal, float4 positionLightSpace)
{
	// Convert shadow map UV and Depth coordinates
    float3 shadowMapCoordinates = float3(positionLightSpace.xyz / positionLightSpace.w);    // Perspective projection divide
	shadowMapCoordinates        = ((shadowMapCoordinates * 0.5) + 0.5);					    // Convert coordinate range from [-1,1] to [0,1]

	// Get shadow map depth coordinates
	//float closestDepth = texture(depthMapTexture, shadowMapCoordinates.xy).r;	// Closest depth value from light's perspective
	float currentDepth = shadowMapCoordinates.z;	// Depth of current fragment from light's perspective

	// POBLEM:   Large dark region at the far end of the light source's frustum.
	// REASON:   Coordinates outside the far plane of the light's orthographic frustum.
	// SOLUTION: Force shadow values larger than 1.0 to 0 (not in shadow).
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
	const int   SAMPLE_OFFSET = 1;
	float       shadowFactor  = 0.0;

	// Width and height of the sampler texture at mipmap level 0
	float  arrayCount;
	float2 texelSize;

    DepthMapTextures2D.GetDimensions(texelSize.x, texelSize.y, arrayCount);
	texelSize = (1.0 / texelSize);
	
	// Sample surrounding texels (9 samples)
    for (int x = -SAMPLE_OFFSET; x <= SAMPLE_OFFSET; x++)
	{
        for (int y = -SAMPLE_OFFSET; y <= SAMPLE_OFFSET; y++)
		{
            float2 texel        = float2(shadowMapCoordinates.xy + (float2(x, y) * texelSize));
            float  closestDepth = DepthMapTextures2D.Sample(DepthMapTextures2DSampler, float3(texel, depthLayer)).r;

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
float GetShadowFactorOmni(int depthLayer, float3 fragPos, float3 lightPosition)
{
	// PROBLEM:  Shadow acne.
	// REASON:   Because the shadow map is limited by resolution,
	//           multiple fragments can sample the same value from the depth map
	//         	 when they're relatively far away from the light source.
	// SOLUTION: Offset depth.
    float offsetBias = 0.15;

	// PCF - Percentage-Closer Filtering
	// Produces softer shadows, making them appear less blocky or hard.
	float3 fragToLight  = (fragPos - lightPosition);
	float  currentDepth = length(fragToLight);
    float  viewDistance = length(CameraPosition.xyz - fragPos);
    float  offsetRadius = ((1.0 + (viewDistance / 25.0)) / 25.0);
	float  shadowFactor = 0.0;

	// Add offsets to a radius around the original fragToLight direction vector to sample from the cubemap
	for(int s = 0; s < NR_OF_POINT_OFFSETS; s++)
    {
		float3 texel        = (fragToLight + ShadowPointSampleOffsets[s] * offsetRadius);
        float  closestDepth = (DepthMapTexturesCube.Sample(DepthMapTexturesCubeSampler, float4(texel, depthLayer)).r * 25.0);

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
float GetSpecularFactor(float3 lightDirection, float3 normal, float3 viewDirection, float shininess)
{
    if (shininess < 0.1)
        return 0.0;

	//return pow(max(dot(viewDirection, reflect(-lightDirection, normal)), 0.0), shininess);	// Phong
    return pow(max(dot(normal, normalize(lightDirection + viewDirection)), 0.0), shininess);    // Blinn-Phong
}

// Spot Light - the impact of light inside the cone
// I = ((theta - outer) / epsilon)
float GetSpotLightFactor(CBLight light, float3 lightDirection)
{
    float theta   = dot(lightDirection, normalize(-light.Direction.xyz));   // Angle between light and spot directions
    float epsilon = (light.Angles.x - light.Angles.y);                      // Angle between inner and outer cones
	
    return clamp(((theta - light.Angles.y) / epsilon), 0.0, 1.0);
}

// Directional light - all light rays have the same direction, independent of the location of the light source. Ex: sun light
float4 GetDirectionalLight(int i, float3 fragPos, float3 normal, float3 viewDirection, float4 materialColor, float4 materialSpec)
{
    CBLight light = LightSources[i];

	// Direction of the light from the fragment surface
    float3 lightDirection = normalize(-light.Direction.xyz);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
    float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection, materialSpec.a);
	
	// Shadow
    float4 positionLightSpace = mul(float4(fragPos, 1.0), light.ViewProjection);
    float  shadowFactor       = (1.0 - GetShadowFactor(i, lightDirection, normal, positionLightSpace));
	
    // Combine the light calculations
    float3 ambientFinal  = (light.Ambient.rgb  * materialColor.rgb);
    float3 diffuseFinal  = (light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    float3 specularFinal = (light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    //return float4((ambientFinal + diffuseFinal + specularFinal), materialColor.a);
    return float4((ambientFinal + (shadowFactor * (diffuseFinal + specularFinal))), materialColor.a);
}

float4 GetPointLight(int i, float3 fragPos, float3 normal, float3 viewDirection, float4 materialColor, float4 materialSpec)
{
    CBLight light = LightSources[i];

	// Direction of the light from the fragment surface
    float3 lightDirection = normalize(light.Position.xyz - fragPos);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
    float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection, materialSpec.a);
	
	// Attenuation = 1 / (constant + linear * distanceToLight + quadratic * distanceToLight^2)
    float attenuationFactor = GetAttenuationFactor(light.Position.xyz, fragPos, light.Attenuation.xyz);
	
	// Shadow
	float shadowFactor = (1.0 - GetShadowFactorOmni(i, fragPos, light.Position.xyz));

    // Combine the light calculations
    float3 ambient  = (attenuationFactor * light.Ambient.rgb  * materialColor.rgb);
    float3 diffuse  = (attenuationFactor * light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    float3 specular = (attenuationFactor * light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    //return float4((ambient + diffuse + specular), materialColor.a);
    return float4((ambient + (shadowFactor * (diffuse + specular))), materialColor.a);
}

float4 GetSpotLight(int i, float3 fragPos, float3 normal, float3 viewDirection, float4 materialColor, float4 materialSpec)
{
    CBLight light = LightSources[i];

	// Direction of the light from the fragment surface
    float3 lightDirection = normalize(light.Position.xyz - fragPos);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
    float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection, materialSpec.a);
	
	// Attenuation = 1 / (constant + linear * distanceToLight + quadratic * distanceToLight^2)
    float attenuationFactor = GetAttenuationFactor(light.Position.xyz, fragPos, light.Attenuation.xyz);

	// Spotlight intensity
    float spotLightFactor = GetSpotLightFactor(light, lightDirection);

	// Shadow
    float4 positionLightSpace = mul(float4(fragPos, 1.0), light.ViewProjection);
    float  shadowFactor       = (1.0 - GetShadowFactor(i, lightDirection, normal, positionLightSpace));

    // Combine the light calculations
    float3 ambient  = (spotLightFactor * attenuationFactor * light.Ambient.rgb  * materialColor.rgb);
    float3 diffuse  = (spotLightFactor * attenuationFactor * light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    float3 specular = (spotLightFactor * attenuationFactor * light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    //return float4((ambient + diffuse + specular), materialColor.a);
	return float4((ambient + (shadowFactor * (diffuse + specular))), materialColor.a);
}

// HDR (HIGH DYNAMIC RANGE) - TONE MAPPING (REINHARD)
float3 GetFragColorHDR(float3 colorRGB)
{
    return (colorRGB / (colorRGB + float3(1.0, 1.0, 1.0)));
}

// sRGB GAMMA CORRECTION
float3 GetFragColorSRGB(float3 colorRGB)
{
	if (EnableSRGB.x > 0.1) {
		float sRGB = (1.0 / 2.2);
		colorRGB.rgb = pow(colorRGB.rgb, float3(sRGB, sRGB, sRGB));
	}

	return colorRGB;
}

float4 GetFragColorLight(float3 fragPos, float4 materialColor, float4 materialSpecular, float3 cameraView, float3 normal)
{
	float4 fragColor = float4(0.0, 0.0, 0.0, 0.0);

    // LIGHT SOURCES
    for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if (LightSources[i].Active.x > 0.1)
		{
    		// ID_ICON_LIGHT_SPOT = 17
			if (LightSources[i].Active.y > 16.9)
				fragColor += GetSpotLight(i, fragPos, normal, cameraView, materialColor, materialSpecular);
			// ID_ICON_LIGHT_POINT = 16
			else if (LightSources[i].Active.y > 15.9)
				fragColor += GetPointLight(i, fragPos, normal, cameraView, materialColor, materialSpecular);
			// ID_ICON_LIGHT_DIRECTIONAL = 15
			else
				fragColor += GetDirectionalLight(i, fragPos, normal, cameraView, materialColor, materialSpecular);
		}
    }

    fragColor.rgb = GetFragColorHDR(fragColor.rgb);
	fragColor.rgb = GetFragColorSRGB(fragColor.rgb);

	return fragColor;
}

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	//output.FragmentNormal = mul(float4(input.VertexNormal, 0.0), MB.Model).xyz;
	//output.FragmentNormal = (float3)mul(float4(input.VertexNormal, 0.0), MB.Normal);
	output.FragmentNormal        = mul(input.VertexNormal, (float3x3)MB.Normal);
	output.FragmentTextureCoords = input.VertexTextureCoords;
    output.FragmentPosition      = mul(float4(input.VertexPosition, 1.0), MB.Model);
	output.ClipSpace             = mul(float4(input.VertexPosition, 1.0), MB.MVP);
	output.GL_Position           = output.ClipSpace;

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	if (ClipFragment(input.FragmentPosition.xyz))
		discard;

	float3 cameraView = normalize(CameraPosition.xyz - input.FragmentPosition.xyz);
	float3 normal     = normalize(input.FragmentNormal);
	float4 color      = float4(0.0, 0.0, 0.0, 0.0);
	float4 specular   = float4(0.0, 0.0, 0.0, 0.0);

	// COMPONENT_WATER = 6
	if (ComponentType.x > 5.9) {
		color    = GetMaterialColorWater(input.FragmentTextureCoords, input.ClipSpace, cameraView, normal);
		specular = MeshSpecular;
	// COMPONENT_TERRAIN = 5
	} else if (ComponentType.x > 4.9) {
		color    = GetMaterialColorTerrain(input.FragmentTextureCoords);
		specular = MeshSpecular;
	// COMPONENT_MODEL = 3, COMPONENT_MESH = 2
	} else {
		color    = GetMaterialColor(input.FragmentTextureCoords);
		specular = GetMaterialSpecular(input.FragmentTextureCoords);
	}

	return GetFragColorLight(input.FragmentPosition.xyz, color, specular, cameraView, normal);
}
