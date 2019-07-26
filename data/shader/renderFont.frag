#version 400
// complex fragment shader to render font, based on vector data

varying float colorMarker;
//out vec4 result;
void main()
{
	gl_FragColor = vec4(colorMarker, colorMarker * 2.0f, colorMarker * 4.0f, 1.0f);

}
