/*
** PT2PLAY v1.1 - 4th of October 2015 - http://16-bits.org
** =======================================================
**
** C port of ProTracker 2.3D's replayer, by 8bitbubsy (Olav Sørensen)
** using the original PT2.3A asm source code + a PT2.3D disassembly.
**
** The only difference is that InvertLoop (EFx) and Karplus-Strong (E8x)
** are handled like the tracker replayer, not the standalone replayer.
**
** The mixer is written to do looping the way Paula (Amiga sound chip) does.
** The BLEP (band-limited step) and filters routines were coded by aciddose.
**
**
** You need to link winmm.lib for this to compile (-lwinmm)
**
** User functions:
**
** #include <stdint.h>
**
** enum
** {
**     CIA_TEMPO_MODE    = 0,
**     VBLANK_TEMPO_MODE = 1
** };
**
** int8_t pt2play_Init(uint32_t outputFreq);
** void pt2play_Close(void);
** void pt2play_PauseSong(int8_t pause);
** void pt2play_PlaySong(uint8_t *moduleData, int8_t tempoMode);
** void pt2play_SetStereoSep(uint8_t percentage);
*/

/* == USER ADJUSTABLE SETTINGS == */
#define STEREO_SEP (25)    /* --> Initial stereo separation in percent - 0 = mono, 100 = hard pan (like Amiga) */
#define NORM_FACTOR (3.5f) /* --> Slightly increase this value if the song is too loud. Decrease if too quiet... */
#define USE_HIGHPASS       /* --> 5.2Hz high-pass filter present in all Amigas - comment out this line for a speed-up */
//#define USE_LOWPASS        /* --> 5kHz low-pass filter in all Amigas except A1200/CD32 - comment out for speed-up and sharpness */
#define USE_BLEP           /* --> Reduces some unwanted aliasing (closer to real Amiga) - comment out this line for a speed-up */
#define PREVENT_CLIPPING   /* --> Clamps the audio output to prevent clipping - comment out this line for a tiny speed-up */
#define ENABLE_E8_EFFECT   /* --> Enable E8x (Karplus-Strong) - comment out this line if E8x is used for something else */
#define LED_FILTER         /* --> Process the LED filter - comment out this line if you don't need the filter, for a speed-up */
/* ------------------------------ */

/* used for faster windows.h parsing when compiling */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h> /* floorf() */
#include <windows.h>
#include <mmsystem.h>

#define M_PI_F  (3.1415927f)
#define M_2PI_F (6.2831855f)

#define SOUND_BUFFERS (7) /* don't change this unless you know what you're doing... */

#ifdef _MSC_VER
#define inline __forceinline
#endif

#ifdef USE_BLEP
#define ZC 8
#define OS 5
#define SP 5
#define NS (ZC * OS / SP)
#define RNS 7 /* RNS = (2^ > NS) - 1 */
#endif

/* STRUCTS */
#ifdef USE_BLEP
typedef struct blep_t
{
    int32_t index;
    int32_t samplesLeft;
    float buffer[RNS + 1];
    float lastValue;
} blep_t;
#endif

typedef struct ptChannel_t
{
    int8_t *n_start;
    int8_t *n_wavestart;
    int8_t *n_loopstart;
    int8_t n_index;
    int8_t n_volume;
    int8_t n_toneportdirec;
    int8_t n_vibratopos;
    int8_t n_tremolopos;
    int8_t n_pattpos;
    int8_t n_loopcount;
    uint8_t n_wavecontrol;
    uint8_t n_glissfunk;
    uint8_t n_sampleoffset;
    uint8_t n_toneportspeed;
    uint8_t n_vibratocmd;
    uint8_t n_tremolocmd;
    uint8_t n_finetune;
    uint8_t n_funkoffset;
    int16_t n_period;
    int16_t n_note;
    int16_t n_wantedperiod;
    uint16_t n_cmd;
    uint32_t n_length;
    uint32_t n_replen;
    uint32_t n_repend;
} ptChannel_t;

typedef struct paulaVoice_t
{
    int8_t *SRC_DAT;
    uint32_t SRC_LEN;
    int8_t *DMA_DAT;
    uint32_t DMA_LEN;
    uint32_t DMA_POS;
    int8_t DMA_ON;
    float SRC_VOL;
    float DELTA;
    float FRAC;
    float LASTDELTA;
    float LASTFRAC;
    float PANL;
    float PANR;
} paulaVoice_t;

#if defined(USE_HIGHPASS) || defined(USE_LOWPASS)
typedef struct lossyIntegrator_t
{
    float buffer[2];
    float coeff[2];
} lossyIntegrator_t;
#endif

#ifdef LED_FILTER
typedef struct ledFilter_t
{
    float led[4];
} ledFilter_t;

typedef struct ledFilterCoeff_t
{
    float led;
    float ledFb;
} ledFilterCoeff_t;
#endif

/* BSS DATA */
static int8_t *mt_SampleStarts[31];
static int8_t mt_TempoMode;
static int8_t mt_SongPos;
static int8_t mt_PosJumpFlag;
static int8_t mt_PBreakFlag;
static int8_t mt_Enable;
static int8_t mt_PBreakPos;
static int8_t mt_PattDelTime;
static int8_t mt_PattDelTime2;
static uint8_t *mt_SongDataPtr;
static uint8_t mt_LowMask;
static uint8_t mt_Counter;
static uint8_t mt_Speed;
static int16_t *mt_PeriodTable = NULL;
static uint16_t mt_PatternPos;
static int32_t soundBufferSize;
static uint32_t mt_PattPosOff;
static float mt_TimerVal;
static float f_outputFreq;
static WAVEHDR waveBlocks[SOUND_BUFFERS];
static ptChannel_t mt_ChanTemp[4];
static paulaVoice_t AUD[4];
static HWAVEOUT hWaveOut;
static WAVEFORMATEX wfx;
#ifdef USE_BLEP
static blep_t blep[4];
static blep_t blepVol[4];
#endif
#ifdef USE_HIGHPASS
static lossyIntegrator_t filterHi;
#endif
#ifdef USE_LOWPASS
static lossyIntegrator_t filterLo;
#endif
#ifdef LED_FILTER
static ledFilterCoeff_t filterLEDC;
static ledFilter_t filterLED;
static uint8_t mt_LEDStatus;
#endif
static float *masterBufferL = NULL;
static float *masterBufferR = NULL;
static int8_t *mixerBuffer = NULL;
static int32_t samplesLeft;
static volatile int8_t mixingMutex;
static volatile int8_t isMixing;
static volatile uint32_t samplesPerFrame;

/* TABLES */
static const uint8_t mt_FunkTable[16] =
{
    0x00, 0x05, 0x06, 0x07, 0x08, 0x0A, 0x0B, 0x0D,
    0x10, 0x13, 0x16, 0x1A, 0x20, 0x2B, 0x40, 0x80
};

static const uint8_t mt_VibratoTable[32] =
{
    0x00, 0x18, 0x31, 0x4A, 0x61, 0x78, 0x8D, 0xA1,
    0xB4, 0xC5, 0xD4, 0xE0, 0xEB, 0xF4, 0xFA, 0xFD,
    0xFF, 0xFD, 0xFA, 0xF4, 0xEB, 0xE0, 0xD4, 0xC5,
    0xB4, 0xA1, 0x8D, 0x78, 0x61, 0x4A, 0x31, 0x18
};

