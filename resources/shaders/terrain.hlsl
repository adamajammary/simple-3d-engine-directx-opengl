static const int MAX_TEXTURES = 6;

struct CBMatrix
{
    matrix Model;
    matrix View;
    matrix Projection;
    matrix MVP;
};

cbuffer TerrainBuffer : register(b0)
{
    CBMatrix MB;

    float4 IsTextured[MAX_TEXTURES];
    float4 TextureScales[MAX_TEXTURES];

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

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

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
	
	float4 blendMapColor           = Textures[4].Sample(TextureSamplers[4], input.FragmentTextureCoords);
	float  backgroundTextureAmount = (1.0 - (blendMapColor.r + blendMapColor.g + blendMapColor.b));
	float2 tiledCoordinates        = float2(input.FragmentTextureCoords.x * TextureScales[0].x, input.FragmentTextureCoords.y * TextureScales[0].y);
	float4 backgroundTextureColor  = (Textures[0].Sample(TextureSamplers[0], tiledCoordinates) * backgroundTextureAmount);
	float4 rTextureColor           = (Textures[1].Sample(TextureSamplers[1], tiledCoordinates) * blendMapColor.r);
	float4 gTextureColor           = (Textures[2].Sample(TextureSamplers[2], tiledCoordinates) * blendMapColor.g);
	float4 bTextureColor           = (Textures[3].Sample(TextureSamplers[3], tiledCoordinates) * blendMapColor.b);
	float4 totalColor              = (backgroundTextureColor + rTextureColor + gTextureColor + bTextureColor);

	// LIGHT COLOR (PHONG REFLECTION)
	float3 lightColor   = (SunLightAmbient + (SunLightDiffuse.rgb * dot(normalize(input.FragmentNormal), normalize(-SunLightDirection))));
	float4 GL_FragColor = float4((totalColor.rgb * lightColor), totalColor.a);

	return GL_FragColor;
}
