#version 450 core

//layout (quads, equal_spacing, ccw) in;

uniform mat4 modelMatrix;
uniform mat4 projMatrix;
uniform mat4 viewMatrix;

uniform float time;

in vec3 worldPosition [];
out vec3 worldPos;

layout (quads) in;

void main (void)
{
	vec4 p1 = mix(gl_in[0].gl_Position,
					gl_in[1].gl_Position,
				  gl_TessCoord.x);
	
	vec4 p2 = mix(gl_in[2].gl_Position,
				  gl_in[3].gl_Position,
				  gl_TessCoord.x);
	
	//gl_Position = mix(p1, p2, gl_TessCoord.y);
	vec4 pos = mix(p1, p2, gl_TessCoord.y);
	
	float speed = 30;
	pos.y += sin(pos.x - (speed * time));
	//pos.y += time;


	//gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(pos.xyz, 1);
	gl_Position = (projMatrix * viewMatrix) * vec4(pos.xyz, 1);


	vec3 w1 = mix(worldPosition[0],
					worldPosition[1],
				  gl_TessCoord.x);
	
	vec3 w2 = mix(worldPosition[2],
				  worldPosition[3],
				  gl_TessCoord.x);

	worldPos = mix(w1, w2, gl_TessCoord.y);
	//worldPos = (modelMatrix * vec4(worldPos,1)).xyz;
}
