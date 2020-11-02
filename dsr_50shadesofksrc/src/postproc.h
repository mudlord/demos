typedef struct
{
	float x, y, z;
	int xsize, ysize;
	GLuint texture;
	GLuint program;
	float xtexoffset, ytexoffset;
	GLuint sprite_vertices, sprite_texcoords, vao;
	GLint attribute_v_coord, attribute_v_texcoord;
	GLint uniform_mvp, uniform_mytexture;
}tv_effect;

FBOELEM postproc_fbo; //1280/640, 720 /360
FBOELEM blur_fbo[4];
tv_effect tv;
tv_effect motionblur;
GLuint tex_topgrad;
int splash_downw;
int splash_downh;
int PingPong = 0;

void init_sc2blur()
{
	memset(&motionblur, 0, sizeof(tv_effect));
	blur_fbo[0] = init_fbo(XRES, YRES);
	blur_fbo[1] = init_fbo(XRES, YRES);
	motionblur.xsize = XRES;
	motionblur.ysize = YRES;
	motionblur.x = XRES / 2;
	motionblur.y = YRES / 2;

	const char vertex_source[] =
		"#version 330\n"
		"in vec4 v_coord;\n"
		"in vec2 v_texcoord;\n"
		"out vec2 ftexcoord;\n"
		"uniform mat4 mvp;\n"
		"void main() {\n"
		"   ftexcoord = v_texcoord;\n"
		"   gl_Position = mvp * v_coord;\n"
		"}\n";
	//okay
	const char motionblur_frag[] =
		"#version 330\n"
		"in vec2 ftexcoord;\n"
		"layout(location = 0) out vec4 FragColor;\n"
		"uniform float mb_amount;"
		"uniform sampler2D tex1;"
		"uniform sampler2D tex2;"
		"uniform vec2 resolution;"
		"void main()"
		"{"
		"vec2 p=gl_FragCoord.rg/resolution.rg;"
		"vec4 color=texture2D(tex1,p);"
		"vec4 color2=texture2D(tex2, p);"
		//"color = color + color2;"
		"vec4 res = color * mb_amount + color2 * (1.0 - mb_amount);"
		//"color /=2;"
		"FragColor = vec4(res);"
		"}";

	GLfloat sprite_vertices[] = {
		0, 0, 0,
		motionblur.xsize, 0, 0,
		0, motionblur.ysize, 0,
		motionblur.xsize, motionblur.ysize, 0,
	};

	glGenVertexArrays(1, &motionblur.vao);
	glBindVertexArray(motionblur.vao);

	glGenBuffers(1, &motionblur.sprite_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, motionblur.sprite_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_STATIC_DRAW);


	GLfloat sprite_texcoords[] = {
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	};
	glGenBuffers(1, &motionblur.sprite_texcoords);
	glBindBuffer(GL_ARRAY_BUFFER, motionblur.sprite_texcoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_texcoords), sprite_texcoords, GL_STATIC_DRAW);

	glBindVertexArray(0);

	initShader((int*)&motionblur.program, vertex_source, (const char*)motionblur_frag);
}

