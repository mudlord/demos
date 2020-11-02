/*
** ST3PLAY v0.72 - 13th of July 2015 - http://16-bits.org
** ======================================================
**
** Changelog from v0.71:
** - Code cleanup
** - Proper check for memory allocations
**
** Changelog from v0.70:
** - Any pan commands except Lxy/Yxy/XA4 should disable channel surround
**
** Changelog from v0.60:
** - Added S2x (non-ST3, set middle-C finetune)
** - Added S6x (non-ST3, tick delay)
** - Added S9E/S9F (non-ST3, play sample backwards/forwards)
** - Fixed a bug in setspd() in Amiga limit mode
** - Proper tracker handling for non-ST3 effects
** - Panbrello (Yxy) didn't set the panning at all (heh)
** - Mxx (set cannel volume) didn't work correctly
**
** C port of Scream Tracker 3.21's replayer, by 8bitbubsy (Olav Sørensen)
** using the original asm source codes by PSI (Sami Tammilehto) of Future Crew
**
** This is by no means a piece of beautiful code, nor is it meant to be...
** It's just an accurate Scream Tracker 3.21 replayer port for people to enjoy.
**
** Non-ST3 additions from other trackers (only handled for non ST3 S3Ms):
**
** - Mixing:
**   * 16-bit sample support
**   * Stereo sample support
**   * 2^31-1 sample length support
**   * Middle-C speeds beyond 65535
**   * Process the last 16 channels as PCM
**   * Process 8 octaves instead of 7
**   * The ability to play samples backwards
**
** - Effects:
**   * Command S2x        (set middle-C finetune)
**   * Command S5x        (panbrello type)
**   * Command S6x        (tick delay)
**   * Command S9x        (sound control - only S90/S91/S9E/S9F)
**   * Command Mxx        (set channel volume)
**   * Command Nxy        (channel volume slide)
**   * Command Pxy        (panning slide)
**   * Command Txx<0x20   (tempo slide)
**   * Command Wxy        (global volume slide)
**   * Command Xxx        (7+1-bit pan) + XA4 for 'surround'
**   * Command Yxy        (panbrello)
**   * Volume Command Pxx (set 4+1-bit panning)
**
** - Variables:
**   * Pan changed from 4-bit (0..15) to 8+1-bit (0..256)
**   * Memory variables for the new N/P/T/W/Y effects
**   * Panbrello counter
**   * Panbrello type
**   * Channel volume multiplier
**   * Channel surround flag
**
** - Other changes:
**   * Added tracker identification to make sure Scream Tracker 3.xx
**     modules are still played exactly like they should. :-)
**
** You need to link winmm.lib for this to compile (-lwinmm)
**
** User functions:
**
** #include <stdint.h>
**
** int8_t st3play_LoadModule(const uint8_t *buffer, uint32_t size);
** int8_t st3play_Init(uint32_t outputFreq, int8_t interpolation);
** void st3play_FreeSong(void);
** void st3play_Close(void);
** void st3play_PauseSong(int8_t pause);
** void st3play_PlaySong(void);
*/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <windows.h>
#include <mmsystem.h>

#define _LERP(x, y, z) ((x) + ((y) - (x)) * (z))
#define SOUND_BUFFERS 7

/* TRACKER ID */
enum
{
    SCREAM_TRACKER  = 1,
    IMAGO_ORPHEUS   = 2,
    IMPULSE_TRACKER = 3,
    SCHISM_TRACKER  = 4,
    OPENMPT         = 5,
    BEROTRACKER     = 6
    /* there is also CREAMTRACKER (7), but let's ignore that for now */
};

/* STRUCTS */
typedef struct chn
{
    int8_t aorgvol;
    int8_t avol;
    uint8_t channelnum;
    uint8_t achannelused;
    uint8_t aglis;
    uint8_t atremor;
    uint8_t atreon;
    uint8_t atrigcnt;
    uint8_t anotecutcnt;
    uint8_t anotedelaycnt;
    uint8_t avibtretype;
    uint8_t note;
    uint8_t ins;
    uint8_t vol;
    uint8_t cmd;
    uint8_t info;
    uint8_t lastins;
    uint8_t lastnote;
    uint8_t alastnfo;
    uint8_t alasteff;
    uint8_t alasteff1;
    int16_t apanpos;
    int16_t avibcnt;
    uint16_t astartoffset;
    uint16_t astartoffset00;
    int32_t ac2spd;
    int32_t asldspd;
    int32_t aorgspd;
    int32_t aspd;

    /* NON-ST3 variables */
    int8_t chanvol;
    uint8_t surround;
    uint8_t apantype;
    uint8_t nxymem;
    uint8_t pxymem;
    uint8_t txxmem;
    uint8_t wxymem;
    uint8_t yxymem;
    int16_t apancnt;
} chn_t;

typedef struct
{
    const int8_t *sampleData;
    int8_t loopEnabled;
    int8_t sixteenBit;
    int8_t stereo;
    int8_t mixing;
    int8_t playBackwards;
    int32_t sampleLength;
    int32_t sampleLoopBegin;
    int32_t sampleLoopEnd;
    int32_t samplePosition;
    int32_t sampleLoopLength;

    float incRate;
    float frac;
    float volumeL;
    float volumeR;
    float orgVolL;
} VOICE;

typedef void (*effect_routine)(chn_t *ch);


/* BSS VARIABLES */
static int8_t tickdelay; /* NON-ST3 */
static int8_t volslidetype;
static int8_t patterndelay;
static int8_t patloopcount;
static uint8_t breakpat;
static uint8_t startrow;
static uint8_t musiccount;
static int16_t np_ord;
static int16_t np_row;
static int16_t np_pat;
static int16_t np_patoff;
static int16_t patloopstart;
static int16_t jumptorow;
static uint16_t patternadd;
static uint16_t patmusicrand;
static int32_t aspdmax;
static int32_t aspdmin;
static uint32_t np_patseg;
static chn_t chn[32];

static int32_t soundBufferSize;
static uint32_t audioFreq;

VOICE voice[32];
static WAVEFORMATEX wfx;
static WAVEHDR waveBlocks[SOUND_BUFFERS];
static HWAVEOUT _hWaveOut;

float f_audioFreq;
float f_masterVolume;

static int8_t samplingInterpolation;
static float *masterBufferL;
static float *masterBufferR;
static int8_t *mixerBuffer;
static int32_t samplesLeft;
static volatile int8_t mixingMutex;
static volatile int8_t isMixing;
static volatile uint32_t samplesPerFrame;

/* GLOBAL VARIABLES */
int8_t ModuleLoaded = 0;
int8_t MusicPaused = 0;
int8_t Playing = 0;

uint8_t *mseg = NULL;
uint16_t instrumentadd;
int8_t lastachannelused;
int8_t tracker;
int8_t oldstvib;
int8_t fastvolslide;
int8_t amigalimits;
uint8_t musicmax;
uint8_t numChannels;
int16_t tempo;
int16_t globalvol;
int8_t stereomode;
uint8_t mastervol;
uint32_t mseg_len;


/* TABLES */
static const int16_t xfinetune_amiga[16] =
{
    7895, 7941, 7985, 8046, 8107, 8169, 8232, 8280,
    8363, 8413, 8463, 8529, 8581, 8651, 8723, 8757
};

static const int8_t retrigvoladd[32] =
{
    0, -1, -2, -4, -8,-16,  0,  0,
    0,  1,  2,  4,  8, 16,  0,  0,
    0,  0,  0,  0,  0,  0, 10,  8,
    0,  0,  0,  0,  0,  0, 24, 32
};

static const uint16_t notespd[12] =
{
    1712 * 16, 1616 * 16, 1524 * 16,
    1440 * 16, 1356 * 16, 1280 * 16,
    1208 * 16, 1140 * 16, 1076 * 16,
    1016 * 16,  960 * 16,  907 * 16
};

static const int16_t vibsin[64] =
{
     0x00, 0x18, 0x31, 0x4A, 0x61, 0x78, 0x8D, 0xA1,
     0xB4, 0xC5, 0xD4, 0xE0, 0xEB, 0xF4, 0xFA, 0xFD,
     0xFF, 0xFD, 0xFA, 0xF4, 0xEB, 0xE0, 0xD4, 0xC5,
     0xB4, 0xA1, 0x8D, 0x78, 0x61, 0x4A, 0x31, 0x18,
     0x00,-0x18,-0x31,-0x4A,-0x61,-0x78,-0x8D,-0xA1,
    -0xB4,-0xC5,-0xD4,-0xE0,-0xEB,-0xF4,-0xFA,-0xFD,
    -0xFF,-0xFD,-0xFA,-0xF4,-0xEB,-0xE0,-0xD4,-0xC5,
    -0xB4,-0xA1,-0x8D,-0x78,-0x61,-0x4A,-0x31,-0x18
};

static const uint8_t vibsqu[64] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const int16_t vibramp[64] =
{
       0, -248,-240,-232,-224,-216,-208,-200,
    -192, -184,-176,-168,-160,-152,-144,-136,
    -128, -120,-112,-104, -96, -88, -80, -72,
     -64,  -56, -48, -40, -32, -24, -16,  -8,
       0,    8,  16,  24,  32,  40,  48,  56,
      64,   72,  80,  88,  96, 104, 112, 120,
     128,  136, 144, 152, 160, 168, 176, 184,
     192,  200, 208, 216, 224, 232, 240, 248
};

typedef struct
{
    uint8_t *_ptr;
    uint32_t _cnt;
    uint8_t *_base;
    uint32_t _bufsiz;
    int32_t _eof;
} MEM;


/* FUNCTION DECLARATIONS */
static MEM *mopen(const uint8_t *src, uint32_t length);
static void mclose(MEM **buf);
static size_t mread(void *buffer, size_t size, size_t count, MEM *buf);
static uint32_t mtell(MEM *buf);
static int32_t meof(MEM *buf);
static void mseek(MEM *buf, int32_t offset, int32_t whence);
void startMixing(void);
void setSamplesPerFrame(uint32_t val);
void setSamplingInterpolation(int8_t value);
void setStereoMode(int8_t value);
int8_t getStereoFlag(void);
void setMasterVolume(uint8_t value);
uint8_t getMasterVol(void);
void voiceSetSource(uint8_t voiceNumber, const int8_t *sampleData,
    int32_t sampleLength, int32_t sampleLoopLength, int32_t sampleLoopEnd,
    int8_t loopEnabled, int8_t sixteenbit, int8_t stereo);
