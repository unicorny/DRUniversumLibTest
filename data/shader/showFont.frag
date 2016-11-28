#version 400
// complex fragment shader to render font, based on vector data

//in vec4 colour;
uniform float fontColor;
//out vec4 result;
void main()
{
	float c = fontColor;
	//c = 1.0f;
	gl_FragColor = vec4(1.0f-c, c, 0.0f, 1.0f);
	//gl_FragColor = vec4(text_coords.x, 0, text_coords.y, 1);
	//result = colour;
	//result.r = 1.0;
}
