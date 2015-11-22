#version 400


layout(location = 0) in vec3 vertices;
uniform mat4 view;
varying vec3 color;
uniform float time;
void main()
{
	//gl_Position    = projection * modelview * gl_Vertex;//projection * modelview * gl_Vertex;
	gl_Position = view * vec4(vertices, 1.0);
	color = vertices;
	//gl_FrontColor  = gl_Color;
	//gl_TexCoord[0] = gl_MultiTexCoord0;
}