static const int8_t mt_PeriodDeltas[576] =
{
      0,-48,-46,-42,-42,-38,-36,-34,-32,-30,-28,-27,-25,-24,-23,-21,-21,-19,
    -18,-17,-16,-15,-14,-14,-12,-12,-12,-10,-10,-10, -9, -8, -8, -8, -7, -7,
     -6,-48,-45,-42,-41,-37,-36,-34,-32,-30,-28,-27,-25,-24,-22,-22,-20,-19,
    -18,-16,-16,-15,-14,-14,-12,-12,-12,-10,-10,-10, -9, -8, -8, -8, -7, -6,
    -12,-48,-44,-43,-39,-38,-35,-34,-31,-30,-28,-27,-25,-24,-22,-21,-20,-19,
    -18,-16,-16,-15,-14,-13,-13,-12,-11,-11,-10, -9,- 9, -8, -8, -8, -7, -6,
    -18,-47,-45,-42,-39,-37,-36,-33,-31,-30,-28,-26,-25,-24,-22,-21,-20,-18,
    -18,-16,-16,-15,-14,-13,-13,-11,-11,-11,-10, -9, -9, -8, -8, -7, -7, -7,
    -24,-47,-44,-42,-39,-37,-35,-33,-31,-29,-28,-26,-25,-24,-22,-20,-20,-18,
    -18,-16,-16,-15,-14,-13,-12,-12,-11,-10,-10, -9, -9, -8, -8, -7, -7, -7,
    -30,-47,-43,-42,-39,-36,-35,-33,-31,-29,-28,-26,-24,-23,-22,-21,-19,-19,
    -17,-16,-16,-15,-13,-13,-13,-11,-11,-10,-10, -9, -9, -8, -8, -7, -7, -7,
    -36,-46,-44,-41,-38,-37,-34,-33,-31,-29,-27,-26,-24,-23,-22,-20,-20,-18,
    -17,-16,-16,-14,-14,-13,-12,-12,-10,-11, -9, -9, -9, -8, -8, -7, -7, -6,
    -42,-46,-43,-41,-38,-36,-35,-32,-30,-29,-27,-26,-24,-23,-21,-21,-19,-18,
    -17,-16,-16,-14,-14,-12,-12,-12,-11,-10,-10, -9, -8, -8, -8, -7, -7, -6,
     51,-51,-48,-46,-42,-42,-38,-36,-34,-32,-30,-28,-27,-25,-24,-23,-21,-21,
    -19,-18,-17,-16,-15,-14,-14,-12,-12,-12,-10,-10,-10, -9, -8, -8, -8, -7,
     44,-50,-48,-45,-42,-40,-39,-35,-34,-32,-30,-28,-27,-25,-24,-22,-22,-20,
    -19,-18,-16,-16,-15,-15,-13,-13,-12,-11,-10,-10,-10, -9, -8, -8, -8, -7,
     38,-50,-48,-44,-43,-39,-38,-35,-34,-31,-30,-28,-27,-25,-24,-22,-21,-20,
    -19,-18,-16,-16,-15,-14,-14,-12,-12,-11,-11,-10, -9, -9, -8, -8, -8, -7,
     31,-49,-47,-45,-42,-39,-37,-36,-33,-31,-30,-28,-26,-25,-24,-22,-21,-20,
    -18,-18,-16,-16,-15,-14,-13,-13,-11,-11,-11,-10, -9, -9, -8, -8, -7, -7,
     25,-49,-47,-44,-42,-39,-37,-35,-33,-31,-30,-27,-26,-25,-24,-22,-20,-20,
    -18,-18,-16,-16,-15,-14,-13,-12,-12,-11,-10,-10, -9, -9, -8, -8, -8, -6,
     19,-49,-47,-43,-42,-39,-36,-35,-33,-31,-29,-28,-26,-24,-23,-22,-21,-19,
    -19,-17,-16,-16,-15,-13,-13,-13,-11,-11,-10,-10, -9, -9, -8, -8, -7, -7,
     12,-48,-46,-44,-41,-38,-37,-34,-33,-31,-29,-27,-26,-24,-23,-22,-20,-20,
    -18,-17,-16,-16,-14,-14,-13,-12,-12,-10,-11, -9, -9, -9, -8, -8, -7, -7,
      6,-48,-46,-43,-41,-38,-36,-35,-32,-30,-29,-27,-26,-24,-23,-21,-21,-19,
    -18,-17,-16,-16,-14,-14,-12,-13,-11,-11,-10,-10, -9, -8, -8, -8, -7, -7
};

#ifdef USE_BLEP
static const uint32_t blepData[48] =
{
    0x3F7FE1F1, 0x3F7FD548, 0x3F7FD6A3, 0x3F7FD4E3,
    0x3F7FAD85, 0x3F7F2152, 0x3F7DBFAE, 0x3F7ACCDF,
    0x3F752F1E, 0x3F6B7384, 0x3F5BFBCB, 0x3F455CF2,
    0x3F26E524, 0x3F0128C4, 0x3EACC7DC, 0x3E29E86B,
    0x3C1C1D29, 0xBDE4BBE6, 0xBE3AAE04, 0xBE48DEDD,
    0xBE22AD7E, 0xBDB2309A, 0xBB82B620, 0x3D881411,
    0x3DDADBF3, 0x3DE2C81D, 0x3DAAA01F, 0x3D1E769A,
    0xBBC116D7, 0xBD1402E8, 0xBD38A069, 0xBD0C53BB,
    0xBC3FFB8C, 0x3C465FD2, 0x3CEA5764, 0x3D0A51D6,
    0x3CEAE2D5, 0x3C92AC5A, 0x3BE4CBF7, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};
#endif

/* MACROS */
#define PaulaRestartDMA(i) AUD[i].FRAC=0.0f;AUD[i].DMA_POS=0;AUD[i].DMA_DAT=AUD[i].SRC_DAT;AUD[i].DMA_LEN=AUD[i].SRC_LEN;AUD[i].DMA_ON=1;
#define PaulaSetVolume(i, x) AUD[i].SRC_VOL=(float)(x)*(1.0f/64.0f);
#define PaulaSetLength(i, x) AUD[i].SRC_LEN=x*2;
#define PaulaSetData(i, x) AUD[i].SRC_DAT=x;
#define PaulaSetPeriod(i, x) if(x){AUD[i].DELTA=(float)(3546895.0f/(float)(x))/f_outputFreq;}
#define SWAP16(x) ((uint16_t)(((x)<<8)|((x)>>8)))
#define PTR2WORD(x) ((uint16_t *)(x))

/* CODE START */

#if defined(USE_HIGHPASS) || defined(USE_LOWPASS)
static float filterRCtoHz(float R, float C)
{
    return (1.0f / (M_2PI_F * R * C));
}

static float filterHzAndRtoC(float R, float HZ)
{
    return (1.0f / (HZ * M_2PI_F * R));
}

static void calcCoeffLossyIntegrator(float sr, float hz, lossyIntegrator_t *filter)
{
    filter->coeff[0] = tanf(M_PI_F * hz / sr);
    filter->coeff[1] = 1.0f / (1.0f + filter->coeff[0]);
}

