uniform vec2 resolution;
uniform vec2 time;
uniform sampler2D tex0;

void main(void)
{
	vec2 tex0coord = gl_TexCoord[0].xy;
	tex0coord.x += sin(tex0coord.y * 3*3.14159 + time*1.5) / 100;
	tex0coord.y += sin(tex0coord.x * 4*3.14159 + time*1.5) / 100;
	vec4 col =  texture2D(tex0, tex0coord).rgba;
	gl_FragColor = vec4(col);
}