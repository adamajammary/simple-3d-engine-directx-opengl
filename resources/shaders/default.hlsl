static const int MAX_TEXTURES = 6;

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

    float4 IsTextured[MAX_TEXTURES];
    float4 TextureScales[MAX_TEXTURES];

    float3 CameraPosition;
    float  CameraPadding1;

    float3 MeshSpecularIntensity;
	float  MeshSpecularShininess;

    float4 MeshDiffuse;

    float3 SunLightSpecularIntensity;
	float  SunLightSpecularShininess;

    float3 SunLightAmbient;
	float  SunLightPadding1;
    float4 SunLightDiffuse;

    float3 SunLightPosition;
	float  SunLightPadding2;
    float3 SunLightDirection;
	float  SunLightPadding3;

    float3 ClipMax;
    float  EnableClipping;
    float3 ClipMin;
    float  ClipPadding1;
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

// Diffuse (color) - the impact of the light on the the fragment surface (angle difference)
float GetDiffuseFactor(float3 normal, float3 lightDirection)
{
	return max(dot(normal, lightDirection), 0.0);
}

// Specular - reflects/mirrors the light direction over the normal
float GetSpecularFactor(float3 lightDirection, float3 normal, float3 viewDirection)
{
    if (MeshSpecularShininess < 1.0)
        return 0.0;

	//return pow(max(dot(viewDirection, reflect(-lightDirection, normal)), 0.0), MeshSpecularShininess);	// Phong
	return pow(max(dot(normal, normalize(lightDirection + viewDirection)), 0.0), MeshSpecularShininess);	// Blinn-Phong
}

// Directional light - all light rays have the same direction, independent of the location of the light source. Ex: sun light
float4 GetDirectionalLight(float3 normal, float3 viewDirection, float4 materialColor, float3 materialSpecIntensity)
{
	// Direction of the light from the fragment surface
    float3 lightDirection = normalize(-SunLightDirection);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection);
	
	// Shadow
	//float4 positionLightSpace = float4(light.light.viewProjectionMatrix * float4(FragmentPosition, 1.0));
	//float shadow             = (1.0 - calculateShadow(lightDirection, normal, light.shadowMapTexture2D, positionLightSpace));
	
    // Combine the light calculations
    float3 ambientFinal  = (SunLightAmbient * materialColor.rgb);
    float3 diffuseFinal  = (SunLightDiffuse.rgb * materialColor.rgb * diffuseFactor);
    float3 specularFinal = (SunLightSpecularIntensity * materialSpecIntensity * specularFactor);
	
    return float4((ambientFinal + diffuseFinal + specularFinal), materialColor.a);
    //return float4((ambientFinal + (shadow * (diffuseFinal + specularFinal))), materialColor.a);
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
		materialSpecular = MeshSpecularIntensity;
	}
	
	float3 normal        = normalize(input.FragmentNormal);					    	// Normal on the fragment surface
    float3 viewDirection = normalize(CameraPosition - input.FragmentPosition.xyz);  // The direction the camera is viewing the fragment surface
	
	// DIRECTIONAL LIGHT
    float4 GL_FragColor = GetDirectionalLight(normal, viewDirection, materialColor, materialSpecular);

	// LIGHT COLOR (PHONG REFLECTION)
    //float3 lightColor = float3(SunLightAmbient + (SunLightDiffuse.rgb * dot(normalize(input.Normal), normalize(-SunLightDirection))));
    //GL_FragColor      = float4((MeshDiffuse.rgb * lightColor), MeshDiffuse.a);

    return GL_FragColor;
}
