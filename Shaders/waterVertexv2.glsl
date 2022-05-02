#version 400 core

in vec2 position;

out vec4 clipSpacePos;
out vec2 texCoords;
out vec3 vectorPointingToCamera;
out vec3 vectorLightToWater;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 cameraPos;
uniform vec3 lightPos;

const float tiling = 3.0f;

void main(void) {

	vec4 worldPos = modelMatrix * vec4(position.x, 0.0, position.y, 1.0);
	clipSpacePos = projMatrix * viewMatrix * worldPos;
	gl_Position = clipSpacePos;
	texCoords = vec2(position.x/2.0 + 0.5, position.y/2.0 + 0.5) * tiling;
	
	vectorPointingToCamera = cameraPos - worldPos.xyz;
	vectorLightToWater = worldPos.xyz - lightPos;
 
}