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
    matrix View;
    matrix Projection;
    matrix MVP;
};

cbuffer DefaultBuffer : register(b0)
{
    CBMatrix MB;

    CBLight LightSources[MAX_LIGHT_SOURCES];

    float4 IsTextured[MAX_TEXTURES];
    float4 TextureScales[MAX_TEXTURES];

    float4 CameraPosition;

    float4 MeshSpecular;
    float4 MeshDiffuse;

    float4 ClipMax;
    float4 ClipMin;
    float4 EnableClipping;

    float4 EnableSRGB;
};

Texture2D    Textures[MAX_TEXTURES]        : register(t0);
SamplerState TextureSamplers[MAX_TEXTURES] : register(s0);

Texture2D    DepthMapTextures2D[MAX_LIGHT_SOURCES] : register(t6);
SamplerState DepthMapTextures2DSampler             : register(s6);

TextureCube  DepthMapTexturesCube[MAX_LIGHT_SOURCES] : register(t19);
SamplerState DepthMapTexturesCubeSampler             : register(s7);

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
	float4 GL_Position           : SV_POSITION;
};

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

// Shadow - the impact of the light on the the fragment from the perspective of the directional light
float GetShadowFactor(float3 lightDirection, float3 normal, Texture2D depthMapTexture, SamplerState depthMapTextureSampler, float4 positionLightSpace)
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
    float2 texelSize;
    depthMapTexture.GetDimensions(texelSize.x, texelSize.y);
    texelSize = (1.0 / texelSize);
	
	// Sample surrounding texels (9 samples)
    for (int x = -SAMPLE_OFFSET; x <= SAMPLE_OFFSET; x++)
	{
        for (int y = -SAMPLE_OFFSET; y <= SAMPLE_OFFSET; y++)
		{
            float2 texel        = float2(shadowMapCoordinates.xy + (float2(x, y) * texelSize));
            float  closestDepth = depthMapTexture.Sample(depthMapTextureSampler, texel).r;
			
			// Check if the fragment is in shadow (0 = not in shadow, 1 = in shadow).
			// Compare current fragment with the surrounding texel's depth.
			shadowFactor += ((currentDepth - offsetBias) > closestDepth ? 1.0 : 0.0);
        }
	}
	
	// Average the results
    shadowFactor /= NR_OF_SAMPLES;
	
	// Reduce the shadow intensity (max 75%) by adding some ambience
	const float MAX_INTENSITY = 0.75;
	
	return min(shadowFactor, MAX_INTENSITY);
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
float4 GetDirectionalLight(int i, float3 fragmentPosition, float3 normal, float3 viewDirection, float4 materialColor, float4 materialSpec)
{
    CBLight light = LightSources[i];

	// Direction of the light from the fragment surface
    float3 lightDirection = normalize(-light.Direction.xyz);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
    float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection, materialSpec.a);
	
	// Shadow
    float4 positionLightSpace = mul(float4(fragmentPosition, 1.0), light.ViewProjection);
    float  shadowFactor       = (1.0 - GetShadowFactor(lightDirection, normal, DepthMapTextures2D[i], DepthMapTextures2DSampler, positionLightSpace));
	
    // Combine the light calculations
    float3 ambientFinal  = (light.Ambient.rgb  * materialColor.rgb);
    float3 diffuseFinal  = (light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    float3 specularFinal = (light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    //return float4((ambientFinal + diffuseFinal + specularFinal), materialColor.a);
    return float4((ambientFinal + (shadowFactor * (diffuseFinal + specularFinal))), materialColor.a);
}

float4 GetPointLight(int i, float3 fragmentPosition, float3 normal, float3 viewDirection, float4 materialColor, float4 materialSpec)
{
    CBLight light = LightSources[i];

	// Direction of the light from the fragment surface
    float3 lightDirection = normalize(light.Position.xyz - fragmentPosition);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
    float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection, materialSpec.a);
	
	// Attenuation = 1 / (constant + linear * distanceToLight + quadratic * distanceToLight^2)
    float attenuationFactor = GetAttenuationFactor(light.Position.xyz, fragmentPosition, light.Attenuation.xyz);
	
	// Shadow
	//float shadow = (1.0 - GetShadow(light.Position, lightDirection, normal, light.ShadowMapTextureCube));
	
    // Combine the light calculations
    float3 ambient  = (attenuationFactor * light.Ambient.rgb  * materialColor.rgb);
    float3 diffuse  = (attenuationFactor * light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    float3 specular = (attenuationFactor * light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    return float4((ambient + diffuse + specular), materialColor.a);
    //return float4((ambient + (shadow * (diffuse + specular))), materialColor.a);
	//return GetShadow(light.position, lightDirection, normal, light.ShadowMapTextureCube);
}

float4 GetSpotLight(int i, float3 fragmentPosition, float3 normal, float3 viewDirection, float4 materialColor, float4 materialSpec)
{
    CBLight light = LightSources[i];

	// Direction of the light from the fragment surface
    float3 lightDirection = normalize(light.Position.xyz - fragmentPosition);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
    float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection, materialSpec.a);
	
	// Attenuation = 1 / (constant + linear * distanceToLight + quadratic * distanceToLight^2)
    float attenuationFactor = GetAttenuationFactor(light.Position.xyz, fragmentPosition, light.Attenuation.xyz);
	
	// Spotlight intensity
    float spotLightFactor = GetSpotLightFactor(light, lightDirection);
	
    // Combine the light calculations
    float3 ambient  = (spotLightFactor * attenuationFactor * light.Ambient.rgb  * materialColor.rgb);
    float3 diffuse  = (spotLightFactor * attenuationFactor * light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    float3 specular = (spotLightFactor * attenuationFactor * light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    return float4((ambient + diffuse + specular), materialColor.a);
    //return (float4(ambient, 1.0) + ((1.0 - GetShadow()) * (diffuse + float4(specular, 1.0))));
}

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	//output.FragmentNormal        = mul(float4(input.VertexNormal, 0.0), MB.Model).xyz;
    //output.FragmentNormal        = (float3)mul(float4(input.VertexNormal, 0.0), MB.Normal);
    output.FragmentNormal        = mul(input.VertexNormal, (float3x3)MB.Normal);
    output.FragmentTextureCoords = input.VertexTextureCoords;
    output.FragmentPosition      = mul(float4(input.VertexPosition, 1.0), MB.Model);
	output.GL_Position           = mul(float4(input.VertexPosition, 1.0), MB.MVP);

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	if (EnableClipping.x > 0.1)
    {
		float4 p = input.FragmentPosition;

		if ((p.x > ClipMax.x) || (p.y > ClipMax.y) || (p.z > ClipMax.z) || (p.x < ClipMin.x) || (p.y < ClipMin.y) || (p.z < ClipMin.z))
			discard;
	}

	float4 materialColor;
    float4 materialSpecular;
	
	// MESH DIFFUSE (COLOR)
	if (IsTextured[0].x > 0.1) {
        float2 tiledCoordinates = float2(input.FragmentTextureCoords.x * TextureScales[0].x, input.FragmentTextureCoords.y * TextureScales[0].y);
        materialColor           = Textures[0].Sample(TextureSamplers[0], tiledCoordinates);
    } else {
		materialColor = MeshDiffuse;
	}
	
	// MESH SPECULAR HIGHLIGHTS
	if (IsTextured[1].x > 0.1) {
        float2 tiledCoordinates = float2(input.FragmentTextureCoords.x * TextureScales[1].x, input.FragmentTextureCoords.y * TextureScales[1].y);
        materialSpecular        = Textures[1].Sample(TextureSamplers[1], tiledCoordinates);
	} else {
		materialSpecular = MeshSpecular;
	}
	
	float3 normal        = normalize(input.FragmentNormal);	    				    	// Normal on the fragment surface
    float3 viewDirection = normalize(CameraPosition.xyz - input.FragmentPosition.xyz);  // The direction the camera is viewing the fragment surface
    float4 GL_FragColor  = float4(0, 0, 0, 0);

    // LIGHT SOURCES
    for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if (LightSources[i].Active.x > 0.1)
        {
    	    // SPOT LIGHT
            if (LightSources[i].Angles.x > 0.1)
                GL_FragColor += GetSpotLight(i, input.FragmentPosition.xyz, normal, viewDirection, materialColor, materialSpecular);
    	    // POINT LIGHT
            else if (LightSources[i].Attenuation.r > 0.1)
                GL_FragColor += GetPointLight(i, input.FragmentPosition.xyz, normal, viewDirection, materialColor, materialSpecular);
    	    // DIRECTIONAL LIGHT
            else
                GL_FragColor += GetDirectionalLight(i, input.FragmentPosition.xyz, normal, viewDirection, materialColor, materialSpecular);
        }
    }

	// LIGHT COLOR (PHONG REFLECTION)
    //float3 lightColor = float3(SunLightAmbient + (SunLightDiffuse.rgb * dot(normalize(input.Normal), normalize(-SunLightDirection))));
    //GL_FragColor      = float4((MeshDiffuse.rgb * lightColor), MeshDiffuse.a);

	// sRGB GAMMA CORRECTION
    if (EnableSRGB.x > 0.1) {
        float sRGB = (1.0 / 2.2);
        GL_FragColor.rgb = pow(GL_FragColor.rgb, float3(sRGB, sRGB, sRGB));
    }

    return GL_FragColor;
}