void draw_sc2blur()
{
	glUseProgram(motionblur.program);


	const char* attribute_name;
	attribute_name = "v_coord";
	motionblur.attribute_v_coord = glGetAttribLocation(motionblur.program, attribute_name);
	attribute_name = "v_texcoord";
	motionblur.attribute_v_texcoord = glGetAttribLocation(motionblur.program, attribute_name);
	const char* uniform_name;
	uniform_name = "mvp";
	motionblur.uniform_mvp = glGetUniformLocation(motionblur.program, uniform_name);
	uniform_name = "tex";
	motionblur.uniform_mytexture = glGetUniformLocation(motionblur.program, uniform_name);


	GLint uniform_resolution = glGetUniformLocation(motionblur.program, "resolution");
	GLint uniform_mbamount = glGetUniformLocation(motionblur.program, "mb_amount");


	glm::mat4 projection = glm::ortho(0.0f, (float)XRES, (float)YRES, 0.0f);
	float  tX = motionblur.xsize / 2.0f;
	float  tY = motionblur.ysize / 2.0f;
	glm::mat4 m_transform = glm::translate(glm::mat4(1.0f), glm::vec3((float)motionblur.x - tX, (float)motionblur.y - tY, 0.0));
	glm::mat4 mvp = projection * m_transform; // * view * model * anim;
	glUniformMatrix4fv(motionblur.uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));


	char temp[5] = { 0 };
	int blurtex[4];
	for (int i = 0; i < 2; i++)
	{
		ZeroMemory(temp, 5);
		sprintf(temp, "tex%d", i + 1, 4);
		blurtex[i] = glGetUniformLocation(motionblur.program, temp);
	}

	for (int i = 0; i < 2; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, blur_fbo[((PingPong + i) % 2)].texture);
		glUniform1i(blurtex[i], i);
	}




	glUniform2f(uniform_resolution, XRES, YRES);
	glUniform1f(uniform_mbamount, mbamount_f);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(motionblur.vao);
	GLfloat sprite_vertices[] = {
		0, 0, 0,
		motionblur.xsize, 0, 0,
		0, motionblur.ysize, 0,
		motionblur.xsize, motionblur.ysize, 0,
	};
	glEnableVertexAttribArray(motionblur.attribute_v_coord);
	glBindBuffer(GL_ARRAY_BUFFER, motionblur.sprite_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(
		motionblur.attribute_v_coord,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
		);
	glEnableVertexAttribArray(motionblur.attribute_v_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, motionblur.sprite_texcoords);
	glVertexAttribPointer(
		motionblur.attribute_v_texcoord,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
		);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(motionblur.attribute_v_coord);
	glDisableVertexAttribArray(motionblur.attribute_v_texcoord);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glUseProgram(0);
}

