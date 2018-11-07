struct MatrixBuffer
{
	matrix Model;
	matrix View;
	matrix Projection;
	matrix MVP;
};

cbuffer WireframeBuffer : register(b0)
{
	MatrixBuffer Matrices;
	float4       Color;
};

struct VS_INPUT
{
	float3 Normal        : NORMAL;
	float3 Position      : POSITION;
	float2 TextureCoords : TEXCOORD;
};

struct FS_INPUT
{
	float4 GL_Position : SV_POSITION;
};

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	output.GL_Position = mul(float4(input.Position, 1.0), Matrices.MVP);

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	return Color;
}
