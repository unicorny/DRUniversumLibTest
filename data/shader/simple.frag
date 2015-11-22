#version 400
// simple fragment shader

// 'time' contains seconds since the program was linked.
//uniform sampler2D texture;

in vec4 colour;
out vec4 result;
void main()
{
	//gl_FragColor = texture2D(texture, gl_TexCoord[0].xy);
	result = colour;
	//result.r = 1.0;
}