static void clearLossyIntegrator(lossyIntegrator_t *filter)
{
    filter->buffer[0] = 0.0f;
    filter->buffer[1] = 0.0f;
}

static inline void lossyIntegrator(lossyIntegrator_t *filter, float *in, float *out)
{
    float output;

    /* left channel low-pass */
    output = (filter->coeff[0] * in[0] + filter->buffer[0]) * filter->coeff[1];
    filter->buffer[0] = filter->coeff[0] * (in[0] - output) + output + 1e-10f;
    out[0] = output;

    /* right channel low-pass */
    output = (filter->coeff[0] * in[1] + filter->buffer[1]) * filter->coeff[1];
    filter->buffer[1] = filter->coeff[0] * (in[1] - output) + output + 1e-10f;
    out[1] = output;
}

static inline void lossyIntegratorHighPass(lossyIntegrator_t *filter, float *in, float *out)
{
    float low[2];

    lossyIntegrator(filter, in, low);

    out[0] = in[0] - low[0];
    out[1] = in[1] - low[1];
}
#endif

#ifdef LED_FILTER
static float calcCoeffLED(float sr, float hz)
{
    if (hz >= (sr / 2.0f))
        return (1.0f);

    return (M_2PI_F * hz / sr);
}

static void clearLEDFilter(ledFilter_t *filter)
{
    filter->led[0] = 0.0f;
    filter->led[1] = 0.0f;
    filter->led[2] = 0.0f;
    filter->led[3] = 0.0f;
}

static inline void lossyIntegratorLED(ledFilterCoeff_t filterC, ledFilter_t *filter, float *in, float *out)
{
    filter->led[0] += (filterC.led * (in[0] - filter->led[0])
        + filterC.ledFb * (filter->led[0] - filter->led[1]) + 1e-10f);
    filter->led[1] += (filterC.led * (filter->led[0] - filter->led[1]) + 1e-10f);
    out[0] = filter->led[1];

    filter->led[2] += (filterC.led * (in[1] - filter->led[2])
        + filterC.ledFb * (filter->led[2] - filter->led[3]) + 1e-10f);
    filter->led[3] += (filterC.led * (filter->led[2] - filter->led[3]) + 1e-10f);
    out[1] = filter->led[3];
}
#endif

#ifdef USE_BLEP
static inline void blepAdd(blep_t *b, float offset, float amplitude)
{
    int8_t n;
    uint32_t i;

    const float *src;
    float f;

    n   = NS;
    i   = (uint32_t)(offset * SP);
    src = (const float *)(blepData) + i + OS;
    f   = (offset * SP) - i;
    i   = b->index;

    while (n--)
    {
        b->buffer[i] += (amplitude * (src[0] + (src[1] - src[0]) * f));

        src += SP;

        i++;
        i &= RNS;
    }

    b->samplesLeft = NS;
}

static inline float blepRun(blep_t *b)
{
    float output;

    output = b->buffer[b->index];
    b->buffer[b->index] = 0.0f;

    b->index++;
    b->index &= RNS;

    b->samplesLeft--;

    return (output);
}
#endif

static void mt_UpdateFunk(ptChannel_t *ch)
{
    int8_t funkspeed;

    funkspeed = ch->n_glissfunk >> 4;
    if (funkspeed > 0)
    {
        ch->n_funkoffset += mt_FunkTable[funkspeed];
        if (ch->n_funkoffset >= 128)
        {
            ch->n_funkoffset = 0;

            if (ch->n_wavestart != NULL) /* added for safety reasons */
            {
                if (++ch->n_wavestart >= (ch->n_loopstart + (ch->n_replen * 2)))
                      ch->n_wavestart  =  ch->n_loopstart;

                *ch->n_wavestart = -1 - *ch->n_wavestart;
            }
        }
    }
}

static void mt_SetGlissControl(ptChannel_t *ch)
{
    ch->n_glissfunk = (ch->n_glissfunk & 0xF0) | (ch->n_cmd & 0x000F);
}

static void mt_SetVibratoControl(ptChannel_t *ch)
{
    ch->n_wavecontrol = (ch->n_wavecontrol & 0xF0) | (ch->n_cmd & 0x000F);
}

static void mt_SetFineTune(ptChannel_t *ch)
{
    ch->n_finetune = ch->n_cmd & 0x000F;
}

static void mt_JumpLoop(ptChannel_t *ch)
{
    if (!mt_Counter)
    {
        if (!(ch->n_cmd & 0x000F))
        {
            ch->n_pattpos = (int8_t)(mt_PatternPos / 16);
        }
        else
        {
            if (!ch->n_loopcount)
            {
                ch->n_loopcount = ch->n_cmd & 0x000F;
            }
            else
            {
                if (!--ch->n_loopcount) return;
            }

            mt_PBreakPos  = ch->n_pattpos;
            mt_PBreakFlag = 1;
        }
    }
}

static void mt_SetTremoloControl(ptChannel_t *ch)
{
    ch->n_wavecontrol = ((ch->n_cmd & 0x000F) << 4) | (ch->n_wavecontrol & 0x0F);
}

static void mt_KarplusStrong(ptChannel_t *ch)
{
#ifdef ENABLE_E8_EFFECT
    int8_t *smpPtr;
    int16_t dat;
    uint16_t len;

    smpPtr = ch->n_loopstart;
    if (smpPtr != NULL)
    {
        len = ((ch->n_replen * 2) & 0xFFFF) - 1;
        while (len--)
        {
            dat = smpPtr[1] + smpPtr[0];

            // "arithmetic shift right" on signed number simulation
            if (dat < 0)
                dat = 0x8000 | ((uint16_t)(dat) >> 1); // 0x8000 = 2^16 - 2^(16-1)
            else
                dat /= 2;

            *smpPtr++ = dat & 0x00FF;
        }

        dat = ch->n_loopstart[0] + smpPtr[0];

        // "arithmetic shift right" on signed number simulation
        if (dat < 0)
            dat = 0x8000 | ((uint16_t)(dat) >> 1); // 0x8000 = 2^16 - 2^(16-1)
        else
            dat /= 2;

        *smpPtr = dat & 0x00FF;
    }
#else
    (void)(ch);
#endif
}

static void mt_DoRetrig(ptChannel_t *ch)
{
    PaulaSetData(ch->n_index,   ch->n_start); /* n_start is increased on 9xx */
    PaulaSetLength(ch->n_index, ch->n_length);
    PaulaSetPeriod(ch->n_index, ch->n_period);
    PaulaRestartDMA(ch->n_index);

    /* these take effect after the current DMA cycle is done */
    PaulaSetData(ch->n_index,   ch->n_loopstart);
    PaulaSetLength(ch->n_index, ch->n_replen);
}

static void mt_RetrigNote(ptChannel_t *ch)
{
    if (ch->n_cmd & 0x000F)
    {
        if (!mt_Counter)
        {
            if (ch->n_note & 0x0FFF) return;
        }

        if (!(mt_Counter % (ch->n_cmd & 0x000F)))
            mt_DoRetrig(ch);
    }
}

