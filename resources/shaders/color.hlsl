static const int MAX_TEXTURES = 6;

struct CBMatrix
{
    matrix Normal;
    matrix Model;
	matrix VP[MAX_TEXTURES];
	matrix MVP;
};

cbuffer ColorBuffer : register(b0)
{
    CBMatrix MB;
	float4   Color;
};

struct VS_INPUT
{
	float3 VertexNormal        : NORMAL;
	float3 VertexPosition      : POSITION;
	float2 VertexTextureCoords : TEXCOORD;
};

struct FS_INPUT
{
	float4 GL_Position : SV_POSITION;
};

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

    output.GL_Position = mul(float4(input.VertexPosition, 1.0), MB.MVP);

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	return Color;
}
