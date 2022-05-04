#version 400 core



out vec4 out_Color;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform vec3 lightColour;

uniform float waveMovementFactor;

const float waveStrength = 0.01f; // calming down the DUDV sampling! Stretching out sample
const float shineDamper = 10.0f;
const float shineStrengthINV = 5f;
const float reflectivity = 0.6f;

//in vec4 clipSpacePos;
//in vec2 texCoords;
//in vec3 vectorPointingToCamera;
//in vec3 vectorLightToWater;

in gs_fs {
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPosition;
	
	vec4 clipSpacePos;
	vec2 texCoords;
	vec3 vectorPointingToCamera;
	vec3 vectorLightToWater;
} IN;


// * clamp(waterDepth / 10, 0.0f, 1.0f); // water depth fixes edging issue
// ^ taper specular highlights off as it tends to edges???

void main(void) {

	// Clip space to NDC
	// (/ 2.0 + 0.5) -> to translate from NDC coordinate system (-1, 1) in x, y to (0, 1) in x, y.
	vec2 ndc = (IN.clipSpacePos.xy / IN.clipSpacePos.w)/2.0f + 0.5f;
	vec2 refractTexCoords = vec2(ndc.x, ndc.y);
	vec2 reflectTexCoords = vec2(ndc.x, 1.0-ndc.y);
	
	float near = 1.0f;
	float far = 15000.0f;
	float depth = texture(depthMap, refractTexCoords).r; // r stores depth
	float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near)); // see explanations!!!!!
	
	depth = gl_FragCoord.z;
	float waterDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near)); // see explanations!!!!!
	
	float waterDepth = floorDistance - waterDistance;
	
	// Samples DUDV map once, then uses that value as distorted texture coordinates to sample DUDV map a second time, and can also be used to sample normal map!
	vec2 distoredTexCoords = texture(dudvMap, vec2(IN.texCoords.x + waveMovementFactor, IN.texCoords.y)).rg * 0.1;
	distoredTexCoords = IN.texCoords + vec2(distoredTexCoords.x, distoredTexCoords.y + waveMovementFactor);
	vec2 totalDistorion = (texture(dudvMap, distoredTexCoords).rg * 2.0 - 1.0) * waveStrength;
	
	refractTexCoords += totalDistorion;
	refractTexCoords = clamp(refractTexCoords, 0.001, 0.999); // clamping to prevent and edge being sampled from the other side (wrapped) after distortion added
	
	reflectTexCoords +=  totalDistorion;
	reflectTexCoords.x = clamp(reflectTexCoords.x, 0.001, 0.999);
	reflectTexCoords.y = clamp(reflectTexCoords.y, 0.001, 0.999);

	vec4 reflectColour = texture(reflectionTexture, reflectTexCoords); // negative flip on y for reflection
	vec4 refractColour = texture(refractionTexture, refractTexCoords);

	//sample normal map
	vec4 normalMapSample = texture(normalMap, distoredTexCoords);
	vec3 normal = vec3(normalMapSample.r * 2.0f - 1.0f, normalMapSample.b * shineStrengthINV, normalMapSample.g * 2.0 - 1.0); // altering r and g for negative normals on x and z
	normal = normalize(normal);

	//fresnel
	vec3 viewVector = normalize(IN.vectorPointingToCamera);
	float refractiveFactor = dot(viewVector, normal);
	refractiveFactor = pow(refractiveFactor, 5.0); // add new 'fresnel strengh' value???
	
	

	// calculating specular highlight by comparing viewVector (vector from frag to camera) to normal value. This is then scaled with shineDamper
	vec3 reflectedLight = reflect(normalize(IN.vectorLightToWater), normal);
	float specular = max(dot(reflectedLight, viewVector), 0.0);
	specular = pow(specular, shineDamper);
	vec3 specularHighlights = lightColour * specular * reflectivity;

	out_Color = mix(reflectColour, refractColour, refractiveFactor); // mixed in accordance to refractive factor (fresnel)
	out_Color = mix(out_Color, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + vec4(specularHighlights, 0.0f); // adding in blue tint
	out_Color.a = clamp(waterDepth / 40, 0.0f, 1.0f);
}