static void mt_VolumeSlide(ptChannel_t *ch)
{
    uint8_t cmd;

    cmd = ch->n_cmd & 0x00FF;
    if (!(cmd & 0xF0))
    {
        ch->n_volume -= (cmd & 0x0F);
        if (ch->n_volume < 0) ch->n_volume = 0;
    }
    else
    {
        ch->n_volume += (cmd >> 4);
        if (ch->n_volume > 64) ch->n_volume = 64;
    }

    PaulaSetVolume(ch->n_index, ch->n_volume);
}

static void mt_VolumeFineUp(ptChannel_t *ch)
{
    if (!mt_Counter)
    {
        ch->n_volume += (ch->n_cmd & 0x000F);
        if (ch->n_volume > 64) ch->n_volume = 64;

        PaulaSetVolume(ch->n_index, ch->n_volume);
    }
}

static void mt_VolumeFineDown(ptChannel_t *ch)
{
    if (!mt_Counter)
    {
        ch->n_volume -= (ch->n_cmd & 0x000F);
        if (ch->n_volume < 0) ch->n_volume = 0;

        PaulaSetVolume(ch->n_index, ch->n_volume);
    }
}

static void mt_NoteCut(ptChannel_t *ch)
{
    if (mt_Counter == (ch->n_cmd & 0x000F))
    {
        ch->n_volume = 0;
        PaulaSetVolume(ch->n_index, 0);
    }
}

static void mt_NoteDelay(ptChannel_t *ch)
{
    if (mt_Counter == (ch->n_cmd & 0x000F))
    {
        if (ch->n_note & 0x0FFF)
            mt_DoRetrig(ch);
    }
}

static void mt_PatternDelay(ptChannel_t *ch)
{
    if (!mt_Counter)
    {
        if (!mt_PattDelTime2)
            mt_PattDelTime = (ch->n_cmd & 0x000F) + 1;
    }
}

static void mt_FunkIt(ptChannel_t *ch)
{
    if (!mt_Counter)
    {
        ch->n_glissfunk = ((ch->n_cmd & 0x000F) << 4) | (ch->n_glissfunk & 0x0F);

        if (ch->n_glissfunk & 0xF0)
            mt_UpdateFunk(ch);
    }
}

static void mt_PositionJump(ptChannel_t *ch)
{
    mt_SongPos = (ch->n_cmd & 0x00FF) - 1; /* 0xFF (B00) jumps to pat 0 */
    mt_PBreakPos = 0;

    mt_PosJumpFlag = 1;
}

static void mt_VolumeChange(ptChannel_t *ch)
{
    ch->n_volume = ch->n_cmd & 0x00FF;
    if (ch->n_volume > 64) ch->n_volume = 64;

    PaulaSetVolume(ch->n_index, ch->n_volume);
}

static void mt_PatternBreak(ptChannel_t *ch)
{
    mt_PBreakPos = (((ch->n_cmd & 0x00F0) >> 4) * 10) + (ch->n_cmd & 0x000F);
    if (mt_PBreakPos > 63)
        mt_PBreakPos = 0;

    mt_PosJumpFlag = 1;
}

static void mt_SetSpeed(ptChannel_t *ch)
{
    if (ch->n_cmd & 0x00FF)
    {
        mt_Counter = 0;

        if (mt_TempoMode || ((ch->n_cmd & 0x00FF) < 32))
            mt_Speed = ch->n_cmd & 0x00FF;
        else
            samplesPerFrame = (uint32_t)(floorf((mt_TimerVal / (float)(ch->n_cmd & 0x00FF)) + 0.5f));
    }
}

static void mt_Arpeggio(ptChannel_t *ch)
{
    uint8_t i;
    uint8_t dat;
    const int16_t *arpPointer;

    dat = mt_Counter % 3;
    if (!dat)
    {
        PaulaSetPeriod(ch->n_index, ch->n_period);
    }
    else
    {
             if (dat == 1) dat = (ch->n_cmd & 0x00F0) >> 4;
        else if (dat == 2) dat =  ch->n_cmd & 0x000F;

        arpPointer = &mt_PeriodTable[36 * ch->n_finetune];
        for (i = 0; i < 36; ++i)
        {
            if (ch->n_period >= arpPointer[i])
            {
                PaulaSetPeriod(ch->n_index, arpPointer[i + dat]);
                break;
            }
        }
    }
}

static void mt_PortaUp(ptChannel_t *ch)
{
    ch->n_period -= ((ch->n_cmd & 0x00FF) & mt_LowMask);
    if ((ch->n_period & 0x0FFF) < 113)
        ch->n_period = (ch->n_period & 0xF000) | 113;

    PaulaSetPeriod(ch->n_index, ch->n_period & 0x0FFF);

    mt_LowMask = 0xFF;
}

static void mt_PortaDown(ptChannel_t *ch)
{
    ch->n_period += ((ch->n_cmd & 0x00FF) & mt_LowMask);
    if ((ch->n_period & 0x0FFF) > 856)
        ch->n_period = (ch->n_period & 0xF000) | 856;

    PaulaSetPeriod(ch->n_index, ch->n_period & 0x0FFF);

    mt_LowMask = 0xFF;
}

static void mt_FilterOnOff(ptChannel_t *ch)
{
#ifdef LED_FILTER
    mt_LEDStatus = !(ch->n_cmd & 0x0001);
#endif
}

static void mt_FinePortaUp(ptChannel_t *ch)
{
    if (!mt_Counter)
    {
        mt_LowMask = 0x0F;
        mt_PortaUp(ch);
    }
}

static void mt_FinePortaDown(ptChannel_t *ch)
{
    if (!mt_Counter)
    {
        mt_LowMask = 0x0F;
        mt_PortaDown(ch);
    }
}

static void mt_SetTonePorta(ptChannel_t *ch)
{
    uint8_t i;
    const int16_t *portaPointer;
    uint16_t note;

    note = ch->n_note & 0x0FFF;
    portaPointer = &mt_PeriodTable[36 * ch->n_finetune];

    i = 0;
    for (;;)
    {
        if (note >= portaPointer[i])
            break;

        if (++i >= 36)
        {
            i = 35;
            break;
        }
    }

    if ((ch->n_finetune & 8) && i) i--;

    ch->n_wantedperiod  = portaPointer[i];
    ch->n_toneportdirec = 0;

         if (ch->n_period == ch->n_wantedperiod) ch->n_wantedperiod  = 0;
    else if (ch->n_period  > ch->n_wantedperiod) ch->n_toneportdirec = 1;
}

static void mt_TonePortNoChange(ptChannel_t *ch)
{
    uint8_t i;
    const int16_t *portaPointer;

    if (ch->n_wantedperiod)
    {
        if (ch->n_toneportdirec)
        {
            ch->n_period -= ch->n_toneportspeed;
            if (ch->n_period <= ch->n_wantedperiod)
            {
                ch->n_period = ch->n_wantedperiod;
                ch->n_wantedperiod = 0;
            }
        }
        else
        {
            ch->n_period += ch->n_toneportspeed;
            if (ch->n_period >= ch->n_wantedperiod)
            {
                ch->n_period = ch->n_wantedperiod;
                ch->n_wantedperiod = 0;
            }
        }

        if (!(ch->n_glissfunk & 0x0F))
        {
            PaulaSetPeriod(ch->n_index, ch->n_period);
        }
        else
        {
            portaPointer = &mt_PeriodTable[36 * ch->n_finetune];

            i = 0;
            for (;;)
            {
                if (ch->n_period >= portaPointer[i])
                    break;

                if (++i >= 36)
                {
                    i = 35;
                    break;
                }
            }

            PaulaSetPeriod(ch->n_index, portaPointer[i]);
        }
    }
}

