static const int MAX_TEXTURES = 6;

struct CBMatrix
{
    matrix Normal;
    matrix Model;
	matrix VP[MAX_TEXTURES];
	matrix MVP;
};

cbuffer SkyboxBuffer : register(b0)
{
    CBMatrix MB;
	float4   EnableSRGB;
};

TextureCube  Textures[MAX_TEXTURES]        : register(t0);
SamplerState TextureSamplers[MAX_TEXTURES] : register(s0);

struct VS_INPUT
{
	float3 VertexNormal        : NORMAL;
	float3 VertexPosition      : POSITION;
	float2 VertexTextureCoords : TEXCOORD;
};

struct FS_INPUT
{
	float3 FragmentTextureCoords : TEXCOORD0;
	float4 GL_Position           : SV_POSITION;
};

// sRGB GAMMA CORRECTION
float3 GetFragColorSRGB(float3 colorRGB)
{
	if (EnableSRGB.x > 0.1) {
		float sRGB = (1.0 / 2.2);
		colorRGB.rgb = pow(colorRGB.rgb, float3(sRGB, sRGB, sRGB));
	}

	return colorRGB;
}

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	output.FragmentTextureCoords = input.VertexPosition;
	output.GL_Position           = mul(float4(input.VertexPosition, 1.0), MB.MVP).xyww;

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	float4 GL_FragColor = Textures[0].Sample(TextureSamplers[0], input.FragmentTextureCoords);
	GL_FragColor.rgb    = GetFragColorSRGB(GL_FragColor.rgb);

	return GL_FragColor;
}