void voiceSetSamplePosition(uint8_t voiceNumber, uint16_t value);
void voiceSetVolume(uint8_t voiceNumber, float vol, uint16_t pan);
void voiceSetSurround(uint8_t voiceNumber, int8_t surround);
void voiceSetSamplingFrequency(uint8_t voiceNumber, uint32_t samplingFrequency);
void voiceSetPlayBackwards(uint8_t voiceNumber, int8_t playBackwards);
void voiceSetReadPosToEnd(uint8_t voiceNumber);

void st3play_FreeSong(void);

static void s_ret(chn_t *ch);
static void s_setgliss(chn_t *ch);
static void s_setfinetune(chn_t *ch); /* NON-ST3 */
static void s_setvibwave(chn_t *ch);
static void s_settrewave(chn_t *ch);
static void s_setpanwave(chn_t *ch); /* NON-ST3 */
static void s_tickdelay(chn_t *ch); /* NON-ST3 */
static void s_setpanpos(chn_t *ch);
static void s_sndcntrl(chn_t *ch); /* NON-ST3 */
static void s_patloop(chn_t *ch);
static void s_notecut(chn_t *ch);
static void s_notecutb(chn_t *ch);
static void s_notedelay(chn_t *ch);
static void s_notedelayb(chn_t *ch);
static void s_patterdelay(chn_t *ch);
static void s_setspeed(chn_t *ch);
static void s_jmpto(chn_t *ch);
static void s_break(chn_t *ch);
static void s_volslide(chn_t *ch);
static void s_slidedown(chn_t *ch);
static void s_slideup(chn_t *ch);
static void s_toneslide(chn_t *ch);
static void s_vibrato(chn_t *ch);
static void s_tremor(chn_t *ch);
static void s_arp(chn_t *ch);
static void s_chanvol(chn_t *ch); /* NON-ST3 */
static void s_chanvolslide(chn_t *ch); /* NON-ST3 */
static void s_vibvol(chn_t *ch);
static void s_tonevol(chn_t *ch);
static void s_panslide(chn_t *ch);
static void s_retrig(chn_t *ch);
static void s_tremolo(chn_t *ch);
static void s_scommand1(chn_t *ch);
static void s_scommand2(chn_t *ch);
static void s_settempo(chn_t *ch);
static void s_finevibrato(chn_t *ch);
static void s_setgvol(chn_t *ch);
static void s_globvolslide(chn_t *ch); /* NON-ST3 */
static void s_setpan(chn_t *ch); /* NON-ST3 */
static void s_panbrello(chn_t *ch); /* NON-ST3 */

static effect_routine ssoncejmp[16] =
{
    s_ret,
    s_setgliss,
    s_setfinetune,
    s_setvibwave,
    s_settrewave,
    s_setpanwave, /* NON-ST3 */
    s_tickdelay, /* NON-ST3 */
    s_ret,
    s_setpanpos,
    s_sndcntrl, /* NON-ST3 */
    s_ret,
    s_patloop,
    s_notecut,
    s_notedelay,
    s_patterdelay,
    s_ret
};

static effect_routine ssotherjmp[16] =
{
    s_ret,
    s_ret,
    s_ret,
    s_ret,
    s_ret,
    s_ret,
    s_ret,
    s_ret,
    s_ret,
    s_ret,
    s_ret,
    s_ret,
    s_notecutb,
    s_notedelayb,
    s_ret,
    s_ret
};

static effect_routine soncejmp[27] =
{
    s_ret,
    s_setspeed,
    s_jmpto,
    s_break,
    s_volslide,
    s_slidedown,
    s_slideup,
    s_ret,
    s_ret,
    s_tremor,
    s_arp,
    s_ret,
    s_ret,
    s_chanvol, /* NON-ST3 */
    s_chanvolslide, /* NON-ST3 */
    s_ret,
    s_panslide, /* NON-ST3 */
    s_retrig,
    s_ret,
    s_scommand1,
    s_settempo,
    s_ret,
    s_ret,
    s_globvolslide, /* NON-ST3 */
    s_setpan, /* NON-ST3 */
    s_panbrello, /* NON-ST3 */
    s_ret
};

static effect_routine sotherjmp[27] =
{
    s_ret,
    s_ret,
    s_ret,
    s_ret,
    s_volslide,
    s_slidedown,
    s_slideup,
    s_toneslide,
    s_vibrato,
    s_tremor,
    s_arp,
    s_vibvol,
    s_tonevol,
    s_ret,
    s_chanvolslide, /* NON-ST3 */
    s_ret,
    s_panslide, /* NON-ST3 */
    s_retrig,
    s_tremolo,
    s_scommand2,
    s_settempo, /* NON-ST3 (for tempo slides) */
    s_finevibrato,
    s_setgvol,
    s_globvolslide, /* NON-ST3 */
    s_ret,
    s_panbrello, /* NON-ST3 */
    s_ret
};


/* CODE START */


static void getlastnfo(chn_t *ch)
{
    if (!ch->info)
        ch->info = ch->alastnfo;
}

static void setspeed(uint8_t val)
{
    if (val)
        musicmax = val;
}

static void settempo(uint16_t val)
{
    if (val > 32)
    {
        tempo = val;
        setSamplesPerFrame(((audioFreq * 5) / 2) / tempo);
    }
}

static void setspd(uint8_t ch)
{
    int32_t tmpspd;

    chn[ch].achannelused |= 0x80;

    tmpspd = chn[ch].aspd;

    if (amigalimits)
    {
        if (chn[ch].aorgspd > aspdmax) chn[ch].aorgspd = aspdmax;
        if (chn[ch].aorgspd < aspdmin) chn[ch].aorgspd = aspdmin;

        if (chn[ch].aspd > aspdmax)
            chn[ch].aspd = aspdmax;
    }

    if ((tracker == SCREAM_TRACKER) || amigalimits)
    {
        if (tmpspd > aspdmax)
            tmpspd = aspdmax;
    }
    else
    {
        /* *ABSOLUTE* max! */
        if (tmpspd > 14317056)
            tmpspd = 14317056;
    }

    if (tmpspd == 0)
    {
        /* cut channel */
        voiceSetSamplingFrequency(ch, 0);
        return;
    }

    if (tmpspd < aspdmin)
    {
        tmpspd = aspdmin;

        if (amigalimits && (chn[ch].aspd < aspdmin))
            chn[ch].aspd = aspdmin;
    }

    /* ST3 actually uses 14317056 (3.579264MHz * 4) instead of 14317456 (1712*8363) */
    if (tmpspd > 0)
        voiceSetSamplingFrequency(ch, 14317056 / tmpspd);
}

static void setvol(uint8_t ch)
{
    chn[ch].achannelused |= 0x80;
    voiceSetVolume(ch, ((float)(chn[ch].avol) / 63.0f) * ((float)(chn[ch].chanvol) / 64.0f) * ((float)(globalvol) / 64.0f), chn[ch].apanpos);
}

static void setpan(uint8_t ch)
{
    voiceSetVolume(ch, ((float)(chn[ch].avol) / 63.0f) * ((float)(chn[ch].chanvol) / 64.0f) * ((float)(globalvol) / 64.0f), chn[ch].apanpos);
}

static int16_t stnote2herz(uint8_t note)
{
    uint8_t tmpnote;
    uint8_t tmpocta;

    if (note == 254) return (0);

    tmpnote = note & 0x0F;
    tmpocta = note >> 4;

    /* ST3 doesn't do this, but do it for safety */
    if (tmpnote > 11) tmpnote = 11;

    /* limit octaves to 8 in ST3 mode */
    if ((tracker == SCREAM_TRACKER) && (tmpocta > 7))
        tmpocta = 7;

    return (notespd[tmpnote] >> tmpocta);
}

static int32_t scalec2spd(uint8_t ch, int32_t spd)
{
    spd *= 8363;

    if (tracker == SCREAM_TRACKER)
    {
        if ((spd / 65536) >= chn[ch].ac2spd)
            return (32767);
    }

    if (chn[ch].ac2spd)
        spd /= chn[ch].ac2spd;

    if (tracker == SCREAM_TRACKER)
    {
        if (spd > 32767)
            return (32767);
    }

    return (spd);
}

/* for Gxx with semitones slide flag */
static int32_t roundspd(uint8_t ch, int32_t spd)
{
    int8_t octa;
    int8_t lastnote;
    int8_t newnote;
    int32_t note;
    int32_t lastspd;
    int32_t newspd;

    newspd = spd * chn[ch].ac2spd;

    if (tracker == SCREAM_TRACKER)
    {
        if ((newspd / 65536) >= 8363)
            return (spd);
    }

    newspd /= 8363;

    /* find octave */
    octa    = 0;
    lastspd = ((1712 * 8) + (907 * 16)) / 2;
    while (lastspd >= newspd)
    {
        octa++;
        lastspd /= 2;
    }

    /* find note */
    lastnote = 0;
    newnote  = 0;

    if (tracker == SCREAM_TRACKER)
        lastspd = 32767;
    else
        lastspd = 32767 * 2;

    while (newnote < 11)
    {
        note = (notespd[newnote] >> octa) - newspd;
        if (note < 0) note *= -1; /* abs() */

        if (note < lastspd)
        {
            lastspd  = note;
            lastnote = newnote;
        }

        newnote++;
    }

    /* get new speed from new note */
    newspd = (stnote2herz((octa << 4) | (lastnote & 0x0F))) * 8363;

    if (tracker == SCREAM_TRACKER)
    {
        if ((newspd / 65536) >= chn[ch].ac2spd)
            return (spd);
    }

    if (chn[ch].ac2spd)
        newspd /= chn[ch].ac2spd;

    return (newspd);
}

static int16_t neworder(void)
{
newOrderSkip:
    np_ord++;

    if ((mseg[0x60 + (np_ord - 1)] == 255) || (np_ord > *((uint16_t *)(&mseg[0x20])))) /* end */
        np_ord = 1;

    if (mseg[0x60 + (np_ord - 1)] == 254) /* skip */
        goto newOrderSkip;

    np_pat       = (int16_t)(mseg[0x60 + (np_ord - 1)]);
    np_patoff    = -1; /* force reseek */
    np_row       = startrow;
    startrow     = 0;
    patmusicrand = 0;
    patloopstart = -1;
    jumptorow    = -1;

    return (np_row);
}

