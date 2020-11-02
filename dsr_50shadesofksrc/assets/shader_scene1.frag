#version 330 core

uniform float fGlobalTime; // in seconds
uniform vec2 v2Resolution; // viewport resolution (in pixels)
vec2 resolution = v2Resolution;
float time = fGlobalTime;
uniform sampler2D texTex1;
uniform sampler2D texTex2;
//uniform float camx;
//uniform float camy;
//uniform float camz;
//uniform float sc1_mengertweak1;
//uniform float sc1_mengertweak2;
//uniform float sc1_mengertweak3;
float camx = -0.25+3.2*cos(0.2*time + 4.0);
float camz = 0.5 + 1*cos(0.2*time + 2.0);
float camy = 1.2;
uniform float sc1_mengertweak1 = 0.225;
uniform float sc1_mengertweak2 = 1.5;
uniform float sc1_mengertweak3 = 1.5;


layout(location = 0) out vec4 FragColor; // out_color must be written in order to see anything

float Pi = 3.1415296;
#define INF 100000.0



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

   float s = 1.;
   for( int m=0; m<6; m++ )
   {
      vec3 a = mod( pos*s, sc1_mengertweak3 )-sc1_mengertweak1;
      s *= sc1_mengertweak2;
      vec3 r = 1.0 - 2.*a;

      float c = sdCross(r)/s;
      d = max(d,-c);
   }
 return d;
}

float opScale( vec3 p, float s )
{
    return mengersponge(p/s)*s;
}

vec2 map( in vec3 pos )
{
  float menger = opScale(pos-vec3( .0,0.5, 0.0),1.4);
    vec2 res = vec2(menger , 2.0 ) ;
    return res;
}

vec2 castRay( in vec3 ro, in vec3 rd )
{
    float t = .0;
    float m = 1.0;
    for( int i=0; i<68; i++ )
    {
	    vec2 res = map( ro+rd*t );
      t += res.x;
	    m = res.y;
    }
    return vec2( t, m );
}


float softshadow( in vec3 ro, in vec3 rd, in float mint, in float tmax )
{
	float res = 1.0;
    float t = mint;
    for( int i=0; i<16; i++ )
    {
		float h = map( ro + rd*t ).x;
        res = min( res, 8.0*h/t );
        t += clamp( h, 0.02, 0.10 );
        if( h<0.001 || t>tmax ) break;
    }
    return clamp( res, 0.0, 1.0 );

}

vec3 calcNormal( in vec3 pos )
{
	vec3 eps = vec3( 0.001, 0.0, 0.0 );
	vec3 nor = vec3(
	    map(pos+eps.xyy).x - map(pos-eps.xyy).x,
	    map(pos+eps.yxy).x - map(pos-eps.yxy).x,
	    map(pos+eps.yyx).x - map(pos-eps.yyx).x );
	return normalize(nor);
}

float calcAO( in vec3 pos, in vec3 nor )
{
	float occ = 0.0;
    float sca = 1.0;
    for( int i=0; i<5; i++ )
    {
        float hr = 0.01 + 0.12*float(i)/4.0;
        vec3 aopos =  nor * hr + pos;
        float dd = map( aopos ).x;
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



vec3 render( in vec3 ro, in vec3 rd )
{ 
    vec3 col = vec3(0.0, 0.0, 0.0);
    vec2 res = castRay(ro,rd);
    float dist = res.x;
	float m = res.y;
    if( m>-0.5 )
    {
        vec3 pos = ro + dist*rd;
        vec3 nor = calcNormal( pos );
        vec3 ref = reflect( rd, nor );


     float diff  = 1.0*clamp(dot(nor, normalize(vec3(1,1,1))), 0.0, 1.0); 
		float diff2 = 0.2*clamp(dot(nor, normalize(vec3(0.7,-1,0.5))), 0.0, 1.0); 
		float diff3 = 0.1*clamp(dot(nor, normalize(vec3(-0.7,-0.4,0.7))), 0.0, 1.0); 
		float spec = pow(clamp(dot(ref, normalize(vec3(1,1,1))), 0.0, 1.0), 50.0); 
    vec3 rf = texture(texTex1, ref.xy).xyz;
    vec3 tex2 = texture(texTex2,ref.xy*2.0).xyz;
    float x=getBorder(16.*ref.xy);
    vec4 color2=(x+1.0)*vec4(.1,.3,.5,0.0);

      if(m == 2.)
      {
    
	     col = diff*vec3(.1,.3,.5) + diff2*vec3(.1,.3,.5)+rf+tex2  + diff3*vec3(.1,.3,.5)*color2.rgb + spec*vec3(1,1,1)  + color2.rgb; 
		   col /= res.x *.44;

        }
    
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
		brdf += 0.02;
		col = col*brdf;

    }
	col *= res.x*0.3;

	return vec3( clamp(col,0.0,1.0) );
}

mat3 setCamera( in vec3 ro, in vec3 ta, float cr )
{
	vec3 cw = normalize(ta-ro);
	vec3 cp = vec3(sin(cr), cos(cr),0.0);
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
    return mat3( cu, cv, cw );
}


void main( void ) 
{
    vec2 p = 2.0 * gl_FragCoord.xy / resolution.xy - 1.0; 
	p.x *= resolution.x / resolution.y; 
		 
	float time2 = 15.0 + time;

	// camera
	vec3 ro = vec3( camx,camy , camz );
		vec3 ta = vec3( -0.5, -0.2, 0.3 );
	// camera-to-world transformation
  mat3 ca = setCamera( ro, ta, 0.0 );
 // ray direction
	vec3 rd = ca * normalize( vec3(p.xy,2.5) );
  // render	
  vec3 col = render( ro, rd );
//	col = pow( col, vec3(1.4545) );
	FragColor = vec4(col,1.0) ; 
}