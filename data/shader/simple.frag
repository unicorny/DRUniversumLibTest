#version 400
// simple fragment shader

// 'time' contains seconds since the program was linked.
uniform sampler2D texture;
varying vec2 text_coords;
//in vec4 colour;
//out vec4 result;
void main()
{
	gl_FragColor = texture2D(texture, text_coords);
	//gl_FragColor = vec4(text_coords.x, 0, text_coords.y, 1);
	//result = colour;
	//result.r = 1.0;
}
