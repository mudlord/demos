#version 330 core

uniform float fGlobalTime; // in seconds
#version 330 core

uniform float fGlobalTime; // in seconds
uniform vec2 v2Resolution; // viewport resolution (in pixels)
vec2 resolution = v2Resolution;
uniform sampler2D texTex1;
uniform sampler2D texTex2;
uniform float sc3_rotatex;
uniform float sc3_rotatey;
uniform float sc3_rotatez;
uniform float sc3_tweak1;
uniform float sc3_tweak2;
uniform float sc3_tweak3;
uniform float sc3_tweak4;
uniform float sc3_tweak5;
uniform float sc3_mengerscale;
uniform float sc3_lightscale;

#version 330 core

uniform float fGlobalTime; // in seconds
uniform vec2 v2Resolution; // viewport resolution (in pixels)
vec2 resolution = v2Resolution;
uniform sampler2D texTex1;
uniform sampler2D texTex2;
float sc3_tweak1 = 1.7; 
float sc3_tweak2 = 2.;
float sc3_tweak3 = 1.;
float sc3_tweak4 = 2.1;
float sc3_tweak5 = 0.0;
float sc3_rotatex = 0.05;
float sc3_rotatey = 0.05;
float sc3_rotatez = .10;
float sc3_mengerscale = 1.7;
float sc3_lightscale = 24.0;

layout(location = 0) out vec4 FragColor; // out_color must be written in order to see anything
float time = fGlobalTime;
float Pi = 3.1415296;

vec3 rotatex(in vec3 p, float ang) { return vec3(p.x, p.y*cos(ang) - p.z*sin(ang), p.y*sin(ang) + p.z*cos(ang)); }
vec3 rotatey(in vec3 p, float ang) { return vec3(p.x*cos(ang) - p.z*sin(ang), p.y, p.x*sin(ang) + p.z*cos(ang)); }
vec3 rotatez(in vec3 p, float ang) { return vec3(p.x*cos(ang) - p.y*sin(ang), p.x*sin(ang) + p.y*cos(ang), p.z); }

#define INF 100000.0
float opS( float d1, float d2 )
{
    return max(-d2,d1);
}

vec2 opU( vec2 d1, vec2 d2 )
{
	return (d1.x<d2.x) ? d1 : d2;
}

vec3 opRep( vec3 p, vec3 c )
{
    return mod(p,c)-0.5*c;
}

vec3 opTwist( vec3 p )
{
    float  c = cos(10.0*p.y+10.0);
    float  s = sin(10.0*p.y+10.0);
    mat2   m = mat2(c,-s,s,c);
    return vec3(m*p.xz,p.y);
}



