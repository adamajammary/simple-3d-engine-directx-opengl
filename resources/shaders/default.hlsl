static const int MAX_TEXTURES = 6;

struct MatrixBuffer
{
	matrix Model;
	matrix View;
	matrix Projection;
	matrix MVP;
};

struct LightBuffer
{
	float4 Color;
	float3 Direction;
	float  Reflection;
	float3 Position;
	float  Shine;
};

cbuffer DefaultBuffer : register(b0)
{
	MatrixBuffer Matrices;
	LightBuffer  SunLight;
	float3       Ambient;
	int          EnableClipping;
	float3       ClipMax;
	int          IsTextured;
	float3       ClipMin;
	float        Padding1;
	float4       MaterialColor;
	float2       TextureScales[MAX_TEXTURES];	// tx = [ [x, y], [x, y], ... ];
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
	float3 Normal           : NORMAL0;
	float2 TextureCoords    : TEXCOORD0;
	float4 FragmentPosition : POSITION0;
	float4 GL_Position      : SV_POSITION;
};

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	//output.Normal           = mul(input.Normal, (float3x3)Matrices.Model);
	//output.Normal           = (float3)mul(float4(input.Normal, 0.0), Matrices.Model);

	output.Normal           = mul(float4(input.Normal,   0.0), Matrices.Model).xyz;
	output.TextureCoords    = input.TextureCoords;
	output.FragmentPosition = mul(float4(input.Position, 1.0), Matrices.Model);
	output.GL_Position      = mul(float4(input.Position, 1.0), Matrices.MVP);

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	if (EnableClipping) {
		float4 p = input.FragmentPosition;
		if ((p.x > ClipMax.x) || (p.y > ClipMax.y) || (p.z > ClipMax.z) || (p.x < ClipMin.x) || (p.y < ClipMin.y) || (p.z < ClipMin.z))
			discard;
	}
	
	float4 GL_FragColor;
	float2 tiledCoordinates = float2(input.TextureCoords.x * TextureScales[0].x, input.TextureCoords.y * TextureScales[0].y);

	// LIGHT COLOR (PHONG REFLECTION)
	float3 lightColor = float3(Ambient + (SunLight.Color.rgb * dot(normalize(input.Normal), normalize(-SunLight.Direction))));

	// TEXTURE
	if (IsTextured) {
		float4 texelColor = Textures[0].Sample(TextureSamplers[0], tiledCoordinates);
		GL_FragColor      = float4((texelColor.rgb * lightColor), texelColor.a);
	} else {
		GL_FragColor = float4((MaterialColor.rgb * lightColor), MaterialColor.a);
	}

	return GL_FragColor;
}
