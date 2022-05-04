#version 450 core

layout(triangles) in;
layout(triangle_strip) out;
layout(max_vertices = 3) out;

in vec3 worldPos[];
in vec4 clipSpacePos[];
in vec2 texCoords[];
in vec3 vectorPointingToCamera[];
in vec3 vectorLightToWater[];


uniform mat4 modelMatrix;

out gs_fs {
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPosition;
	
	vec4 clipSpacePos;
	vec2 texCoords;
	vec3 vectorPointingToCamera;
	vec3 vectorLightToWater;
} OUT;

void main(void) {
	vec3 ab = worldPos[1] - worldPos[0];
	vec3 ac = worldPos[2] - worldPos[0];
	vec3 n = normalize(cross(ab, ac));
	
	// tangent
	vec3 t = normalize(ab);

	// binormal
	vec3 bn = normalize(cross(t, n));

	OUT.normal = n;
	OUT.tangent = t;
	OUT.binormal = bn;
	
	

	// Passing along worldPosition to fragment shader
	for(int i = 0; i < gl_in.length(); i++) {
		OUT.worldPosition = worldPos[i];
		
		OUT.clipSpacePos = clipSpacePos[i];
		OUT.texCoords = texCoords[i];
		OUT.vectorPointingToCamera = vectorPointingToCamera[i];
		OUT.vectorLightToWater = vectorLightToWater[i];
		
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();	
	}
	EndPrimitive();
}