static void mt_TonePortamento(ptChannel_t *ch)
{
    if (ch->n_cmd & 0x00FF)
    {
        ch->n_toneportspeed = ch->n_cmd & 0x00FF;
        ch->n_cmd &= 0xFF00;
    }

    mt_TonePortNoChange(ch);
}

static void mt_VibratoNoChange(ptChannel_t *ch)
{
    uint8_t vibratoTemp;
    int16_t vibratoData;

    vibratoTemp = (ch->n_vibratopos / 4) & 31;
    vibratoData = ch->n_wavecontrol & 3;

    if (!vibratoData)
    {
        vibratoData = mt_VibratoTable[vibratoTemp];
    }
    else
    {
        if (vibratoData == 1)
        {
            if (ch->n_vibratopos < 0)
                vibratoData = 255 - (vibratoTemp * 8);
            else
                vibratoData = vibratoTemp * 8;
        }
        else
        {
            vibratoData = 255;

        }
    }

    vibratoData = (vibratoData * (ch->n_vibratocmd & 0x0F)) / 128;

    if (ch->n_vibratopos < 0)
        vibratoData = ch->n_period - vibratoData;
    else
        vibratoData = ch->n_period + vibratoData;

    PaulaSetPeriod(ch->n_index, vibratoData);

    ch->n_vibratopos += ((ch->n_vibratocmd >> 4) * 4);
}

static void mt_Vibrato(ptChannel_t *ch)
{
    if (ch->n_cmd & 0x00FF)
    {
        if (ch->n_cmd & 0x000F)
            ch->n_vibratocmd = (ch->n_vibratocmd & 0xF0) | (ch->n_cmd & 0x000F);

        if (ch->n_cmd & 0x00F0)
            ch->n_vibratocmd = (ch->n_cmd & 0x00F0) | (ch->n_vibratocmd & 0x0F);
    }

    mt_VibratoNoChange(ch);
}

static void mt_TonePlusVolSlide(ptChannel_t *ch)
{
    mt_TonePortNoChange(ch);
    mt_VolumeSlide(ch);
}

static void mt_VibratoPlusVolSlide(ptChannel_t *ch)
{
    mt_VibratoNoChange(ch);
    mt_VolumeSlide(ch);
}

static void mt_Tremolo(ptChannel_t *ch)
{
    int8_t tremoloTemp;
    int16_t tremoloData;

    if (ch->n_cmd & 0x00FF)
    {
        if (ch->n_cmd & 0x000F)
            ch->n_tremolocmd = (ch->n_tremolocmd & 0xF0) | (ch->n_cmd & 0x000F);

        if (ch->n_cmd & 0x00F0)
            ch->n_tremolocmd = (ch->n_cmd & 0x00F0) | (ch->n_tremolocmd & 0x0F);
    }

    tremoloTemp = (ch->n_tremolopos / 4) & 31;
    tremoloData = (ch->n_wavecontrol >> 4) & 3;

    if (!tremoloData)
    {
        tremoloData = mt_VibratoTable[tremoloTemp];
    }
    else
    {
        if (tremoloData == 1)
        {
            if (ch->n_vibratopos < 0) /* PT bug, should've been n_tremolopos */
                tremoloData = 255 - (tremoloTemp * 8);
            else
                tremoloData = tremoloTemp * 8;
        }
        else
        {
            tremoloData = 255;
        }
    }

    tremoloData = (tremoloData * (ch->n_tremolocmd & 0x0F)) / 64;

    if (ch->n_tremolopos < 0)
    {
        tremoloData = ch->n_volume - tremoloData;
        if (tremoloData < 0) tremoloData = 0;
    }
    else
    {
        tremoloData = ch->n_volume + tremoloData;
        if (tremoloData > 64) tremoloData = 64;
    }

    PaulaSetVolume(ch->n_index, tremoloData);

    ch->n_tremolopos += ((ch->n_tremolocmd >> 4) * 4);
}

static void mt_SampleOffset(ptChannel_t *ch)
{
    uint16_t newOffset;

    if (ch->n_cmd & 0x00FF)
        ch->n_sampleoffset = ch->n_cmd & 0x00FF;

    newOffset = ch->n_sampleoffset * 128;
    if (newOffset < ch->n_length)
    {
        ch->n_length -=  newOffset;
        ch->n_start  += (newOffset * 2);
    }
    else
    {
        ch->n_length = 1; /* this must NOT be set to 0! 1 is the correct value */
    }
}

static void mt_E_Commands(ptChannel_t *ch)
{
    switch ((ch->n_cmd & 0x00F0) >> 4)
    {
        case 0x00: mt_FilterOnOff(ch);       break;
        case 0x01: mt_FinePortaUp(ch);       break;
        case 0x02: mt_FinePortaDown(ch);     break;
        case 0x03: mt_SetGlissControl(ch);   break;
        case 0x04: mt_SetVibratoControl(ch); break;
        case 0x05: mt_SetFineTune(ch);       break;
        case 0x06: mt_JumpLoop(ch);          break;
        case 0x07: mt_SetTremoloControl(ch); break;
        case 0x08: mt_KarplusStrong(ch);     break;
        case 0x09: mt_RetrigNote(ch);        break;
        case 0x0A: mt_VolumeFineUp(ch);      break;
        case 0x0B: mt_VolumeFineDown(ch);    break;
        case 0x0C: mt_NoteCut(ch);           break;
        case 0x0D: mt_NoteDelay(ch);         break;
        case 0x0E: mt_PatternDelay(ch);      break;
        case 0x0F: mt_FunkIt(ch);            break;
    }
}

static void mt_CheckMoreEfx(ptChannel_t *ch)
{
    switch ((ch->n_cmd & 0x0F00) >> 8)
    {
        case 0x09: mt_SampleOffset(ch); break;
        case 0x0B: mt_PositionJump(ch); break;
        case 0x0D: mt_PatternBreak(ch); break;
        case 0x0E: mt_E_Commands(ch);   break;
        case 0x0F: mt_SetSpeed(ch);     break;
        case 0x0C: mt_VolumeChange(ch); break;

        default: PaulaSetPeriod(ch->n_index, ch->n_period); break;
    }
}

