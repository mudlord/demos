#include "libretro.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "sys/msys.h"
#include "starbg.h"
#include "metaballs.h"
#include "sys/msys_glext.h"
#include "libmodplug/modplug.h"
#include "music.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
static struct retro_hw_render_callback hw_render;
ModPlugFile* m_player;


static char *funciones = {
    // multitexture
    "glActiveTextureARB\x0"
    "glClientActiveTextureARB\x0"
    "glMultiTexCoord2fARB\x0"
    // programs
    "glDeleteProgramsARB\x0"
    "glBindProgramARB\x0"
    "glProgramStringARB\x0"
    "glProgramLocalParameter4fvARB\x0"
	"glProgramEnvParameter4fvARB\x0"
    // textures 3d
    "glTexImage3D\x0"
    // vbo-ibo
    "glBindBufferARB\x0"
    "glBufferDataARB\x0"
    "glBufferSubDataARB\x0"
    "glDeleteBuffersARB\x0"

	// shader
	"glCreateProgram\x0"
	"glCreateShader\x0"
	"glShaderSource\x0"
	"glCompileShader\x0"
	"glAttachShader\x0"
	"glLinkProgram\x0"
	"glUseProgram\x0"
    "glUniform4fv\x0"
    "glUniform1i\x0"
    "glGetUniformLocationARB\x0"
	"glGetObjectParameterivARB\x0"
	"glGetInfoLogARB\x0"

    "glLoadTransposeMatrixf\x0"

    "glBindRenderbufferEXT\x0"
    "glDeleteRenderbuffersEXT\x0"
    "glRenderbufferStorageEXT\x0"
    "glBindFramebufferEXT\x0"
    "glDeleteFramebuffersEXT\x0"
    "glCheckFramebufferStatusEXT\x0"
    "glFramebufferTexture1DEXT\x0"
    "glFramebufferTexture2DEXT\x0"
    "glFramebufferTexture3DEXT\x0"
    "glFramebufferRenderbufferEXT\x0"
    "glGenerateMipmapEXT\x0"
	"glCompressedTexImage2DARB\x0"
	"wglSwapIntervalEXT\x0"
	"glGenFramebuffersEXT\x0"
	 "glGenRenderbuffersEXT\x0"
	 "glUniform1fv\x0"
	 "glUniform2fv\x0"
	 "glBlendFuncSeparateEXT\x0"

    };

void *msys_oglfunc[NUMFUNCIONES];

//--- c o d e ---------------------------------------------------------------

int msys_glextInit( void )
{
    char *str = funciones;
    for( int i=0; i<NUMFUNCIONES; i++ )
        {
        msys_oglfunc[i] = (void*)hw_render.get_proc_address( str );
        str += 1+strlen( str );

        if( !msys_oglfunc[i] )
			return( 0 );
        }


    return( 1 );
}

void retro_init(void)
{
    ModPlug_Settings settings;
    ModPlug_GetSettings(&settings);
    settings.mFrequency = 44100;
    settings.mBits = 16;
    settings.mResamplingMode = 2;
    settings.mChannels = 2;
    settings.mLoopCount = -1; //hack
    ModPlug_SetSettings(&settings);
    m_player = ModPlug_Load(music, music_len);
    

}

void retro_deinit(void)
{
    if(m_player)ModPlug_Unload(m_player);
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   (void)port;
   (void)device;
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "Building Up And Down";
   info->library_version  = "v1";
   info->need_fullpath    = false;
   info->valid_extensions = NULL; // Anything is fine, we don't care.
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
	struct retro_game_geometry geom = { 1024,768, 1024,768,4.0/3.0 };
	struct retro_system_timing time = { 60.0, 44100.0};
	info->geometry = geom;
	info->timing = time;
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_set_environment(retro_environment_t cb)
{
	environ_cb = cb;
	bool no_rom = true;
	cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void runmusic()
{
    int16_t samples_buffer[2048];
    size_t samples_count = sizeof(samples_buffer);
    ModPlug_Read(m_player, samples_buffer, samples_count);
    audio_batch_cb(samples_buffer,samples_count/2);
}

void retro_run(void)
{
    runmusic();

	static int32_t lastTime = GetTime();
	int32_t currTime = GetTime();
	int32_t deltaTime = currTime - lastTime;
	lastTime = currTime;
	static float sceneTime = 0;
	sceneTime += (float)deltaTime/1000.f;
	static float texttime = 0;
	texttime += (float)deltaTime/1000.f;
	
	Resize(XRES,YRES);
	
    stars_render(sceneTime);
    metaballs_render();

    oglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, hw_render.get_current_framebuffer());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	stars_draw();
	metaballs_draw();

   video_cb(RETRO_HW_FRAME_BUFFER_VALID, 1024,768, 0);
}

static void context_reset(void)
{
   fprintf(stderr, "Context reset!\n");
   msys_glextInit();
   Resize(XRES,YRES);
   srand(NULL);
   metaballs_init();
   stars_init();
}

bool retro_load_game(const struct retro_game_info *info)
{
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      fprintf(stderr, "XRGB8888 is not supported.\n");
      return false;
   }

   hw_render.context_type = RETRO_HW_CONTEXT_OPENGL;
   hw_render.context_reset = context_reset;
   hw_render.depth = true;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render))
      return false;

   fprintf(stderr, "Loaded game!\n");
   (void)info;
   return true;
}

void retro_unload_game(void)
{}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
   (void)type;
   (void)info;
   (void)num;
   return false;
}

size_t retro_serialize_size(void)
{
   return 0;
}

bool retro_serialize(void *data, size_t size)
{
   (void)data;
   (void)size;
   return false;
}

bool retro_unserialize(const void *data, size_t size)
{
   (void)data;
   (void)size;
   return false;
}

void *retro_get_memory_data(unsigned id)
{
   (void)id;
   return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
   (void)id;
   return 0;
}

void retro_reset(void)
{}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}

