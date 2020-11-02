/*
**  -- PTBUDDY by 8bitbubsy -- 28th of September 2013  8:28AM
** Accurate ProTracker 2.3a Replayer - "Just Like an Amiga"-edition
**    -------------
**
** Ported from 68k asm (PT2.3a_replay_cia.S).
** Some changes added from newer PT releases.
**
** This project aims to get the closest Amiga sound output
** as I can possibly make, also the replayer is supposed
** to be as accurate as possible as well.
**
** Amiga high/low pass + LED filter emulation is implemented,
** as well as BLEP (band-limited steps) to simulate the
** Amiga sound output even further.
**
** I'll probably look into PWM volume in the future for
** even more realistic Paula simulation.
**
** Thanks to aciddose/adejr for the BLEP and filter
** routines.
**
** HINT: Use my mod2h.exe tool for easy injection of modules
** in your production, followed by an EXE packer.
**
*/

#define WIN32_LEAN_AND_MEAN // for stripping windows.h include

#define USE_BLEP      // band-limited steps

//#define USE_HARDPAN   // 100% stereo separation,
                      //    recommended for experimental use only

#define USE_FILTERS   // lowpass, highpass, LED

#define FILTER_TYPE 1 // 0 = A500 (4421Hz lowpass), 1 = A1200 (no lowpass)

// BLEP
#define ZC 8
#define OS 5
#define SP 5
#define NS (ZC * OS / SP)
#define RNS 7 // RNS = (2^ > NS) - 1
// ----

#define SOUND_BUF_LEN 4096
#define SOUND_BUF_NUM 4

#ifdef USE_HARDPAN
#define PAN_L    0
#define PAN_R  255
#else
#define PAN_L   96
#define PAN_R  160
#endif

#include <stdio.h>
#include <stdlib.h>
#include <windows.h> // for mixer stream
#include <mmsystem.h> // for mixer stream

// Structs
typedef struct
{
    // These must be in this order
    short n_note;
    unsigned short n_cmd;
    char n_index;
    // -----------

    char *n_start;
    char *n_wavestart;
    char *n_loopstart;
    char n_efx;
    char n_volume;
    char n_toneportdirec;
    char n_vibratopos;
    char n_tremolopos;
    char n_pattpos;
    char n_loopcount;   
    unsigned char n_cmdlo;
    unsigned char n_wavecontrol;
    unsigned char n_glissfunk;
    unsigned char n_sampleoffset;
    unsigned char n_toneportspeed;
    unsigned char n_vibratocmd;
    unsigned char n_tremolocmd;
    unsigned char n_funkoffset;
    short n_period;
    short n_finetune;
    short n_wantedperiod;
    unsigned short n_length;
    unsigned short n_replen;
    unsigned short n_repend; 
} PT_CHN;

typedef struct
{
    char *DAT;
    char *REPDAT;
    char VOL;
    char TRIGGER;
    unsigned short POS;
    unsigned short LEN;
    unsigned short REPLEN;
    float INC;  
    float FRAC;
} PA_CHN;

#ifdef USE_BLEP
typedef struct blep_data
{
    int index;
    int samplesLeft;
    float buffer[RNS + 1];
    float lastValue; 
} BLEP;
#endif

