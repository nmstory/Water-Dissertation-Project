#version 400 core

in vec3 position;

//out vec4 clipSpacePos;
//out vec2 texCoords;
//out vec3 vectorPointingToCamera;
//out vec3 vectorLightToWater;

out vec3 worldPos;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 cameraPos;
uniform vec3 lightPos;

//const float tiling = 3.0f;

void main(void) {
	
	vec4 worldPosition = (modelMatrix * vec4(position, 1));
	worldPos = worldPosition.xyz;
	
	gl_Position = vec4(position , 1.0);
	








	//vec4 worldPosition = modelMatrix * vec4(position.x, 0.0, position.y, 1.0);
	//clipSpacePos = projMatrix * viewMatrix * worldPosition;
	//gl_Position = clipSpacePos;
	
	
	//vectorPointingToCamera = cameraPos - worldPosition.xyz;
	//vectorLightToWater = worldPosition.xyz - lightPos;
	
	//worldPos = worldPosition.xyz;
 
}