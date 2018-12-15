static const int MAX_LIGHT_SOURCES = 16;
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
};

struct CBMatrix
{
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
};

Texture2D    Textures[MAX_TEXTURES]        : register(t0);
SamplerState TextureSamplers[MAX_TEXTURES] : register(s0);

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

// Attenuation = 1 / (c + l * d + q * d^2)
float GetAttenuationFactor(float3 lightPosition, float3 fragmentPosition, float3 attenuation)
{
	// Distance from fragment surface to the light source
    float distanceToLight = length(lightPosition - fragmentPosition);

	float constant     = attenuation.x;
	float linearFactor = (attenuation.y * distanceToLight);
	float quadratic    = (attenuation.z * distanceToLight * distanceToLight);
	
    return (1.0f / (constant + linearFactor + quadratic + 0.0001));
	//return (1.0f / (constant + linearFactor + 0.0001));
}

// Diffuse (color) - the impact of the light on the the fragment surface (angle difference)
float GetDiffuseFactor(float3 normal, float3 lightDirection)
{
	return max(dot(normal, lightDirection), 0.0);
}

// Specular - reflects/mirrors the light direction over the normal
float GetSpecularFactor(float3 lightDirection, float3 normal, float3 viewDirection)
{
    if (MeshSpecular.w < 1.0)
        return 0.0;

	//return pow(max(dot(viewDirection, reflect(-lightDirection, normal)), 0.0), MeshSpecular.w);	// Phong
	return pow(max(dot(normal, normalize(lightDirection + viewDirection)), 0.0), MeshSpecular.w);   // Blinn-Phong
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
float4 GetDirectionalLight(CBLight light, float3 normal, float3 viewDirection, float4 materialColor, float3 materialSpecIntensity)
{
	// Direction of the light from the fragment surface
    float3 lightDirection = normalize(-light.Direction.xyz);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection);
	
	// Shadow
	//float4 positionLightSpace = float4(light.light.viewProjectionMatrix * float4(FragmentPosition, 1.0));
	//float shadow             = (1.0 - calculateShadow(lightDirection, normal, light.shadowMapTexture2D, positionLightSpace));
	
    // Combine the light calculations
    float3 ambientFinal  = (light.Ambient.rgb  * materialColor.rgb);
    float3 diffuseFinal  = (light.Diffuse.rgb  * materialColor.rgb     * diffuseFactor);
    float3 specularFinal = (light.Specular.rgb * materialSpecIntensity * specularFactor);
	
    return float4((ambientFinal + diffuseFinal + specularFinal), materialColor.a);
    //return float4((ambientFinal + (shadow * (diffuseFinal + specularFinal))), materialColor.a);
}

float4 GetPointLight(CBLight light, float3 fragmentPosition, float3 normal, float3 viewDirection, float4 materialColor, float3 materialSpecIntensity)
{
	// Direction of the light from the fragment surface
    float3 lightDirection = normalize(light.Position.xyz - fragmentPosition);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection);
	
	// Attenuation = 1 / (constant + linear * distanceToLight + quadratic * distanceToLight^2)
    float attenuation = GetAttenuationFactor(light.Position.xyz, fragmentPosition, light.Attenuation.xyz);
	
	// Shadow
	//float shadow = (1.0 - GetShadow(light.Position, lightDirection, normal, light.ShadowMapTextureCube));
	
    // Combine the light calculations
    float3 ambient  = (attenuation * light.Ambient.rgb  * materialColor.rgb);
    float3 diffuse  = (attenuation * light.Diffuse.rgb  * diffuseFactor  * materialColor.rgb);
    float3 specular = (attenuation * light.Specular.rgb * specularFactor * materialSpecIntensity);
	
    return float4((ambient + diffuse + specular), materialColor.a);
    //return float4((ambient + (shadow * (diffuse + specular))), materialColor.a);
	//return GetShadow(light.position, lightDirection, normal, light.ShadowMapTextureCube);
}

