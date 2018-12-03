#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef GL_FRAGMENT_PRECISION_HIGH
	precision highp float;
#else
	precision mediump float;
#endif

const int MAX_TEXTURES = 6;

layout(location = 0) in vec3 FragmentNormal;
layout(location = 1) in vec4 FragmentPosition;
layout(location = 2) in vec2 FragmentTextureCoords;

layout(location = 0) out vec4 GL_FragColor;

layout(binding = 1) uniform DefaultBuffer
{
    vec4 IsTextured[MAX_TEXTURES];
    vec4 TextureScales[MAX_TEXTURES];

	vec3  CameraPosition;
	float CameraPadding1;

    vec3  MeshSpecularIntensity;
	float MeshSpecularShininess;

    vec4  MeshDiffuse;

    vec3  SunLightSpecularIntensity;
	float SunLightSpecularShininess;

    vec3  SunLightAmbient;
	float SunLightPadding1;
    vec4  SunLightDiffuse;

    vec3  SunLightPosition;
	float SunLightPadding2;
    vec3  SunLightDirection;
	float SunLightPadding3;

    vec3  ClipMax;
	float EnableClipping;
    vec3  ClipMin;
    float ClipPadding1;
} db;

layout(binding = 2) uniform sampler2D Textures[MAX_TEXTURES];

// Diffuse (color) - the impact of the light on the the fragment surface (angle difference)
float GetDiffuseFactor(vec3 normal, vec3 lightDirection)
{
	return max(dot(normal, lightDirection), 0.0);
}

// Specular - reflects/mirrors the light direction over the normal
float GetSpecularFactor(vec3 lightDirection, vec3 normal, vec3 viewDirection)
{
	if (db.MeshSpecularShininess < 1.0)
		return 0.0;

	//return pow(max(dot(viewDirection, reflect(-lightDirection, normal)), 0.0), db.MeshSpecularShininess);	// Phong
	return pow(max(dot(normal, normalize(lightDirection + viewDirection)), 0.0), db.MeshSpecularShininess);	// Blinn-Phong
}

// Directional light - all light rays have the same direction, independent of the location of the light source. Ex: sun light
vec4 GetDirectionalLight(vec3 normal, vec3 viewDirection, vec4 materialColor, vec3 materialSpecIntensity)
{
	// Direction of the light from the fragment surface
	vec3 lightDirection = normalize(-db.SunLightDirection);
	
    // Diffuse - the impact of the light on the the fragment surface (angle difference)
	float diffuseFactor = GetDiffuseFactor(normal, lightDirection);
	
    // Specular - reflects/mirrors the light direction over the normal
	float specularFactor = GetSpecularFactor(lightDirection, normal, viewDirection);
	
	// Shadow
	//vec4 positionLightSpace = vec4(light.light.viewProjectionMatrix * vec4(FragmentPosition, 1.0));
	//float shadow             = (1.0 - calculateShadow(lightDirection, normal, light.shadowMapTexture2D, positionLightSpace));
	
    // Combine the light calculations
    vec3 ambientFinal  = (db.SunLightAmbient * materialColor.rgb);
    vec3 diffuseFinal  = (db.SunLightDiffuse.rgb * materialColor.rgb * diffuseFactor);
    vec3 specularFinal = (db.SunLightSpecularIntensity * materialSpecIntensity * specularFactor);
	
    return vec4((ambientFinal + diffuseFinal + specularFinal), materialColor.a);
    //return vec4((ambientFinal + (shadow * (diffuseFinal + specularFinal))), materialColor.a);
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
	vec3 materialSpecular;
	
	// MESH DIFFUSE (COLOR)
	if (db.IsTextured[0].x > 0.1)
	{
		vec2 tiledCoordinates = vec2(FragmentTextureCoords.x * db.TextureScales[0].x, FragmentTextureCoords.y * db.TextureScales[0].y);

		materialColor = texture(Textures[0], tiledCoordinates);
	} else {
		materialColor = db.MeshDiffuse;
	}	
	
	// MESH SPECULAR HIGHLIGHTS
	if (db.IsTextured[1].x > 0.1)
	{
		vec2 tiledCoordinates = vec2(FragmentTextureCoords.x * db.TextureScales[1].x, FragmentTextureCoords.y * db.TextureScales[1].y);

		materialSpecular = texture(Textures[1], tiledCoordinates).rgb;
	} else {
		materialSpecular = db.MeshSpecularIntensity;
	}
	
	vec3 normal        = normalize(FragmentNormal);								// Normal on the fragment surface
	vec3 viewDirection = normalize(db.CameraPosition - FragmentPosition.xyz);	// The direction the camera is viewing the fragment surface
	
	// DIRECTIONAL LIGHT
	GL_FragColor = GetDirectionalLight(normal, viewDirection, materialColor, materialSpecular);

	// LIGHT COLOR (PHONG REFLECTION)
	//vec3 lightColor = (db.SunLightAmbient + (db.SunLightDiffuse.rgb * dot(normalize(FragmentNormal), normalize(-db.SunLightDirection))));
	//GL_FragColor    = vec4((db.MeshDiffuse.rgb * lightColor), db.MeshDiffuse.a);
}
