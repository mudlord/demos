uniform vec2 resolution;
uniform vec2 time;
uniform sampler2D tex0;
uniform sampler2D tex1;
void main(void)
{
	//the centre point for each blob
	vec2 blobcoord = -1.0 + 2.0 * gl_FragCoord.xy / resolution.xy;
	vec2 move1,move2;
	move1.x = cos(time*1.5)*0.5;
	move1.y = sin(time*2.0)*0.5;
	move2.x = cos(time*2.5)*0.5;
	move2.y = sin(time*3.0)*0.5;

	vec2 uv;
	vec2 camera;
	camera.x = sin(time*0.534);
	camera.y = sin(time*0.3146);

	float a = atan(blobcoord.y-camera.y,blobcoord.x-camera.x);
	float r = sqrt(dot(blobcoord-camera,blobcoord-camera));
	uv.x += 0.534*time.x+1.0/r;
	uv.y += a;
	vec3 col =  texture2D(tex1,uv).rgb;
	col*=r;


	//radius for each blob
	float r1 =(dot(blobcoord-move1,blobcoord-move1))*8.0;
	float r2 =(dot(blobcoord+move2,blobcoord+move2))*16.0;
	float metaball =(col.r/r1+col.g/r2+col.b/2);
	vec2 posix = vec2(r1/sqrt(r2),r2/sqrt(r1));
	vec3 col2 =  texture2D(tex0, posix.xy/3.0).rgb;
	col.rgb += metaball;
	col2 *= col;
	gl_FragColor = vec4(col2,1.0);
}