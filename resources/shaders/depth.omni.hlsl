static const int MAX_TEXTURES      = 6;
static const int VERTICES_PER_FACE = 3;

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

struct GS_OUTPUT
{
	float4 FragmentPosition : POSITION0;
	float4 GL_Position      : SV_POSITION;
	uint   Layer            : SV_RenderTargetArrayIndex;
};

// VERTEX SHADER
float4 VS(VS_INPUT input) : SV_Position
{
	return mul(float4(input.VertexPosition, 1.0), MB.Model);
}

// GEOMETRY SHADER
[maxvertexcount(18)]
void GS(triangle float4 inputPosition[3] : SV_Position, inout TriangleStream<GS_OUTPUT> outputStream)
{
	for (int face = 0; face < MAX_TEXTURES; face++)
	{
		GS_OUTPUT output;

		output.Layer = face;

		for (int vertex = 0; vertex < VERTICES_PER_FACE; vertex++)
		{
			output.FragmentPosition = inputPosition[vertex];
			output.GL_Position      = mul(output.FragmentPosition, MB.VP[face]);

			outputStream.Append(output);
		}

		outputStream.RestartStrip();
	}
}

// FRAGMENT/PIXEL/COLOR SHADER
float PS(GS_OUTPUT input) : SV_Depth
{
	return (length(input.FragmentPosition.xyz - LightPosition.xyz) / 25.0);
}