/* updates np_patseg and np_patoff */
static void seekpat(void)
{
    uint8_t dat;
    int16_t i;
    int16_t j;

    if (np_patoff == -1) /* seek must be done */
    {
        np_patseg = (*((uint16_t *)(&mseg[patternadd + (np_pat * 2)]))) * 16;
        if (np_patseg)
        {
            j = 2; /* skip packed pat len flag */

            /* increase np_patoff on patbreak */
            if (np_row)
            {
                i = np_row;
                while (i)
                {
                    dat = mseg[np_patseg + j++];
                    if (!dat)
                    {
                        i--;
                    }
                    else
                    {
                        /* skip ch data */
                        if (dat & 0x20) j += 2;
                        if (dat & 0x40) j += 1;
                        if (dat & 0x80) j += 2;
                    }
                }
            }

            np_patoff = j;
        }
    }
}

static uint8_t getnote(void)
{
    uint8_t dat;
    uint8_t ch;
    int16_t i;

    if (!np_patseg || (np_patseg >= mseg_len) || (np_pat >= *((uint16_t *)(&mseg[0x24]))))
        return (255);

    i = np_patoff;
    for (;;)
    {
        dat = mseg[np_patseg + i++];
        if (!dat) /* end of row */
        {
            np_patoff = i;
            return (255);
        }

        if (!(mseg[0x40 + (dat & 0x1F)] & 0x80))
        {
            ch = dat & 0x1F; /* channel to trigger */
            break;
        }

        /* channel is off, skip data */
        if (dat & 0x20) i += 2;
        if (dat & 0x40) i += 1;
        if (dat & 0x80) i += 2;
    }

    if (dat & 0x20)
    {
        chn[ch].note = mseg[np_patseg + i++];
        chn[ch].ins  = mseg[np_patseg + i++];

        if (chn[ch].note != 255) chn[ch].lastnote = chn[ch].note;
        if (chn[ch].ins)         chn[ch].lastins  = chn[ch].ins;
    }

    if (dat & 0x40)
        chn[ch].vol = mseg[np_patseg + i++];

    if (dat & 0x80)
    {
        chn[ch].cmd  = mseg[np_patseg + i++];
        chn[ch].info = mseg[np_patseg + i++];
    }

    np_patoff = i;
    return (ch);
}

static void doamiga(uint8_t ch)
{
    uint8_t *insdat;
    int8_t loop;
    uint32_t insoffs;
    uint32_t inslen;
    uint32_t insrepbeg;
    uint32_t insrepend;

    if (chn[ch].ins)
    {
        chn[ch].lastins = chn[ch].ins;
        chn[ch].astartoffset = 0;

        if (chn[ch].ins <= *((uint16_t *)(&mseg[0x22]))) /* added for safety reasons */
        {
            insdat = &mseg[(*((uint16_t *)(&mseg[instrumentadd + ((chn[ch].ins - 1) * 2)]))) * 16];
            if (insdat[0])
            {
                if (insdat[0] == 1)
                {
                    chn[ch].ac2spd = *((uint32_t *)(&insdat[0x20]));

                    if ((tracker == OPENMPT) || (tracker == BEROTRACKER))
                    {
                        if ((chn[ch].cmd == ('S' - 64)) && ((chn[ch].info & 0xF0) == 0x20))
                            chn[ch].ac2spd = xfinetune_amiga[chn[ch].info & 0x0F];
                    }

                    chn[ch].avol = (int8_t)(insdat[0x1C]);

                         if (chn[ch].avol <  0) chn[ch].avol =  0;
                    else if (chn[ch].avol > 63) chn[ch].avol = 63;

                    chn[ch].aorgvol = chn[ch].avol;
                    setvol(ch);

                    insoffs = ((insdat[0x0D] << 16) | (insdat[0x0F] << 8) | insdat[0x0E]) * 16;

                    inslen    = *((uint32_t *)(&insdat[0x10]));
                    insrepbeg = *((uint32_t *)(&insdat[0x14]));
                    insrepend = *((uint32_t *)(&insdat[0x18]));

                    if (insrepbeg > inslen) insrepbeg = inslen;
                    if (insrepend > inslen) insrepend = inslen;

                    loop = 0;
                    if ((insdat[0x1F] & 1) && inslen && (insrepend > insrepbeg))
                        loop = 1;

                    /* This specific phase differs from what sound card driver you use in ST3...
                    ** GUS: Don't set new voice sample. SB: Set new voice sample.
                    ** Let's use the GUS model here.
                    */
                    if ((chn[ch].cmd != ('G' - 64)) && (chn[ch].cmd != ('L' - 64)))
                    {
                        voiceSetSource(ch, (const int8_t *)(&mseg[insoffs]), inslen,
                            insrepend - insrepbeg, insrepend, loop,
                            insdat[0x1F] & 4, insdat[0x1F] & 2);
                    }
                }
                else
                {
                    chn[ch].lastins = 0;
                }
            }
        }
    }

    /* continue only if we have an active instrument on this channel */
    if (!chn[ch].lastins) return;

    if (chn[ch].cmd == ('O' - 64))
    {
        if (!chn[ch].info)
        {
            chn[ch].astartoffset = chn[ch].astartoffset00;
        }
        else
        {
            chn[ch].astartoffset   = 256 * chn[ch].info;
            chn[ch].astartoffset00 = chn[ch].astartoffset;
        }
    }

    if (chn[ch].note != 255)
    {
        if (chn[ch].note == 254)
        {
            chn[ch].aspd    = 0;
            chn[ch].avol    = 0;
            chn[ch].asldspd = 65535;

            setspd(ch);
            setvol(ch);

            /* shutdown channel */
            voiceSetSource(ch, NULL, 0, 0, 0, 0, 0, 0);
            voiceSetSamplePosition(ch, 0);
        }
        else
        {
            chn[ch].lastnote = chn[ch].note;

            if ((chn[ch].cmd != ('G' - 64)) && (chn[ch].cmd != ('L' - 64)))
                voiceSetSamplePosition(ch, chn[ch].astartoffset);

            if ((tracker == OPENMPT) || (tracker == BEROTRACKER))
            {
                voiceSetPlayBackwards(ch, 0);
                if ((chn[ch].cmd == ('S' - 64)) && (chn[ch].info == 0x9F))
                    voiceSetReadPosToEnd(ch);
            }

            if (!chn[ch].aorgspd || ((chn[ch].cmd != ('G' - 64)) && (chn[ch].cmd != ('L' - 64))))
            {
                chn[ch].aspd    = scalec2spd(ch, stnote2herz(chn[ch].note));
                chn[ch].aorgspd = chn[ch].aspd;
                chn[ch].avibcnt = 0;
                chn[ch].apancnt = 0;

                setspd(ch);
            }

            chn[ch].asldspd = scalec2spd(ch, stnote2herz(chn[ch].note));
        }
    }

    if (chn[ch].vol != 255)
    {
        if (chn[ch].vol <= 64)
        {
            chn[ch].avol    = chn[ch].vol;
            chn[ch].aorgvol = chn[ch].vol;

            setvol(ch);

            return;
        }

        /* NON-ST3, but let's handle it no matter what tracker */
        if ((chn[ch].vol >= 128) && (chn[ch].vol <= 192))
        {
            chn[ch].surround = 0;
            voiceSetSurround(ch, 0);

            chn[ch].apanpos = (chn[ch].vol - 128) * 4;
            setpan(ch);
        }
    }
}

static void donewnote(uint8_t ch, int8_t notedelayflag)
{
    if (notedelayflag)
    {
        chn[ch].achannelused = 0x81;
    }
    else
    {
        if (chn[ch].channelnum > lastachannelused)
        {
            lastachannelused = chn[ch].channelnum + 1;

            /* sanity fix */
            if (lastachannelused > 31) lastachannelused = 31;
        }

        chn[ch].achannelused = 0x01;

        if (chn[ch].cmd == ('S' - 64))
        {
            if ((chn[ch].info & 0xF0) == 0xD0)
                return;
        }
    }

    doamiga(ch);
}

static void donotes(void)
{
    uint8_t i;
    uint8_t ch;

    for (i = 0; i < 32; ++i)
    {
        chn[i].note = 255;
        chn[i].vol  = 255;
        chn[i].ins  = 0;
        chn[i].cmd  = 0;
        chn[i].info = 0;
    }

    seekpat();

    for (;;)
    {
        ch = getnote();
        if (ch == 255) break; /* end of row/channels */

        if ((mseg[0x40 + ch] & 0x7F) <= 15) /* no adlib channel types yet */
            donewnote(ch, 0);
    }
}

/* tick 0 commands */
static void docmd1(void)
{
    uint8_t i;

    for (i = 0; i < (lastachannelused + 1); ++i)
    {
        if (chn[i].achannelused)
        {
            if (chn[i].info)
                chn[i].alastnfo = chn[i].info;

            if (chn[i].cmd)
            {
                chn[i].achannelused |= 0x80;

                if (chn[i].cmd == ('D' - 64))
                {
                    /* fix retrig if Dxy */
                    chn[i].atrigcnt = 0;

                    /* fix speed if tone port noncomplete */
                    if (chn[i].aspd != chn[i].aorgspd)
                    {
                        chn[i].aspd = chn[i].aorgspd;
                        setspd(i);
                    }
                }
                else
                {
                    if (chn[i].cmd != ('I' - 64))
                    {
                        chn[i].atremor = 0;
                        chn[i].atreon  = 1;
                    }

                    if ((chn[i].cmd != ('H' - 64)) &&
                        (chn[i].cmd != ('U' - 64)) &&
                        (chn[i].cmd != ('K' - 64)) &&
                        (chn[i].cmd != ('R' - 64)))
                    {
                        chn[i].avibcnt |= 0x80;
                    }

                    /* NON-ST3 */
                    if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
                    {
                        if (chn[i].cmd != ('Y' - 64))
                            chn[i].apancnt |= 0x80;
                    }
                }

                if (chn[i].cmd < 27)
                {
                    volslidetype = 0;
                    soncejmp[chn[i].cmd](&chn[i]);
                }
            }
            else
            {
                /* NON-ST3 */
                if (tracker != SCREAM_TRACKER)
                { 
                    /* recalc pans */
                    setpan(i);
                    voiceSetSurround(i, chn[i].surround);
                }

                /* fix retrig if no command */
                chn[i].atrigcnt = 0;

                /* fix speed if tone port noncomplete */
                if (chn[i].aspd != chn[i].aorgspd)
                {
                    chn[i].aspd  = chn[i].aorgspd;
                    setspd(i);
                }
            }
        }
    }
}

