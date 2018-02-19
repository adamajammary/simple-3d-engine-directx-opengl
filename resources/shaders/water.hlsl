static const int MAX_TEXTURES = 6;

struct CameraBuffer
{
	float3 Position;
	float  Near;
	float  Far;
	float3 Padding1;
};

struct LightBuffer
{
	float4 Color;
	float3 Direction;
	float  Reflection;
	float3 Position;
	float  Shine;
};

struct MatrixBuffer
{
	matrix Model;
	matrix View;
	matrix Projection;
	matrix MVP;
};

cbuffer DefaultBuffer : register(b0)
{
	CameraBuffer CameraMain;
	MatrixBuffer Matrices;
	LightBuffer  SunLight;
	int          EnableClipping;
	float3       ClipMax;
	float3       ClipMin;
	float        MoveFactor;
	float        WaveStrength;
	float3       Padding1;
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
	float2 TextureCoords    : TEXCOORD0;
	float4 FragmentPosition : POSITION0;
	float4 ClipSpace        : POSITION1;
	float4 GL_Position      : SV_POSITION;
};

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	output.TextureCoords    = input.TextureCoords;
	output.FragmentPosition = mul(float4(input.Position, 1.0), Matrices.Model);
	output.ClipSpace        = mul(float4(input.Position, 1.0), Matrices.MVP);
	output.GL_Position      = output.ClipSpace;

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	// CLIPPING PLANE
	if (EnableClipping) {
		float4 p = input.FragmentPosition;
		if ((p.x > ClipMax.x) || (p.y > ClipMax.y) || (p.z > ClipMax.z) || (p.x < ClipMin.x) || (p.y < ClipMin.y) || (p.z < ClipMin.z))
			discard;
	}
	
	float2 tiledCoordinates = float2(input.TextureCoords.x * TextureScales[0].x, input.TextureCoords.y * TextureScales[0].y);

	// PROJECTIVE TEXTURE
	float2 normalizedDeviceSpace   = ((input.ClipSpace.xy / input.ClipSpace.w) * 0.5 + 0.5);	// [-1,1] => [0,1]
	float2 reflectionTextureCoords = float2(normalizedDeviceSpace.x, normalizedDeviceSpace.y);
	float2 refractionTextureCoords = float2(normalizedDeviceSpace.x, -normalizedDeviceSpace.y);

	// DU/DV MAP - DISTORTION
	float2 distortedTextureCoords = (Textures[2].Sample(TextureSamplers[2], float2(tiledCoordinates.x + MoveFactor, tiledCoordinates.y)).rg * 0.1);
	distortedTextureCoords        = (tiledCoordinates + float2(distortedTextureCoords.x, distortedTextureCoords.y + MoveFactor));
	float2 totalDistortion        = ((Textures[2].Sample(TextureSamplers[2], distortedTextureCoords).rg * 2.0 - 1.0) * WaveStrength);

	reflectionTextureCoords += totalDistortion;
	refractionTextureCoords += totalDistortion;
	
	float4 reflectionColor = Textures[0].Sample(TextureSamplers[0], reflectionTextureCoords);
	float4 refractionColor = Textures[1].Sample(TextureSamplers[1], refractionTextureCoords);

	// NORMAL MAP
	float4 normalMapColor = Textures[3].Sample(TextureSamplers[3], distortedTextureCoords);
	float3 normal         = normalize(float3((normalMapColor.r * 2.0 - 1.0), (normalMapColor.b * 7.0), (normalMapColor.g * 2.0 - 1.0)));

	// FRESNEL EFFECT
	float3 fromSurfaceToCamera = (CameraMain.Position - input.FragmentPosition.xyz);
	float3 fromLightToSurface  = (input.FragmentPosition.xyz - SunLight.Position);
	float3 viewVector          = normalize(fromSurfaceToCamera);
	float  refractionFactor    = clamp(pow(dot(viewVector, normal), 10.0), 0.0, 1.0);	// higher power => less reflective (transparent)
	float3 reflectedLight      = reflect(normalize(fromLightToSurface), normal);
	float  specular            = pow(max(0.0, dot(reflectedLight, viewVector)), SunLight.Shine);
	float3 specularHighlights  = (SunLight.Color.rgb * specular * SunLight.Reflection);
	float4 GL_FragColor        = (lerp(reflectionColor, refractionColor, refractionFactor) + float4(specularHighlights, 0.0));

	return reflectionColor;
}