static void mt_CheckEfx(ptChannel_t *ch)
{
    mt_UpdateFunk(ch);

    if (ch->n_cmd & 0x0FFF)
    {
        switch ((ch->n_cmd & 0x0F00) >> 8)
        {
            case 0x00: mt_Arpeggio(ch);            break;
            case 0x01: mt_PortaUp(ch);             break;
            case 0x02: mt_PortaDown(ch);           break;
            case 0x03: mt_TonePortamento(ch);      break;
            case 0x04: mt_Vibrato(ch);             break;
            case 0x05: mt_TonePlusVolSlide(ch);    break;
            case 0x06: mt_VibratoPlusVolSlide(ch); break;
            case 0x0E: mt_E_Commands(ch);          break;
            case 0x07:
                PaulaSetPeriod(ch->n_index, ch->n_period);
                mt_Tremolo(ch);
            break;
            case 0x0A:
                PaulaSetPeriod(ch->n_index, ch->n_period);
                mt_VolumeSlide(ch);
            break;

            default: PaulaSetPeriod(ch->n_index, ch->n_period); break;
        }
    }
    else
    {
        PaulaSetPeriod(ch->n_index, ch->n_period);
    }
}

static void mt_SetPeriod(ptChannel_t *ch)
{
    uint8_t i;
    uint16_t note;

    note = ch->n_note & 0x0FFF;
    for (i = 0; i < 36; ++i)
    {
        if (note >= mt_PeriodTable[i]) break;
    }

    if (i < 36)
        ch->n_period = mt_PeriodTable[(36 * ch->n_finetune) + i];

    if ((ch->n_cmd & 0x0FF0) != 0x0ED0) /* no note delay */
    {
        if (!(ch->n_wavecontrol & 0x04)) ch->n_vibratopos = 0;
        if (!(ch->n_wavecontrol & 0x40)) ch->n_tremolopos = 0;

        PaulaSetLength(ch->n_index, ch->n_length);
        PaulaSetData(ch->n_index, ch->n_start);

        if (ch->n_length == 0)
        {
            ch->n_loopstart = 0;
            ch->n_replen = 1;
        }

        PaulaSetPeriod(ch->n_index, ch->n_period);
        PaulaRestartDMA(ch->n_index);
    }

    mt_CheckMoreEfx(ch);
}

static void mt_PlayVoice(ptChannel_t *ch)
{
    uint8_t *dataPtr;
    uint8_t sample;
    uint8_t cmd;
    uint16_t sampleOffset;
    uint16_t repeat;

    if (!ch->n_note && !ch->n_cmd)
        PaulaSetPeriod(ch->n_index, ch->n_period);

    dataPtr = &mt_SongDataPtr[mt_PattPosOff];

    ch->n_note = (dataPtr[0] << 8) | dataPtr[1];
    ch->n_cmd  = (dataPtr[2] << 8) | dataPtr[3];

    sample = (dataPtr[0] & 0xF0) | (dataPtr[2] >> 4);
    if ((sample >= 1) && (sample <= 31)) /* PT2 BUG FIX: don't do samples >31 */
    {
        sample--;
        sampleOffset = 42 + (30 * sample);

        ch->n_start    = mt_SampleStarts[sample];
        ch->n_finetune = mt_SongDataPtr[sampleOffset + 2] & 0x0F;
        ch->n_volume   = mt_SongDataPtr[sampleOffset + 3];
        ch->n_length   = *PTR2WORD(&mt_SongDataPtr[sampleOffset + 0]);
        ch->n_replen   = *PTR2WORD(&mt_SongDataPtr[sampleOffset + 6]);

        /* PT2 BUG FIX: don't blow our eardrums... */
        if (ch->n_volume > 64)
            ch->n_volume = 64;

        PaulaSetVolume(ch->n_index, ch->n_volume);

        repeat = *PTR2WORD(&mt_SongDataPtr[sampleOffset + 4]);
        if (repeat > 0)
        {
            ch->n_loopstart = ch->n_start + (repeat * 2);
            ch->n_wavestart = ch->n_loopstart;
            ch->n_length    = repeat + ch->n_replen;
        }
        else
        {
            ch->n_loopstart = ch->n_start;
            ch->n_wavestart = ch->n_start;
        }
    }

    if (ch->n_note & 0x0FFF)
    {
        if ((ch->n_cmd & 0x0FF0) == 0x0E50) /* set finetune */
        {
            mt_SetFineTune(ch);
            mt_SetPeriod(ch);
        }
        else
        {
            cmd = (ch->n_cmd & 0x0F00) >> 8;
            if ((cmd == 0x03) || (cmd == 0x05))
            {
                mt_SetTonePorta(ch);
                mt_CheckMoreEfx(ch);
            }
            else if (cmd == 0x09)
            {
                mt_CheckMoreEfx(ch);
                mt_SetPeriod(ch);
            }
            else
            {
                mt_SetPeriod(ch);
            }
        }
    }
    else
    {
        mt_CheckMoreEfx(ch);
    }

    mt_PattPosOff += 4;
}

static void mt_NextPosition(void)
{
    mt_PatternPos  = mt_PBreakPos * 16;
    mt_PBreakPos   = 0;
    mt_PosJumpFlag = 0;

    mt_SongPos = (mt_SongPos + 1) & 0x7F;
    if (mt_SongPos >= mt_SongDataPtr[950])
        mt_SongPos = 0;
}

static void mt_MusicCallback(void)
{
    uint8_t i;

    mt_Counter++;
    if (mt_Counter >= mt_Speed)
    {
        mt_Counter = 0;

        if (!mt_PattDelTime2)
        {
            mt_PattPosOff = (1084 + (mt_SongDataPtr[952 + mt_SongPos] * 1024)) + mt_PatternPos;

            for (i = 0; i < 4; ++i)
            {
                mt_PlayVoice(&mt_ChanTemp[i]);

                /* these take effect after the current DMA cycle is done */
                PaulaSetData(i,   mt_ChanTemp[i].n_loopstart);
                PaulaSetLength(i, mt_ChanTemp[i].n_replen);
            }
        }
        else
        {
            for (i = 0; i < 4; ++i)
                mt_CheckEfx(&mt_ChanTemp[i]);
        }

        mt_PatternPos += 16;

        if (mt_PattDelTime)
        {
            mt_PattDelTime2 = mt_PattDelTime;
            mt_PattDelTime = 0;
        }

        if (mt_PattDelTime2)
        {
            mt_PattDelTime2--;
            if (mt_PattDelTime2) mt_PatternPos -= 16;
        }

        if (mt_PBreakFlag)
        {
            mt_PatternPos = mt_PBreakPos * 16;
            mt_PBreakPos  = 0;
            mt_PBreakFlag = 0;
        }

        if ((mt_PatternPos >= 1024) || mt_PosJumpFlag)
            mt_NextPosition();
    }
    else
    {
        for (i = 0; i < 4; ++i)
            mt_CheckEfx(&mt_ChanTemp[i]);

        if (mt_PosJumpFlag) mt_NextPosition();
    }
}

