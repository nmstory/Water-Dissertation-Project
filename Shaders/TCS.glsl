#version 450 core

layout (vertices = 4) out;

in vec3 worldPos [];
out vec3 worldPosition[];

void main(void)
{
	if(gl_InvocationID == 0) {
		gl_TessLevelInner[0] = 2000.0;
		gl_TessLevelInner[1] = 2000.0;
		gl_TessLevelOuter[0] = 1000.0;
		gl_TessLevelOuter[1] = 1000.0;
		gl_TessLevelOuter[2] = 1000.0;
		gl_TessLevelOuter[3] = 1000.0;
	}
	
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	
	worldPosition[gl_InvocationID] = worldPos[gl_InvocationID];
}
