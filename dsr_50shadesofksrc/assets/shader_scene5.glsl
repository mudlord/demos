#version 330 core

uniform float fGlobalTime; // in seconds
uniform vec2 v2Resolution; // viewport resolution (in pixels)
vec2 resolution = v2Resolution;

uniform sampler2D texTex1;
uniform sampler2D texTex2;

layout(location = 0) out vec4 FragColor; // out_color must be written in order to see anything
float time = fGlobalTime;
float Pi = 3.1415296;
#define INF 100000.0
float sc1_mengercrossup = -0.7;
float sc1_cubeup = 0.15;
float sc1_crossmorph = 1.0;
float sc1_mengertweak1 = 0.33;
//----------------------------------------------------------------------

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
         length(max(d,0.0)) ;
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
   for( float m=0; m<5; m++ )
   {
      vec3 a = mod( pos*s, 2 )-sc1_mengertweak1;
      s *= 2.0;
      vec3 r = 1.0 - 1.5*a;
      
      r+= sin(time)*0.47;
      float c = sdCross(r)/s;
      d = max(d,-c);
   }
 return d;
}

float opScale( vec3 p, float s )
{
    return mengersponge(p/s)*s;
}


float sdBox2( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0)) +clamp(sin((p.x+p.y+p.z)*6.0)*0.02, 0.0, 1.0);
}

float sdCross2( in vec3 p )
{
  float da = sdBox2(p.xyz,vec3(INF,1.0,1.0));
  float db = sdBox2(p.yzx,vec3(1.0,INF,1.0));
  float dc = sdBox2(p.zxy,vec3(1.0,1.0,INF));
  return min(da,min(db,dc));
}

float opScale2( vec3 p, float s )
{
    return sdCross2(p/s)*s;
}


vec2 map( in vec3 pos )
{    
  vec3 q = mod(pos,2.)-0.5*2.;
  vec3 p = pos;
  

 float roundbox = length(max(abs(p-vec3( .0,sc1_cubeup, 0.0))-vec3(0.25),0.0)) - 0.01 + clamp(sin((p.x+p.y+p.z)*23.0)
     *0.02, 0.0, 1.0);   
  float menger = opScale(q-vec3( 0.0,sc1_mengercrossup, 0.0),1.);
  menger = mix(menger,roundbox,0.);
  float cross = opScale2(q-vec3( 0.0,sc1_mengercrossup, 0.0),0.1);
 roundbox = mix(cross,roundbox,sc1_crossmorph);
    vec2 res = opU( vec2( sdPlane(pos), 1.0 ),
	                vec2(menger , 2.0 ) );
   res = opU(res,vec2(roundbox,2.0)); 
    return res;
}

vec2 castRay( in vec3 ro, in vec3 rd )
{
    float t = 0.0;
    float m = -1.0;
    for( int i=0; i<75; i++ )
    {
	    vec2 res = map( ro+rd*t );
      t += res.x;
	    m = res.y;
      if (res.x <= 0.001)return vec2( t, m );
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
		
		   vec3 l1 = normalize(vec3(sin(time-0.5)*0.7, sin(time-0.5)*0.5, 1));
	vec3 l2 = normalize(vec3(-1, -1, sin(time-0.5)*0.7));
vec3 l3 = normalize(vec3(-1, -1, 0));
float spec1 = pow(max(0.5, dot(reflect(l1, nor), rd)), 20.0);
			float spec2 = pow(max(0.5, dot(reflect(l2, nor), rd)), 20.0);
float spec3 = pow(max(0.5, dot(reflect(l3, nor), rd)), 20.0);
vec3 lights = (spec3*vec3(1,0,0)) + (spec2*vec3(1,1,0))+ (spec*vec3(1,1,1)) + (spec1*vec3(0,0,1)); 		
		
    vec3 rf = texture(texTex1, ref.xy).xyz;
    vec3 tex2 = texture(texTex2,ref.xy*2.0).xyz;
    float x=getBorder(16.*ref.xy);
    vec4 color2=(x+1.0)*vec4(.1,.3,.5,0.0);

      if(m == 2.)
      {
    
	     col = diff*vec3(.1,.3,.5) + diff2*vec3(.1,.3,.5)+rf+tex2  + diff3*vec3(.1,.3,.5)*color2.rgb + lights; 
		   col *=res.x;

        }

if (m==3.)
{
       col = vec3(.1,.3,.5); 
		   col *=res.x;
}
    

        if( m == 1. )
        {
            
            float f = mod( floor(5.0*pos.z) + floor(5.0*pos.x), 2.0);
            col =diff2+rf+tex2  +f+ diff3*vec3(.1,.3,.5)*color2.rgb + spec*vec3(1,1,1)  + color2.rgb; 
            col /= res.x *0.54;

        }
       float occ = calcAO( pos, nor );
		   vec3  lig = normalize( vec3(-0.6, 0.7, -0.5) );
		   float amb = clamp( 0.5+0.5*ref.y, 0.0, 1.0 );
        float dif = clamp( dot( ref, lig ), 0.0, 1.0 );
        float bac = clamp( dot( ref, normalize(vec3(-lig.x,0.0,-lig.z))), 0.0, 1.0 )*clamp( 1.0-pos.y,0.0,1.0);
        float dom = smoothstep( -0.1, 0.1, ref.y );
        float fre = pow( clamp(1.0+dot(ref,rd),0.0,1.0), 2.0 );
        dif *= softshadow( pos, lig, 0.02, 2.5 );
        dom *= softshadow( pos, ref, 0.02, 2.5 );

		    vec3 brdf = vec3(0.0);
        brdf += 0.30*amb*vec3(0.50,0.70,1.00)*occ;
        brdf += 0.40*dom*vec3(0.50,0.70,1.00)*occ;
        brdf += 0.30*bac*vec3(0.25,0.25,0.25)*occ;
        brdf += 0.40*fre*vec3(1.00,1.00,1.00)*occ;
		brdf += 0.02;
		col = col*brdf;

    }
	col /= res.x*0.7;

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
  vec2 p = 2.0 * gl_FragCoord.xy / resolution - 1.0;
	p.x *= resolution.x/resolution.y;
		 
	float time2 = 15.0 + time;

	// camera	
	vec3 ro = vec3( -0.25+3.2*cos(0.1*time + 6.0), 1.0 + .5, 0.5 + 2.2*sin(0.1*time + 6.0) );
		vec3 ta = vec3( -0.5, -0.4, 0.5 );
	// camera-to-world transformation
  mat3 ca = setCamera( ro, ta, 0.0 );
 // ray direction
	vec3 rd = ca * normalize( vec3(p.xy,2.5) );
  // render	
  vec3 col = render( ro, rd );
	col = pow( col, vec3(1.4545) );
	FragColor = vec4(col,1.0) ; 
}