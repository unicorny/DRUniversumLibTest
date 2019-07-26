#version 400


layout(location = 0) in vec3 vertices;

varying float colorMarker;

void main()
{
	//gl_Position    = projection * modelview * gl_Vertex;//projection * modelview * gl_Vertex;
	
	// static projection matrix
	gl_Position = vec4(vec2(vertices.x, vertices.z)*2.0-1.0, 0.0, 1.0);
	colorMarker = vertices.y;

}
