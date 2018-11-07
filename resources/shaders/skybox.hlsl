static const int MAX_TEXTURES = 6;

struct MatrixBuffer
{
	matrix Model;
	matrix View;
	matrix Projection;
	matrix MVP;
};

cbuffer DefaultBuffer : register(b0)
{
	MatrixBuffer Matrices;
};

TextureCube  Textures[MAX_TEXTURES]        : register(t0);
SamplerState TextureSamplers[MAX_TEXTURES] : register(s0);

struct VS_INPUT
{
	float3 Normal        : NORMAL;
	float3 Position      : POSITION;
	float2 TextureCoords : TEXCOORD;
};

struct FS_INPUT
{
	float3 TextureCoords : TEXCOORD0;
	float4 GL_Position   : SV_POSITION;
};

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	output.TextureCoords = input.Position;
	output.GL_Position   = mul(float4(input.Position, 1.0), Matrices.MVP).xyww;

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	float4 GL_FragColor = Textures[0].Sample(TextureSamplers[0], input.TextureCoords);
	return GL_FragColor;
}