/* tick >0 commands */
static void docmd2(void)
{
    uint8_t i;

    for (i = 0; i < (lastachannelused + 1); ++i)
    {
        if (chn[i].achannelused)
        {
            if (chn[i].cmd)
            {
                chn[i].achannelused |= 0x80;

                if (chn[i].cmd < 27)
                {
                    volslidetype = 0;
                    sotherjmp[chn[i].cmd](&chn[i]);
                }
            }
        }
    }
}

void dorow(void) /* periodically called from mixer */
{
    patmusicrand = (((patmusicrand * 0xCDEF) >> 16) + 0x1727) & 0x0000FFFF;

    if (!musiccount)
    {
        if (patterndelay)
        {
            np_row--;
            docmd1();
            patterndelay--;
        }
        else
        {
            donotes();
            docmd1();
        }
    }
    else
    {
        docmd2();
    }

    musiccount++;
    if (musiccount >= (musicmax + tickdelay))
    {
        tickdelay = 0;
        np_row++;

        if (jumptorow != -1)
        {
            np_row = jumptorow;
            jumptorow = -1;
        }

        /* np_row = 0..63, 64 = get new pat */
        if ((np_row >= 64) || (!patloopcount && breakpat))
        {
            if (breakpat == 255)
            {
                breakpat = 0;
                Playing  = 0;

                return;
            }

            breakpat = 0;
            np_row = neworder(); /* if breakpat, np_row = break row */
        }

        musiccount = 0;
    }
}

static void loadheaderparms(void)
{
    uint8_t *insdat;
    uint16_t insnum;
    uint32_t i;
    uint32_t j;
    uint32_t inslen;
    uint32_t insoff;

    /* set to init defaults first */
    oldstvib = 0;
    setspeed(6);
    settempo(125);
    aspdmin = 64;
    aspdmax = 32767;
    globalvol = 64;
    amigalimits = 0;
    fastvolslide = 0;
    setStereoMode(0);
    setMasterVolume(48);

    tracker = mseg[0x29] >> 4;

    if (mseg[0x33])
    {
        if (mseg[0x33] & 0x80)
            setStereoMode(1);

        if (mseg[0x33] & 0x7F)
        {
            if ((mseg[0x33] & 0x7F) < 16)
                setMasterVolume(16);
            else
                setMasterVolume(mseg[0x33] & 0x7F);
        }
    }

    if (mseg[0x32])
        settempo(mseg[0x32]);

    if (mseg[0x31] != 255)
        setspeed(mseg[0x31]);

    if (mseg[0x30] != 255)
    {
        globalvol = mseg[0x30];
        if (globalvol > 64)
            globalvol = 64;
    }

    if (mseg[0x26] != 255)
    {
        amigalimits  = mseg[0x26] & 0x10;
        fastvolslide = mseg[0x26] & 0x40;

        if (amigalimits)
        {
            aspdmax = 1712 * 2;
            aspdmin =  907 / 2;
        }
    }

    /* force fastvolslide if ST3.00 */
    if (*((uint16_t *)(&mseg[0x28])) == 0x1300)
        fastvolslide = 1;

    oldstvib = mseg[0x26] & 0x01;

    if (*((uint16_t *)(&mseg[0x2A])))
    {
        /* we have unsigned samples, convert to signed */

        insnum = *((uint16_t *)(&mseg[0x22]));
        for (i = 0; i < insnum; ++i)
        {
            insdat = &mseg[*((uint16_t *)(&mseg[instrumentadd + (i * 2)])) * 16];
            insoff = ((insdat[0x0D] << 16) | (insdat[0x0F] << 8) | insdat[0x0E]) * 16;

            if (insoff && (insdat[0] == 1))
            {
                inslen = *((uint32_t *)(&insdat[0x10]));

                if (insdat[0x1F] & 2) inslen *= 2; /* stereo */

                if (insdat[0x1F] & 4)
                {
                    /* 16-bit */
                    for (j = 0; j < inslen; ++j)
                        *((int16_t *)(&mseg[insoff + (j * 2)])) = *((uint16_t *)(&mseg[insoff + (j * 2)])) - 0x8000;
                }
                else
                {
                    /* 8-bit */
                    for (j = 0; j < inslen; ++j)
                        mseg[insoff + j] = mseg[insoff + j] - 0x80;
                }
            }
        }
    }
}

void st3play_PlaySong(void)
{
    uint8_t i;
    uint8_t dat;
    int16_t pan;

    if (!ModuleLoaded) return;

    memset(voice, 0, sizeof (voice));

    loadheaderparms();

    np_ord = 0;
    neworder();

    /* set up pans */
    for (i = 0; i < 32; ++i)
    {
        pan = (mseg[0x33] & 0x80) ? ((mseg[0x40 + i] & 0x08) ? 192 : 64) : 128;

        if (mseg[0x35] == 0xFC) /* non-default pannings follow */
        {
            dat = mseg[(patternadd + (*((uint16_t *)(&mseg[0x24])) * 2)) + i];
            if (dat & 0x20)
                pan = (dat & 0x0F) * 16;
        }

        chn[i].apanpos = stereomode ? pan : 128;
        setpan(i);
    }

    Playing = 1;
    setSamplesPerFrame(((audioFreq * 5) / 2 / tempo));
    isMixing = 1;

    if (MusicPaused)
        MusicPaused = 0;
}

int8_t st3play_LoadModule(const uint8_t *moduleData, uint32_t dataLength)
{
    char SCRM[4];

    uint32_t filesize;
    uint16_t i;

    MEM *modhandle;

    if (ModuleLoaded)
        st3play_FreeSong();

    ModuleLoaded = 0;

    modhandle = mopen(moduleData, dataLength);
    if (modhandle == NULL) return (0);

    mseek(modhandle, 0, SEEK_END);
    filesize = mtell(modhandle);
    mseek(modhandle, 0, SEEK_SET);

    if (filesize < 0x30)
    {
        mseg_len = 0;
        mclose(&modhandle);
        return (0);
    }

    mseek(modhandle, 0x2C, SEEK_SET);
    mread(SCRM, 1, 4, modhandle);
    if (memcmp(SCRM, "SCRM", 4))
    {
        mseg_len = 0;
        mclose(&modhandle);
        return (0);
    }

    mseek(modhandle, 0, SEEK_SET);

    if (mseg)
    {
        free(mseg);
        mseg = NULL;
    }

    mseg = (uint8_t *)(malloc(filesize));
    if (mseg == NULL)
    {
        mseg_len = 0;
        mclose(&modhandle);
        return (0);
    }

    if (mread(mseg, 1, filesize, modhandle) != filesize)
    {
        mseg_len = 0;
        mclose(&modhandle);
        return (0);
    }

    mclose(&modhandle);

    mseg_len = filesize;

    instrumentadd    = 0x60          +  mseg[0x20];
    patternadd       = instrumentadd + (mseg[0x22] * 2);
    musiccount       = 0;
    patterndelay     = 0;
    patloopstart     = 0;
    patloopcount     = 0;
    np_row           = 0;
    np_pat           = 0;
    startrow         = 0;
    np_patseg        = 0;
    np_patoff        = 0;
    breakpat         = 0;
    patmusicrand     = 0;
    volslidetype     = 0;
    jumptorow        = 0;

    /* zero all channel vars */
    memset(chn, 0, sizeof (chn));

    numChannels = 0;
    for (i = 0; i < 32; ++i)
    {
        if (!(mseg[0x40 + i] & 0x80))
            numChannels++;

        chn[i].channelnum   = (int8_t)(i);
        chn[i].achannelused = 0x80;
        chn[i].chanvol      = 0x40;
    }

    lastachannelused = 1;

    /* count *real* amounts of orders */
    i = *((uint16_t *)(&mseg[0x20]));
    while (i)
    {
        if (mseg[0x60 + (i - 1)] != 255)
            break;

        i--;
    }
    *((uint16_t *)(&mseg[0x20])) = i;

    ModuleLoaded = 1;

    return (1);
}


/* EFFECTS */

/* non-used effects */
static void s_ret(chn_t *ch) { (void)(ch); }
/* ---------------- */

static void s_setgliss(chn_t *ch)
{
    ch->aglis = ch->info & 0x0F;
}

static void s_setfinetune(chn_t *ch)
{
    /* this function does nothing in ST3 and many other trackers */
    if ((tracker == OPENMPT) || (tracker == BEROTRACKER))
        ch->ac2spd = xfinetune_amiga[ch->info & 0x0F];
} 

static void s_setvibwave(chn_t *ch)
{
    ch->avibtretype = (ch->avibtretype & 0xF0) | ((ch->info << 1) & 0x0F);
}

static void s_settrewave(chn_t *ch)
{
    ch->avibtretype = ((ch->info << 5) & 0xF0) | (ch->avibtretype & 0x0F);
}

static void s_setpanwave(chn_t *ch) /* NON-ST3 */
{
    if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
        ch->apantype = ch->info & 0x0F;
}

static void s_tickdelay(chn_t *ch) /* NON-ST3 */
{
    if (   (tracker == OPENMPT)
        || (tracker == BEROTRACKER)
        || (tracker == IMPULSE_TRACKER)
        || (tracker == SCHISM_TRACKER)
       )
    {
        tickdelay += (ch->info & 0x0F);
    }
}

static void s_setpanpos(chn_t *ch)
{
    ch->surround = 0;
    voiceSetSurround(ch->channelnum, 0);

    ch->apanpos = (ch->info & 0x0F) * 16;
    setpan(ch->channelnum);
}

static void s_sndcntrl(chn_t *ch) /* NON-ST3 */
{
    if ((ch->info & 0x0F) == 0x00)
    {
        if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
        {
            ch->surround = 0;
            voiceSetSurround(ch->channelnum, 0);
        }
    }
    else if ((ch->info & 0x0F) == 0x01)
    {
        if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
        {
            ch->surround = 1;
            voiceSetSurround(ch->channelnum, 1);
        }
    }
    else if ((ch->info & 0x0F) == 0x0E)
    {
        if ((tracker == OPENMPT) || (tracker == BEROTRACKER))
            voiceSetPlayBackwards(ch->channelnum, 0);
    }
    else if ((ch->info & 0x0F) == 0x0F)
    {
        if ((tracker == OPENMPT) || (tracker == BEROTRACKER))
            voiceSetPlayBackwards(ch->channelnum, 1);
    }
}