float sdPlane( vec3 p )
{
	return p.y;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

float sdCross( in vec3 p )
{
  float da = sdBox(p.xyz,vec3(INF,1.0,1.0));
  float db = sdBox(p.yzx,vec3(1.0,INF,1.0));
  float dc = sdBox(p.zxy,vec3(1.0,1.0,INF));
  return min(da,min(db,dc));
}



float mengersponge(vec3 pos)
{
 float d = sdBox(pos,vec3(1.));

   float s = sc3_tweak1;
   for( int m=0; m<4; m++ )
   {
      vec3 a = mod( pos*s, 2.0 )-.5;
      s *= sc3_tweak2;
      vec3 r = sc3_tweak3 - sc3_tweak4*a;

      float c = sdCross(r)/s;
      d = max(d,-c);
   }
 return d;
}

float opScale( vec3 p, float s )
{
    return mengersponge(p/s)*s;
}



float scene(vec3 pos)
{
  if (sc3_rotatex > 0.0)
  pos = rotatex(pos, sc3_rotatex*time - 3.0);
  if (sc3_rotatez > 0.0)
  pos = rotatez(pos, sc3_rotatez*time - 2.0);
  if (sc3_rotatey > 0.0)
  pos = rotatey(pos, sc3_rotatey*time - 2.0);
  if (sc3_tweak5 > 0.0)
  pos.z -= 0.14*time;
  vec3 q = mod(pos,2.)-.5*2.;
  q = abs(q);
  float menger = opScale(q-vec3( .0,-0.7, 0.0),sc3_mengerscale);
  vec2 res = vec2(menger , 2.0 ) ;
  return res.x; 


}

vec3 get_normal(vec3 p)
{
	vec3 eps = vec3(0.001, 0.0, 0.0); 
	float nx = scene(p + eps.xyy) - scene(p - eps.xyy); 
	float ny = scene(p + eps.yxy) - scene(p - eps.yxy); 
	float nz = scene(p + eps.yyx) - scene(p - eps.yyx); 
	return normalize(vec3(nx,ny,nz)); 
}

float calcAO( in vec3 pos, in vec3 nor )
{
	float occ = 0.0;
    float sca = 1.0;
    for( int i=0; i<5; i++ )
    {
        float hr = 0.01 + 0.12*float(i)/4.0;
        vec3 aopos =  nor * hr + pos;
        float dd = scene( aopos );
        occ += -(dd-hr)*sca;
        sca *= 0.95;
    }
    return clamp( 1.0 - 3.0*occ, 0.0, 1.0 );    
}


vec2 noise(vec2 t)
{
return t=vec2(dot(t,vec2(127.1,311.7)),dot(t,vec2(269.5,183.3))),fract(sin(t)*43758.5);
}
float voronoi(vec2 v)
	{
	vec2 r=floor(v),e=fract(v);
	float f=8.;
	for(float u=-1.;u<=1.;u++)
	for(float m=-1.;m<=1.;m++)
	{
	vec2 g=vec2(m,u),d=noise(r+g);
	d=.1+.5*sin(time+4.2831*d);
	vec2 s=g+d-e;
	float o=dot(s,s);
	f=min(f,o);
	}
	return f;
}


float getBorder( in vec2 p )
{
    float dis = voronoi( p );

    return 1.0 - smoothstep( 0.0, .75, dis );
}




vec3 plasma(vec2 fragCoord)
{
	float u_k = 3.;
    float v = 0.0;
    vec2 c =fragCoord * u_k - u_k/2.0;
    v += sin((c.x+time));
    v += sin((c.y+time)/2.0);
    v += sin((c.x+c.y+time)/4.0);
    c += u_k/2.0 * vec2(sin(time/3.0), cos(time/2.0));
    v += sin(sqrt(c.x*c.x+c.y*c.y+1.0)+time);
    v = v*2.0;
    float r=sin(Pi*v);
    float g= sin(Pi*v+2.*Pi/4.);
    float b=sin(Pi*v+4.*Pi/3.);
    vec3 col = vec3(r,g , b);
    return vec3(col*.5 + .5); 
}

void main( void ) 
{
	vec2 p = 2.0 * gl_FragCoord.xy / resolution - 1.0; 
	p.x *= resolution.x / resolution.y; 
	 vec2 sp=gl_FragCoord.rg/resolution.rg;
	vec3 ro = vec3(0.0, 0.0, 1.); 
	vec3 rd = normalize(vec3(p.x, p.y, -1.6)); 
	vec3 color = vec3(0.0);   
	vec3 pos = ro; 
	float dist = 0.0; 

	 
	for (int i = 0; i < 85; i++) 
	{
		float d = scene(pos); 
		pos += rd*d; 
		dist += d; 
	}
  
   
		vec3 nor = get_normal(pos);
		vec3 ref = reflect(normalize(pos - ro),nor); 

		float diff  = 1.0*clamp(dot(nor, normalize(vec3(1,1,1))), 0.0, 1.0); 
		float diff2 = 0.2*clamp(dot(nor, normalize(vec3(0.7,-1,0.5))), 0.0, 1.0); 
		float spec = pow(clamp(dot(ref, normalize(vec3(1,1,1))), 0.0, 1.0), 50.0); 
    

   vec3 l1 = normalize(vec3(sin(time-0.5)*0.5, sin(time-0.5)*0.5, -1));
	 vec3 l2 = normalize(vec3(sin(time-0.5)*0.5, -1, sin(time-0.5)*0.5));
   vec3 l3 = normalize(vec3(-1, sin(time-0.5)*0.5, sin(time-0.5)*0.5));
   vec3 l4 = normalize(vec3(sin(time-0.5)*0.5, sin(time-0.5)*0.5, sin(time-0.5)*0.5));
vec3 l5 = normalize(vec3(sin(time-0.5)*0.5, sin(time-0.7)*0.3, sin(time-0.5)*0.5));
   float spec1 = pow(max(0.5, dot(reflect(l1, nor), rd)), sc3_lightscale);
	float spec2 = pow(max(0.5, dot(reflect(l2, nor), rd)), sc3_lightscale);
   float spec3 = pow(max(0.5, dot(reflect(l3, nor), rd)), sc3_lightscale);
float spec4 = pow(max(0.5, dot(reflect(l4, nor), rd)), sc3_lightscale);
float spec5 = pow(max(0.5, dot(reflect(l4, nor), rd)), sc3_lightscale);

    vec3 rf = texture(texTex1, ref.xy*0.5).xyz;
    vec3 tex2 = texture(texTex2,ref.xy*1.0).xyz;
    float x=getBorder(16.*sp);
    vec4 color2=(x+1.0)*vec4(.1,.3,.5,0.0);
		color = diff*vec3(.1,.3,.5) + diff2*vec3(.1,.3,.5)+rf+tex2 *color2.rgb*plasma(ref.xy)+ (spec3*plasma(ref.xy)) + (spec2*vec3(1,1,0))+ (spec*plasma(ref.xy)*1.5) + (spec1*plasma(ref.xy))
    + (spec4*plasma(ref.xy)) + (spec5*plasma(ref.xy)); 
  
      float occ = calcAO( pos, nor );
		   vec3  lig = normalize( vec3(-0.6, 0.7, -0.5) );
		   float amb = clamp( 0.5+0.5*ref.y, 0.0, 1.0 );
        float bac = clamp( dot( ref, normalize(vec3(-lig.x,0.0,-lig.z))), 0.0, 1.0 )*clamp( 1.0-pos.y,0.0,1.0);
        float dom = smoothstep( -0.1, 0.1, ref.y );
        float fre = pow( clamp(1.0+dot(ref,rd),0.0,1.0), 2.0 );
 
		    vec3 brdf = vec3(0.0);
        brdf += 0.30*amb*vec3(0.50,0.70,1.00)*occ;
        brdf += 0.40*dom*vec3(0.50,0.70,1.00)*occ;
        brdf += 0.30*bac*vec3(0.25,0.25,0.25)*occ;
        brdf += 0.40*fre*vec3(1.00,1.00,1.00)*occ;
		brdf += .12;
		color = color*brdf;
    
    
	  float sh = clamp( pow(6.5/length(pos - ro), 2.0), 0.0, 1.0);
    vec3 light = normalize(vec3(1.0,0.9,0.3));
    float shf = dot(rd,light);
    float sh1 = pow( max( shf,0.0), 4.1);
    vec3 shcol = mix(vec3(.1,.3,.5), vec3(.1,.3,.5), sh1);
    color.rgb = mix(shcol, color.rgb, sh); // add fog    


    //color /= clamp((dist*1),1.,1.);
	FragColor = vec4(color, 1.0); 
}