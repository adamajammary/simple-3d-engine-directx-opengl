static const int MAX_TEXTURES = 6;

struct CBMatrix
{
    matrix Normal;
    matrix Model;
	matrix VP[MAX_TEXTURES];
	matrix MVP;
};

cbuffer DepthBuffer : register(b0)
{
    CBMatrix MB;
	float4   LightPosition;
};

struct VS_INPUT
{
	float3 VertexNormal        : NORMAL;
	float3 VertexPosition      : POSITION;
	float2 VertexTextureCoords : TEXCOORD;
};

/*
struct GS_INPUT
{
	//
};

struct FS_INPUT
{
	float4 position   : SV_Position;
	float4 GL_FragPos : SV_POSITION;
};
*/

// VERTEX SHADER
float4 VS(VS_INPUT input) : SV_Position
{
    return mul(float4(input.VertexPosition, 1.0), MB.Model);
}

// GEOMETRY SHADER
/*float4 GS(GS_INPUT input) : SV_Position
{
	//
}
*/

// FRAGMENT/PIXEL/COLOR SHADER
//float PS(float4 position : SV_Position) : SV_Depth
void PS(float4 position : SV_Position)
{
	//gl_FragDepth = (length(GL_FragPos.xyz - LightPosition.xyz) / 25.0f);
}
