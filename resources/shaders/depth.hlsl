struct CBMatrix
{
    matrix Normal;
    matrix Model;
    matrix View;
    matrix Projection;
    matrix MVP;
};

cbuffer DepthBuffer : register(b0)
{
    CBMatrix MB;
};

struct VS_INPUT
{
	float3 VertexNormal        : NORMAL;
	float3 VertexPosition      : POSITION;
	float2 VertexTextureCoords : TEXCOORD;
};

// VERTEX SHADER
float4 VS(VS_INPUT input) : SV_Position
{
    return mul(float4(input.VertexPosition, 1.0), MB.MVP);
}

// FRAGMENT/PIXEL/COLOR SHADER
//float PS(FS_INPUT input) : SV_Depth
void PS(float4 position : SV_Position)
{
	//return position.z;
}
