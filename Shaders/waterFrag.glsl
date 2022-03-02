#version 450 core

// Texture uniforms
uniform sampler2D diffuseWater;
uniform sampler2D bumpWater;

// Screen size
uniform int screenWidth;
uniform int screenHeight;

// Lighting uniforms
uniform vec3 	cameraPos;
uniform vec4 	lightColour;
uniform vec3	lightPos;
uniform float 	lightRadius;

// VP
uniform mat4 projMatrix;
uniform mat4 viewMatrix;

in gs_fs {
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPosition;
} IN;

out vec4 fragColour;

void main(void)
{
	//fragColour = vec4(0.29803922, 0.52941176, 0.96470588, 0.4);

	vec4 diffuse = vec4(texture(diffuseWater, IN.worldPosition.xz).rgb, 1);
	vec3 bumpNormal = texture(bumpWater, IN.worldPosition.xz).rgb;

	// World space reconstruction
	vec4 fc = gl_FragCoord;
	fc.x = fc.x / screenWidth;
	fc.y = fc.y / screenHeight;

	// NDC
	fc.x = (fc.x * 2) - 1;
	fc.y = (fc.y * 2) - 1;
	fc.z = (fc.z * 2) - 1;

	fc = fc * inverse(projMatrix * viewMatrix);

	fc.x = fc.x / fc.w;
	fc.y = fc.y / fc.w;
	fc.z = fc.z / fc.w;

	vec3 worldPos = fc.xyz;

	vec3 incident = normalize(lightPos - worldPos);
	vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));

	float lambert = max(dot(incident , bumpNormal), 0.0f);
	float distance = length(lightPos - IN.worldPosition);
	float attenuation = 1.0f - clamp(distance / lightRadius, 0.0, 1.0);

	float specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
	specFactor = pow(specFactor , 60.0 );

	vec3 surface = (diffuse.rgb * lightColour.rgb);
	fragColour.rgb = surface * lambert * attenuation;
	fragColour.rgb += (lightColour.rgb * specFactor) * attenuation * 0.33;
	fragColour.rgb += surface * 0.1f;
	fragColour.a = diffuse.a;

	//fragColour.r = -bumpNormal.x;
	//fragColour.g = -bumpNormal.y;
	//fragColour.b = -bumpNormal.z;
	//fragColour.rgb = IN.normal;
	fragColour.a = 1;
	
}