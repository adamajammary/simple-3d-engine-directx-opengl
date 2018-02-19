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
	float4       MaterialColor;
	int          IsTransparent;
	float3       Padding1;
};

Texture2D    Textures[MAX_TEXTURES]        : register(t0);
SamplerState TextureSamplers[MAX_TEXTURES] : register(s0);

struct VS_INPUT
{
	float3 Normal        : NORMAL;
	float3 Position      : POSITION;
	float2 TextureCoords : TEXCOORD;
};

struct FS_INPUT
{
	float2 TextureCoords : TEXCOORD0;
	float4 GL_Position   : SV_POSITION;
};

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	output.TextureCoords = float2(((input.Position.x + 1.0) * 0.5), ((input.Position.y + 1.0) * 0.5));
	output.GL_Position   = mul(float4(input.Position.xy, 0.0, 1.0), Matrices.Model);

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	float4 GL_FragColor;
	float4 sampledColor = Textures[5].Sample(TextureSamplers[5], input.TextureCoords);

	if (IsTransparent)
		GL_FragColor = sampledColor;
	else
		GL_FragColor = float4(sampledColor.rgb, MaterialColor.a);

	return GL_FragColor;
}