static void s_patloop(chn_t *ch)
{
    if (!(ch->info & 0x0F))
    {
        patloopstart = np_row;
        return;
    }

    if (!patloopcount)
    {
        patloopcount = (ch->info & 0x0F) + 1;

        if (patloopstart == -1)
            patloopstart = 0; /* default loopstart */
    }

    if (patloopcount > 1)
    {
        patloopcount--;

        jumptorow = patloopstart;
        np_patoff = -1; /* force reseek */
    }
    else
    {
        patloopcount = 0;
        patloopstart = np_row + 1;
    }
}

static void s_notecut(chn_t *ch)
{
    ch->anotecutcnt = ch->info & 0x0F;
}

static void s_notecutb(chn_t *ch)
{
    if (ch->anotecutcnt)
    {
        ch->anotecutcnt--;
        if (!ch->anotecutcnt)
            voiceSetSamplingFrequency(ch->channelnum, 0); /* cut note */
    }
}

static void s_notedelay(chn_t *ch)
{
    ch->anotedelaycnt = ch->info & 0x0F;
}

static void s_notedelayb(chn_t *ch)
{
    if (ch->anotedelaycnt)
    {
        ch->anotedelaycnt--;
        if (!ch->anotedelaycnt)
            donewnote(ch->channelnum, 1); /* 1 = notedelay end */
    }
}

static void s_patterdelay(chn_t *ch)
{
    if (patterndelay == 0)
        patterndelay = ch->info & 0x0F;
}

static void s_setspeed(chn_t *ch)
{
    setspeed(ch->info);
}

static void s_jmpto(chn_t *ch)
{
    if (ch->info != 0xFF)
    {
        breakpat = 1;
        np_ord = ch->info;
    }
    else
    {
        breakpat = 255;
    }
}

static void s_break(chn_t *ch)
{
    startrow = ((ch->info >> 4) * 10) + (ch->info & 0x0F);
    breakpat = 1;
}

static void s_volslide(chn_t *ch)
{
    uint8_t infohi;
    uint8_t infolo;

    getlastnfo(ch);

    infohi = ch->info >> 4;
    infolo = ch->info & 0x0F;

    if (infolo == 0x0F)
    {
        if (!infohi)
            ch->avol -= infolo;
        else if (!musiccount)
            ch->avol += infohi;
    }
    else if (infohi == 0x0F)
    {
        if (!infolo)
            ch->avol += infohi;
        else if (!musiccount)
            ch->avol -= infolo;
    }
    else if (fastvolslide || musiccount)
    {
        if (!infolo)
            ch->avol += infohi;
        else
            ch->avol -= infolo;
    }
    else
    {
        return; /* illegal slide */
    }

         if (ch->avol <  0) ch->avol =  0;
    else if (ch->avol > 63) ch->avol = 63;

    setvol(ch->channelnum);

         if (volslidetype == 1) s_vibrato(ch);
    else if (volslidetype == 2) s_toneslide(ch);
}

static void s_slidedown(chn_t *ch)
{
    if (ch->aorgspd)
    {
        getlastnfo(ch);

        if (musiccount)
        {
            if (ch->info >= 0xE0) return; /* no fine slides here */

            ch->aspd += (ch->info * 4);
            if (ch->aspd > 32767) ch->aspd = 32767;
        }
        else
        {
            if (ch->info <= 0xE0) return; /* only fine slides here */

            if (ch->info <= 0xF0)
            {
                ch->aspd += (ch->info & 0x0F);
                if (ch->aspd > 32767) ch->aspd = 32767;
            }
            else
            {
                ch->aspd += ((ch->info & 0x0F) * 4);
                if (ch->aspd > 32767) ch->aspd = 32767;
            }
        }

        ch->aorgspd = ch->aspd;
        setspd(ch->channelnum);
    }
}

static void s_slideup(chn_t *ch)
{
    if (ch->aorgspd)
    {
        getlastnfo(ch);

        if (musiccount)
        {
            if (ch->info >= 0xE0) return; /* no fine slides here */

            ch->aspd -= (ch->info * 4);
            if (ch->aspd < 0) ch->aspd = 0;
        }
        else
        {
            if (ch->info <= 0xE0) return; /* only fine slides here */

            if (ch->info <= 0xF0)
            {
                ch->aspd -= (ch->info & 0x0F);
                if (ch->aspd < 0) ch->aspd = 0;
            }
            else
            {
                ch->aspd -= ((ch->info & 0x0F) * 4);
                if (ch->aspd < 0) ch->aspd = 0;
            }
        }

        ch->aorgspd = ch->aspd;
        setspd(ch->channelnum);
    }
}

static void s_toneslide(chn_t *ch)
{
    if (volslidetype == 2) /* we came from an Lxy (toneslide+volslide) */
    {
        ch->info = ch->alasteff1;
    }
    else
    {
        if (!ch->aorgspd)
        {
            if (!ch->asldspd)
                return;

            ch->aorgspd = ch->asldspd;
            ch->aspd    = ch->asldspd;
        }

        if (!ch->info)
            ch->info = ch->alasteff1;
        else
            ch->alasteff1 = ch->info;
   }

    if (ch->aorgspd != ch->asldspd)
    {
        if (ch->aorgspd < ch->asldspd)
        {
            ch->aorgspd += (ch->info * 4);
            if (ch->aorgspd > ch->asldspd)
                ch->aorgspd = ch->asldspd;
        }
        else
        {
            ch->aorgspd -= (ch->info * 4);
            if (ch->aorgspd < ch->asldspd)
                ch->aorgspd = ch->asldspd;
        }

        if (ch->aglis)
            ch->aspd = roundspd(ch->channelnum, ch->aorgspd);
        else
            ch->aspd = ch->aorgspd;

        setspd(ch->channelnum);
    }
}

static void s_vibrato(chn_t *ch)
{
    int8_t type;
    int16_t cnt;
    int32_t dat;

    if (volslidetype == 1) /* we came from a Kxy (vibrato+volslide) */
    {
        ch->info = ch->alasteff;
    }
    else
    {
        if (!ch->info)
            ch->info = ch->alasteff;

        if (!(ch->info & 0xF0))
            ch->info = (ch->alasteff & 0xF0) | (ch->info & 0x0F);

        ch->alasteff = ch->info;
    }

    if (ch->aorgspd)
    {
        cnt  = ch->avibcnt;
        type = (ch->avibtretype & 0x0E) >> 1;
        dat  = 0;

        /* sine */
        if ((type == 0) || (type == 4))
        {
            if (type == 4)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsin[cnt / 2];
        }

        /* ramp */
        else if ((type == 1) || (type == 5))
        {
            if (type == 5)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibramp[cnt / 2];
        }

        /* square */
        else if ((type == 2) || (type == 6))
        {
            if (type == 6)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsqu[cnt / 2];
        }

        /* random */
        else if ((type == 3) || (type == 7))
        {
            if (type == 7)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsin[cnt / 2];
            cnt += (patmusicrand & 0x1E);
        }

        if (oldstvib)
            dat = ((dat * (ch->info & 0x0F)) >> 4) + ch->aorgspd;
        else
            dat = ((dat * (ch->info & 0x0F)) >> 5) + ch->aorgspd;

        ch->aspd = dat;
        setspd(ch->channelnum);

        ch->avibcnt = (cnt + ((ch->info >> 4) * 2)) & 0x7E;
    }
}

static void s_tremor(chn_t *ch)
{
    getlastnfo(ch);

    if (ch->atremor)
    {
        ch->atremor--;
        return;
    }

    if (ch->atreon)
    {
        ch->atreon = 0;

        ch->avol = 0;
        setvol(ch->channelnum);

        ch->atremor = ch->info & 0x0F;
    }
    else
    {
        ch->atreon = 1;

        ch->avol = ch->aorgvol;
        setvol(ch->channelnum);

        ch->atremor = ch->info >> 4;
    }
}

static void s_arp(chn_t *ch)
{
    int8_t note;
    int8_t octa;
    int8_t noteadd;
    uint8_t tick;

    getlastnfo(ch);

    tick = musiccount % 3;

         if (tick == 1) noteadd = ch->info >> 4;
    else if (tick == 2) noteadd = ch->info & 0x0F;
    else                noteadd = 0;

    /* check for octave overflow */
    octa =  ch->lastnote & 0xF0;
    note = (ch->lastnote & 0x0F) + noteadd;

    while (note >= 12)
    {
        note -= 12;
        octa += 16;
    }

    ch->aspd = scalec2spd(ch->channelnum, stnote2herz(octa | note));
    setspd(ch->channelnum);
}

static void s_chanvol(chn_t *ch) /* NON-ST3 */
{
    if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
    {
        if (ch->info <= 0x40)
            ch->chanvol = ch->info;
            
        setvol(ch->channelnum);
    }
}

static void s_chanvolslide(chn_t *ch) /* NON-ST3 */
{
    uint8_t infohi;
    uint8_t infolo;

    if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
    {
        if (ch->info)
            ch->nxymem = ch->info;
        else
            ch->info = ch->nxymem;

        infohi = ch->nxymem >> 4;
        infolo = ch->nxymem & 0x0F;

        if (infolo == 0x0F)
        {
            if (!infohi)
                ch->chanvol -= infolo;
            else if (!musiccount)
                ch->chanvol += infohi;
        }
        else if (infohi == 0x0F)
        {
            if (!infolo)
                ch->chanvol += infohi;
            else if (!musiccount)
                ch->chanvol -= infolo;
        }
        else if (musiccount) /* don't rely on fastvolslide flag here */
        {
            if (!infolo)
                ch->chanvol += infohi;
            else
                ch->chanvol -= infolo;
        }
        else
        {
            return; /* illegal slide */
        }

             if (ch->chanvol <  0) ch->chanvol =  0;
        else if (ch->chanvol > 64) ch->chanvol = 64;

        setvol(ch->channelnum);
    }
}

static void s_vibvol(chn_t *ch)
{
    volslidetype = 1;
    s_volslide(ch);
}

static void s_tonevol(chn_t *ch)
{
    volslidetype = 2;
    s_volslide(ch);
}

