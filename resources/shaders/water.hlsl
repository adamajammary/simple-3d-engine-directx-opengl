static const int MAX_LIGHT_SOURCES = 13;
static const int MAX_TEXTURES      = 6;

struct CBLight
{
    float4 Active;
    float4 Ambient;
    float4 Angles;
    float4 Attenuation;
    float4 Diffuse;
    float4 Direction;
    float4 Position;
    float4 Specular;
    matrix ViewProjection;
};

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

    CBLight LightSources[MAX_LIGHT_SOURCES];

    float4 IsTextured[MAX_TEXTURES];
    float4 TextureScales[MAX_TEXTURES];

    float4 CameraPosition;
 
    float4 MeshSpecular;
    float4 MeshDiffuse;

    float4 ClipMax;
    float4 ClipMin;
    float4 EnableClipping;

    float4 EnableSRGB;
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
	float3 fromSurfaceToCamera = (DB.CameraPosition.xyz - input.FragmentPosition.xyz);
	float3 viewVector          = normalize(fromSurfaceToCamera);
	float  refractionFactor    = clamp(pow(dot(viewVector, normal), 10.0), 0.0, 1.0);	// higher power => less reflective (transparent)
    float4 GL_FragColor        = float4(0, 0, 0, 0);

    for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if ((DB.LightSources[i].Active.x < 0.1) || (DB.LightSources[i].Angles.x > 0.1) || (DB.LightSources[i].Attenuation.r > 0.1))
            continue;

        float3 fromLightToSurface = (input.FragmentPosition.xyz - DB.LightSources[i].Position.xyz);
        float3 reflectedLight     = reflect(normalize(fromLightToSurface), normal);
        float  specular           = pow(max(0.0, dot(reflectedLight, viewVector)), DB.LightSources[i].Specular.r);
        float3 specularHighlights = (DB.LightSources[i].Diffuse.rgb * specular * DB.LightSources[i].Specular.a);

        GL_FragColor += (lerp(reflectionColor, refractionColor, refractionFactor) + float4(specularHighlights, 0.0));
    }

	//float4 GL_FragColor = float4(1.0, 0.0, 0.0, 1.0);
	//float4 GL_FragColor = Textures[0].Sample(TextureSamplers[0], input.TextureCoords);

	// sRGB GAMMA CORRECTION
    if (DB.EnableSRGB.x > 0.1) {
        float sRGB = (1.0 / 2.2);
        GL_FragColor.rgb = pow(GL_FragColor.rgb, float3(sRGB, sRGB, sRGB));
    }

	return GL_FragColor;
}
