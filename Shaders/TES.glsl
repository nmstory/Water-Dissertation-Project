#version 450 core

uniform mat4 modelMatrix;
uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform vec3 cameraPos;
uniform vec3 lightPos;

//uniform float time;

in vec3 worldPosition [];

out vec3 worldPos;
out vec4 clipSpacePos;
out vec2 texCoords;
out vec3 vectorPointingToCamera;
out vec3 vectorLightToWater;

const float tiling = 3.0f;

layout (quads) in;

void main (void)
{
	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(p1, p2, gl_TessCoord.y);
	
	// Generating waves (where amplitude = 7, speed = 5)
	//pos.y += 7 * sin(pos.x - (5 * time));
	gl_Position = (projMatrix * viewMatrix) * vec4(pos.xyz, 1); // model Matrixxxxxxxxxx?

	// Passing world position to the geometry shader
	worldPos = pos.xyz; //THIS IS POSSIBLY WRONGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG
	vec4 worldPositionv = modelMatrix * vec4(pos.x, 0.0, pos.y, 1.0);
	
	texCoords = vec2(pos.x/2.0 + 0.5, pos.y/2.0 + 0.5) * tiling;
	
	vectorPointingToCamera = cameraPos - worldPositionv.xyz;
	vectorLightToWater = worldPositionv.xyz - lightPos;
	
	clipSpacePos = projMatrix * viewMatrix * worldPositionv;
}