static void s_panslide(chn_t *ch) /* NON-ST3 */
{
    uint8_t infohi;
    uint8_t infolo;

    if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
    {
        if (ch->info)
            ch->pxymem = ch->info;
        else
            ch->info = ch->pxymem;

        infohi = ch->pxymem >> 4;
        infolo = ch->pxymem & 0x0F;

        if (infolo == 0x0F)
        {
            if (!infohi)
                ch->apanpos += (infolo * 4);
            else if (!musiccount)
                ch->apanpos -= (infohi * 4);
        }
        else if (infohi == 0x0F)
        {
            if (!infolo)
                ch->apanpos -= (infohi * 4);
            else if (!musiccount)
                ch->apanpos += (infolo * 4);
        }
        else if (musiccount) /* don't rely on fastvolslide flag here */
        {
            if (!infolo)
                ch->apanpos -= (infohi * 4);
            else
                ch->apanpos += (infolo * 4);
        }
        else
        {
            return; /* illegal slide */
        }

             if (ch->apanpos <   0) ch->apanpos =   0;
        else if (ch->apanpos > 256) ch->apanpos = 256;

        setpan(ch->channelnum);
    }
}

static void s_retrig(chn_t *ch)
{
    uint8_t infohi;

    getlastnfo(ch);
    infohi = ch->info >> 4;

    if (!(ch->info & 0x0F) || (ch->atrigcnt < (ch->info & 0x0F)))
    {
        ch->atrigcnt++;
        return;
    }

    ch->atrigcnt = 0;

    voiceSetPlayBackwards(ch->channelnum, 0);
    voiceSetSamplePosition(ch->channelnum, 0);

    if (!retrigvoladd[16 + infohi])
        ch->avol += retrigvoladd[infohi];
    else
        ch->avol = (int8_t)((ch->avol * retrigvoladd[16 + infohi]) / 16);

         if (ch->avol > 63) ch->avol = 63;
    else if (ch->avol <  0) ch->avol =  0;

    setvol(ch->channelnum);

    ch->atrigcnt++; /* probably a mistake? Keep it anyways. */
}

static void s_tremolo(chn_t *ch)
{
    int8_t type;
    int16_t cnt;
    int16_t dat;

    getlastnfo(ch);

    if (!(ch->info & 0xF0))
        ch->info = (ch->alastnfo & 0xF0) | (ch->info & 0x0F);

    ch->alastnfo = ch->info;

    if (ch->aorgvol)
    {
        cnt  = ch->avibcnt;
        type = ch->avibtretype >> 5;
        dat  = 0;

        /* sine */
        if ((type == 0) || (type == 4))
        {
            if (type == 4)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsin[cnt / 2];
        }

        /* ramp */
        else if ((type == 1) || (type == 5))
        {
            if (type == 5)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibramp[cnt / 2];
        }

        /* square */
        else if ((type == 2) || (type == 6))
        {
            if (type == 6)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsqu[cnt / 2];
        }

        /* random */
        else if ((type == 3) || (type == 7))
        {
            if (type == 7)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsin[cnt / 2];
            cnt += (patmusicrand & 0x1E);
        }

        dat = ((dat * (ch->info & 0x0F)) >> 7) + ch->aorgvol;

             if (dat > 63) dat = 63;
        else if (dat <  0) dat =  0;

        ch->avol = (int8_t)(dat);
        setvol(ch->channelnum);

        ch->avibcnt = (cnt + ((ch->info >> 4) * 2)) & 0x7E;
    }
}

static void s_scommand1(chn_t *ch)
{
    getlastnfo(ch);
    ssoncejmp[ch->info >> 4](ch);
}

static void s_scommand2(chn_t *ch)
{
    getlastnfo(ch);
    ssotherjmp[ch->info >> 4](ch);
}

static void s_settempo(chn_t *ch)
{
    if (!musiccount && (ch->info >= 0x20))
        tempo = ch->info;

    /* NON-ST3 tempo slide */
    if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
    {
        if (!musiccount)
        {
            if (!ch->info)
                ch->info = ch->txxmem;
            else
                ch->txxmem = ch->info;
        }
        else if (musiccount)
        {
            if (ch->info <= 0x0F)
            {
                tempo -= ch->info;
                if (tempo < 32)
                    tempo = 32;
            }
            else if (ch->info <= 0x1F)
            {
                tempo += (ch->info - 0x10);
                if (tempo > 255)
                    tempo = 255;
            }
        }
    }
    /* ------------------ */

    settempo(tempo);
}

static void s_finevibrato(chn_t *ch)
{
    int8_t type;
    int16_t cnt;
    int32_t dat;

    if (!ch->info)
        ch->info = ch->alasteff;

    if (!(ch->info & 0xF0))
        ch->info = (ch->alasteff & 0xF0) | (ch->info & 0x0F);

    ch->alasteff = ch->info;

    if (ch->aorgspd)
    {
        cnt  =  ch->avibcnt;
        type = (ch->avibtretype & 0x0E) >> 1;
        dat  = 0;

        /* sine */
        if ((type == 0) || (type == 4))
        {
            if (type == 4)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsin[cnt / 2];
        }

        /* ramp */
        else if ((type == 1) || (type == 5))
        {
            if (type == 5)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibramp[cnt / 2];
        }

        /* square */
        else if ((type == 2) || (type == 6))
        {
            if (type == 6)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsqu[cnt / 2];
        }

        /* random */
        else if ((type == 3) || (type == 7))
        {
            if (type == 7)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsin[cnt / 2];
            cnt += (patmusicrand & 0x1E);
        }

        if (oldstvib)
            dat = ((dat * (ch->info & 0x0F)) >> 6) + ch->aorgspd;
        else
            dat = ((dat * (ch->info & 0x0F)) >> 7) + ch->aorgspd;

        ch->aspd = dat;
        setspd(ch->channelnum);

        ch->avibcnt = (cnt + ((ch->info >> 4) * 2)) & 0x7E;
    }
}

static void s_setgvol(chn_t *ch)
{
    if (ch->info <= 64)
        globalvol = ch->info;
}

static void s_globvolslide(chn_t *ch) /* NON-ST3 */
{
    uint8_t i;
    uint8_t infohi;
    uint8_t infolo;

    if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
    {
        if (ch->info)
            ch->wxymem = ch->info;
        else
            ch->info = ch->wxymem;

        infohi = ch->wxymem >> 4;
        infolo = ch->wxymem & 0x0F;

        if (infolo == 0x0F)
        {
            if (!infohi)
                globalvol -= infolo;
            else if (!musiccount)
                globalvol += infohi;
        }
        else if (infohi == 0x0F)
        {
            if (!infolo)
                globalvol += infohi;
            else if (!musiccount)
                globalvol -= infolo;
        }
        else if (musiccount) /* don't rely on fastvolslide flag here */
        {
            if (!infolo)
                globalvol += infohi;
            else
                globalvol -= infolo;
        }
        else
        {
            return; /* illegal slide */
        }

             if (globalvol <  0) globalvol =  0;
        else if (globalvol > 64) globalvol = 64;

        /* update all channels now */
        for (i = 0; i < (lastachannelused + 1); ++i)
            setvol(i);
    }
}

static void s_setpan(chn_t *ch) /* NON-ST3 */
{
    /*
    ** this one should work even in mono mode
    ** for newer trackers that exports as ST3
    */
    if (ch->info <= 0x80)
    {
        ch->surround = 0;
        voiceSetSurround(ch->channelnum, 0);

        ch->apanpos = ch->info * 2;
        setpan(ch->channelnum);
    }
    else if (ch->info == 0xA4) /* surround */
    {
        if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
        {
            ch->surround = 1;
            voiceSetSurround(ch->channelnum, 1);
        }
    }
}

static void s_panbrello(chn_t *ch) /* NON-ST3 */
{
    int8_t type;
    int16_t cnt;
    int16_t dat;

    if ((tracker != SCREAM_TRACKER) && (tracker != IMAGO_ORPHEUS))
    {
        if (!musiccount)
        {
            if (!ch->info)
                ch->info = ch->alasteff;
            else
                ch->yxymem = ch->info;

            if (!(ch->info & 0xF0))
                ch->info = (ch->yxymem & 0xF0) | (ch->info & 0x0F);

            if (!(ch->info & 0x0F))
                ch->info = (ch->info & 0xF0) | (ch->yxymem & 0x0F);
        }

        cnt  = ch->apancnt;
        type = ch->apantype;
        dat  = 0;

        /* sine */
        if ((type == 0) || (type == 4))
        {
            if (type == 4)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsin[cnt / 2];
        }

        /* ramp */
        else if ((type == 1) || (type == 5))
        {
            if (type == 5)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibramp[cnt / 2];
        }

        /* square */
        else if ((type == 2) || (type == 6))
        {
            if (type == 6)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsqu[cnt / 2];
        }

        /* random */
        else if ((type == 3) || (type == 7))
        {
            if (type == 7)
            {
                cnt &= 0x7F;
            }
            else
            {
                if (cnt & 0x80) cnt = 0;
            }

            dat = vibsin[cnt / 2];
            cnt += (patmusicrand & 0x1E);
        }

        dat = ((dat * (ch->info & 0x0F)) >> 4) + ch->apanpos;

             if (dat <   0) dat =   0;
        else if (dat > 256) dat = 256;

        voiceSetVolume(ch->channelnum,
              ((float)(chn[ch->channelnum].avol)    / 63.0f)
            * ((float)(chn[ch->channelnum].chanvol) / 64.0f)
            * ((float)(globalvol)                   / 64.0f), dat);

        ch->apancnt = (cnt + ((ch->info >> 6) * 2)) & 0x7E;
    }
}

void setSamplesPerFrame(uint32_t val)
{
    samplesPerFrame = val;
}

void setSamplingInterpolation(int8_t value)
{
    samplingInterpolation = value;
}

void setStereoMode(int8_t value)
{
    stereomode = value;
}

void setMasterVolume(uint8_t value)
{
    mastervol = value;
    f_masterVolume = (float)(value) / 127.0f;
}

