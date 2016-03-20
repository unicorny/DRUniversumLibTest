#version 400


layout(location = 0) in vec3 vertices;
uniform mat4 proj;
varying vec3 color;
void main()
{
	//gl_Position    = projection * modelview * gl_Vertex;//projection * modelview * gl_Vertex;
	gl_Position = vec4(vec2(vertices.x, vertices.z)*2.0-1.0, 0.0, 1.0);
	color = vertices;
	//gl_FrontColor  = gl_Color;
	//gl_TexCoord[0] = gl_MultiTexCoord0;
}
