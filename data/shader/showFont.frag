#version 400
// complex fragment shader to render font, based on vector data

//in vec4 colour;
uniform float fontColor;
varying float colorMarker;
//out vec4 result;
void main()
{
	float c = fontColor;
	//c = 1.0f;
	if(colorMarker < 0.0f) gl_FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	else if(colorMarker > 0.0f) gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	else gl_FragColor = vec4(1.0f-c, c, colorMarker*0.5f+0.5, 1.0f);
	//gl_FragColor = vec4(text_coords.x, 0, text_coords.y, 1);
	//result = colour;
	//result.r = 1.0;
}