static void mt_Init(uint8_t *mt_Data)
{
    uint8_t *sampleStarts;
    int8_t pattNum;
    uint8_t i;
    uint16_t *p;
    uint16_t j;
    uint16_t lastPeriod;

    for (i = 0; i < 4; ++i)
        mt_ChanTemp[i].n_index = i;

    mt_SongDataPtr = mt_Data;

    pattNum = 0;
    for (i = 0; i < 128; ++i)
    {
        if (mt_SongDataPtr[952 + i] > pattNum)
            pattNum = mt_SongDataPtr[952 + i];
    }
    pattNum++;

    sampleStarts = &mt_SongDataPtr[1084 + (pattNum * 1024)];
    for (i = 0; i < 31; ++i)
    {
        mt_SampleStarts[i] = (int8_t *)(sampleStarts);
        p = PTR2WORD(&mt_SongDataPtr[42 + (30 * i)]);

        /* swap bytes in words (Amiga word -> Intel word) */
        p[0] = SWAP16(p[0]); /* n_length */
        p[2] = SWAP16(p[2]); /* n_repeat */
        p[3] = SWAP16(p[3]); /* n_replen */

        /* loop point sanity checking */
        if ((p[2] + p[3]) > p[0])
        {
            if (((p[2] / 2) + p[3]) <= p[0])
            {
                /* fix for poorly converted STK->PT modules */
                p[2] /= 2;
            }
            else
            {
                /* loop points are still illegal, deactivate loop */
                p[2] = 0;
                p[3] = 1;
            }
        }

        if (p[3] <= 1)
        {
            p[3] = 1; /* fix illegal loop length (f.ex. from "Fastracker II" .MODs) */

            /* if no loop, zero first two samples of data to prevent "beep" */
            sampleStarts[0] = 0;
            sampleStarts[1] = 0;
        }

        sampleStarts += (p[0] * 2);
    }

    if (mt_PeriodTable)
    {
        free(mt_PeriodTable);
        mt_PeriodTable = NULL;
    }

    /*
    ** +14 for 14 extra zeroes to prevent access violation on -1
    ** (15 unsigned) finetuned samples with B-3 >+1 note arpeggios.
    ** PT was never bug free. :-)
    */
    mt_PeriodTable = (int16_t *)(calloc((36 * 16) + 14, sizeof (int16_t)));

    for (i = 0; i < 16; ++i)
    {
        lastPeriod = 856;
        for (j = 0; j < 36; ++j)
            lastPeriod = mt_PeriodTable[(36 * i) + j] = lastPeriod
                + mt_PeriodDeltas[(36 * i) + j];
    }

    mt_Speed        = 6;
    mt_Counter      = 0;
    mt_SongPos      = 0;
    mt_PatternPos   = 0;
    mt_Enable       = 0;
    mt_PattDelTime  = 0;
    mt_PattDelTime2 = 0;
    mt_PBreakPos    = 0;
    mt_PosJumpFlag  = 0;
    mt_PBreakFlag   = 0;
    mt_LowMask      = 0xFF;
}

/* these are used to create an equal powered panning */
static float sinApx(float x)
{
    x = x * (2.0f - x);
    return (x * 1.09742972f + x * x * 0.31678383f);
}

static float cosApx(float x)
{
    x = (1.0f - x) * (1.0f + x);
    return (x * 1.09742972f + x * x * 0.31678383f);
}
/* ------------------------------------------------- */

static void mt_genPans(int8_t stereoSeparation)
{
    uint8_t scaledPanPos;

    float p;

    if (stereoSeparation > 100)
        stereoSeparation = 100;

    scaledPanPos = (stereoSeparation * 128) / 100;

    p = (128 - scaledPanPos) * (1.0f / 256.0f);
    AUD[0].PANL = cosApx(p);
    AUD[0].PANR = sinApx(p);
    AUD[3].PANL = cosApx(p);
    AUD[3].PANR = sinApx(p);

    p = (128 + scaledPanPos) * (1.0f / 256.0f);
    AUD[1].PANL = cosApx(p);
    AUD[1].PANR = sinApx(p);
    AUD[2].PANL = cosApx(p);
    AUD[2].PANR = sinApx(p);
}

static void mixSampleBlock(int16_t *streamOut, uint32_t numSamples)
{
    uint8_t i;
    int16_t *sndOut;
    uint16_t j;

    float tempSample;
    float tempVolume;
    float out[2];

    paulaVoice_t *v;

#ifdef USE_BLEP
    blep_t *bSmp;
    blep_t *bVol;
#endif

    memset(masterBufferL, 0, sizeof (float) * numSamples);
    memset(masterBufferR, 0, sizeof (float) * numSamples);

    for (i = 0; i < 4; ++i)
    {
        v = &AUD[i];

#ifdef USE_BLEP
        bSmp = &blep[i];
        bVol = &blepVol[i];
#endif

        if (v->DMA_ON)
        {
            for (j = 0; j < numSamples; ++j)
            {
                tempSample = (v->DMA_DAT == NULL) ? 0.0f : ((float)(v->DMA_DAT[v->DMA_POS]) * (1.0f / 128.0f));
                tempVolume = (v->DMA_DAT == NULL) ? 0.0f : v->SRC_VOL;

#ifdef USE_BLEP
                if (tempSample != bSmp->lastValue)
                {
                    if ((v->LASTDELTA > 0.0f) && (v->LASTDELTA > v->LASTFRAC))
                        blepAdd(bSmp, v->LASTFRAC / v->LASTDELTA, bSmp->lastValue - tempSample);

                    bSmp->lastValue = tempSample;
                }

                if (tempVolume != bVol->lastValue)
                {
                    blepAdd(bVol, 0.0f, bVol->lastValue - tempVolume);
                    bVol->lastValue = tempVolume;
                }

                if (bSmp->samplesLeft) tempSample += blepRun(bSmp);
                if (bVol->samplesLeft) tempVolume += blepRun(bVol);
#endif

                tempSample *= tempVolume;
                masterBufferL[j] += (tempSample * v->PANL);
                masterBufferR[j] += (tempSample * v->PANR);

                v->FRAC += v->DELTA;
                if (v->FRAC >= 1.0f)
                {
                    v->FRAC     -= 1.0f;
                    v->LASTFRAC  = v->FRAC;
                    v->LASTDELTA = v->DELTA;

                    if (++v->DMA_POS == v->DMA_LEN)
                    {
                        v->DMA_POS = 0;

                        /* fetch register values now */
                        v->DMA_LEN = v->SRC_LEN;
                        v->DMA_DAT = v->SRC_DAT;
                    }
                }
            }
        }
    }

    sndOut = streamOut;
    for (j = 0; j < numSamples; ++j)
    {
        if (mt_Enable)
        {
            out[0] = masterBufferL[j];
            out[1] = masterBufferR[j];

#ifdef USE_LOWPASS
            lossyIntegrator(&filterLo, out, out);
#endif

#ifdef LED_FILTER
            if (mt_LEDStatus)
                lossyIntegratorLED(filterLEDC, &filterLED, out, out);
#endif

#ifdef USE_HIGHPASS
            lossyIntegratorHighPass(&filterHi, out, out);
#endif

            out[0] *= (32767.0f / NORM_FACTOR);
            out[1] *= (32767.0f / NORM_FACTOR);

#ifdef PREVENT_CLIPPING
                 if (out[0] < -32768.0f) out[0] = -32768.0f;
            else if (out[0] >  32767.0f) out[0] =  32767.0f;
                 if (out[1] < -32768.0f) out[1] = -32768.0f;
            else if (out[1] >  32767.0f) out[1] =  32767.0f;
#endif
            *sndOut++ = (int16_t)(out[0]);
            *sndOut++ = (int16_t)(out[1]);
        }
        else
        {
            *sndOut++ = 0;
            *sndOut++ = 0;
        }
    }
}

