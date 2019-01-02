#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_LIGHT_SOURCES = 13;
const int MAX_TEXTURES      = 6;

struct CBLight
{
    vec4 Active;
    vec4 Ambient;
    vec4 Angles;
    vec4 Attenuation;
    vec4 Diffuse;
    vec4 Direction;
    vec4 Position;
    vec4 Specular;
	mat4 ViewProjection;
};

layout(location = 0) in vec3 FragmentNormal;
layout(location = 1) in vec4 FragmentPosition;
layout(location = 2) in vec2 FragmentTextureCoords;

layout(location = 0) out vec4 GL_FragColor;

layout(binding = 1) uniform DefaultBuffer
{
    CBLight LightSources[MAX_LIGHT_SOURCES];

    vec4 IsTextured[MAX_TEXTURES];
    vec4 TextureScales[MAX_TEXTURES];

	vec4 CameraPosition;

    vec4 MeshSpecular;
    vec4 MeshDiffuse;

    vec4 ClipMax;
    vec4 ClipMin;
	vec4 EnableClipping;

    vec4 EnableSRGB;
} db;

layout(binding = 2) uniform sampler2D   Textures[MAX_TEXTURES];
layout(binding = 3) uniform sampler2D   DepthMapTextures2D[MAX_LIGHT_SOURCES];
layout(binding = 4) uniform samplerCube DepthMapTexturesCube[MAX_LIGHT_SOURCES];

// Attenuation = (1 / (c + (l * d) + (q * d^2))
float GetAttenuationFactor(vec3 lightPosition, vec3 attenuation)
{
    float distanceToLight = length(lightPosition - FragmentPosition.xyz); // Distance from fragment surface to the light source
	float constantFactor  = attenuation.x;
    float linearFactor    = (attenuation.y * distanceToLight);
	
    // WITHOUT SRGB - LINEAR
    if (db.EnableSRGB.x < 0.1)
        return (1.0f / (constantFactor + linearFactor + 0.0001));

    // WITH SRGB - QUADRATIC
    float quadratic = (attenuation.z * distanceToLight * distanceToLight);

    return (1.0f / (constantFactor + linearFactor + quadratic + 0.0001));
}

// Diffuse (color) - the impact of the light on the the fragment surface (angle difference)
float GetDiffuseFactor(vec3 normal, vec3 lightDirection)
{
	return max(dot(normal, lightDirection), 0.0);
}

// Shadow - the impact of the light on the the fragment from the perspective of the directional light
float GetShadowFactor(vec3 lightDirection, vec3 normal, sampler2D depthMapTexture, vec4 positionLightSpace)
{
	// Convert shadow map UV and Depth coordinates
	vec3 shadowMapCoordinates = vec3(positionLightSpace.xyz / positionLightSpace.w);	// Perspective projection divide
	shadowMapCoordinates      = ((shadowMapCoordinates * 0.5) + 0.5);					// Convert coordinate range from [-1,1] to [0,1]
	
	// TODO: DEBUG VULKAN
	//return min(texture(depthMapTexture, FragmentTextureCoords).r, 0.75);

	// Get shadow map depth coordinates
	//float closestDepth = texture(depthMapTexture, shadowMapCoordinates.xy).r;	// Closest depth value from light's perspective
	float currentDepth = shadowMapCoordinates.z;	// Depth of current fragment from light's perspective

	// POBLEM:   Large dark region at the far end of the light source's frustum.
	// REASON:   Coordinates outside the far plane of the light's orthographic frustum.
	// SOLUTION: Force shadow values larger than 1.0 to 0 (not in shadow).
	if (currentDepth > 1.0)
		return 0.0;
	
	// PROBLEM:  Shadow acne.
	// REASON:   Because the shadow map is limited by resolution,
	//           multiple fragments can sample the same value from the depth map
	//         	 when they're relatively far away from the light source.
	// SOLUTION: Offset depth.
	const float MIN_OFFSET = 0.005;
	const float MAX_OFFSET = 0.05;
	float       offsetBias = max(MAX_OFFSET * (1.0 - dot(normal, lightDirection)), MIN_OFFSET);
	
	// PCF - Percentage-Closer Filtering
	// Produces softer shadows, making them appear less blocky or hard.
	float nrOfSamples  = 3.0;
	int   sampleOffset = 1;
	float shadowFactor = 0.0;

	// Width and height of the sampler texture at mipmap level 0
	vec2 texelSize = (1.0 / textureSize(depthMapTexture, 0));
	
	// Sample surrounding texels (9 samples)
	for (int x = -sampleOffset; x <= sampleOffset; x++)
	{
		for (int y = -sampleOffset; y <= sampleOffset; y++)
		{
			vec2  texel        = vec2(shadowMapCoordinates.xy + (vec2(x, y) * texelSize));
			float closestDepth = texture(depthMapTexture, texel).r;
			
			// Check if the fragment is in shadow (0 = not in shadow, 1 = in shadow).
			// Compare current fragment with the surrounding texel's depth.
			shadowFactor += ((currentDepth - offsetBias) > closestDepth ? 1.0 : 0.0);
		}
	}
	
	// Average the results
	shadowFactor /= (nrOfSamples * nrOfSamples);
	
	// Reduce the shadow intensity (max 75%) by adding some ambience
	const float MAX_INTENSITY = 0.75;

	return min(shadowFactor, MAX_INTENSITY);
}

