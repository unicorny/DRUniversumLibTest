#version 400


layout(location = 0) in vec3 vertices;
//layout(location = 1) in vec4 color;
//out vec4 colour;
uniform mat4 view;
uniform float time;
varying vec2 text_coords;
void main()
{
	//gl_Position    = projection * modelview * gl_Vertex;//projection * modelview * gl_Vertex;
	gl_Position = view * vec4(vertices, 1.0);
	text_coords = vertices.xz;
	//colour = color;
	//gl_FrontColor  = gl_Color;
	//gl_TexCoord[0] = gl_MultiTexCoord0;
}