void init_postproc()
{
	memset(&tv, 0, sizeof(tv_effect));
	postproc_fbo = init_fbo(XRES, YRES);
	tv.xsize = XRES;
	tv.ysize = YRES;
	tv.x = XRES / 2;
	tv.y = YRES / 2;

	const char vertex_source[] =
		"#version 330\n"
		"in vec4 v_coord;\n"
		"in vec2 v_texcoord;\n"
		"out vec2 ftexcoord;\n"
		"uniform mat4 mvp;\n"
		"void main() {\n"
		"   ftexcoord = v_texcoord;\n"
		"   gl_Position = mvp * v_coord;\n"
		"}\n";





	const char fragment_source[] = ""
		"#version 330\n"
		"in vec2 ftexcoord;\n"
		"layout(location = 0) out vec4 fragColor;\n"
		"uniform sampler2D tex;"
		"uniform vec2 resolution;"
		"uniform float time,frameLimit,frameShape,frameSharpness,strp_cnt,strp_trnsinst,toblack,noisemix,flip_y,tv_artifacts;"
		"float t(float t)"
		"{"
		"vec2 v=vec2(t,t);"
		"float f=12.9898,s=78.233,r=43758.5,b=dot(v.rg,vec2(f,s)),x=mod(b,3.14);"
		"return fract(sin(x)*r);"
		"}"
		"float f(vec2 t)"
		"{"
		"vec2 r=vec2(time*.0021,time*.0001);"
		"return fract(sin(dot(t.rg,vec2(12.9898,78.233)+r))*43758.5);"
		"}"
		"vec4 f(vec2 t,sampler2D v)"
		"{"
		"float s=floor(t.g*max(1.,strp_cnt))/max(1.,strp_cnt),b=f(vec2(0.,-s)),e=f(vec2(0.,s))*strp_trnsinst;"
		"vec2 r=vec2(mod(t.r+e*.25,1.),t.g+min(1.,1.+mod(t.r+e*.25,1.))-1.);"
		"vec4 g=texture2D(v,r);"
		"return g;"
		"}"
		"vec2 t(vec2 t,float r)"
		"{"
		"return t=(t-.5)*2.,t*=.5,t.r*=1.+pow(abs(t.g)/r,2.),t.g*=1.+pow(abs(t.r)/r,2.),t=t+.5,t;"
		"}"
		"float f(float t,float v,float r)"
		"{"
		"float s=step(v,t)-step(r,t),f=(t-v)/(r-v)*s;"
		"return(1.-f)*s;"
		"}"
		"float n(vec2 t)"
		"{"
		"float r=f(t*time*5.);"
		"return f(mod(t.g*2.+time/2.+sin(time+sin(time*.63)),2.),.5,.6)*r;"
		"}"
		"void main()"
		"{"
		"vec2 v=gl_FragCoord.rg/resolution.rg;"
		"if(flip_y==1.)"
		"v.g=1.-v.g;"
		"v=t(v,2.4);"
		"vec2 r=-1.+2.*v,s=v,e=s;"
		"mat3 g=mat3(.299,-.147,.615,.587,-.289,-.515,.114,.436,-.1),b=mat3(1.,1.,1.,0.,-.395,2.032,1.14,-.581,0.);"
		"float i=1.;"
		"i-=t(s.r*.1+s.g*50.+time)*.5;"
		"if(tv_artifacts==2.)"
		"{"
		"e.g+=t(time*12.)*.004;"
		"e.r+=t(s.g*39.+time*20.)*.01044-.012;"
		"float m=sin(time*10.+s.g*9.),x=t(time*235.);"
		"e.r+=m*clamp(x-.91,0.,2.)*2.5;"
		"float c=sin(time*5.+s.g*12.),n=t(time*235.);"
		"e.r+=c*clamp(n-.94,0.,1.)*.5;"
		"float a=t(time*511.);"
		"e.g+=clamp(a-.98,-.1,1.)*.01;"
		"e.r+=t(dot(s,vec2(10.,56.))+time*121.)*.013;"
		"}"
		"vec3 m=vec3(0.);"
		"float n=.3,x=-.002;"
		"if(tv_artifacts==1.)"
		"n=.7,n+=sin(time*2.3)*.2,n+=sin(time*5.52)*.1,n+=sin(time*23.)*.1,n+=t(s.g*59.+time*40.)*.4,n*=.5,x=-.005+t(s.g*45.+time*23.)*.003;"
		"for(int a=10;a>=0;a-=1)"
		"{"
		"float c=float(a)/10.;"
		"if(c<0.)"
		"c=0.;"
		"float d=c*-.05*n+x,y=c*.1*n+x;"
		"vec3 k=(vec3(1.)-pow(vec3(c),vec3(.2,1.,1.)))*.2;"
		"vec2 p=e+vec2(d,0.),o=e+vec2(y,0.);"
		"m+=g*f(p,tex).rgb*k;"
		"m+=g*f(o,tex).rgb*k;"
		"}"
		"m.r=m.r*.2+(g*f(e,tex).rgb).r*.8;"
		"if(tv_artifacts>.5)"
		"{"
		"float a=t(time*666.);"
		"if(a>.91)"
		"m.gb=vec2(0.);"
		"float c=0.,k=max(0.,t(time*440.+v.g*440.)-.995)/.015;"
		"if(k>.5)"
		"m.gb=vec2(0.);"
		"c=(.5+sin(v.r*3.+time*10.)*.5)*k;"
		"m.r=mix(m.r,t(time*11.+e.r*13.+e.g*12.),c);"
		"}"
		"vec4 a=vec4(0.);"
		"a.rgb=b*m*i;"
		"float c=e.g*resolution.g*resolution.g/resolution.g;"
		"vec3 k=mix(vec3(1.,.7,1.),vec3(.7,1.,.7),floor(mod(c,2.)));"
		"a.rgb*=k;"
		"float d=f(e*time*5.);"
		"vec4 y=vec4(d,d,d,1.);"
		"a.rgb=mix(a.rgb,y.rgb,noisemix);"
		"float p=(1.-r.r*r.r)*(1.-r.g*r.g),o=clamp(frameSharpness*(pow(p,frameShape)-frameLimit),0.,1.);"
		"a.rgb*=o;"
		"a.rgb-=toblack;"
		"if(toblack>.95)"
		"a.rgb=vec3(0.);"
		"fragColor=vec4(a.rgb,1.);"
		"}";


	/*const char  fragment_source[] =
	"#version 330\n"
	"in vec2 ftexcoord;\n"
	"layout(location = 0) out vec4 FragColor;\n"
	"uniform sampler2D mytexture;\n"
	"void main()"
	"{"
	"vec2 coords=ftexcoord;\n"
	"coords.y = -coords.y;\n"
	"FragColor=texture2D(mytexture,coords);"
	"}";*/

	GLfloat sprite_vertices[] = {
		0, 0, 0,
		tv.xsize, 0, 0,
		0, tv.ysize, 0,
		tv.xsize, tv.ysize, 0,
	};

	glGenVertexArrays(1, &tv.vao);
	glBindVertexArray(tv.vao);

	glGenBuffers(1, &tv.sprite_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, tv.sprite_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_STATIC_DRAW);


	GLfloat sprite_texcoords[] = {
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	};
	glGenBuffers(1, &tv.sprite_texcoords);
	glBindBuffer(GL_ARRAY_BUFFER, tv.sprite_texcoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_texcoords), sprite_texcoords, GL_STATIC_DRAW);

	glBindVertexArray(0);

	initShader((int*)&tv.program, vertex_source, (const char*)fragment_source);

}

