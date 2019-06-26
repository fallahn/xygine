#version 120

uniform float u_time;
uniform vec2 u_buns;

void main()
{
	gl_FragColor = vec4(vec3(u_time, u_buns / 100.0), 0.5);
	//first version
}




