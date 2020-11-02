

//float frameLimit = .35;
//float frameShape = .4;
//float frameSharpness = 3.;
//float strp_cnt = 32.;
//float strp_trnsinst = 0.10;
//float toblack = 0.0;
//float noisemix = .065;
//float flip_y = 0.0;
//float tv_artifacts = 1.0;*/
#version 330
layout(location = 0) out vec4 fragColor;
uniform sampler2D tex;
uniform vec2 resolution;
uniform float time;
uniform float frameLimit;
uniform float frameShape;
uniform float frameSharpness;
uniform float strp_cnt;
uniform float strp_trnsinst;
uniform float toblack;
uniform float noisemix;
uniform float flip_y;
uniform float tv_artifacts;

float rand(float x)
{
    vec2 co = vec2(x,x);
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt= dot(co.xy ,vec2(a,b));
    float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

float getRand(vec2 co)
{
   vec2 randOffset = vec2(time * 0.0021,time * 0.0001);
    return fract(sin(dot(co.xy, vec2(12.9898,78.233) + randOffset)) * 43758.5453);
}

vec4 process(vec2 coords, sampler2D image)
{
	float stripeId = floor(coords.y * max(1.0, strp_cnt)) / max(1.0, strp_cnt);
	float stripeRand = getRand(vec2(0.0, -stripeId));
	float stripeOffset = getRand(vec2(0.0, stripeId)) * strp_trnsinst;
	// offsetting
	vec2 texCoords= vec2(mod(coords.x + stripeOffset * 0.25, 1.0), 
						  coords.y + min(1.0, 1.0 + mod(coords.x + stripeOffset * 0.25, 1.0)) - 1.0);					  
						  
	vec4 img = vec4(texture2D(image, texCoords));

	vec4 col = img;
    return col;
}

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

float s(float v,float t,float f)
		{
		float r=step(t,v)-step(f,v),e=(v-t)/(f-t)*r;
		return(1.-e)*r;
		}
float scrolllines(vec2 v)
{
 float r=getRand(v*time*5.);
 return s(mod(v.g*2.+time/2.+sin(time+sin(time*.63)),2.),.5,.6)*r;
}


void main()
{
	vec2 uv = gl_FragCoord.xy / resolution.xy;
	if(flip_y == 1.)
	uv.y = 1.0 - uv.y;
    uv = crt(uv, 2.0);
    vec2 p=-1.0+2.0*uv;
    vec2 uv_q = uv;
	vec2 uv_n = uv_q;
    mat3 rgbtoyuv = mat3(0.299, -0.147,  0.615, 0.587, -0.289, -0.515, 0.114, 0.436, -0.100);
	mat3 yuvtorgb = mat3(1.000, 1.000, 1.000, 0.000, -0.395, 2.032, 1.140, -0.581, 0.000);
    float shade = 1.0;
    shade -= rand(uv_q.x * 0.1 + uv_q.y * 50.0 + time) * 0.5;

	

    if (tv_artifacts == 1.)
    {
    	uv_n.y += rand(time * 12.) * 0.004;
	    // small scanline-based X-position noise = tape wrinkles
        float runran = rand(time * 666.0);
		if (runran > 0.91)
		uv_n.x += rand(uv_q.y*39.0+time * 20.0) * 0.00044 - 0.012;
		// global sinus wobbling
		float xsin = sin(time * 10.0 + uv_q.y * 9.0);
		float fugran = rand(time * 235.0);
		uv_n.x += xsin * clamp(fugran - 0.91, 0.0, 1.0) * 0.5;
		float xsin3 = sin(time * 5.0 + uv_q.y * 12.0);
		float fugran3 = rand(time * 235.0);
		uv_n.x += xsin3 * clamp(fugran3 - 0.94, 0.0, 1.0) * 0.5;
		// y poition jumping
		float fugran2 = rand(time * 511.0);
		uv_n.y += clamp(fugran2 - 0.98, -0.1, 1.0) * 0.01;
		// x-position noise = little tape jitter 
		uv_n.x += rand(dot(uv_q,vec2(10.0,56.0)) + time * 21.0) * 0.003;
	}

    vec3 yuv = vec3(0.0);
	float fix = 0.3;
	float lumadelay = -0.002;

	// chroma bleed wobbling
    if (tv_artifacts == 1)
    {
		fix = 0.7;
		fix += sin(time * 2.3) * 0.2;
		fix += sin(time * 5.52) * 0.1;
		fix += sin(time * 23.0) * 0.1;
		fix += rand(uv_q.y*59.0+time * 40.0) * 0.4;
		fix *= 0.5;
		lumadelay = -0.005 + rand(uv_q.y*45.+time * 23.0) * 0.003;
	}
    
    for (float x = 1.0; x >= 0.0; x -= 0.1)
	{
		float x1 = (x * -0.05)* fix + lumadelay;
		float x2 = (x * 0.1)* fix + lumadelay;
		vec3 mult = (vec3(1.0) - pow(vec3(x), vec3(0.2, 1.0, 1.0))) * 0.2;
		vec2 uv1 = uv_n + vec2(x1,0.0);
		vec2 uv2 = uv_n + vec2(x2,0.0);
		vec2 uv1b = uv_n + vec2(x1, 1.0/resolution.x);
		vec2 uv2b = uv_n + vec2(x2, 1.0/resolution.x);
		yuv += (rgbtoyuv * process(uv1,tex).rgb) * mult;
		yuv += (rgbtoyuv * process(uv2,tex).rgb) * mult;
	}

	// normalize Y a bit
	yuv.r = yuv.r * 0.2 + (rgbtoyuv *  process(uv_n,tex).rgb).r * 0.8;
    
    if (tv_artifacts > 0.5)
	{
		// turn to grayscale when tape is really bad
		float runran = rand(time * 666.0);
		if (runran > 0.91)
			yuv.gb = vec2(0.0);

		float noiseamount = 0.0;
		float noiseenable = max(0.0, rand(time * 440.0 + uv.y * 440.0) - 0.995) / 0.015;
		if (noiseenable > 0.5)
				yuv.gb = vec2(0.0);
		noiseamount = (0.5 + sin(uv.x * 3.0 + time * 10.0) * 0.5) * noiseenable;
		yuv.r = mix(yuv.r, rand(time * 11. + uv_n.x * 13. + uv_n.y * 12.), noiseamount);
	}
    vec4 col = vec4(0.0);
    col.rgb = yuvtorgb * yuv * shade;

    float mod_factor = uv_n.y * resolution.y * resolution.y / resolution.y;
	vec3 dotMaskWeights = mix(vec3(1.0, 0.7, 1.0),vec3(0.7, 1.0, 0.7),floor(mod(mod_factor, 2.0)));
	col.rgb*= dotMaskWeights;
    float rand = getRand (uv_n*time*5.);
    vec4 noise = vec4(rand,rand,rand,1.0);
    col.rgb = mix(col.rgb,noise.rgb,noisemix);
    //scroll lines
   // col.rgb+=scrolllines(uv);
	float f = (1.0 - p.x *p.x) * (1.0 - p.y *p.y);
	float frame = clamp(frameSharpness * (pow(f, frameShape) - frameLimit), 0.0, 1.0);
	col.rgb*=frame;
    col -=toblack;
	fragColor = vec4(col.rgb,1.0);
}