void voiceSetSource(uint8_t voiceNumber, const int8_t *sampleData,
    int32_t sampleLength, int32_t sampleLoopLength, int32_t sampleLoopEnd,
    int8_t loopEnabled, int8_t sixteenbit, int8_t stereo)
{
    VOICE *v;

    v = &voice[voiceNumber];

    v->sampleData       = sampleData;
    v->sampleLength     = sampleLength;
    v->sampleLoopBegin  = sampleLoopEnd - sampleLoopLength;
    v->sampleLoopEnd    = sampleLoopEnd;
    v->sampleLoopLength = sampleLoopLength;
    v->loopEnabled      = loopEnabled;
    v->sixteenBit       = sixteenbit;
    v->stereo           = stereo;
    v->mixing           = 1;

    if (v->samplePosition >= v->sampleLength)
        v->samplePosition  = 0;
}

void voiceSetSamplePosition(uint8_t voiceNumber, uint16_t value)
{
    VOICE *v;

    v = &voice[voiceNumber];

    v->samplePosition = value;

    v->frac   = 0.0f;
    v->mixing = 1;

    if (v->loopEnabled)
    {
        while (v->samplePosition >= v->sampleLoopEnd)
               v->samplePosition -= v->sampleLoopLength;
    }
    else if (v->samplePosition >= v->sampleLength)
    {
        v->mixing         = 0;
        v->samplePosition = 0;
    }
}

void voiceSetVolume(uint8_t voiceNumber, float vol, uint16_t pan)
{
    float p;

    VOICE *v;

    v = &voice[voiceNumber];
    p = (float)(pan) / 256.0f;

    v->volumeL = vol * (1.0f - p);
    v->volumeR = vol * p;
    v->orgVolL = v->volumeL; /* backup for surround */
}

void voiceSetSurround(uint8_t voiceNumber, int8_t surround)
{
    if (surround)
    {
        chn[voiceNumber].apanpos = 128;
        setpan(voiceNumber);

        voice[voiceNumber].volumeL = -voice[voiceNumber].orgVolL;
    }
    else
    {
        voice[voiceNumber].volumeL =  voice[voiceNumber].orgVolL;
    }
}

void voiceSetSamplingFrequency(uint8_t voiceNumber, uint32_t samplingFrequency)
{
    voice[voiceNumber].incRate = (float)(samplingFrequency) / f_audioFreq;
}

void voiceSetPlayBackwards(uint8_t voiceNumber, int8_t playBackwards)
{
    voice[voiceNumber].playBackwards = playBackwards;
}

void voiceSetReadPosToEnd(uint8_t voiceNumber)
{
    voice[voiceNumber].samplePosition = voice[voiceNumber].sampleLength - 1;
}

void mix8b(uint8_t ch, uint32_t samples)
{
    uint8_t intFrac;
    int32_t samplePosition2;
    uint32_t j;

    float sample;

    VOICE *v;

    v = &voice[ch];

    for (j = 0; (j < samples) && v->sampleData; ++j)
    {
        samplePosition2 = v->samplePosition + 1;

        if (samplingInterpolation)
        {
            if (v->loopEnabled)
            {
                if (samplePosition2 >= v->sampleLoopEnd)
                    samplePosition2  = v->sampleLoopBegin;
            }
            else
            {
                if (samplePosition2 >= v->sampleLength)
                    samplePosition2  = v->sampleLength - 1;
            }

            sample = _LERP(v->sampleData[v->samplePosition], v->sampleData[samplePosition2], voice[ch].frac);
        }
        else
        {
            sample = v->sampleData[v->samplePosition];
        }

        sample *= 256.0f;

        masterBufferL[j] += (sample * v->volumeL);
        masterBufferR[j] += (sample * v->volumeR);

        v->frac += v->incRate;
        while (v->frac >= 1.0f)
        {
            intFrac = (uint8_t)(v->frac);
            v->frac -= intFrac;

            if (v->playBackwards)
            {
                v->samplePosition -= intFrac;

                if (v->loopEnabled)
                {
                    if (v->samplePosition < v->sampleLoopBegin)
                        v->samplePosition = v->sampleLoopEnd - (v->sampleLoopBegin - v->samplePosition);
                }
                else if (v->samplePosition < 0)
                {
                    v->mixing         = 0;
                    v->samplePosition = 0;
                    v->frac           = 0.0f;

                    j = samples;
                    break;
                }
            }
            else
            {
                v->samplePosition += intFrac;

                if (v->loopEnabled)
                {
                    if (v->samplePosition >= v->sampleLoopEnd)
                        v->samplePosition  = v->sampleLoopBegin + (v->samplePosition - v->sampleLoopEnd);
                }
                else if (v->samplePosition >= v->sampleLength)
                {
                    v->mixing         = 0;
                    v->samplePosition = 0;
                    v->frac           = 0.0f;

                    j = samples;
                    break;
                }
            }
        }
    }
}

void mix8bstereo(uint8_t ch, uint32_t samples)
{
    uint8_t intFrac;
    int32_t samplePosition2;
    uint32_t j;

    float sampleL;
    float sampleR;

    VOICE *v;

    v = &voice[ch];

    for (j = 0; (j < samples) && v->sampleData; ++j)
    {
        samplePosition2 = v->samplePosition + 1;

        if (samplingInterpolation)
        {
            if (v->loopEnabled)
            {
                if (samplePosition2 >= v->sampleLoopEnd)
                    samplePosition2  = v->sampleLoopBegin;
            }
            else
            {
                if (samplePosition2 >= v->sampleLength)
                    samplePosition2  = v->sampleLength - 1;
            }

            sampleL = _LERP(v->sampleData[v->samplePosition], v->sampleData[samplePosition2], v->frac);
            sampleR = _LERP(v->sampleData[v->sampleLength + v->samplePosition], v->sampleData[v->sampleLength + samplePosition2], v->frac);
        }
        else
        {
            sampleL = v->sampleData[v->samplePosition];
            sampleR = v->sampleData[v->sampleLength + v->samplePosition];
        }

        sampleL *= 256.0f;
        sampleR *= 256.0f;

        masterBufferL[j] += (sampleL * v->volumeL);
        masterBufferR[j] += (sampleR * v->volumeR);

        v->frac += v->incRate;
        while (v->frac >= 1.0f)
        {
            intFrac = (uint8_t)(v->frac);
            v->frac -= intFrac;

            if (v->playBackwards)
            {
                v->samplePosition -= intFrac;

                if (v->loopEnabled)
                {
                    if (v->samplePosition < v->sampleLoopBegin)
                        v->samplePosition = v->sampleLoopEnd - (v->sampleLoopBegin - v->samplePosition);
                }
                else if (v->samplePosition < 0)
                {
                    v->mixing         = 0;
                    v->samplePosition = 0;
                    v->frac           = 0.0f;

                    j = samples;
                    break;
                }
            }
            else
            {
                v->samplePosition += intFrac;

                if (v->loopEnabled)
                {
                    if (v->samplePosition >= v->sampleLoopEnd)
                        v->samplePosition  = v->sampleLoopBegin + (v->samplePosition - v->sampleLoopEnd);
                }
                else if (v->samplePosition >= v->sampleLength)
                {
                    v->mixing         = 0;
                    v->samplePosition = 0;
                    v->frac           = 0.0f;

                    j = samples;
                    break;
                }
            }
        }
    }
}

void mix16b(uint8_t ch, uint32_t samples)
{
    uint8_t intFrac;
    int32_t samplePosition2;
    uint32_t j;

    const int16_t *sampleData;

    float sample;

    VOICE *v;

    v = &voice[ch];

    sampleData = (int16_t *)(v->sampleData);

    for (j = 0; (j < samples) && v->sampleData; ++j)
    {
        samplePosition2 = v->samplePosition + 1;

        if (samplingInterpolation)
        {
            if (v->loopEnabled)
            {
                if (samplePosition2 >= v->sampleLoopEnd)
                    samplePosition2  = v->sampleLoopBegin;
            }
            else
            {
                if (samplePosition2 >= v->sampleLength)
                    samplePosition2  = v->sampleLength - 1;
            }

            sample = _LERP(sampleData[v->samplePosition], sampleData[samplePosition2], voice[ch].frac);
        }
        else
        {
            sample = sampleData[v->samplePosition];
        }

        masterBufferL[j] += (sample * v->volumeL);
        masterBufferR[j] += (sample * v->volumeR);

        v->frac += v->incRate;
        while (v->frac >= 1.0f)
        {
            intFrac = (uint8_t)(v->frac);
            v->frac -= intFrac;

            if (v->playBackwards)
            {
                v->samplePosition -= intFrac;

                if (v->loopEnabled)
                {
                    if (v->samplePosition < v->sampleLoopBegin)
                        v->samplePosition = v->sampleLoopEnd - (v->sampleLoopBegin - v->samplePosition);
                }
                else if (v->samplePosition < 0)
                {
                    v->mixing         = 0;
                    v->samplePosition = 0;
                    v->frac           = 0.0f;

                    j = samples;
                    break;
                }
            }
            else
            {
                v->samplePosition += intFrac;

                if (v->loopEnabled)
                {
                    if (v->samplePosition >= v->sampleLoopEnd)
                        v->samplePosition  = v->sampleLoopBegin + (v->samplePosition - v->sampleLoopEnd);
                }
                else if (v->samplePosition >= v->sampleLength)
                {
                    v->mixing         = 0;
                    v->samplePosition = 0;
                    v->frac           = 0.0f;

                    j = samples;
                    break;
                }
            }
        }
    }
}

void mix16bstereo(uint8_t ch, uint32_t samples)
{
    uint8_t intFrac;
    int32_t samplePosition2;
    uint32_t j;

    const int16_t *sampleData;

    float sampleL;
    float sampleR;

    VOICE *v;

    v = &voice[ch];

    sampleData = (int16_t *)(v->sampleData);

    for (j = 0; (j < samples) && v->sampleData; ++j)
    {
        samplePosition2 = v->samplePosition + 1;

        if (samplingInterpolation)
        {
            if (v->loopEnabled)
            {
                if (samplePosition2 >= v->sampleLoopEnd)
                    samplePosition2  = v->sampleLoopBegin;
            }
            else
            {
                if (samplePosition2 >= v->sampleLength)
                    samplePosition2  = v->sampleLength - 1;
            }

            sampleL = _LERP(sampleData[v->samplePosition], sampleData[samplePosition2], v->frac);
            sampleR = _LERP(sampleData[v->sampleLength + v->samplePosition], sampleData[v->sampleLength + samplePosition2], v->frac);
        }
        else
        {
            sampleL = sampleData[v->samplePosition];
            sampleR = sampleData[v->sampleLength + v->samplePosition];
        }

        masterBufferL[j] += (sampleL * v->volumeL);
        masterBufferR[j] += (sampleR * v->volumeR);

        v->frac += v->incRate;
        while (v->frac >= 1.0f)
        {
            intFrac = (uint8_t)(v->frac);
            v->frac -= intFrac;

            if (v->playBackwards)
            {
                v->samplePosition -= intFrac;

                if (v->loopEnabled)
                {
                    if (v->samplePosition < v->sampleLoopBegin)
                        v->samplePosition = v->sampleLoopEnd - (v->sampleLoopBegin - v->samplePosition);
                }
                else if (v->samplePosition < 0)
                {
                    v->mixing         = 0;
                    v->samplePosition = 0;
                    v->frac           = 0.0f;

                    j = samples;
                    break;
                }
            }
            else
            {
                v->samplePosition += intFrac;

                if (v->loopEnabled)
                {
                    if (v->samplePosition >= v->sampleLoopEnd)
                        v->samplePosition  = v->sampleLoopBegin + (v->samplePosition - v->sampleLoopEnd);
                }
                else if (v->samplePosition >= v->sampleLength)
                {
                    v->mixing         = 0;
                    v->samplePosition = 0;
                    v->frac           = 0.0f;

                    j = samples;
                    break;
                }
            }
        }
    }
}