float4 GetSpotLight(CBLight light, float3 fragmentPosition, float3 normal, float3 viewDirection, float4 materialColor, float3 materialSpecIntensity)
{
	// Direction of the light from the fragment surface
    float3 lightDirection = normalize(light.Position.xyz - fragmentPosition);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection);
	
	// Attenuation = 1 / (constant + linear * distanceToLight + quadratic * distanceToLight^2)
    float attenuationFactor = GetAttenuationFactor(light.Position.xyz, fragmentPosition, light.Attenuation.xyz);
	
	// Spotlight intensity
    float spotLightFactor = GetSpotLightFactor(light, lightDirection);
	
    // Combine the light calculations
    float3 ambient  = (spotLightFactor * attenuationFactor * light.Ambient.rgb  * materialColor.rgb);
    float3 diffuse  = (spotLightFactor * attenuationFactor * light.Diffuse.rgb  * diffuseFactor  * materialColor.rgb);
    float3 specular = (spotLightFactor * attenuationFactor * light.Specular.rgb * specularFactor * materialSpecIntensity);
	
    return float4((ambient + diffuse + specular), materialColor.a);
    //return (float4(ambient, 1.0) + ((1.0 - GetShadow()) * (diffuse + float4(specular, 1.0))));
}

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	//output.Normal           = mul(input.Normal, (float3x3)Matrices.Model);
	//output.Normal           = (float3)mul(float4(input.Normal, 0.0), Matrices.Model);

	output.FragmentNormal        = mul(float4(input.VertexNormal, 0.0), MB.Model).xyz;
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
    float3 materialSpecular;
	
	// MESH DIFFUSE (COLOR)
	if (IsTextured[0].x > 0.1)
	{
        float2 tiledCoordinates = float2(input.FragmentTextureCoords.x * TextureScales[0].x, input.FragmentTextureCoords.y * TextureScales[0].y);

        materialColor = Textures[0].Sample(TextureSamplers[0], tiledCoordinates);
    } else {
		materialColor = MeshDiffuse;
	}
	
	// MESH SPECULAR HIGHLIGHTS
	if (IsTextured[1].x > 0.1)
	{
        float2 tiledCoordinates = float2(input.FragmentTextureCoords.x * TextureScales[1].x, input.FragmentTextureCoords.y * TextureScales[1].y);

        materialSpecular = Textures[1].Sample(TextureSamplers[1], tiledCoordinates).rgb;
	} else {
		materialSpecular = MeshSpecular.xyz;
	}
	
	float3 normal        = normalize(input.FragmentNormal);	    				    	// Normal on the fragment surface
    float3 viewDirection = normalize(CameraPosition.xyz - input.FragmentPosition.xyz);  // The direction the camera is viewing the fragment surface
    float4 GL_FragColor  = float4(0, 0, 0, 0);

    // LIGHT SOURCES
    for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if (LightSources[i].Active.x < 0.1)
            continue;

    	// SPOT LIGHT
        if (LightSources[i].Angles.x > 0.1)
            GL_FragColor += GetSpotLight(LightSources[i], input.FragmentPosition.xyz, normal, viewDirection, materialColor, materialSpecular);
    	// POINT LIGHT
        else if (LightSources[i].Attenuation.r > 0.1)
            GL_FragColor += GetPointLight(LightSources[i], input.FragmentPosition.xyz, normal, viewDirection, materialColor, materialSpecular);
    	// DIRECTIONAL LIGHT
        else
            GL_FragColor += GetDirectionalLight(LightSources[i], normal, viewDirection, materialColor, materialSpecular);
    }

	// LIGHT COLOR (PHONG REFLECTION)
    //float3 lightColor = float3(SunLightAmbient + (SunLightDiffuse.rgb * dot(normalize(input.Normal), normalize(-SunLightDirection))));
    //GL_FragColor      = float4((MeshDiffuse.rgb * lightColor), MeshDiffuse.a);

    return GL_FragColor;
}
