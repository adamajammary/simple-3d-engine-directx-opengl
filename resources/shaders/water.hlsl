static const int MAX_TEXTURES = 6;

struct CBMatrix
{
	matrix Model;
    matrix View;
    matrix Projection;
    matrix MVP;
};

struct CBDefault
{
    CBMatrix MB;

    float4 IsTextured[MAX_TEXTURES];
    float4 TextureScales[MAX_TEXTURES];

    float3 CameraPosition;
    float  CameraPadding1;

    float3 MeshSpecularIntensity;
	float  MeshSpecularShininess;

    float4 MeshDiffuse;

    float3 SunLightSpecularIntensity;
	float  SunLightSpecularShininess;

    float3 SunLightAmbient;
	float  SunLightPadding1;
    float4 SunLightDiffuse;

    float3 SunLightPosition;
	float  SunLightPadding2;
    float3 SunLightDirection;
	float  SunLightPadding3;

    float3 ClipMax;
    float  EnableClipping;
    float3 ClipMin;
    float  ClipPadding1;
};

cbuffer WaterBuffer : register(b0)
{
	float  MoveFactor;
	float  WaveStrength;
    float2 Padding1;

    CBDefault DB;
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
	float4 FragmentPosition      : POSITION0;
	float4 ClipSpace             : POSITION1;
	float4 GL_Position           : SV_POSITION;
};

// VERTEX SHADER
FS_INPUT VS(VS_INPUT input)
{
    FS_INPUT output;

	output.FragmentTextureCoords = input.VertexTextureCoords;
    output.FragmentPosition      = mul(float4(input.VertexPosition, 1.0), DB.MB.Model);
	output.ClipSpace             = mul(float4(input.VertexPosition, 1.0), DB.MB.MVP);
	output.GL_Position           = output.ClipSpace;

	return output;
}

// FRAGMENT/PIXEL/COLOR SHADER
float4 PS(FS_INPUT input) : SV_Target
{
	// CLIPPING PLANE
    if (DB.EnableClipping.x > 0.1)
    {
		float4 p = input.FragmentPosition;

        if ((p.x > DB.ClipMax.x) || (p.y > DB.ClipMax.y) || (p.z > DB.ClipMax.z) || (p.x < DB.ClipMin.x) || (p.y < DB.ClipMin.y) || (p.z < DB.ClipMin.z))
			discard;
	}
	
    float2 tiledCoordinates = float2(input.FragmentTextureCoords.x * DB.TextureScales[0].x, input.FragmentTextureCoords.y * DB.TextureScales[0].y);

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
	float3 fromSurfaceToCamera = (DB.CameraPosition - input.FragmentPosition.xyz);
    float3 fromLightToSurface  = (input.FragmentPosition.xyz - DB.SunLightPosition);
	float3 viewVector          = normalize(fromSurfaceToCamera);
	float  refractionFactor    = clamp(pow(dot(viewVector, normal), 10.0), 0.0, 1.0);	// higher power => less reflective (transparent)
	float3 reflectedLight      = reflect(normalize(fromLightToSurface), normal);
	float  specular            = pow(max(0.0, dot(reflectedLight, viewVector)), DB.SunLightSpecularIntensity[0]);
	float3 specularHighlights  = (DB.SunLightDiffuse.rgb * specular * DB.SunLightSpecularShininess);
	float4 GL_FragColor        = (lerp(reflectionColor, refractionColor, refractionFactor) + float4(specularHighlights, 0.0));
	//float4 GL_FragColor = float4(1.0, 0.0, 0.0, 1.0);
	//float4 GL_FragColor = Textures[0].Sample(TextureSamplers[0], input.TextureCoords);

	return GL_FragColor;
}