void mixSampleBlock(int16_t *outputStream, uint32_t sampleBlockLength)
{
    int16_t *streamPointer;
    uint8_t i;
    uint32_t j;

    float outL;
    float outR;

    streamPointer = outputStream;

    memset(masterBufferL, 0, sampleBlockLength * sizeof (float));
    memset(masterBufferR, 0, sampleBlockLength * sizeof (float));

    for (i = 0; i < 32; ++i)
    {
        if (voice[i].incRate && voice[i].mixing)
        {
            if (voice[i].stereo)
            {
                if (voice[i].sixteenBit)
                    mix16bstereo(i, sampleBlockLength);
                else
                    mix8bstereo(i, sampleBlockLength);
            }
            else
            {
                if (voice[i].sixteenBit)
                    mix16b(i, sampleBlockLength);
                else
                    mix8b(i, sampleBlockLength);
            }
        }
    }

    for (j = 0; j < sampleBlockLength; ++j)
    {
        if (MusicPaused)
        {
            outL = 0.0f;
            outR = 0.0f;
        }
        else
        {
            outL = masterBufferL[j] * f_masterVolume;
            outR = masterBufferR[j] * f_masterVolume;

                 if (outL >  32767.0f) outL =  32767.0f;
            else if (outL < -32768.0f) outL = -32768.0f;
                 if (outR >  32767.0f) outR =  32767.0f;
            else if (outR < -32768.0f) outR = -32768.0f;
        }

        *streamPointer++ = (int16_t)(outL);
        *streamPointer++ = (int16_t)(outR);
    }
}

void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    int16_t *outputStream;
    int32_t sampleBlock;
    int32_t samplesTodo;

    WAVEHDR *waveBlockHeader;

    /* make compiler happy! */
    (void)(dwParam2);
    (void)(dwInstance);

    if (uMsg == MM_WOM_DONE)
    {
        mixingMutex = 1;

        waveBlockHeader = (WAVEHDR *)(dwParam1);
        waveOutUnprepareHeader(hWaveOut, waveBlockHeader, sizeof (WAVEHDR));

        if (isMixing)
        {
            memcpy(waveBlockHeader->lpData, mixerBuffer, soundBufferSize);

            waveOutPrepareHeader(hWaveOut, waveBlockHeader, sizeof (WAVEHDR));
            waveOutWrite(hWaveOut, waveBlockHeader, sizeof (WAVEHDR));

            outputStream = (int16_t *)(mixerBuffer);
            sampleBlock  = soundBufferSize / 4;

            while (sampleBlock)
            {
                samplesTodo = (sampleBlock < samplesLeft) ? sampleBlock : samplesLeft;
                if (samplesTodo > 0)
                {
                    mixSampleBlock(outputStream, samplesTodo);

                    outputStream += (samplesTodo * 2);
                    sampleBlock  -=  samplesTodo;
                    samplesLeft  -=  samplesTodo;
                }
                else
                {
                    if (Playing && !MusicPaused)
                        dorow();

                    samplesLeft = samplesPerFrame;
                }
            }
        }

        mixingMutex = 0;
    }
}

int8_t openMixer(uint32_t _samplingFrequency, uint32_t _soundBufferSize)
{
    uint8_t i;
    MMRESULT r;

    if (!_hWaveOut)
    {
        audioFreq           = _samplingFrequency;
        f_audioFreq         = (float)(audioFreq);
        soundBufferSize     = _soundBufferSize;
        masterBufferL       = (float *)(malloc(soundBufferSize * sizeof (float)));
        masterBufferR       = (float *)(malloc(soundBufferSize * sizeof (float)));
        wfx.nSamplesPerSec  = audioFreq;
        wfx.wBitsPerSample  = 16;
        wfx.nChannels       = 2;
        wfx.cbSize          = 0;
        wfx.wFormatTag      = WAVE_FORMAT_PCM;
        wfx.nBlockAlign     = (wfx.wBitsPerSample * wfx.nChannels) / 8;
        wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

        if ((masterBufferL == NULL) || (masterBufferR == NULL))
            return (0); /* gets free'd later */

        r = waveOutOpen(&_hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)(waveOutProc), 0L, CALLBACK_FUNCTION);
        if (r != MMSYSERR_NOERROR)
            return (0);

        for (i = 0; i < SOUND_BUFFERS; ++i)
        { 
            waveBlocks[i].dwBufferLength = soundBufferSize;

            waveBlocks[i].lpData = (LPSTR)(calloc(soundBufferSize, 1));
            if (waveBlocks[i].lpData == NULL)
                return (0); /* gets free'd later */

            waveOutPrepareHeader(_hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
            waveOutWrite(_hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
        }

        mixerBuffer = (int8_t *)(calloc(soundBufferSize, 1));
        if (mixerBuffer == NULL)
            return (0); /* gets free'd later */

        isMixing = 1;
        return (1);
    }

    return (1);
}

void closeMixer(void)
{
    uint32_t i;

    if (isMixing)
    {
        isMixing = 0;
        while (mixingMutex) Sleep(1);

        if (_hWaveOut)
        {
            for (i = 0; i < SOUND_BUFFERS; ++i)
            {
                if (waveBlocks[i].lpData != NULL)
                {
                    waveOutUnprepareHeader(_hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
                    waveBlocks[i].dwFlags &= ~WHDR_PREPARED;

                    Sleep(50);

                    if (waveBlocks[i].lpData != NULL)
                    {
                        free(waveBlocks[i].lpData);
                        waveBlocks[i].lpData = NULL;
                    }
                }
            }

            waveOutReset(_hWaveOut);
            waveOutClose(_hWaveOut);

            _hWaveOut = 0;

            if (mixerBuffer != NULL)
            {
                free(mixerBuffer);
                mixerBuffer = NULL;
            }

            if (masterBufferL != NULL)
            {
                free(masterBufferL);
                masterBufferL = NULL;
            }

            if (masterBufferR != NULL)
            {
                free(masterBufferR);
                masterBufferR = NULL;
            }
        }
    }
}

int8_t st3play_Init(uint32_t outputFreq, int8_t interpolation)
{
    numChannels = 32;
    stereomode = 1;
    globalvol = 64;
    mastervol = 48;
    setSamplingInterpolation(interpolation);
    setSamplesPerFrame(((outputFreq * 5) / 2 / 125));

    if (!openMixer(outputFreq, 2048))
    {
        closeMixer();
        return (0);
    }

    return (1);
}

void st3play_FreeSong(void)
{
    Playing = 0;
    MusicPaused = 1;

    memset(voice, 0, sizeof (voice));

    while (mixingMutex) Sleep(1);

    if (mseg != NULL)
    {
        free(mseg);
        mseg = NULL;
    }

    ModuleLoaded = 0;
}

void st3play_Close(void)
{
    Playing     = 0;
    MusicPaused = 1;

    while (mixingMutex) {}

    closeMixer();
}

static MEM *mopen(const uint8_t *src, uint32_t length)
{
    MEM *b;
    if ((src == NULL) || (length == 0)) return (NULL);

    b = (MEM *)(malloc(sizeof (MEM)));
    if (b == NULL) return (NULL);

    b->_base   = (uint8_t *)(src);
    b->_ptr    = (uint8_t *)(src);
    b->_cnt    = length;
    b->_bufsiz = length;
    b->_eof    = 0;

    return (b);
}

static void mclose(MEM **buf)
{
    if (*buf != NULL)
    {
        free(*buf);
        *buf = NULL;
    }
}

static size_t mread(void *buffer, size_t size, size_t count, MEM *buf)
{
    size_t wrcnt;
    int32_t pcnt;

    if (buf       == NULL) return (0);
    if (buf->_ptr == NULL) return (0);

    wrcnt = size * count;
    if ((size == 0) || buf->_eof) return (0);

    pcnt = (buf->_cnt > wrcnt) ? wrcnt : buf->_cnt;
    memcpy(buffer, buf->_ptr, pcnt);

    buf->_cnt -= pcnt;
    buf->_ptr += pcnt;

    if (buf->_cnt <= 0)
    {
        buf->_ptr = buf->_base + buf->_bufsiz;
        buf->_cnt = 0;
        buf->_eof = 1;
    }

    return (pcnt / size);
}

static uint32_t mtell(MEM *buf)
{
    return (buf->_ptr - buf->_base);
}

static int32_t meof(MEM *buf)
{
    if (buf == NULL)
        return (1);

    return (buf->_eof);
}

static void mseek(MEM *buf, int32_t offset, int32_t whence)
{
    if (buf == NULL) return;

    if (buf->_base)
    {
        switch (whence)
        {
            case SEEK_SET: buf->_ptr  = buf->_base + offset;                break;
            case SEEK_CUR: buf->_ptr += offset;                             break;
            case SEEK_END: buf->_ptr  = buf->_base + buf->_bufsiz + offset; break;
            default: break;
        }

        buf->_eof = 0;
        if (buf->_ptr >= (buf->_base + buf->_bufsiz))
        {
            buf->_ptr = buf->_base + buf->_bufsiz;
            buf->_eof = 1;
        }

        buf->_cnt = (buf->_base + buf->_bufsiz) - buf->_ptr;
    }
}

/* END OF FILE */