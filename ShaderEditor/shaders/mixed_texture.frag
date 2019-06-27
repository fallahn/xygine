#version 120

uniform sampler2D u_texture;
uniform sampler2D u_otherTexture;

void main()
{
	vec4 a = texture2D(u_texture, gl_TexCoord[0].xy);
	vec4 b = texture2D(u_otherTexture, gl_TexCoord[0].xy);
	gl_FragColor = mix(a, b, 0.5);
}



