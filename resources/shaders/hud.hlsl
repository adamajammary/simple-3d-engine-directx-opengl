static const int MAX_TEXTURES = 6;

struct CBMatrix
{
    matrix Model;
    matrix View;
    matrix Projection;
    matrix MVP;
};

cbuffer HUDBuffer : register(b0)
{
    CBMatrix MB;

	float4 MaterialColor;
    float4 IsTransparent;
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
	float2 FragmentTextureCoords : TEXCOORD0;
	float4 GL_Position           : SV_POSITION;
};

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	output.FragmentTextureCoords = float2(((input.VertexPosition.x + 1.0) * 0.5), ((input.VertexPosition.y + 1.0) * 0.5));
	output.GL_Position   = mul(float4(input.VertexPosition.xy, 0.0, 1.0), MB.Model);

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	float4 GL_FragColor;
	float4 sampledColor = Textures[5].Sample(TextureSamplers[5], input.FragmentTextureCoords);

	if (IsTransparent.x > 0.1)
		GL_FragColor = sampledColor;
	else
		GL_FragColor = float4(sampledColor.rgb, MaterialColor.a);

	return GL_FragColor;
}
