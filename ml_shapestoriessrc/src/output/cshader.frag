#version 330
in vec2 vertexuv;
layout(location = 0) out vec4 FragColor;
uniform sampler2D tex;
uniform sampler2D noisetex;
uniform float time;
uniform vec2 resolution;


float scanline(vec2 uv) {
	return sin(resolution.y * uv.y * 0.7 - time * 10.0);
}

float slowscan(vec2 uv) {
	return sin(resolution.y * uv.y * 0.02 + time * 6.0);
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
    float r = smoothrnd(uv*time);
	return clamp(r,0.9,1.2);
}

// from https://www.shadertoy.com/view/4sf3Dr
// Thanks, Jasper
vec2 crt(vec2 coord, float bend)
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

vec2 colorshift(vec2 uv, float amount, float rand) {
	
	return vec2(
		uv.x,
		uv.y + amount * rand // * sin(uv.y * resolution.y * 0.12 + time)
	);
}

vec2 scandistort(vec2 uv) {
	float scan1 = clamp(cos(uv.y * 2.0 + time), 0.0, 1.0);
	float scan2 = clamp(cos(uv.y * 2.0 + time + 4.0) * 10.0, 0.0, 1.0) ;
	float amount = scan1 * scan2 * uv.x; 
	
	uv.x -= 0.05 * mix(texture2D(noisetex, vec2(uv.x, amount)).r * amount, amount, 0.9);

	return uv; 
}

float vignette(vec2 uv) {
	uv = (uv - 0.5) * 0.98;
	return clamp(pow(cos(uv.x * 3.1415), 1.2) * pow(cos(uv.y * 3.1415), 1.2) * 50.0, 0.0, 1.0);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / resolution.xy;
	vec2 noisecoord = uv;
	vec2 sd_uv = scandistort(uv);
	vec2 crt_uv = crt(sd_uv, 2.0);
	
	
	
	//float rand_r = sin(time * 3.0 + sin(time)) * sin(time * 0.2);
	//float rand_g = clamp(sin(time * 1.52 * uv.y + sin(time)) * sin(time* 1.2), 0.0, 1.0);
	vec4 color;
	vec3 rand;
	rand.r = rnd(uv*time*4.0);
	rand.b = rnd(uv*time*3.0);
	rand.g = rnd(uv*time*2.0);
	
	color.r = texture2D(tex, crt(colorshift(sd_uv, 0.025, rand.r), 2.0)).r;
	color.g = texture2D(tex, crt(colorshift(sd_uv, 0.01, rand.g), 2.0)).g;
	color.b = texture2D(tex, crt(colorshift(sd_uv, 0.024, rand.b), 2.0)).b;	
		
	vec4 scanline_color = vec4(scanline(crt_uv));
	vec4 slowscan_color = vec4(slowscan(crt_uv));
	
	FragColor = mix(color, mix(scanline_color, slowscan_color, 0.5), 0.05) *
		vignette(uv) *
		noise(uv);
}