// Specular - reflects/mirrors the light direction over the normal
float GetSpecularFactor(vec3 lightDirection, vec3 normal, vec3 viewDirection, float shininess)
{
	if (shininess < 0.1)
		return 0.0;

	//return pow(max(dot(viewDirection, reflect(-lightDirection, normal)), 0.0), shininess);	// Phong
	return pow(max(dot(normal, normalize(lightDirection + viewDirection)), 0.0), shininess);	// Blinn-Phong
}

// Spot Light - the impact of light inside the cone
// I = ((theta - outer) / epsilon)
float GetSpotLightFactor(CBLight light, vec3 lightDirection)
{
	float theta   = dot(lightDirection, normalize(-light.Direction.xyz));	// Angle between light and spot directions
    float epsilon = (light.Angles.x - light.Angles.y);						// Angle between inner and outer cones
	
	return clamp(((theta - light.Angles.y) / epsilon), 0.0, 1.0);
}

// Directional light - all light rays have the same direction, independent of the location of the light source. Ex: sun light
vec4 GetDirectionalLight(int i, vec3 normal, vec3 viewDirection, vec4 materialColor, vec4 materialSpec)
{
	CBLight light = db.LightSources[i];

	// Direction of the light from the fragment surface
	vec3 lightDirection = normalize(-light.Direction.xyz);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection, materialSpec.a);
	
	// Shadow
	vec4  positionLightSpace = (light.ViewProjection * vec4(FragmentPosition.xyz, 1.0));
	float shadowFactor       = (1.0 - GetShadowFactor(lightDirection, normal, DepthMapTextures2D[i], positionLightSpace));

    // Combine the light calculations
    vec3 ambientFinal  = (light.Ambient.rgb  * materialColor.rgb);
    vec3 diffuseFinal  = (light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    vec3 specularFinal = (light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    //return vec4((ambientFinal + diffuseFinal + specularFinal), materialColor.a);
    return vec4((ambientFinal + (shadowFactor * (diffuseFinal + specularFinal))), materialColor.a);
}

vec4 GetPointLight(int i, vec3 normal, vec3 viewDirection, vec4 materialColor, vec4 materialSpec)
{
	CBLight light = db.LightSources[i];

	// Direction of the light from the fragment surface
	vec3 lightDirection = normalize(light.Position.xyz - FragmentPosition.xyz);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection, materialSpec.a);
	
	// Attenuation = 1 / (constant + linear * distanceToLight + quadratic * distanceToLight^2)
	float attenuationFactor = GetAttenuationFactor(light.Position.xyz, light.Attenuation.xyz);
	
	// Shadow
	//float shadow = (1.0 - GetShadowFactor(light.Position, lightDirection, normal, DepthMapTexturesCube[i]));
	
    // Combine the light calculations
    vec3 ambient  = (attenuationFactor * light.Ambient.rgb  * materialColor.rgb);
    vec3 diffuse  = (attenuationFactor * light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    vec3 specular = (attenuationFactor * light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    return vec4((ambient + diffuse + specular), materialColor.a);
    //return vec4((ambient + (shadow * (diffuse + specular))), materialColor.a);
	//return GetShadow(light.position, lightDirection, normal, light.ShadowMapTextureCube);
}

vec4 GetSpotLight(int i, vec3 normal, vec3 viewDirection, vec4 materialColor, vec4 materialSpec)
{
	CBLight light = db.LightSources[i];

	// Direction of the light from the fragment surface
	vec3 lightDirection = normalize(light.Position.xyz - FragmentPosition.xyz);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection, materialSpec.a);
	
	// Attenuation = 1 / (constant + linear * distanceToLight + quadratic * distanceToLight^2)
	float attenuationFactor = GetAttenuationFactor(light.Position.xyz, light.Attenuation.xyz);
	
	// Spotlight intensity
	float spotLightFactor = GetSpotLightFactor(light, lightDirection);
	
    // Combine the light calculations
    vec3 ambient  = (spotLightFactor * attenuationFactor * light.Ambient.rgb  * materialColor.rgb);
    vec3 diffuse  = (spotLightFactor * attenuationFactor * light.Diffuse.rgb  * materialColor.rgb * diffuseFactor);
    vec3 specular = (spotLightFactor * attenuationFactor * light.Specular.rgb * materialSpec.rgb  * specularFactor);
	
    return vec4((ambient + diffuse + specular), materialColor.a);
    //return (vec4(ambient, 1.0) + ((1.0 - GetShadow()) * (diffuse + vec4(specular, 1.0))));
}

void main()
{
	if (db.EnableClipping.x > 0.1)
	{
		vec4 p = FragmentPosition;

		if ((p.x > db.ClipMax.x) || (p.y > db.ClipMax.y) || (p.z > db.ClipMax.z) || (p.x < db.ClipMin.x) || (p.y < db.ClipMin.y) || (p.z < db.ClipMin.z))
			discard;
	}

	vec4 materialColor;
	vec4 materialSpecular;
	
	// MESH DIFFUSE (COLOR)
	if (db.IsTextured[0].x > 0.1) {
		vec2 tiledCoordinates = vec2(FragmentTextureCoords.x * db.TextureScales[0].x, FragmentTextureCoords.y * db.TextureScales[0].y);
		materialColor = texture(Textures[0], tiledCoordinates);
	} else {
		materialColor = db.MeshDiffuse;
	}	
	
	// MESH SPECULAR HIGHLIGHTS
	if (db.IsTextured[1].x > 0.1) {
		vec2 tiledCoordinates = vec2(FragmentTextureCoords.x * db.TextureScales[1].x, FragmentTextureCoords.y * db.TextureScales[1].y);
		materialSpecular      = texture(Textures[1], tiledCoordinates);
	} else {
		materialSpecular = db.MeshSpecular;
	}
	
	vec3 normal        = normalize(FragmentNormal);									// Normal on the fragment surface
	vec3 viewDirection = normalize(db.CameraPosition.xyz - FragmentPosition.xyz);	// The direction the camera is viewing the fragment surface
	GL_FragColor       = vec4(0);

    // LIGHT SOURCES
    for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if (db.LightSources[i].Active.x < 0.1)
            continue;

    	// SPOT LIGHT
        if (db.LightSources[i].Angles.x > 0.1)
            GL_FragColor += GetSpotLight(i, normal, viewDirection, materialColor, materialSpecular);
    	// POINT LIGHT
        else if (db.LightSources[i].Attenuation.r > 0.1)
            GL_FragColor += GetPointLight(i, normal, viewDirection, materialColor, materialSpecular);
    	// DIRECTIONAL LIGHT
        else
            GL_FragColor += GetDirectionalLight(i, normal, viewDirection, materialColor, materialSpecular);
    }

	// LIGHT COLOR (PHONG REFLECTION)
	//vec3 lightColor = (db.SunLightAmbient + (db.SunLightDiffuse.rgb * dot(normalize(FragmentNormal), normalize(-db.SunLightDirection))));
	//GL_FragColor    = vec4((db.MeshDiffuse.rgb * lightColor), db.MeshDiffuse.a);

	// sRGB GAMMA CORRECTION
    if (db.EnableSRGB.x > 0.1) {
		float sRGB = (1.0 / 2.2);
		GL_FragColor.rgb = pow(GL_FragColor.rgb, vec3(sRGB, sRGB, sRGB));
	}
}