void draw_postproc(int xres, int yres, float scenetime,float flip_y)
{
	glUseProgram(tv.program);


	const char* attribute_name;
	attribute_name = "v_coord";
	tv.attribute_v_coord = glGetAttribLocation(tv.program, attribute_name);
	attribute_name = "v_texcoord";
	tv.attribute_v_texcoord = glGetAttribLocation(tv.program, attribute_name);
	const char* uniform_name;
	uniform_name = "mvp";
	tv.uniform_mvp = glGetUniformLocation(tv.program, uniform_name);
	uniform_name = "tex";
	tv.uniform_mytexture = glGetUniformLocation(tv.program, uniform_name);


	GLint uniform_resolution = glGetUniformLocation(tv.program, "resolution");
	GLint uniform_time = glGetUniformLocation(tv.program, "time");

	GLint uniform_framelimit = glGetUniformLocation(tv.program, "frameLimit");
	GLint uniform_frameShape = glGetUniformLocation(tv.program, "frameShape");
	GLint uniform_frameSharpness = glGetUniformLocation(tv.program, "frameSharpness");
	GLint uniform_strp_cnt = glGetUniformLocation(tv.program, "strp_cnt");
	GLint uniform_strp_trnsinst = glGetUniformLocation(tv.program, "strp_trnsinst");
	GLint uniform_toblack = glGetUniformLocation(tv.program, "toblack");
	GLint uniform_noisemix = glGetUniformLocation(tv.program, "noisemix");
	GLint uniform_flipy = glGetUniformLocation(tv.program, "flip_y");
	GLint uniform_tvartifacts = glGetUniformLocation(tv.program, "tv_artifacts");


	glm::mat4 projection = glm::ortho(0.0f, (float)XRES, (float)YRES, 0.0f);
	float  tX = tv.xsize / 2.0f;
	float  tY = tv.ysize / 2.0f;
	glm::mat4 m_transform = glm::translate(glm::mat4(1.0f), glm::vec3((float)tv.x - tX, (float)tv.y - tY, 0.0));
	glm::mat4 mvp = projection * m_transform; // * view * model * anim;
	glUniformMatrix4fv(tv.uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tv.texture);
	glUniform1i(tv.uniform_mytexture, 0);
	glUniform2f(uniform_resolution, XRES, YRES);
	glUniform1f(uniform_time, scenetime);

	glUniform1f(uniform_framelimit, titleframelimit_f);
	glUniform1f(uniform_frameShape, titleframeshape_f);
	glUniform1f(uniform_frameSharpness, titleframesharpness_f);
	glUniform1f(uniform_strp_cnt, titlestrpcnt_f);
	glUniform1f(uniform_strp_trnsinst, titlestrpintens_f);
	glUniform1f(uniform_toblack, titlestoblack_f);
	glUniform1f(uniform_noisemix, titlesnoisemix_f);
	glUniform1f(uniform_flipy, flip_y);
	glUniform1f(uniform_tvartifacts, tv_artifacts);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(tv.vao);
	GLfloat sprite_vertices[] = {
		0, 0, 0,
		tv.xsize, 0, 0,
		0, tv.ysize, 0,
		tv.xsize, tv.ysize, 0,
	};
	glEnableVertexAttribArray(tv.attribute_v_coord);
	glBindBuffer(GL_ARRAY_BUFFER, tv.sprite_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(
		tv.attribute_v_coord,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
		);
	glEnableVertexAttribArray(tv.attribute_v_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, tv.sprite_texcoords);
	glVertexAttribPointer(
		tv.attribute_v_texcoord,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
		);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(tv.attribute_v_coord);
	glDisableVertexAttribArray(tv.attribute_v_texcoord);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glUseProgram(0);
}