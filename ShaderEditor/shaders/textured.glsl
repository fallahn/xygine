#version 120

uniform sampler2D u_texture;
uniform vec4 u_colour;

void main()
{
	gl_FragColor = texture2D(u_texture, gl_TexCoord[0].xy);
}