// Arrays
static PT_CHN mt_Chan1temp =
{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

static PT_CHN mt_Chan2temp =
{ 0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

static PT_CHN mt_Chan3temp =
{ 0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

static PT_CHN mt_Chan4temp =
{ 0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

static PA_CHN AUD[4] =
{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

static char *mt_SampleStarts[31] =
{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

static short *mt_PeriodTable;

#ifdef USE_BLEP
static BLEP mt_blep[4];
static BLEP mt_blepVol[4];
#endif

// Constant Tables
static const float mt_PaulaPansL[4] =
{ (float)PAN_R, (float)PAN_L, (float)PAN_L, (float)PAN_R };

static const float mt_PaulaPansR[4] =
{ (float)PAN_L, (float)PAN_R, (float)PAN_R, (float)PAN_L };

static const unsigned char mt_FunkTable[16] =
{
	0,5,6,7,8,10,11,13,16,19,22,26,32,43,64,128
};

static const unsigned char mt_VibratoTable[32] = 
{
	0, 24, 49, 74, 97,120,141,161,
	180,197,212,224,235,244,250,253,
	255,253,250,244,235,224,212,197,
	180,161,141,120, 97, 74, 49, 24
};

static const char mt_PeriodDiffs[576] =
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
static const unsigned int mt_blepData[48] =
{
    0x3f7fe1f1,0x3f7fd548,0x3f7fd6a3,0x3f7fd4e3,
    0x3f7fad85,0x3f7f2152,0x3f7dbfae,0x3f7accdf,
    0x3f752f1e,0x3f6b7384,0x3f5bfbcb,0x3f455cf2,
    0x3f26e524,0x3f0128c4,0x3eacc7dc,0x3e29e86b,
    0x3c1c1d29,0xbde4bbe6,0xbe3aae04,0xbe48dedd,
    0xbe22ad7e,0xbdb2309a,0xbb82b620,0x3d881411,
    0x3ddadbf3,0x3de2c81d,0x3daaa01f,0x3d1e769a,
    0xbbc116d7,0xbd1402e8,0xbd38a069,0xbd0c53bb,
    0xbc3ffb8c,0x3c465fd2,0x3cea5764,0x3d0a51d6,
    0x3ceae2d5,0x3c92ac5a,0x3be4cbf7,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0x00000000,0x00000000,0x00000000
};
#endif

// Win32 Mixer Variables
static WAVEHDR *waveBlocks;
static HWAVEOUT hWaveOut;
static WAVEFORMATEX wfx;
static LPSTR mixerBuffer;

// General Variables
static char mt_TempoMode;
static char mt_SongPos;
static char mt_PosJumpFlag;
static char mt_PBreakFlag;
static char mt_Enable;
static char mt_PBreakPos;
static char mt_PattDelTime;
static char mt_PattDelTime2;
static unsigned char *mt_SongDataPtr;
static unsigned char mt_LowMask;
static unsigned char mt_Counter;
static unsigned char mt_Speed;
static short mt_SampleCounter;
static short mt_SamplesPerFrame;
static unsigned short mt_PatternPos;
static float *AUDBUFL;
static float *AUDBUFR;
static int mt_TimerVal;
static int mt_MixFreq;
static unsigned int mt_PattPosOff;
static unsigned int mt_PattOff;
#ifdef USE_FILTERS
#if FILTER_TYPE == 0
static float mt_filterCoeff_LP;
static float mt_filter_LP_L;
static float mt_filter_LP_R;
#endif
static float mt_filterCoeff_LED1;
static float mt_filterCoeff_LED2;
static float mt_filter_LED_L1;
static float mt_filter_LED_R1;
static float mt_filter_LED_L2;
static float mt_filter_LED_R2;
static float mt_filterCoeff_HP;
static float mt_filter_HP_L;
static float mt_filter_HP_R;
static char mt_LEDFilterState;
#endif

// Macros
#define mt_PaulaStop(i) AUD[i].POS=0;AUD[i].FRAC=0.0f;AUD[i].TRIGGER=0;
#define mt_PaulaStart(i) AUD[i].POS=0;AUD[i].FRAC=0.0f;AUD[i].TRIGGER=1;
#define mt_PaulaSetVol(i, x) AUD[i].VOL=x;
#define mt_PaulaSetLen(i, x) AUD[i].LEN=x;
#define mt_PaulaSetDat(i, x) AUD[i].DAT=x;
#define mt_PaulaSetLoop(i, x, y) if(x)AUD[i].REPDAT=x;AUD[i].REPLEN=y;
#define mt_PaulaSetPer(i, x) if(x)AUD[i].INC=(3546895.0f/(float)(x))/(float)mt_MixFreq;
#define mt_AmigaWord(x) (unsigned short)(((x)<<8)|((x)>>8))
#define LERP(I, F) (I[0]+F*(I[1]-I[0]))

#ifdef USE_FILTERS
#define FLT_RC_COEFF_LP  (6.2831853f * 4421.0f) // f = 1/(R*C)/(2*pi)
#define FLT_RC_COEFF_LED (6.2831853f * 3090.0f) // f = 1/(R*C)/(2*pi)
#define FLT_RC_COEFF_HP  (6.2831853f *    5.2f) // f = 1/(R*C)/(2*pi)
#define FLT_DENORM_OFS 1E-10f
#endif

#ifdef USE_BLEP
void blepAdd(BLEP *b, float offset, float amplitude);
float blepRun(BLEP *b);
#endif

/* CODE START */

static void mt_paulaMix(int numSamples, short *streamOut)
{
    char i;
    short *sndOut;
    short j;
    float L;
    float R;
    float smp;
    float vol;
    float delta;
 
    if (mt_Enable)
    {
        memset(AUDBUFL, 0, sizeof (float) * numSamples);
        memset(AUDBUFR, 0, sizeof (float) * numSamples);

        for (i = 0; i < 4; ++i)
        {
            if (AUD[i].TRIGGER && AUD[i].DAT)
            {
                for (j = 0; j < numSamples; ++j)
                {
                    smp = (float)AUD[i].DAT[AUD[i].POS];
                    vol = (float)AUD[i].VOL;

#ifdef USE_BLEP
                    if (smp != mt_blep[i].lastValue)
                    {
                        delta = mt_blep[i].lastValue - smp;
                        mt_blep[i].lastValue = smp;
                        blepAdd(&mt_blep[i], AUD[i].FRAC / AUD[i].INC,
                            delta);
                    }

                    if (vol != mt_blepVol[i].lastValue)
                    {
                        delta = mt_blepVol[i].lastValue - vol;
                        mt_blepVol[i].lastValue = vol;
                        blepAdd(&mt_blepVol[i], 0.0, delta);
                    }

                    if (mt_blep[i].samplesLeft > 0)
                        smp += blepRun(&mt_blep[i]);

                    if (mt_blepVol[i].samplesLeft > 0)
                        vol += blepRun(&mt_blepVol[i]);
#endif

                    smp *= vol;

                    AUDBUFL[j] += (smp * mt_PaulaPansL[i]);
                    AUDBUFR[j] += (smp * mt_PaulaPansR[i]);

                    AUD[i].FRAC += AUD[i].INC;
                    if (AUD[i].FRAC >= 1.0f)
                    {
                        AUD[i].POS++;
                        AUD[i].FRAC--;

                        if (AUD[i].POS >= AUD[i].LEN)
                        {
                            if (AUD[i].REPLEN > 2)
                            {
                                AUD[i].DAT = AUD[i].REPDAT;
                                AUD[i].POS -= AUD[i].LEN;
                                AUD[i].LEN = AUD[i].REPLEN;
                            }
                            else
                            {
                                AUD[i].POS = 0;
                                AUD[i].TRIGGER = 0;
                                break;
                            }
                        }
                    }
                }
            }

#ifdef USE_BLEP
            if ((j < numSamples) && (AUD[i].DAT == NULL)
                && ((mt_blep[i].samplesLeft    > 0)
                ||  (mt_blepVol[i].samplesLeft > 0)))
            {
                for (; j < numSamples; ++j)
                {
                    smp = mt_blep[i].lastValue;
                    vol = mt_blepVol[i].lastValue;

                    if (mt_blep[i].samplesLeft > 0)
                        smp += blepRun(&mt_blep[i]);

                    if (mt_blepVol[i].samplesLeft > 0)
                        vol += blepRun(&mt_blepVol[i]);

                    smp *= vol;

                    AUDBUFL[j] += (smp * mt_PaulaPansL[i]);
                    AUDBUFR[j] += (smp * mt_PaulaPansR[i]);
                }
            }
#endif
        }

        sndOut = streamOut;
        for (j = 0; j < numSamples; ++j)
        {
            L = (float)AUDBUFL[j];
            R = (float)AUDBUFR[j];

#ifdef USE_FILTERS
#if FILTER_TYPE == 0
            // Low pass
            L = mt_filter_LP_L += ((mt_filterCoeff_LP
                * (L - mt_filter_LP_L))
                + FLT_DENORM_OFS);

            R = mt_filter_LP_R += ((mt_filterCoeff_LP
                * (R - mt_filter_LP_R))
                + FLT_DENORM_OFS);
#endif

            // LED filter
            if (mt_LEDFilterState == 1)
            {
                mt_filter_LED_L1 += (mt_filterCoeff_LED1
                    * (L - mt_filter_LED_L1)
                    + mt_filterCoeff_LED2
                    * (mt_filter_LED_L1 - mt_filter_LED_L2)
                    + FLT_DENORM_OFS);
                mt_filter_LED_L2 += (mt_filterCoeff_LED1
                    * (mt_filter_LED_L1 - mt_filter_LED_L2)
                    + FLT_DENORM_OFS);

                mt_filter_LED_R1 += (mt_filterCoeff_LED1
                    * (R - mt_filter_LED_R1)
                    + mt_filterCoeff_LED2
                    * (mt_filter_LED_R1 - mt_filter_LED_R2)
                    + FLT_DENORM_OFS);
                mt_filter_LED_R2 += (mt_filterCoeff_LED1
                    * (mt_filter_LED_R1 - mt_filter_LED_R2)
                    + FLT_DENORM_OFS);

                L = mt_filter_LED_L2;
                R = mt_filter_LED_R2;
            }

            // High pass
            L -= mt_filter_HP_L;
            R -= mt_filter_HP_R;

            mt_filter_HP_L += (mt_filterCoeff_HP * L
                + FLT_DENORM_OFS);

            mt_filter_HP_R += (mt_filterCoeff_HP * R
                + FLT_DENORM_OFS);
#endif

            // Normalize
#ifdef USE_HARDPAN
            L *= (1.0f / (128.0f + 32.0f));
            R *= (1.0f / (128.0f + 32.0f));
#else
            L *= (1.0f / 128.0f);
            R *= (1.0f / 128.0f);
#endif

            // Clamp
            if      (L < -32768.0f) L = -32768.0f;
            else if (L >  32767.0f) L =  32767.0f;
            if      (R < -32768.0f) R = -32768.0f;
            else if (R >  32767.0f) R =  32767.0f;

            *sndOut++ = (short)L;
            *sndOut++ = (short)R;
        }
    }
}

static void mt_UpdateFunk(PT_CHN *ch)
{
    if (ch->n_glissfunk & 0xF0)
    {
        ch->n_funkoffset += mt_FunkTable[ch->n_glissfunk >> 4];
        if (ch->n_funkoffset & 128)
        {
            ch->n_funkoffset = 0;

            if (ch->n_wavestart)
            {
                ch->n_wavestart++;
                if (ch->n_wavestart >= (ch->n_loopstart + ch->n_replen))
                    ch->n_wavestart = ch->n_loopstart;

                *ch->n_wavestart = -1 - *ch->n_wavestart;
            }
        }
    }
}

static void mt_FilterOnOff(PT_CHN *ch)
{
#ifdef USE_FILTERS
    mt_LEDFilterState = (ch->n_cmdlo & 1) ? 0 : 1;
#endif
}

static void mt_SetGlissControl(PT_CHN *ch)
{
    ch->n_glissfunk = (ch->n_glissfunk & 0xF0) | (ch->n_cmdlo & 0x0F);
}

static void mt_SetVibratoControl(PT_CHN *ch)
{
    ch->n_wavecontrol = (ch->n_wavecontrol & 0xF0) | (ch->n_cmdlo & 0x0F);
}

static void mt_SetFineTune(PT_CHN *ch)
{
    ch->n_finetune = 36 * (ch->n_cmdlo & 0x0F);
}

static void mt_JumpLoop(PT_CHN *ch)
{
    if (!mt_Counter)
    {
        if (ch->n_cmd == 0x0E60) // empty param
        {
            ch->n_pattpos = mt_PatternPos >> 4;
        }
        else
        {
            if (!ch->n_loopcount)
            {
                ch->n_loopcount = ch->n_cmdlo & 0x0F;
            }
            else
            {
                ch->n_loopcount--;
                if (!ch->n_loopcount) return;
            }

            mt_PBreakPos = ch->n_pattpos;
            mt_PBreakFlag = 1;
        }
    }
}

static void mt_SetTremoloControl(PT_CHN *ch)
{
    ch->n_wavecontrol = (ch->n_cmdlo << 4) | (ch->n_wavecontrol & 0x0F);
}

static void mt_RetrigNote(PT_CHN *ch)
{
    if (ch->n_cmdlo != 0x90) // no empty param
    {
        if (!mt_Counter)
        {
            if (ch->n_note) return;
        }

        if (!(mt_Counter % (ch->n_cmdlo & 0x0F)))
        {
            mt_PaulaSetDat(ch->n_index, ch->n_start);
            mt_PaulaSetLen(ch->n_index, ch->n_length);
            mt_PaulaSetLoop(ch->n_index, ch->n_loopstart, ch->n_replen);
            mt_PaulaStart(ch->n_index);
        }
    }
}

static void mt_VolumeSlide(PT_CHN *ch)
{
    if (ch->n_cmdlo & 0xF0)
    {
        ch->n_volume += (ch->n_cmdlo >> 4);
        if (ch->n_volume > 64) ch->n_volume = 64;  
    }
    else
    {
        ch->n_volume -= (ch->n_cmdlo & 0x0F);
        if (ch->n_volume < 0) ch->n_volume = 0;
    }

    mt_PaulaSetVol(ch->n_index, ch->n_volume);
}

static void mt_VolumeFineUp(PT_CHN *ch)
{
    if (!mt_Counter)
    {
        ch->n_volume += (ch->n_cmdlo & 0x0F);
        if (ch->n_volume > 64) ch->n_volume = 64;

        mt_PaulaSetVol(ch->n_index, ch->n_volume);
    }
}

static void mt_VolumeFineDown(PT_CHN *ch)
{
    if (!mt_Counter)
    {
        ch->n_volume -= (ch->n_cmdlo & 0x0F);
        if (ch->n_volume < 0) ch->n_volume = 0;

        mt_PaulaSetVol(ch->n_index, ch->n_volume);
    }
}

static void mt_NoteCut(PT_CHN *ch)
{
    if (mt_Counter == (ch->n_cmdlo & 0x0F))
    {
        ch->n_volume = 0;
        mt_PaulaSetVol(ch->n_index, 0);
    }
}

static void mt_NoteDelay(PT_CHN *ch)
{
    if (mt_Counter == (ch->n_cmdlo & 0x0F))
    {
        if (ch->n_note)
        {
            mt_PaulaSetDat(ch->n_index, ch->n_start);
            mt_PaulaSetLen(ch->n_index, ch->n_length);
            mt_PaulaSetLoop(ch->n_index, ch->n_loopstart, ch->n_replen);
            mt_PaulaStart(ch->n_index);
        }
    }
}

static void mt_PatternDelay(PT_CHN *ch)
{
    if (!mt_Counter)
    {
        if (!mt_PattDelTime2)
            mt_PattDelTime = (ch->n_cmdlo & 0x0F) + 1;
    }
}

static void mt_FunkIt(PT_CHN *ch)
{
    if (!mt_Counter)
    {
        ch->n_glissfunk = (ch->n_cmdlo << 4) | (ch->n_glissfunk & 0x0F);
        mt_UpdateFunk(ch);
    }
}

static void mt_PositionJump(PT_CHN *ch)
{
    mt_SongPos = ch->n_cmdlo - 1; // 0xFF (B00) jumps to pat 0
    mt_PBreakPos = 0;
    mt_PosJumpFlag = 1;
}

static void mt_VolumeChange(PT_CHN *ch)
{
    ch->n_volume = ch->n_cmdlo;
    if (ch->n_volume > 64) ch->n_volume = 64;

    mt_PaulaSetVol(ch->n_index, ch->n_volume);
}

static void mt_PatternBreak(PT_CHN *ch)
{
    mt_PBreakPos = ((ch->n_cmdlo >> 4) * 10) + (ch->n_cmdlo & 0x0F);
    if (mt_PBreakPos > 63) mt_PBreakPos = 0;

    mt_PosJumpFlag = 1;
}

static void mt_SetSpeed(PT_CHN *ch)
{
    if (ch->n_cmdlo)
    {
        mt_Counter = 0;

        if (mt_TempoMode || (ch->n_cmdlo < 32))
            mt_Speed = ch->n_cmdlo;          
        else
            mt_SamplesPerFrame = mt_TimerVal / ch->n_cmdlo;
    }
}

static void mt_Arpeggio(PT_CHN *ch)
{
    char i;
    unsigned char dat;
    const short *arpPointer;
    
    dat = mt_Counter % 3;
    if (!dat)
    {
        mt_PaulaSetPer(ch->n_index, ch->n_period);
    }
    else
    {
        if (dat == 1)
            dat = ch->n_cmdlo >> 4;
        else if (dat == 2)
            dat = ch->n_cmdlo & 0x0F;

        arpPointer = &mt_PeriodTable[ch->n_finetune];
        for (i = 0; i < 36; ++i)
        {
            if (ch->n_period >= arpPointer[i])
            {
                mt_PaulaSetPer(ch->n_index, arpPointer[i + dat]);
                break;
            }
        }
    }
}

static void mt_PortaUp(PT_CHN *ch)
{
    ch->n_period -= (ch->n_cmdlo & mt_LowMask);
    if (ch->n_period < 113) ch->n_period = 113;

    mt_PaulaSetPer(ch->n_index, ch->n_period);
    mt_LowMask = 0xFF;
}

static void mt_PortaDown(PT_CHN *ch)
{
    ch->n_period += (ch->n_cmdlo & mt_LowMask);
    if (ch->n_period > 856) ch->n_period = 856;

    mt_PaulaSetPer(ch->n_index, ch->n_period);
    mt_LowMask = 0xFF;
}

static void mt_FinePortaUp(PT_CHN *ch)
{
    if (!mt_Counter)
    {
        mt_LowMask = 0x0F;
        mt_PortaUp(ch);
    }
}

static void mt_FinePortaDown(PT_CHN *ch)
{
    if (!mt_Counter)
    {
        mt_LowMask = 0x0F;
        mt_PortaDown(ch);
    }
}

static void mt_SetTonePorta(PT_CHN *ch)
{
    char i;
    const short *portaPointer;
    
    portaPointer = &mt_PeriodTable[ch->n_finetune];
    for (i = 0; i < 36; ++i)
    {
        if (ch->n_note >= portaPointer[i])
            break;
    }

    if ((ch->n_finetune >= (36 * 8)) && i) i--;

    ch->n_wantedperiod = portaPointer[i];
    ch->n_toneportdirec = 0;

    if (ch->n_period == ch->n_wantedperiod)
        ch->n_wantedperiod = 0;
    else if (ch->n_period > ch->n_wantedperiod)
        ch->n_toneportdirec = 1;
}

static void mt_TonePortNoChange(PT_CHN *ch)
{
    char i;
    const short *portaPointer;

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
            mt_PaulaSetPer(ch->n_index, ch->n_period);
        }
        else
        {
            portaPointer = &mt_PeriodTable[ch->n_finetune];
            for (i = 0; i < 36; ++i)
            {
                if (ch->n_period >= portaPointer[i])
                {
                    mt_PaulaSetPer(ch->n_index, portaPointer[i]);
                    break;
                }
            }
        }
    }
}

static void mt_TonePortamento(PT_CHN *ch)
{
    if (ch->n_cmdlo)
    {
        ch->n_toneportspeed = ch->n_cmdlo;
        ch->n_cmdlo = 0;
    }

    mt_TonePortNoChange(ch);
}

static void mt_VibratoNoChange(PT_CHN *ch)
{
    unsigned char vibratoTemp;
    short vibratoData; 

    vibratoTemp = (ch->n_vibratopos >> 2) & 0x1F;
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
                vibratoData = 255 - (vibratoTemp << 3);
            else
                vibratoData = vibratoTemp << 3;
        }
        else
        {
            vibratoData = 255;
        }
    }

    vibratoData = (vibratoData * (ch->n_vibratocmd & 0x0F)) >> 7;

    if (ch->n_vibratopos < 0)
        vibratoData = ch->n_period - vibratoData;
    else
        vibratoData = ch->n_period + vibratoData;

    mt_PaulaSetPer(ch->n_index, vibratoData);

    ch->n_vibratopos += ((ch->n_vibratocmd >> 2) & 0x3C);
}

static void mt_Vibrato(PT_CHN *ch)
{
    if (ch->n_cmdlo)
    {
        if (ch->n_cmdlo & 0x0F)
            ch->n_vibratocmd = (ch->n_vibratocmd & 0xF0) | (ch->n_cmdlo & 0x0F);
        
        if (ch->n_cmdlo & 0xF0)
            ch->n_vibratocmd = (ch->n_cmdlo & 0xF0) | (ch->n_vibratocmd & 0x0F);

        ch->n_cmdlo = 0;
    }

    mt_VibratoNoChange(ch);
}

static void mt_TonePlusVolSlide(PT_CHN *ch)
{
    mt_TonePortNoChange(ch);
    mt_VolumeSlide(ch);
}

static void mt_VibratoPlusVolSlide(PT_CHN *ch)
{
    mt_VibratoNoChange(ch);
    mt_VolumeSlide(ch);
}

static void mt_Tremolo(PT_CHN *ch)
{
    char tremoloTemp;
    short tremoloData;

    if (ch->n_cmdlo)
    {
        if (ch->n_cmdlo & 0x0F)
            ch->n_tremolocmd = (ch->n_tremolocmd & 0xF0) | (ch->n_cmdlo & 0x0F);
        
        if (ch->n_cmdlo & 0xF0)
            ch->n_tremolocmd = (ch->n_cmdlo & 0xF0) | (ch->n_tremolocmd & 0x0F);

        ch->n_cmdlo = 0;
    }

    tremoloTemp = (ch->n_tremolopos >> 2) & 0x1F;
    tremoloData = (ch->n_wavecontrol >> 4) & 3;

    if (!tremoloData)
    {
        tremoloData = mt_VibratoTable[tremoloTemp];
    }
    else
    {
        if (tremoloData == 1)
        {
            if (ch->n_vibratopos < 0) // PT bug, but don't fix this one
                tremoloData = 255 - (tremoloTemp << 3);
            else
                tremoloData = tremoloTemp << 3;
        }
        else
        {
            tremoloData = 255;
        }
    }

    tremoloData = (tremoloData * (ch->n_tremolocmd & 0x0F)) >> 6;

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

    mt_PaulaSetVol(ch->n_index, (char)tremoloData);

    ch->n_tremolopos += ((ch->n_tremolocmd >> 2) & 0x3C);
}

static void mt_SampleOffset(PT_CHN *ch)
{
    unsigned short newOffset;

    if (ch->n_cmdlo)
    {
        ch->n_sampleoffset = ch->n_cmdlo;
        ch->n_cmdlo = 0;
    }

    newOffset = ch->n_sampleoffset << 8;
    if (newOffset < ch->n_length)
    {
        ch->n_length -= newOffset;
        ch->n_start += newOffset;
    }
    else
    {
        ch->n_length = 1;
    }  
}

static void mt_E_Commands(PT_CHN *ch)
{
    switch (ch->n_cmdlo >> 4)
    {
        case 0x00: mt_FilterOnOff(ch);       break;
        case 0x01: mt_FinePortaUp(ch);       break;
        case 0x02: mt_FinePortaDown(ch);     break;
        case 0x03: mt_SetGlissControl(ch);   break;
        case 0x04: mt_SetVibratoControl(ch); break;
        case 0x05: mt_SetFineTune(ch);       break;
        case 0x06: mt_JumpLoop(ch);          break;
        case 0x07: mt_SetTremoloControl(ch); break;
        case 0x09: mt_RetrigNote(ch);        break;
        case 0x0A: mt_VolumeFineUp(ch);      break;
        case 0x0B: mt_VolumeFineDown(ch);    break;
        case 0x0C: mt_NoteCut(ch);           break;
        case 0x0D: mt_NoteDelay(ch);         break;
        case 0x0E: mt_PatternDelay(ch);      break;
        case 0x0F: mt_FunkIt(ch);            break;     
    }
}

static void mt_CheckMoreEfx(PT_CHN *ch)
{
    mt_UpdateFunk(ch);

    switch (ch->n_efx)
    {
        case 0x09: mt_SampleOffset(ch); break;
        case 0x0B: mt_PositionJump(ch); break;
        case 0x0C: mt_VolumeChange(ch); break;
        case 0x0D: mt_PatternBreak(ch); break;
        case 0x0E: mt_E_Commands(ch);   break;
        case 0x0F: mt_SetSpeed(ch);     break;

        default: mt_PaulaSetPer(ch->n_index, ch->n_period); break;
    }
}

static void mt_CheckEfx(PT_CHN *ch)
{
    mt_UpdateFunk(ch);

    if (ch->n_cmd)
    {
        switch (ch->n_efx)
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
                mt_PaulaSetPer(ch->n_index, ch->n_period);
                mt_Tremolo(ch);
            break;
            case 0x0A:
                mt_PaulaSetPer(ch->n_index, ch->n_period);
                mt_VolumeSlide(ch);
            break;

            default: mt_PaulaSetPer(ch->n_index, ch->n_period); break;
        }
    }
    else
    {
        mt_PaulaSetPer(ch->n_index, ch->n_period);
    }
}

static void mt_SetPeriod(PT_CHN *ch)
{
    char i;
 
    for (i = 0; i < 36; ++i)
    {
        if (ch->n_note >= mt_PeriodTable[i]) break;
    }

    ch->n_period = mt_PeriodTable[ch->n_finetune + i];

    if ((ch->n_cmd & 0x0FF0) != 0x0ED0) // no note delay
    {
        if (!(ch->n_wavecontrol & 0x04)) ch->n_vibratopos = 0;
        if (!(ch->n_wavecontrol & 0x40)) ch->n_tremolopos = 0;
   
        mt_PaulaSetDat(ch->n_index, ch->n_start);
        mt_PaulaSetLen(ch->n_index, ch->n_length);
        mt_PaulaSetPer(ch->n_index, ch->n_period);
        mt_PaulaStart(ch->n_index);
    }

    mt_CheckMoreEfx(ch);
}

static void mt_PlayVoice(PT_CHN *ch)
{
    unsigned char sample;
    unsigned char pattData[4];
    short sampleOffset;
    unsigned int repeat;
    
    if (!*((unsigned int *)ch)) // no channel data on this row
        mt_PaulaSetPer(ch->n_index, ch->n_period);

    *((unsigned int *)pattData) = *((unsigned int *)&mt_SongDataPtr[mt_PattPosOff]);

    mt_PattPosOff += 4;

    ch->n_note = ((pattData[0] & 0x0F) << 8) | pattData[1];
    ch->n_cmd = ((pattData[2] & 0x0F) << 8) | pattData[3];
    ch->n_cmdlo = pattData[3];
    ch->n_efx = ch->n_cmd >> 8;

    sample = (pattData[0] & 0xF0) | (pattData[2] >> 4);
    if (sample && (sample <= 32))
    {
        sample--;
        sampleOffset = 42 + (30 * sample);

        ch->n_start = mt_SampleStarts[sample];
        ch->n_finetune = 36 * mt_SongDataPtr[sampleOffset + 2];
        ch->n_volume = mt_SongDataPtr[sampleOffset + 3];

        ch->n_length = *((unsigned short *)&mt_SongDataPtr[sampleOffset]);
        ch->n_replen = *((unsigned short *)&mt_SongDataPtr[sampleOffset + 6]);

        repeat = *((unsigned short *)&mt_SongDataPtr[sampleOffset + 4]);
        if (ch->n_replen > 2)
        {
            ch->n_wavestart = ch->n_loopstart = ch->n_start + repeat;
            ch->n_length = repeat + ch->n_replen;
        }
        else
        {
            ch->n_wavestart = ch->n_loopstart = ch->n_start;
        }
    }

    if (ch->n_note)
    {
        if ((ch->n_cmd & 0x0FF0) == 0x0E50) // Set Finetune
        {
            mt_SetFineTune(ch);
            mt_SetPeriod(ch);
        }
        else
        {
            if ((ch->n_efx == 0x03) || (ch->n_efx == 0x05))
            {
                mt_SetTonePorta(ch);
                mt_CheckMoreEfx(ch);
            }
            else if (ch->n_efx == 0x09)
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
}

static void mt_NextPosition(void)
{
    mt_PatternPos = mt_PBreakPos << 4;
    mt_PBreakPos = 0;
    mt_PosJumpFlag = 0;

    mt_SongPos = (mt_SongPos + 1) & 0x7F;
    if (mt_SongPos >= mt_SongDataPtr[950]) mt_SongPos = 0;

    mt_PattOff = 1084 + (mt_SongDataPtr[952 + mt_SongPos] << 10);
}

static void mt_MusicIRQ(void)
{
    if (mt_Enable)
    {
        mt_Counter++;
        if (mt_Counter >= mt_Speed)
        {
            mt_Counter = 0;

            if (!mt_PattDelTime2)
            {
                mt_PattPosOff = mt_PattOff + mt_PatternPos;

                mt_PlayVoice(&mt_Chan1temp);        
                mt_PaulaSetVol(0, mt_Chan1temp.n_volume);
                
                mt_PlayVoice(&mt_Chan2temp);
                mt_PaulaSetVol(1, mt_Chan2temp.n_volume);

                mt_PlayVoice(&mt_Chan3temp);
                mt_PaulaSetVol(2, mt_Chan3temp.n_volume);

                mt_PlayVoice(&mt_Chan4temp);
                mt_PaulaSetVol(3, mt_Chan4temp.n_volume);

                mt_PaulaSetLoop(0, mt_Chan1temp.n_loopstart, mt_Chan1temp.n_replen);
                mt_PaulaSetLoop(1, mt_Chan2temp.n_loopstart, mt_Chan2temp.n_replen);
                mt_PaulaSetLoop(2, mt_Chan3temp.n_loopstart, mt_Chan3temp.n_replen);
                mt_PaulaSetLoop(3, mt_Chan4temp.n_loopstart, mt_Chan4temp.n_replen);
            }
            else
            {
                mt_CheckEfx(&mt_Chan1temp);
                mt_CheckEfx(&mt_Chan2temp);
                mt_CheckEfx(&mt_Chan3temp);
                mt_CheckEfx(&mt_Chan4temp);
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
                mt_PatternPos = mt_PBreakPos << 4;
                mt_PBreakPos = 0;
                mt_PBreakFlag = 0;
            }

            if ((mt_PatternPos >= 1024) || mt_PosJumpFlag)
                mt_NextPosition();
        }
        else
        {
            mt_CheckEfx(&mt_Chan1temp);
            mt_CheckEfx(&mt_Chan2temp);
            mt_CheckEfx(&mt_Chan3temp);
            mt_CheckEfx(&mt_Chan4temp);

            if (mt_PosJumpFlag) mt_NextPosition();
        }
    }
}

static void mt_Init(unsigned char *mt_Data)
{
    unsigned char *sampleStarts;   
    char pattNum;
    char iffHdrFound;   
    unsigned char i;
    unsigned short *p;
    unsigned short j; 
    unsigned short tmpLen;
    unsigned short lastPeriod;
    
    mt_SongDataPtr = mt_Data;

    pattNum = 0;
    for (i = 0; i < 128; ++i)
    {
        if (mt_SongDataPtr[952 + i] > pattNum)
            pattNum = mt_SongDataPtr[952 + i];
    }
    pattNum++;

    sampleStarts = &mt_SongDataPtr[1084 + (pattNum << 10)];
    for (i = 0; i < 31; ++i)
    {
        mt_SampleStarts[i] = (char *)sampleStarts;

        // AmigaWord->Word, clamp unsigned 15-bit and do N*2
        p = ((unsigned short *)&mt_SongDataPtr[42 + (30 * i)]);    
        if (*p  ) *p = mt_AmigaWord(*p); if (*p > 32767) *p = 32767; *p++ <<= 1;
        if (*++p) *p = mt_AmigaWord(*p); if (*p > 32767) *p = 32767; *p   <<= 1;
        if (*++p) *p = mt_AmigaWord(*p); if (*p > 32767) *p = 32767; *p   <<= 1;

        // IFF header removal (if found)
        tmpLen = *(p -= 3);
        if (tmpLen > 8) 
        {
            iffHdrFound = 0;

            for (j = 0; j < (tmpLen - 8); ++j)
            {
                if (!memcmp(mt_SampleStarts[i] + j, "8SVXVHDR", 8))
                    iffHdrFound = 1;      

                if (iffHdrFound && !memcmp(mt_SampleStarts[i] + j, "BODY", 4))
                {
                    *p = tmpLen - (j + 8);
                    mt_SampleStarts[i] += (j + 8);
                    break;
                }
            }
        }

        sampleStarts += tmpLen;
    }

    // +14 for 14 extra zeroes to prevent access violation on -1
    // (15 unsigned) finetuned samples with B-3 >+1 note arpeggios.
    //PT was never bug free. :-)
    mt_PeriodTable = (short *)calloc((36 * 16) + 14, sizeof (short));
    for (i = 0; i < 16; ++i)
    {
        lastPeriod = 856;
        for (j = 0; j < 36; ++j)
            lastPeriod = mt_PeriodTable[(36 * i) + j] = lastPeriod
                + mt_PeriodDiffs[(36 * i) + j];
    }

    mt_Speed = 6;
    mt_Counter = 0;
    mt_SongPos = 0;
    mt_PatternPos = 0;
    mt_Enable = 0;
    mt_PattDelTime = 0;
    mt_PattDelTime2 = 0;
    mt_PBreakPos = 0;
    mt_PosJumpFlag = 0;
    mt_PBreakFlag = 0;
    mt_LowMask = 0xFF;
    mt_PattOff = 1084 + (mt_SongDataPtr[952] << 10);
}

static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg,
    DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    WAVEHDR *currWh;
    short *out;
    short samples;
    short tempSamples;
    
    if (uMsg == MM_WOM_DONE)
    {
        currWh = (WAVEHDR *)dwParam1;
        waveOutUnprepareHeader(hWaveOut, currWh, sizeof (WAVEHDR));

        if (mt_Enable)
        {
            memcpy(currWh->lpData, mixerBuffer, SOUND_BUF_LEN);
            waveOutPrepareHeader(hWaveOut, currWh, sizeof (WAVEHDR));
            waveOutWrite(hWaveOut, currWh, sizeof (WAVEHDR));

            out = (short *)mixerBuffer;
            samples = SOUND_BUF_LEN / 4;

            while (samples)
            {
                tempSamples = samples;

                if (mt_SampleCounter == 0)
                {
                    mt_MusicIRQ();
                    mt_SampleCounter += mt_SamplesPerFrame;
                }

                mt_SampleCounter -= samples;
                if (mt_SampleCounter < 0)
                {
                    tempSamples += mt_SampleCounter;
                    mt_SampleCounter = 0;
                }

                mt_paulaMix(tempSamples, out);

                out += (tempSamples * 2);
                samples -= tempSamples;
            }
        }
    }
}

int ptBuddyPlay(unsigned char *modData, char timerType, int soundFrequency)
{
    char i;
    MMRESULT r;

    mt_Enable = 0;

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = soundFrequency;
    wfx.wBitsPerSample = 16;   
    wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
    wfx.cbSize = 0;

    r = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc,
        (DWORD_PTR)NULL, CALLBACK_FUNCTION);
    if (r != MMSYSERR_NOERROR) return (0);

    waveBlocks = (WAVEHDR *)calloc(SOUND_BUF_NUM, sizeof (WAVEHDR));
    for (i = 0; i < SOUND_BUF_NUM; ++i)
    { 
        waveBlocks[i].dwBufferLength = SOUND_BUF_LEN;
        waveBlocks[i].lpData = (LPSTR)calloc(SOUND_BUF_LEN, sizeof (char));
        waveOutPrepareHeader(hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
        waveOutWrite(hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
    }

    mixerBuffer = (char *)calloc(SOUND_BUF_LEN, sizeof (char));

    AUDBUFL = (float *)calloc(SOUND_BUF_LEN, sizeof (float));
    AUDBUFR = (float *)calloc(SOUND_BUF_LEN, sizeof (float));

    mt_Init(modData);

    mt_MixFreq = soundFrequency;
    mt_TempoMode = timerType ? 1 : 0; // 0 = cia, 1 = vblank
    mt_TimerVal = (soundFrequency * 125) / 50;
    mt_SamplesPerFrame = mt_TimerVal / 125;

    mt_SampleCounter = 0;
    mt_Enable = 1;

#ifdef USE_FILTERS
#if FILTER_TYPE == 0
    mt_filterCoeff_LP = FLT_RC_COEFF_LP / (float)soundFrequency;
    mt_filter_LP_L = 0.0f;
    mt_filter_LP_R = 0.0f;
#endif

    mt_filterCoeff_LED1 = FLT_RC_COEFF_LED / (float)soundFrequency;
    mt_filterCoeff_LED2 = 0.125f + 0.125f / (1.0f - mt_filterCoeff_LED1);
    mt_filter_LED_L1 = 0.0f;
    mt_filter_LED_R1 = 0.0f;
    mt_filter_LED_L2 = 0.0f;
    mt_filter_LED_R2 = 0.0f;

    mt_filterCoeff_HP = FLT_RC_COEFF_HP / (float)soundFrequency;
    mt_filter_HP_L = 0.0f;
    mt_filter_HP_R = 0.0f;

    mt_LEDFilterState = 0;
#endif

#ifdef USE_BLEP
    memset(mt_blep, 0, sizeof (mt_blep));
    memset(mt_blepVol, 0, sizeof (mt_blepVol));
#endif

    return (1);
}

void ptBuddyClose(void)
{
    char i;

    mt_Enable = 0;

    for (i = 0; i < SOUND_BUF_NUM; ++i)
    {
        while (!(waveBlocks[i].dwFlags & WHDR_DONE)) {}
        waveOutUnprepareHeader(hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
        free(waveBlocks[i].lpData);
    }

    waveOutClose(hWaveOut);

    free(waveBlocks);
    free(mixerBuffer);
    free(AUDBUFL);
    free(AUDBUFR);   
    free(mt_PeriodTable);
}

#ifdef USE_BLEP
void blepAdd(BLEP *b, float offset, float amplitude)
{
    char n;
    int i;
    const float *src;
    float f;

    if (offset > 0.9999999f) // weird bugfix, TODO: Inspect
        return;

    n = NS;
    i = (int)(offset * (float)SP);
    src = ((const float *)mt_blepData) + i + SP;
    f = offset * (float)(SP - i);
    i = b->index;

    while (n--)
    {
        b->buffer[i] += (amplitude * LERP(src, f));
        src += SP;
        i++;
        i &= RNS;
    }

    b->samplesLeft = NS;
}

float blepRun(BLEP *b)
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

/* CODE END */


/* END OF FILE */