static void CALLBACK waveOutProc(HWAVEOUT _hWaveOut, UINT uMsg,
    DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    int16_t *outputStream;
    int32_t  sampleBlock;
    int32_t  samplesTodo;

    WAVEHDR *waveBlockHeader;

    /* make compiler happy! (warning C4100) */
    (void)(dwParam2);
    (void)(dwInstance);

    if (uMsg == MM_WOM_DONE)
    {
        mixingMutex = 1;

        waveBlockHeader = (WAVEHDR *)(dwParam1);
        waveOutUnprepareHeader(_hWaveOut, waveBlockHeader, sizeof (WAVEHDR));

        if (isMixing)
        {
            memcpy(waveBlockHeader->lpData, mixerBuffer, soundBufferSize);

            waveOutPrepareHeader(_hWaveOut, waveBlockHeader, sizeof (WAVEHDR));
            waveOutWrite(_hWaveOut, waveBlockHeader, sizeof (WAVEHDR));

            outputStream = (int16_t *)(mixerBuffer);
            sampleBlock  = soundBufferSize / 4;

            while (sampleBlock)
            {
                samplesTodo = (sampleBlock < samplesLeft) ? sampleBlock : samplesLeft;
                if (samplesTodo > 0)
                {
                    mixSampleBlock(outputStream, samplesTodo);
                    outputStream += (samplesTodo * 2);

                    sampleBlock -= samplesTodo;
                    samplesLeft -= samplesTodo;
                }
                else
                {
                    if (mt_Enable)
                        mt_MusicCallback();

                    samplesLeft = samplesPerFrame;
                }
            }
        }

        mixingMutex = 0;
    }
}

void pt2play_PauseSong(int8_t pause)
{
    mt_Enable = pause ? 0 : 1;
}

void pt2play_PlaySong(uint8_t *moduleData, int8_t tempoMode)
{
    uint8_t i;

    mt_Enable = 0;

    memset(AUD, 0, sizeof (AUD));
    for (i = 0; i < 4; ++i)
    {
        AUD[i].DMA_DAT = NULL;
        AUD[i].SRC_DAT = NULL;
    }

    mt_Init(moduleData);
    mt_genPans(STEREO_SEP);

#ifdef USE_BLEP
    memset(blep,    0, sizeof (blep));
    memset(blepVol, 0, sizeof (blepVol));
#endif

#ifdef USE_LOWPASS
    clearLossyIntegrator(&filterLo);
#endif

#ifdef LED_FILTER
    clearLEDFilter(&filterLED);
#endif

#ifdef USE_HIGHPASS
    clearLossyIntegrator(&filterHi);
#endif

    mt_TempoMode = tempoMode ? 1 : 0; /* 0 = cia, 1 = vblank */
    mt_Enable = 1;

#ifdef LED_FILTER
    mt_LEDStatus = 0;
#endif
}

void pt2play_SetStereoSep(uint8_t percentage)
{
    mt_genPans(percentage);
}

static int8_t openMixer(uint32_t _samplingFrequency, uint32_t _soundBufferSize)
{
    uint8_t i;

#ifdef USE_LOWPASS
    float loR;
    float loC;
    float loHz;
#endif

#ifdef USE_HIGHPASS
    float hiR;
    float hiC;
    float hiHz;
#endif

    MMRESULT r;

    if (!hWaveOut)
    {
        f_outputFreq        = (float)(_samplingFrequency);
        soundBufferSize     = _soundBufferSize;
        masterBufferL       = (float *)(malloc(soundBufferSize * sizeof (float)));
        masterBufferR       = (float *)(malloc(soundBufferSize * sizeof (float)));
        wfx.nSamplesPerSec  = _samplingFrequency;
        wfx.wBitsPerSample  = 16;
        wfx.nChannels       = 2;
        wfx.cbSize          = 0;
        wfx.wFormatTag      = WAVE_FORMAT_PCM;
        wfx.nBlockAlign     = (wfx.wBitsPerSample * wfx.nChannels) / 8;
        wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

        if ((masterBufferL == NULL) || (masterBufferR == NULL))
            return (0); /* gets free'd later */

        r = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)(waveOutProc), 0L, CALLBACK_FUNCTION);
        if (r != MMSYSERR_NOERROR) return (0);

        for (i = 0; i < SOUND_BUFFERS; ++i)
        {
            waveBlocks[i].dwBufferLength = soundBufferSize;
            waveBlocks[i].lpData = (LPSTR)(calloc(soundBufferSize, 1));
            if (waveBlocks[i].lpData == NULL)
                return (0); /* gets free'd later */

            waveOutPrepareHeader(hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
            waveOutWrite(hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
        }

        mixerBuffer = (int8_t *)(calloc(soundBufferSize, 1));
        if (mixerBuffer == NULL)
            return (0); /* gets free'd later */

        mt_TimerVal = (f_outputFreq * 125.0f) / 50.0f;
        samplesPerFrame = (uint32_t)(floorf((mt_TimerVal / 125.0f) + 0.5f));

#ifdef USE_LOWPASS
        loR  = 750.0f;
        loC  = filterHzAndRtoC(loR, 5000.0f);
        loHz = filterRCtoHz(loR, loC);

        calcCoeffLossyIntegrator(f_outputFreq, loHz, &filterLo);
#endif

#ifdef LED_FILTER
        filterLEDC.led = calcCoeffLED(f_outputFreq, 3090.0f);
        filterLEDC.ledFb = 0.125f + 0.125f / (1.0f - filterLEDC.led); // Fb = 0.125 : Q ~= 1/sqrt(2) (Butterworth)
#endif

#ifdef USE_HIGHPASS
        hiR  = 2000.0f;
        hiC  = filterHzAndRtoC(hiR, 5.2f);
        hiHz = filterRCtoHz(hiR, hiC);

        calcCoeffLossyIntegrator(f_outputFreq, hiHz, &filterHi);
#endif

        isMixing = 1;
        return (1);
    }

    return (1);
}


void pt2play_Close(void)
{
    uint8_t i;

    mt_Enable = 0;

    if (isMixing)
    {
        isMixing = 0;
        while (mixingMutex) Sleep(1);

        if (hWaveOut)
        {
            for (i = 0; i < SOUND_BUFFERS; ++i)
            {
                if (waveBlocks[i].lpData != NULL)
                {
                    waveOutUnprepareHeader(hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
                    waveBlocks[i].dwFlags &= ~WHDR_PREPARED;
                    
                    Sleep(50);

                    free(waveBlocks[i].lpData);
                    waveBlocks[i].lpData = NULL;
                }
            }

            waveOutReset(hWaveOut);
            waveOutClose(hWaveOut);

            hWaveOut = 0;

            if (mixerBuffer    != NULL) free(mixerBuffer);    mixerBuffer    = NULL;
            if (masterBufferL  != NULL) free(masterBufferL);  masterBufferL  = NULL;
            if (masterBufferR  != NULL) free(masterBufferR);  masterBufferR  = NULL;
            if (mt_PeriodTable != NULL) free(mt_PeriodTable); mt_PeriodTable = NULL;
        }
    }
}

int8_t pt2play_Init(uint32_t outputFreq)
{
    if (!openMixer(outputFreq, 2048))
    {
        pt2play_Close();
        return (0);
    }

    return (1);
}

/* END OF FILE */