#version 330
in vec2 vertexuv;
uniform sampler2D tex;
uniform float time;
uniform vec2 resolution;

float rnd(vec2 x) 
{
	int n = int(x.x * 40 + x.y * 6400);
	n = (n << 13) ^ n;
	return 1 - float( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824;
}

float smoothrnd(vec2 x)
{
	x = mod(x,1000.0);
    vec2 a = fract(x);
    x -= a;
    vec2 u = a*a*(3.0-2.0*a);
    return mix(
	mix(rnd(x+vec2(0,0)),rnd(x+vec2(1,0)), u.x),
	mix(rnd(x+vec2(0,1)),rnd(x+vec2(1,1)), u.x), u.y);
}


float noise(vec2 uv) {
    float r = rnd(uv*time);
	return clamp(r,0.9,1.2);
}

float onOff(float a, float b, float c)
{
	return step(c, sin(time + a*cos(time*b)));
}

float ramp(float y, float start, float end)
{
	float inside = step(start,y) - step(end,y);
	float fact = (y-start)/(end-start)*inside;
	return (1.-fact) * inside;
	
}

float stripes(vec2 uv)
{
	
	float noi = rnd(uv*time*5.0);
	return ramp(mod(uv.y*4. + time/2.+sin(time + sin(time*0.63)),1.),0.5,0.6)*noi;
}

vec2 colorShift(vec2 uv) {
	return vec2(
		uv.x,
		uv.y + sin(time)*0.02
	);
}

vec2 colorshift(vec2 uv, float amount, float rand) {
	
	return vec2(
		uv.x,
		uv.y + amount * rand // * sin(uv.y * resolution.y * 0.12 + time)
	);
}

vec3 getVideo(vec2 uv)
{
	vec2 look = uv;
	float window = 1./(1.+20.*(look.y-mod(time/4.,1.))*(look.y-mod(time/4.,1.)));
	look.x = look.x + sin(look.y*10. + time)/50.*onOff(4.,4.,.3)*(1.+cos(time*80.))*window;
	float vShift = 0.4*onOff(2.,3.,.9)*(sin(time)*sin(time*20.) + 
										 (0.5 + 0.1*sin(time*200.)*cos(time)));
	look.y = mod(look.y + vShift, 1.);
	
	vec3 color;
	vec3 rand;
	rand.r = rnd(uv*time*4.0);
	rand.b = rnd(uv*time*3.0);
	rand.g = rnd(uv*time*2.0);
	color.r = texture2D(tex, colorshift(look, 0.025, rand.r)).r;
	color.g = texture2D(tex, colorshift(look, 0.01, rand.g)).g;
	color.b = texture2D(tex, colorshift(look, 0.024, rand.b)).b;
	
	return color;
}

vec2 screenDistort(vec2 coord, float bend)
{
	// put in symmetrical coords
	coord = (coord - 0.5) * 2.0;

	coord *= 0.5;	
	
	// deform coords
	coord.x *= 1.0 + pow((abs(coord.y) / bend), 2.0);
	coord.y *= 1.0 + pow((abs(coord.x) / bend), 2.0);

	// transform back to 0.0 - 1.0 space
	coord  = (coord / 1.0) + 0.5;

	return coord;
}

vec2 scandistort(vec2 uv) {
	float scan1 = clamp(cos(uv.y * 2.0 + time), 0.0, 1.0);
	float scan2 = clamp(cos(uv.y * 2.0 + time + 4.0) * 10.0, 0.0, 1.0) ;
	float amount = scan1 * scan2 * uv.x; 
	uv.x -= 0.05 * mix(rnd(uv*time) * amount, amount, 0.9);

	return uv; 
}

void main(void)
{
	vec2 uv = vertexuv;
	vec2 sd_uv = scandistort(uv);
	//uv = screenDistort(sd_uv,2.0);
	vec3 video = getVideo(uv);
	float vigAmt = 3.+.3*sin(time + 5.*cos(time*5.));
	float vignette = (1.-vigAmt*(uv.y-.5)*(uv.y-.5))*(1.-vigAmt*(uv.x-.5)*(uv.x-.5));
	
	video += stripes(uv);
	video += rnd(uv*time)/4.0;
	video *= vignette;
	video *= (12.+mod(uv.y*30.+time,1.))/13.;
	
	gl_FragColor = vec4(video,1.0);
}