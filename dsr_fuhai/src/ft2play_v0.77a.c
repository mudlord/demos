/*
** FT2PLAY v0.77a - 17th of September 2016 - http://16-bits.org
** ============================================================
**
** Very accurate, high quality C port of FastTracker 2.09's replayer,
** by Olav "8bitbubsy" Sørensen. Using the original pascal+asm source
** codes by Fredrik "Mr.H" Huss (Triton). Don't ask me to give them to you.
**
** Thanks to aciddose (and kode54) for coding the vol/sample ramp.
** The volume ramp is tuned to that of FT2 (5ms).
**
** (extreme) non-FT2 extensions:
** - Max 127 channels (was 32)
** - Non-even amount-of-channels number (FT2 supports *even* numbers only)
** - Max 256 instruments (was 128)
** - Max 32 samples per instrument (was 16)
** - Max 1024 rows per pattern (was 256)
** - Stereo samples
**
** These additions shouldn't break FT2 accuracy unless I'm wrong.
**
** You need to link winmm.lib for this to compile (-lwinmm)
** Alternatively, you can wrap around your own audio callback for using
** ft2play on other platforms.
**
** Changelog from v0.77:
** - Code cleanup and possible vol ramp bug fix
**
** Changelog from v0.76:
** - Bug fix in Lxx (Set Envelope Pos)
**
** Changelog from v0.75:
** - Bug fixes (more like real FT2)
** - Higher placebo quality (and more safe) mixer
**
** Changelog from v0.72:
** - Channel mixer changes
** - Arpeggio overflow bug fix
** - Pattern delay (EDx) sample bug fix
** - RelocateTon() finetune bug fix
** - Envelopes bug fix
** - Auto vibrato bug fix
**
** Changelog from v0.71:
** - Code cleanup
**
** Changelog from v0.70:
** - Code cleanup
** - Proper check for memory allocations
**
** Changelog from v0.68:
** - Volume ramp didn't work correctly with LERP and some non-looping samples
** - One for-loop value was wrong (254 instead of 256)
** - Some mixer optimizations
** - Some visual code change/cleanup
**
** Changelog from v0.67:
** - Bug in GetNewNote() (cmd 390 would fail)
**
** Changelog from v0.66:
** - Auto-vibrato was wrong on every type except for sine
**
** Changelog from v0.65:
** - RelocateTon() was less accurate, changed back to the older one
**    and made it a little bit safer.
** - Forgot to zero out the internal Stm channels in StopVoices().
**
** Changelog from v0.64:
** - Fixed a critical bug in the finetune calculation.
**
**
** User functions:
**
** #include <stdint.h>
**
** int8_t ft2play_LoadModule(const uint8_t *moduleData, uint32_t dataLength);
** int8_t ft2play_Init(uint32_t outputFreq, int8_t lerpMixFlag, int8_t volRampFlag);
** void ft2play_FreeSong(void);
** void ft2play_Close(void);
** void ft2play_PauseSong(int8_t pause);
** void ft2play_PlaySong(void);
*/

#define _USE_MATH_DEFINES

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include <mmsystem.h>

#define ABS(a) (((a) < 0) ? -(a) : (a))
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define LERP(x, y, z) ((x) + ((y) - (x)) * (z))

/* don't change these two unless you know what you're doing... */
#define MIX_BUF_NUM 7
#define MIX_BUF_LEN 2048

enum
{
    IS_Vol    = 1,
    IS_Period = 2,
    IS_NyTon  = 4,
    IS_Pan    = 8
};


/* *** STRUCTS *** (remember 1-byte alignment for header/loader structs) */
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif
typedef struct SongHeaderTyp_t
{
    char Sig[17];
    char Name[21];
    char ProggName[20];
    uint16_t Ver;
    int32_t HeaderSize;
    uint16_t Len;
    uint16_t RepS;
    uint16_t AntChn;
    uint16_t AntPtn;
    uint16_t AntInstrs;
    uint16_t Flags;
    uint16_t DefTempo;
    uint16_t DefSpeed;
    uint8_t SongTab[256];
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
SongHeaderTyp;

typedef struct SampleHeaderTyp_t
{
    int32_t Len;
    int32_t RepS;
    int32_t RepL;
    uint8_t vol;
    int8_t Fine;
    uint8_t Typ;
    uint8_t Pan;
    int8_t RelTon;
    uint8_t skrap;
    char Name[22];
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
SampleHeaderTyp;

typedef struct InstrHeaderTyp_t
{
    int32_t InstrSize;
    char Name[22];
    uint8_t Typ;
    uint16_t AntSamp;
    int32_t SampleSize;
    uint8_t TA[96];
    int16_t EnvVP[12][2];
    int16_t EnvPP[12][2];
    uint8_t EnvVPAnt;
    uint8_t EnvPPAnt;
    uint8_t EnvVSust;
    uint8_t EnvVRepS;
    uint8_t EnvVRepE;
    uint8_t EnvPSust;
    uint8_t EnvPRepS;
    uint8_t EnvPRepE;
    uint8_t EnvVTyp;
    uint8_t EnvPTyp;
    uint8_t VibTyp;
    uint8_t VibSweep;
    uint8_t VibDepth;
    uint8_t VibRate;
    uint16_t FadeOut;
    uint8_t MIDIOn;
    uint8_t MIDIChannel;
    int16_t MIDIProgram;
    int16_t MIDIBend;
    int8_t Mute;
    uint8_t Reserved[15];
    SampleHeaderTyp Samp[32];
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
InstrHeaderTyp;

typedef struct PatternHeaderTyp_t
{
    int32_t PatternHeaderSize;
    uint8_t Typ;
    uint16_t PattLen;
    uint16_t DataLen;
}
#ifdef __GNUC__
__attribute__ ((packed))
#endif
PatternHeaderTyp;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef struct SongTyp_t
{
    uint16_t Len;
    uint16_t RepS;
    uint8_t AntChn;
    uint16_t AntPtn;
    uint16_t AntInstrs;
    int16_t SongPos;
    int16_t PattNr;
    int16_t PattPos;
    int16_t PattLen;
    uint16_t Speed;
    uint16_t Tempo;
    uint16_t InitSpeed;
    uint16_t InitTempo;
    int16_t GlobVol;
    uint16_t Timer;
    uint8_t PattDelTime;
    uint8_t PattDelTime2;
    uint8_t PBreakFlag;
    uint8_t PBreakPos;
    uint8_t PosJumpFlag;
    uint8_t SongTab[256];
    uint16_t Ver;
} SongTyp;

typedef struct SampleTyp_t
{
    int32_t Len;
    int32_t RepS;
    int32_t RepL;
    uint8_t Vol;
    int8_t Fine;
    uint8_t Typ;
    uint8_t Pan;
    int8_t RelTon;
    uint8_t skrap;
    char Name[22];
    int8_t *Pek;
} SampleTyp;

typedef struct InstrTyp_t
{
    uint32_t SampleSize;
    uint8_t TA[96];
    int16_t EnvVP[12][2];
    int16_t EnvPP[12][2];
    uint8_t EnvVPAnt;
    uint8_t EnvPPAnt;
    uint8_t EnvVSust;
    uint8_t EnvVRepS;
    uint8_t EnvVRepE;
    uint8_t EnvPSust;
    uint8_t EnvPRepS;
    uint8_t EnvPRepE;
    uint8_t EnvVTyp;
    uint8_t EnvPTyp;
    uint8_t VibTyp;
    uint8_t VibSweep;
    uint8_t VibDepth;
    uint8_t VibRate;
    uint16_t FadeOut;
    uint8_t MIDIOn;
    uint8_t MIDIChannel;
    uint16_t MIDIProgram;
    uint16_t MIDIBend;
    uint8_t Mute;
    uint8_t Reserved[15];
    uint16_t AntSamp;
    SampleTyp Samp[32];
} InstrTyp;

typedef struct StmTyp_t
{
    SampleTyp InstrOfs;
    InstrTyp InstrSeg;
    float FinalVol;
    int8_t OutVol;
    int8_t RealVol;
    int8_t RelTonNr;
    int8_t FineTune;
    int16_t RealPeriod;
    int32_t FadeOutAmp;
    int16_t EnvVIPValue;
    int16_t EnvPIPValue;
    uint8_t OldVol;
    uint8_t OldPan;
    int16_t OutPan;
    uint16_t OutPeriod;
    float FinalPan;
    uint16_t FinalPeriod;
    uint8_t EnvSustainActive;
    uint16_t SmpStartPos;
    uint16_t InstrNr;
    uint16_t ToneType;
    uint8_t EffTyp;
    uint8_t Eff;
    uint8_t SmpOffset;
    uint16_t WantPeriod;
    uint8_t WaveCtrl;
    uint8_t Status;
    uint8_t PortaDir;
    uint8_t GlissFunk;
    uint16_t PortaSpeed;
    uint8_t VibPos;
    uint8_t TremPos;
    uint8_t VibSpeed;
    uint8_t VibDepth;
    uint8_t TremSpeed;
    uint8_t TremDepth;
    uint8_t PattPos;
    uint8_t LoopCnt;
    uint8_t VolSlideSpeed;
    uint8_t FVolSlideUpSpeed;
    uint8_t FVolSlideDownSpeed;
    uint8_t FPortaUpSpeed;
    uint8_t FPortaDownSpeed;
    uint8_t EPortaUpSpeed;
    uint8_t EPortaDownSpeed;
    uint8_t PortaUpSpeed;
    uint8_t PortaDownSpeed;
    uint8_t RetrigSpeed;
    uint8_t RetrigCnt;
    uint8_t RetrigVol;
    uint8_t VolKolVol;
    uint8_t TonNr;
    uint16_t FadeOutSpeed;
    uint16_t EnvVCnt;
    uint8_t EnvVPos;
    uint16_t EnvVAmp;
    uint16_t EnvPCnt;
    uint8_t EnvPPos;
    uint16_t EnvPAmp;
    uint8_t EVibPos;
    uint16_t EVibAmp;
    uint16_t EVibSweep;
    uint8_t TremorSave;
    uint8_t TremorPos;
    uint8_t GlobVolSlideSpeed;
    uint8_t PanningSlideSpeed;
    uint8_t Mute;
    uint8_t Nr;
	uint8_t noteTriggered;
} StmTyp;

typedef struct TonTyp_t
{
    uint8_t Ton;
    uint8_t Instr;
    uint8_t Vol;
    uint8_t EffTyp;
    uint8_t Eff;
} TonTyp;

typedef struct
{
    const int8_t *sampleData;
    int8_t loopEnabled;
    int8_t sample16bit;
    int8_t stereo;
    int8_t loopBidi;
    int8_t loopingBackwards;
    int32_t sampleLength;
    int32_t sampleLoopBegin;
    int32_t sampleLoopEnd;
    int32_t samplePosition;
    int32_t sampleLoopLength;

    float samplePositionDelta;
    float samplePositionFrac;
    float volume; // temporary - used in voiceSetPanning()
    float volumeL;
    float volumeR;
    float targetVolR;
    float targetVolL;
    float panL;
    float panR;
    float volDeltaL;
    float volDeltaR;
    float fader;
    float faderDelta;
    float faderDest;
} VOICE;

typedef struct
{
    uint8_t *_ptr;
    uint32_t _cnt;
    uint8_t *_base;
    uint32_t _bufsiz;
    int32_t _eof;
} MEM;

#define InstrHeaderSize (sizeof (InstrHeaderTyp) - (32 * sizeof (SampleHeaderTyp)))

enum
{
    MAX_VOICES   = 127,
    TOTAL_VOICES = 254,
    SPARE_OFFSET = 127
};

/* FUNCTION DECLARATIONS */
static MEM *mopen(const uint8_t *src, uint32_t length);
static void mclose(MEM **buf);
static size_t mread(void *buffer, size_t size, size_t count, MEM *buf);
static uint32_t mtell(MEM *buf);
static int32_t meof(MEM *buf);
static void mseek(MEM *buf, int32_t offset, int32_t whence);
void startMixing(void);
void setSamplesPerFrame(uint32_t val);
void voiceSetSource(uint8_t i, const int8_t *sampleData,
    int32_t sampleLength,  int32_t sampleLoopLength,
    int32_t sampleLoopEnd, int8_t loopFlag,
    int8_t sixteenbit, int8_t stereo, int32_t position);
void voiceSetPanning(uint8_t i, float pan);
void voiceSetVolume(uint8_t i, float vol);
void voiceSetSamplingFrequency(uint8_t i, uint32_t samplingFrequency);
void ft2play_FreeSong(void);


/* TABLES AND VARIABLES */
static uint16_t AmigaFinePeriod[12 * 8] =
{
    907,900,894,887,881,875,868,862,856,850,844,838,
    832,826,820,814,808,802,796,791,785,779,774,768,
    762,757,752,746,741,736,730,725,720,715,709,704,
    699,694,689,684,678,675,670,665,660,655,651,646,
    640,636,632,628,623,619,614,610,604,601,597,592,
    588,584,580,575,570,567,563,559,555,551,547,543,
    538,535,532,528,524,520,516,513,508,505,502,498,
    494,491,487,484,480,477,474,470,467,463,460,457
};

/* this table is so small that generating it is almost as big */
static uint8_t VibTab[32] =
{
    0,  24,  49, 74, 97,120,141,161,
    180,197,212,224,235,244,250,253,
    255,253,250,244,235,224,212,197,
    180,161,141,120, 97, 74, 49, 24
};

static TonTyp *Patt[256];
StmTyp Stm[MAX_VOICES];
static uint16_t PattLens[256];
static InstrTyp *Instr[255 + 1];
static VOICE voice[TOTAL_VOICES];
static WAVEHDR waveBlocks[MIX_BUF_NUM];
static HWAVEOUT _hWaveOut;
static int8_t samplingInterpolation;
static int8_t volumeRamping;
static int32_t sampleCounter;
static uint32_t SpeedVal;
static int8_t LinearFrqTab;
static uint32_t soundBufferSize;
static uint32_t audioFreq;
SongTyp Song;
static WAVEFORMATEX wfx;
static float f_audioFreq;
static volatile int8_t mixingMutex;
static volatile int8_t isMixing;
static float f_samplesPerFrame005;
static float f_samplesPerFrame010;

/* pre-NULL'd pointers */
static int8_t *VibSineTab     = NULL;
static int16_t *linearPeriods = NULL;
static int16_t *amigaPeriods  = NULL;
static int16_t *Note2Period   = NULL;
static uint32_t *LogTab       = NULL;
static TonTyp *NilPatternLine = NULL;
static float *masterBufferL   = NULL;
static float *masterBufferR   = NULL;
static int8_t *MixingBuffer   = NULL;

/* globally accessed */
int8_t ModuleLoaded = 0;
int8_t MusicPaused  = 0;
int8_t Playing      = 0;

/* CODE START */
static int8_t voiceIsActive(uint8_t i)
{
    return (voice[i].sampleData != NULL);
}

static void RetrigVolume(StmTyp *ch)
{
    ch->RealVol = ch->OldVol;
    ch->OutVol  = ch->OldVol;
    ch->OutPan  = ch->OldPan;
    ch->Status |= (IS_Vol + IS_Pan);
}

static void RetrigEnvelopeVibrato(StmTyp *ch)
{
    if (!(ch->WaveCtrl & 0x04)) ch->VibPos  = 0;
    if (!(ch->WaveCtrl & 0x40)) ch->TremPos = 0;

    ch->RetrigCnt = 0;
    ch->TremorPos = 0;

    ch->EnvSustainActive = 1;

    if (ch->InstrSeg.EnvVTyp & 1)
    {
        ch->EnvVCnt = 0xFFFF;
        ch->EnvVPos = 0;
    }

    if (ch->InstrSeg.EnvPTyp & 1)
    {
        ch->EnvPCnt = 0xFFFF;
        ch->EnvPPos = 0;
    }

    /* FT2 doesn't check if fadeout is more than 32768 */
    ch->FadeOutSpeed = ch->InstrSeg.FadeOut;
    ch->FadeOutAmp   = 32768;

    if (ch->InstrSeg.VibDepth)
    {
        ch->EVibPos = 0;

        if (ch->InstrSeg.VibSweep)
        {
            ch->EVibAmp   = 0;
            ch->EVibSweep = (ch->InstrSeg.VibDepth * 256) / ch->InstrSeg.VibSweep;
        }
        else
        {
            ch->EVibAmp   = ch->InstrSeg.VibDepth * 256;
            ch->EVibSweep = 0;
        }
    }
}

static void KeyOff(StmTyp *ch)
{
    ch->EnvSustainActive = 0;

    if (!(ch->InstrSeg.EnvPTyp & 1)) /* yes, FT2 does this (!) */
    {
        if (ch->EnvPCnt >= ch->InstrSeg.EnvPP[ch->EnvPPos][0])
            ch->EnvPCnt  = ch->InstrSeg.EnvPP[ch->EnvPPos][0] - 1;
    }

    if (ch->InstrSeg.EnvVTyp & 1)
    {
        if (ch->EnvVCnt >= ch->InstrSeg.EnvVP[ch->EnvVPos][0])
            ch->EnvVCnt  = ch->InstrSeg.EnvVP[ch->EnvVPos][0] - 1;
    }
    else
    {
        ch->RealVol = 0;
        ch->OutVol  = 0;
        ch->Status |= IS_Vol;
    }
}

static uint32_t GetFrequenceValue(uint16_t period)
{
    uint16_t index;

    if (!period) return (0);

    if (LinearFrqTab)
    {
        index = (12 * 192 * 4) - period;
        return (LogTab[index % (12 * 16 * 4)] >> ((14 - (index / (12 * 16 * 4))) & 0x1F));
    }
    else
    {
        return ((1712 * 8363) / period);
    }
}

static void StartTone(uint8_t Ton, uint8_t EffTyp, uint8_t Eff, StmTyp *ch)
{
    SampleTyp *s;

    uint16_t tmpTon;
    uint8_t samp;
    uint8_t tonLookUp;
    uint8_t tmpFTune;

    /* if we came from Rxy (retrig), we didn't check note (Ton) yet */
    if (Ton == 97)
    {
        KeyOff(ch);
        return;
    }

    if (!Ton)
    {
        Ton = ch->TonNr;
        if (!Ton) return; /* if still no note, return. */
    }
    /* ------------------------------------------------------------ */

    ch->TonNr = Ton;

    if (Instr[ch->InstrNr] != NULL)
        ch->InstrSeg = *Instr[ch->InstrNr];
    else
        ch->InstrSeg = *Instr[0]; /* placeholder for invalid samples */

    /* non-FT2 security fix */
    tonLookUp = Ton - 1;
    if (tonLookUp > 95) tonLookUp = 95;
    /* -------------------- */

    ch->Mute = ch->InstrSeg.Mute;

    samp = ch->InstrSeg.TA[tonLookUp] & 0x1F;
    s = &ch->InstrSeg.Samp[samp];

    ch->InstrOfs = *s;
    ch->RelTonNr = s->RelTon;

    Ton += ch->RelTonNr;
    if (Ton >= (12 * 10)) return;

    ch->OldVol = s->Vol;
    ch->OldPan = s->Pan;

    if ((EffTyp == 0x0E) && ((Eff & 0xF0) == 0x50))
        ch->FineTune = ((Eff & 0x0F) * 16) - 128; /* result is now -128 .. 127 */
    else
        ch->FineTune = s->Fine;

    if (Ton > 0)
    {
        /* "arithmetic shift right" on signed number simulation */
        if (ch->FineTune >= 0)
            tmpFTune = ch->FineTune / (1 << 3);
        else
            tmpFTune = 0xE0 | ((uint8_t)(ch->FineTune) >> 3); /* 0xE0 = 2^8 - 2^(8-3) */

        tmpTon = ((Ton - 1) * 16) + ((tmpFTune + 16) & 0xFF);

        if (tmpTon < ((12 * 10 * 16) + 16)) /* should never happen, but FT2 does this check */
        {
            if (Note2Period != NULL)
                ch->RealPeriod = Note2Period[tmpTon];

            ch->OutPeriod  = ch->RealPeriod;
        }
    }

    ch->Status |= (IS_Period + IS_Vol + IS_Pan + IS_NyTon);

    if (EffTyp == 9)
    {
        if (Eff)
            ch->SmpOffset = ch->Eff;

        ch->SmpStartPos = ch->SmpOffset * 256;
    }
    else
    {
        ch->SmpStartPos = 0;
    }
}

static void MultiRetrig(StmTyp *ch)
{
    int8_t vol;
    int8_t cmd;
    uint8_t cnt;

    cnt = ch->RetrigCnt + 1;
    if (cnt < ch->RetrigSpeed)
    {
        ch->RetrigCnt = cnt;
        return;
    }

    ch->RetrigCnt = 0;

    vol = ch->RealVol;
    cmd = ch->RetrigVol;

    /* 0x00 and 0x08 are not handled, ignore them */

    if (cmd == 0x01)
    {
        vol -= 1;
        if (vol < 0) vol = 0;
    }
    else if (cmd == 0x02)
    {
        vol -= 2;
        if (vol < 0) vol = 0;
    }
    else if (cmd == 0x03)
    {
        vol -= 4;
        if (vol < 0) vol = 0;
    }
    else if (cmd == 0x04)
    {
        vol -= 8;
        if (vol < 0) vol = 0;
    }
    else if (cmd == 0x05)
    {
        vol -= 16;
        if (vol < 0) vol = 0;
    }
    else if (cmd == 0x06)
    {
        vol = (vol >> 1) + (vol >> 3) + (vol >> 4);
    }
    else if (cmd == 0x07)
    {
        vol >>= 1;
    }
    else if (cmd == 0x09)
    {
        vol += 1;
        if (vol > 64) vol = 64;
    }
    else if (cmd == 0x0A)
    {
        vol += 2;
        if (vol > 64) vol = 64;
    }
    else if (cmd == 0x0B)
    {
        vol += 4;
        if (vol > 64) vol = 64;
    }
    else if (cmd == 0x0C)
    {
        vol += 8;
        if (vol > 64) vol = 64;
    }
    else if (cmd == 0x0D)
    {
        vol += 16;
        if (vol > 64) vol = 64;
    }
    else if (cmd == 0x0E)
    {
        vol = (vol >> 1) + vol;
        if (vol > 64) vol = 64;
    }
    else if (cmd == 0x0F)
    {
        vol += vol;
        if (vol > 64) vol = 64;
    }

    ch->RealVol = vol;
    ch->OutVol  = vol;

    if ((ch->VolKolVol >= 0x10) && (ch->VolKolVol <= 0x50))
    {
        ch->OutVol  = ch->VolKolVol - 0x10;
        ch->RealVol = ch->OutVol;
    }
    else if ((ch->VolKolVol >= 0xC0) && (ch->VolKolVol <= 0xCF))
    {
        ch->OutPan = (ch->VolKolVol & 0x0F) << 4;
    }

    StartTone(0, 0, 0, ch);
}

static void CheckEffects(StmTyp *ch)
{
    int8_t envUpdate;
    uint8_t tmpEff;
    uint8_t tmpEffHi;
    int16_t newEnvPos;
    int8_t envPos;
    uint16_t i;

    /* *** VOLUME COLUMN EFFECTS (TICK 0) *** */

    /* set volume */
    if ((ch->VolKolVol >= 0x10) && (ch->VolKolVol <= 0x50))
    {
        ch->OutVol  = ch->VolKolVol - 0x10;
        ch->RealVol = ch->OutVol;

        ch->Status |= IS_Vol;
    }

    /* fine volume slide down */
    else if ((ch->VolKolVol & 0xF0) == 0x80)
    {
        ch->RealVol -= (ch->VolKolVol & 0x0F);
        if (ch->RealVol < 0) ch->RealVol = 0;

        ch->OutVol  = ch->RealVol;
        ch->Status |= IS_Vol;
    }

    /* fine volume slide up */
    else if ((ch->VolKolVol & 0xF0) == 0x90)
    {
        ch->RealVol += (ch->VolKolVol & 0x0F);
        if (ch->RealVol > 64) ch->RealVol = 64;

        ch->OutVol  = ch->RealVol;
        ch->Status |= IS_Vol;
    }

    /* set vibrato speed */
    else if ((ch->VolKolVol & 0xF0) == 0xA0)
        ch->VibSpeed = (ch->VolKolVol & 0x0F) * 4;

    /* set panning */
    else if ((ch->VolKolVol & 0xF0) == 0xC0)
    {
        ch->OutPan  = (ch->VolKolVol & 0x0F) * 16;
        ch->Status |= IS_Pan;
    }


    /* *** MAIN EFFECTS (TICK 0) *** */


    if ((ch->EffTyp == 0) && (ch->Eff == 0)) return;

    /* 8xx - set panning */
    if (ch->EffTyp == 8)
    {
        ch->OutPan  = ch->Eff;
        ch->Status |= IS_Pan;
    }

    /* Bxx - position jump */
    else if (ch->EffTyp == 11)
    {
        Song.SongPos = (int16_t)(ch->Eff) - 1;
        Song.PBreakPos   = 0;
        Song.PosJumpFlag = 1;
    }

    /* Cxx - set volume */
    else if (ch->EffTyp == 12)
    {
        ch->RealVol = ch->Eff;
        if (ch->RealVol > 64) ch->RealVol = 64;

        ch->OutVol  = ch->RealVol;
        ch->Status |= IS_Vol;
    }

    /* Dxx - pattern break */
    else if (ch->EffTyp == 13)
    {
        Song.PosJumpFlag = 1;

        tmpEff = ((ch->Eff >> 4) * 10) + (ch->Eff & 0x0F);
        if (tmpEff <= 63)
            Song.PBreakPos = tmpEff;
        else
            Song.PBreakPos = 0;
    }

    /* Exx - E effects */
    else if (ch->EffTyp == 14)
    {
        /* E1x - fine period slide up */
        if ((ch->Eff & 0xF0) == 0x10)
        {
            tmpEff = ch->Eff & 0x0F;
            if (!tmpEff)
                tmpEff = ch->FPortaUpSpeed;

            ch->FPortaUpSpeed = tmpEff;

            ch->RealPeriod -= (tmpEff * 4);
            if (ch->RealPeriod < 1) ch->RealPeriod = 1;

            ch->OutPeriod = ch->RealPeriod;
            ch->Status   |= IS_Period;
        }

        /* E2x - fine period slide down */
        else if ((ch->Eff & 0xF0) == 0x20)
        {
            tmpEff = ch->Eff & 0x0F;
            if (!tmpEff)
                tmpEff = ch->FPortaDownSpeed;

            ch->FPortaDownSpeed = tmpEff;

            ch->RealPeriod += (tmpEff * 4);
            if (ch->RealPeriod > (32000 - 1)) ch->RealPeriod = 32000 - 1;

            ch->OutPeriod = ch->RealPeriod;
            ch->Status   |= IS_Period;
        }

        /* E3x - set glissando type */
        else if ((ch->Eff & 0xF0) == 0x30) ch->GlissFunk = ch->Eff & 0x0F;

        /* E4x - set vibrato waveform */
        else if ((ch->Eff & 0xF0) == 0x40) ch->WaveCtrl = (ch->WaveCtrl & 0xF0) | (ch->Eff & 0x0F);

        /* E5x (set finetune) is handled in StartTone() */

        /* E6x - pattern loop */
        else if ((ch->Eff & 0xF0) == 0x60)
        {
            if (ch->Eff == 0x60) /* E60, empty param */
            {
                ch->PattPos = Song.PattPos & 0x00FF;
            }
            else
            {
                if (!ch->LoopCnt)
                {
                    ch->LoopCnt = ch->Eff & 0x0F;

                    Song.PBreakPos  = ch->PattPos;
                    Song.PBreakFlag = 1;
                }
                else
                {
                    ch->LoopCnt--;
                    if (ch->LoopCnt)
                    {
                        Song.PBreakPos  = ch->PattPos;
                        Song.PBreakFlag = 1;
                    }
                }
            }
        }

        /* E7x - set tremolo waveform */
        else if ((ch->Eff & 0xF0) == 0x70) ch->WaveCtrl = ((ch->Eff & 0x0F) << 4) | (ch->WaveCtrl & 0x0F);

        /* E8x - set 4-bit panning (NON-FT2) */
        else if ((ch->Eff & 0xF0) == 0x80)
        {
            ch->OutPan  = (ch->Eff & 0x0F) * 16;
            ch->Status |= IS_Pan;
        }

        /* EAx - fine volume slide up */
        else if ((ch->Eff & 0xF0) == 0xA0)
        {
            tmpEff = ch->Eff & 0x0F;
            if (!tmpEff)
                tmpEff = ch->FVolSlideUpSpeed;

            ch->FVolSlideUpSpeed = tmpEff;

            ch->RealVol += tmpEff;
            if (ch->RealVol > 64) ch->RealVol = 64;

            ch->OutVol  = ch->RealVol;
            ch->Status |= IS_Vol;
        }

        /* EBx - fine volume slide down */
        else if ((ch->Eff & 0xF0) == 0xB0)
        {
            tmpEff = ch->Eff & 0x0F;
            if (!tmpEff)
                tmpEff = ch->FVolSlideDownSpeed;

            ch->FVolSlideDownSpeed = tmpEff;

            ch->RealVol -= tmpEff;
            if (ch->RealVol < 0) ch->RealVol = 0;

            ch->OutVol = ch->RealVol;
            ch->Status |= IS_Vol;
        }

        /* ECx - note cut */
        else if ((ch->Eff & 0xF0) == 0xC0)
        {
            if (ch->Eff == 0xC0) /* empty param */
            {
                ch->RealVol = 0;
                ch->OutVol  = 0;
                ch->Status |= IS_Vol;
            }
        }

        /* EEx - pattern delay */
        else if ((ch->Eff & 0xF0) == 0xE0)
        {
            if (!Song.PattDelTime2)
                Song.PattDelTime = (ch->Eff & 0x0F) + 1;
        }
    }

    /* Fxx - set speed/tempo */
    else if (ch->EffTyp == 15)
    {
        if (ch->Eff >= 32)
        {
            Song.Speed = ch->Eff;
            setSamplesPerFrame((audioFreq * 5) / 2 / Song.Speed);
        }
        else
        {
            Song.Tempo = ch->Eff;
            Song.Timer = ch->Eff;
        }
    }

    /* Gxx - set global volume */
    else if (ch->EffTyp == 16)
    {
        Song.GlobVol = ch->Eff;
        if (Song.GlobVol > 64) Song.GlobVol = 64;

        for (i = 0; i < Song.AntChn; ++i)
            Stm[i].Status |= IS_Vol;
    }

    /* Lxx - set vol and pan envelope position */
    else if (ch->EffTyp == 21)
    {
        /* *** VOLUME ENVELOPE *** */
        if (ch->InstrSeg.EnvVTyp & 1)
        {
            ch->EnvVCnt = ch->Eff - 1;

            envPos = 0;
            envUpdate = 1;
            newEnvPos = ch->Eff;

            if (ch->InstrSeg.EnvVPAnt > 1)
            {
                envPos++;
                for (i = 0; i < ch->InstrSeg.EnvVPAnt - 1; ++i)
                {
                    if (newEnvPos < ch->InstrSeg.EnvVP[envPos][0])
                    {
                        envPos--;

                        newEnvPos -= ch->InstrSeg.EnvVP[envPos][0];
                        if (newEnvPos == 0)
                        {
                            envUpdate = 0;
                            break;
                        }

                        if (ch->InstrSeg.EnvVP[envPos + 1][0] <= ch->InstrSeg.EnvVP[envPos][0])
                        {
                            envUpdate = 1;
                            break;
                        }

                        ch->EnvVIPValue = ((ch->InstrSeg.EnvVP[envPos + 1][1] - ch->InstrSeg.EnvVP[envPos][1]) & 0x00FF) << 8;
                        ch->EnvVIPValue /= (ch->InstrSeg.EnvVP[envPos + 1][0] - ch->InstrSeg.EnvVP[envPos][0]);

                        ch->EnvVAmp = (ch->EnvVIPValue * (newEnvPos - 1)) + ((ch->InstrSeg.EnvVP[envPos][1] & 0x00FF) << 8);

                        envPos++;

                        envUpdate = 0;
                        break;
                    }

                    envPos++;
                }

                if (envUpdate) envPos--;
            }

            if (envUpdate)
            {
                ch->EnvVIPValue = 0;
                ch->EnvVAmp = (ch->InstrSeg.EnvVP[envPos][1] & 0x00FF) << 8;
            }

            if (envPos >= ch->InstrSeg.EnvVPAnt)
            {
                envPos = ch->InstrSeg.EnvVPAnt - 1;
                if (envPos < 0)
                    envPos = 0;
            }

            ch->EnvVPos = envPos;
        }

        /* *** PANNING ENVELOPE *** */
        if (ch->InstrSeg.EnvVTyp & 2) /* probably an FT2 bug */
        {
            ch->EnvPCnt = ch->Eff - 1;

            envPos = 0;
            envUpdate = 1;
            newEnvPos = ch->Eff;

            if (ch->InstrSeg.EnvPPAnt > 1)
            {
                envPos++;
                for (i = 0; i < ch->InstrSeg.EnvPPAnt - 1; ++i)
                {
                    if (newEnvPos < ch->InstrSeg.EnvPP[envPos][0])
                    {
                        envPos--;

                        newEnvPos -= ch->InstrSeg.EnvPP[envPos][0];
                        if (newEnvPos == 0)
                        {
                            envUpdate = 0;
                            break;
                        }

                        if (ch->InstrSeg.EnvPP[envPos + 1][0] <= ch->InstrSeg.EnvPP[envPos][0])
                        {
                            envUpdate = 1;
                            break;
                        }

                        ch->EnvPIPValue = ((ch->InstrSeg.EnvPP[envPos + 1][1] - ch->InstrSeg.EnvPP[envPos][1]) & 0x00FF) << 8;
                        ch->EnvPIPValue /= (ch->InstrSeg.EnvPP[envPos + 1][0] - ch->InstrSeg.EnvPP[envPos][0]);

                        ch->EnvPAmp = (ch->EnvPIPValue * (newEnvPos - 1)) + ((ch->InstrSeg.EnvPP[envPos][1] & 0x00FF) << 8);

                        envPos++;

                        envUpdate = 0;
                        break;
                    }

                    envPos++;
                }

                if (envUpdate) envPos--;
            }

            if (envUpdate)
            {
                ch->EnvPIPValue = 0;
                ch->EnvPAmp = (ch->InstrSeg.EnvPP[envPos][1] & 0x00FF) << 8;
            }

            if (envPos >= ch->InstrSeg.EnvPPAnt)
            {
                envPos = ch->InstrSeg.EnvPPAnt - 1;
                if (envPos < 0)
                    envPos = 0;
            }

            ch->EnvPPos = envPos;
        }
    }

    /* Rxy - note multi retrigger */
    else if (ch->EffTyp == 27)
    {
        tmpEff = ch->Eff & 0x0F;
        if (!tmpEff)
            tmpEff = ch->RetrigSpeed;

        ch->RetrigSpeed = tmpEff;

        tmpEffHi = ch->Eff >> 4;
        if (!tmpEffHi)
            tmpEffHi = ch->RetrigVol;

        ch->RetrigVol = tmpEffHi;

        if (!ch->VolKolVol) MultiRetrig(ch);
    }

    /* X1x - extra fine period slide up */
    else if ((ch->EffTyp == 33) && ((ch->Eff & 0xF0) == 0x10))
    {
        tmpEff = ch->Eff & 0x0F;
        if (!tmpEff)
            tmpEff = ch->EPortaUpSpeed;

        ch->EPortaUpSpeed = tmpEff;

        ch->RealPeriod -= tmpEff;
        if (ch->RealPeriod < 1) ch->RealPeriod = 1;

        ch->OutPeriod = ch->RealPeriod;
        ch->Status   |= IS_Period;
    }

    /* X2x - extra fine period slide down */
    else if ((ch->EffTyp == 33) && ((ch->Eff & 0xF0) == 0x20))
    {
        tmpEff = ch->Eff & 0x0F;
        if (!tmpEff)
            tmpEff = ch->EPortaDownSpeed;

        ch->EPortaDownSpeed = tmpEff;

        ch->RealPeriod += tmpEff;
        if (ch->RealPeriod > (32000 - 1)) ch->RealPeriod = 32000 - 1;

        ch->OutPeriod = ch->RealPeriod;
        ch->Status   |= IS_Period;
    }
}

static void fixTonePorta(StmTyp *ch, TonTyp *p, uint8_t inst)
{
    uint16_t portaTmp;
    uint8_t tmpFTune;

    if (p->Ton)
    {
        if (p->Ton == 97)
        {
            KeyOff(ch);
        }
        else
        {
            /* "arithmetic shift right" on signed number simulation */
            if (ch->FineTune >= 0)
                tmpFTune = ch->FineTune / (1 << 3);
            else
                tmpFTune = 0xE0 | ((uint8_t)(ch->FineTune) >> 3); /* 0xE0 = 2^8 - 2^(8-3) */

            portaTmp = ((((p->Ton - 1) + ch->RelTonNr) & 0x00FF) * 16) + ((tmpFTune + 16) & 0x00FF);

            if (portaTmp < ((12 * 10 * 16) + 16))
            {
                if (Note2Period != NULL)
                    ch->WantPeriod = Note2Period[portaTmp];

                     if (ch->WantPeriod == ch->RealPeriod) ch->PortaDir = 0;
                else if (ch->WantPeriod  > ch->RealPeriod) ch->PortaDir = 1;
                else                                       ch->PortaDir = 2;
            }
        }
    }

    if (inst)
    {
        RetrigVolume(ch);

        if (p->Ton != 97)
            RetrigEnvelopeVibrato(ch);
    }
}

static void GetNewNote(StmTyp *ch, TonTyp *p)
{
    uint8_t inst;

    ch->VolKolVol = p->Vol;

    if (!ch->EffTyp)
    {
        if (ch->Eff)
        {
            /* we have an arpeggio running, set period back */
            ch->OutPeriod = ch->RealPeriod;
            ch->Status   |= IS_Period;
        }
    }
    else
    {
        if ((ch->EffTyp == 4) || (ch->EffTyp == 6))
        {
            /* we have a vibrato running */
            if ((p->EffTyp != 4) && (p->EffTyp != 6))
            {
                /* but it's ending at the next (this) row, so set period back */
                ch->OutPeriod = ch->RealPeriod;
                ch->Status   |= IS_Period;
            }
        }
    }

    ch->EffTyp = p->EffTyp;
    ch->Eff    = p->Eff;
    ch->ToneType = (p->Instr << 8) | p->Ton;

    /* 'inst' var is used for later if checks... */
    inst = p->Instr;
    if (inst)
    {
        if ((Song.AntInstrs > 128) || (inst <= 128)) /* >128 insnum hack */
            ch->InstrNr = inst;
        else
            inst = 0;
    }

    if (p->EffTyp == 0x0E)
    {
        if ((p->Eff >= 0xD1) && (p->Eff <= 0xDF))
            return; /* we have a note delay (ED1..EDF) */
    }

    if (!((p->EffTyp == 0x0E) && (p->Eff == 0x90))) /* E90 is 'retrig' speed 0 */
    {
        if ((ch->VolKolVol & 0xF0) == 0xF0) /* gxx */
        {
            if (ch->VolKolVol & 0x0F)
                ch->PortaSpeed = (ch->VolKolVol & 0x0F) * 64;

            fixTonePorta(ch, p, inst);

            CheckEffects(ch);
            return;
        }

        if ((p->EffTyp == 3) || (p->EffTyp == 5)) /* 3xx or 5xx */
        {
            if ((p->EffTyp != 5) && p->Eff)
                ch->PortaSpeed = p->Eff * 4;

            fixTonePorta(ch, p, inst);

            CheckEffects(ch);
            return;
        }

        if ((p->EffTyp == 0x14) && !p->Eff) /* K00 (KeyOff) */
        {
            KeyOff(ch);

            if (inst)
                RetrigVolume(ch);

            CheckEffects(ch);
            return;
        }

        if (!p->Ton)
        {
            if (inst)
            {
                RetrigVolume(ch);
                RetrigEnvelopeVibrato(ch);
            }

            CheckEffects(ch);
            return;
        }
    }

    if (p->Ton == 97)
        KeyOff(ch);
    else
        StartTone(p->Ton, p->EffTyp, p->Eff, ch);

    if (inst)
    {
        RetrigVolume(ch);

        if (p->Ton != 97)
            RetrigEnvelopeVibrato(ch);
    }

    CheckEffects(ch);
}

static void FixaEnvelopeVibrato(StmTyp *ch)
{
    uint16_t envVal;
    uint8_t envPos;
    int8_t envInterpolateFlag;
    int8_t envDidInterpolate;
    int16_t autoVibVal;
    int16_t autoVibAmp;
    int16_t tmpPeriod;

    /* *** FADEOUT *** */
    if (!ch->EnvSustainActive)
    {
        ch->Status |= IS_Vol;

        if (ch->FadeOutSpeed > 0)
        {
            ch->FadeOutAmp -= ch->FadeOutSpeed;
            if (ch->FadeOutAmp <= 0)
            {
                ch->FadeOutAmp   = 0;
                ch->FadeOutSpeed = 0;
            }
        }
    }

    if (!ch->Mute)
    {
        /* *** VOLUME ENVELOPE *** */
        envVal = 0;
        if (ch->InstrSeg.EnvVTyp & 1)
        {
            envDidInterpolate = 0;
            envPos = ch->EnvVPos;

            ch->EnvVCnt++;
            if (ch->EnvVCnt == ch->InstrSeg.EnvVP[envPos][0])
            {
                ch->EnvVAmp = (ch->InstrSeg.EnvVP[envPos][1] & 0x00FF) << 8;

                envPos++;
                if (ch->InstrSeg.EnvVTyp & 4)
                {
                    envPos--;

                    if (envPos == ch->InstrSeg.EnvVRepE)
                    {
                        if (!(ch->InstrSeg.EnvVTyp & 2) || (envPos != ch->InstrSeg.EnvVSust) || ch->EnvSustainActive)
                        {
                            envPos = ch->InstrSeg.EnvVRepS;

                            ch->EnvVCnt =  ch->InstrSeg.EnvVP[envPos][0];
                            ch->EnvVAmp = (ch->InstrSeg.EnvVP[envPos][1] & 0x00FF) << 8;
                        }
                    }

                    envPos++;
                }

                if (envPos < ch->InstrSeg.EnvVPAnt)
                {
                    envInterpolateFlag = 1;
                    if ((ch->InstrSeg.EnvVTyp & 2) && ch->EnvSustainActive)
                    {
                        if ((envPos - 1) == ch->InstrSeg.EnvVSust)
                        {
                            envPos--;
                            ch->EnvVIPValue = 0;
                            envInterpolateFlag = 0;
                        }
                    }

                    if (envInterpolateFlag)
                    {
                        ch->EnvVPos = envPos;

                        if (ch->InstrSeg.EnvVP[envPos][0] > ch->InstrSeg.EnvVP[envPos - 1][0])
                        {
                            ch->EnvVIPValue = ((ch->InstrSeg.EnvVP[envPos][1] - ch->InstrSeg.EnvVP[envPos - 1][1]) & 0x00FF) << 8;
                            ch->EnvVIPValue /= (ch->InstrSeg.EnvVP[envPos][0] - ch->InstrSeg.EnvVP[envPos - 1][0]);

                            envVal = ch->EnvVAmp;
                            envDidInterpolate = 1;
                        }
                    }
                }
                else
                {
                    ch->EnvVIPValue = 0;
                }
            }

            if (!envDidInterpolate)
            {
                ch->EnvVAmp += ch->EnvVIPValue;

                envVal = ch->EnvVAmp;
                if ((envVal >> 8) > 0x40)
                {
                    if ((envVal >> 8) > ((0x40 + 0xC0) / 2))
                        envVal = 16384;
                    else
                        envVal = 0;

                    ch->EnvVIPValue = 0;
                }
            }

            envVal >>= 8;

            ch->FinalVol = (Song.GlobVol / 64.0f) * (envVal / 64.0f) * (ch->OutVol / 64.0f) * (ch->FadeOutAmp / 32768.0f);
            ch->Status  |= IS_Vol;
        }
        else
        {
            ch->FinalVol = (Song.GlobVol / 64.0f) * (ch->OutVol / 64.0f) * (ch->FadeOutAmp / 32768.0f);
        }

        ch->FinalVol = CLAMP(ch->FinalVol, 0.0f, 1.0f);
    }
    else
    {
        ch->FinalVol = 0.0f;
    }

    /* *** PANNING ENVELOPE *** */
    envVal = 0;
    if (ch->InstrSeg.EnvPTyp & 1)
    {
        envDidInterpolate = 0;
        envPos = ch->EnvPPos;

        ch->EnvPCnt++;
        if (ch->EnvPCnt == ch->InstrSeg.EnvPP[envPos][0])
        {
            ch->EnvPAmp = (ch->InstrSeg.EnvPP[envPos][1] & 0x00FF) << 8;

            envPos++;
            if (ch->InstrSeg.EnvPTyp & 4)
            {
                envPos--;

                if (envPos == ch->InstrSeg.EnvPRepE)
                {
                    if (!(ch->InstrSeg.EnvPTyp & 2) || (envPos != ch->InstrSeg.EnvPSust) || ch->EnvSustainActive)
                    {
                        envPos = ch->InstrSeg.EnvPRepS;

                        ch->EnvPCnt =  ch->InstrSeg.EnvPP[envPos][0];
                        ch->EnvPAmp = (ch->InstrSeg.EnvPP[envPos][1] & 0x00FF) << 8;
                    }
                }

                envPos++;
            }

            if (envPos < ch->InstrSeg.EnvPPAnt)
            {
                envInterpolateFlag = 1;
                if ((ch->InstrSeg.EnvPTyp & 2) && ch->EnvSustainActive)
                {
                    if ((envPos - 1) == ch->InstrSeg.EnvPSust)
                    {
                        envPos--;
                        ch->EnvPIPValue = 0;
                        envInterpolateFlag = 0;
                    }
                }

                if (envInterpolateFlag)
                {
                    ch->EnvPPos = envPos;

                    if (ch->InstrSeg.EnvPP[envPos][0] > ch->InstrSeg.EnvPP[envPos - 1][0])
                    {
                        ch->EnvPIPValue = ((ch->InstrSeg.EnvPP[envPos][1] - ch->InstrSeg.EnvPP[envPos - 1][1]) & 0x00FF) << 8;
                        ch->EnvPIPValue /= (ch->InstrSeg.EnvPP[envPos][0] - ch->InstrSeg.EnvPP[envPos - 1][0]);

                        envVal = ch->EnvPAmp;
                        envDidInterpolate = 1;
                    }
                }
            }
            else
            {
                ch->EnvPIPValue = 0;
            }
        }

        if (!envDidInterpolate)
        {
            ch->EnvPAmp += ch->EnvPIPValue;

            envVal = ch->EnvPAmp;
            if ((envVal >> 8) > 0x40)
            {
                if ((envVal >> 8) > ((0x40 + 0xC0) / 2))
                    envVal = 16384;
                else
                    envVal = 0;

                ch->EnvPIPValue = 0;
            }
        }

        envVal >>= 8;

        ch->FinalPan = (ch->OutPan + (((envVal - 32.0f) * (128.0f - fabsf(ch->OutPan - 128.0f))) / 32.0f)) / 255.0f;
        ch->Status  |= IS_Pan;
    }
    else
    {
        ch->FinalPan = ch->OutPan / 255.0f;
    }

    ch->FinalPan = CLAMP(ch->FinalPan, 0.0f, 1.0f);

    /* *** AUTO VIBRATO *** */
    if (ch->InstrSeg.VibDepth)
    {
        if (ch->EVibSweep)
        {
            autoVibAmp = ch->EVibSweep;
            if (ch->EnvSustainActive)
            {
                autoVibAmp += ch->EVibAmp;
                if ((autoVibAmp >> 8) > ch->InstrSeg.VibDepth)
                {
                    ch->EVibAmp   = ch->InstrSeg.VibDepth << 8;
                    ch->EVibSweep = 0;
                }

                ch->EVibAmp = autoVibAmp;
            }
        }
        else
        {
            autoVibAmp = ch->EVibAmp;
        }

        ch->EVibPos += ch->InstrSeg.VibRate;

        /* square */
        if (ch->InstrSeg.VibTyp == 1)
            autoVibVal = (ch->EVibPos > 127) ? 64 : -64;

        /* ramp up */
        else if (ch->InstrSeg.VibTyp == 2)
            autoVibVal = (((ch->EVibPos >> 1) + 64) & 127) - 64;

        /* ramp down */
        else if (ch->InstrSeg.VibTyp == 3)
            autoVibVal = (((0 - (ch->EVibPos >> 1)) + 64) & 127) - 64;

        /* sine */
        else
            autoVibVal = VibSineTab[ch->EVibPos];

        autoVibVal <<= 2;

        tmpPeriod = ch->OutPeriod + ((autoVibVal * autoVibAmp) >> 16);
        if (tmpPeriod > (32000 - 1)) tmpPeriod = 0; /* yes, FT2 zeroes it out */

        ch->FinalPeriod = tmpPeriod;

        ch->Status  |= IS_Period;
    }
    else
    {
        ch->FinalPeriod = ch->OutPeriod;
    }
}

int16_t RelocateTon(int16_t inPeriod, int8_t addNote, StmTyp *ch)
{
    int8_t i;
    int8_t fineTune;

    int16_t oldPeriod;
    int16_t addPeriod;

    int32_t outPeriod;

    if (Note2Period == NULL)
        return (0);

    /* "arithmetic shift right" on signed number simulation */
    if (ch->FineTune >= 0)
        fineTune = ch->FineTune / (1 << 3);
    else
        fineTune = 0xE0 | ((uint8_t)(ch->FineTune) >> 3); /* 0xE0 = 2^8 - 2^(8-3) */

    fineTune += 16;
    fineTune += fineTune;

    oldPeriod = 0;
    addPeriod = (8 * 12 * 16) * 2;

    for (i = 0; i < 8; ++i)
    {
        outPeriod = (((oldPeriod + addPeriod) >> 1) & 0xFFE0) + fineTune;
        if (outPeriod < fineTune)
            outPeriod += (1 << 8);

        if (outPeriod < 16)
            outPeriod = 16;

        if (inPeriod >= Note2Period[(outPeriod - 16) >> 1])
        {
            outPeriod -= fineTune;
            if (outPeriod & 0x00010000)
                outPeriod = (outPeriod - (1 << 8)) & 0x0000FFE0;

            addPeriod = (int16_t)(outPeriod);
        }
        else
        {
            outPeriod -= fineTune;
            if (outPeriod & 0x00010000)
                outPeriod = (outPeriod - (1 << 8)) & 0x0000FFE0;

            oldPeriod = (int16_t)(outPeriod);
        }
    }

    outPeriod = oldPeriod + fineTune;
    if (outPeriod < fineTune)
        outPeriod += (1 << 8);

    if (outPeriod < 0)
        outPeriod = 0;

    outPeriod += (addNote << 5);
    if (outPeriod >= ((((8 * 12 * 16) + 15) * 2) - 1))
        outPeriod = ((8 * 12 * 16) + 15) * 2;

    return (Note2Period[outPeriod >> 1]); /* 16-bit look-up, shift it down */
}

static void TonePorta(StmTyp *ch)
{
    if (ch->PortaDir)
    {
        if (ch->PortaDir > 1)
        {
            ch->RealPeriod -= ch->PortaSpeed;
            if (ch->RealPeriod <= ch->WantPeriod)
            {
                ch->PortaDir   = 1;
                ch->RealPeriod = ch->WantPeriod;
            }
        }
        else
        {
            ch->RealPeriod += ch->PortaSpeed;
            if (ch->RealPeriod >= ch->WantPeriod)
            {
                ch->PortaDir   = 1;
                ch->RealPeriod = ch->WantPeriod;
            }
        }

        if (ch->GlissFunk) /* semi-tone slide flag */
            ch->OutPeriod = RelocateTon(ch->RealPeriod, 0, ch);
        else
            ch->OutPeriod = ch->RealPeriod;

        ch->Status |= IS_Period;
    }
}

static void Volume(StmTyp *ch) /* actually volume slide */
{
    uint8_t tmpEff;

    tmpEff = ch->Eff;
    if (!tmpEff)
        tmpEff = ch->VolSlideSpeed;

    ch->VolSlideSpeed = tmpEff;

    if (!(tmpEff & 0xF0))
    {
        ch->RealVol -= tmpEff;
        if (ch->RealVol < 0) ch->RealVol = 0;
    }
    else
    {
        ch->RealVol += (tmpEff >> 4);
        if (ch->RealVol > 64) ch->RealVol = 64;
    }

    ch->OutVol  = ch->RealVol;
    ch->Status |= IS_Vol;
}

static void Vibrato2(StmTyp *ch)
{
    uint8_t tmpVib;

    tmpVib = (ch->VibPos / 4) & 0x1F;

    switch (ch->WaveCtrl & 0x03)
    {
        /* 0: sine */
        case 0:
        {
            tmpVib = VibTab[tmpVib];
        }
        break;

        /* 1: ramp */
        case 1:
        {
            tmpVib *= 8;
            if (ch->VibPos >= 128) tmpVib ^= 0xFF;
        }
        break;

        /* 2/3: square */
        default:
        {
            tmpVib = 255;
        }
        break;
    }

    tmpVib = (tmpVib * ch->VibDepth) / 32;

    if (ch->VibPos >= 128)
        ch->OutPeriod = ch->RealPeriod - tmpVib;
    else
        ch->OutPeriod = ch->RealPeriod + tmpVib;

    ch->Status |= IS_Period;
    ch->VibPos += ch->VibSpeed;
}

static void Vibrato(StmTyp *ch)
{
    if (ch->Eff)
    {
        if (ch->Eff & 0x0F) ch->VibDepth = ch->Eff & 0x0F;
        if (ch->Eff & 0xF0) ch->VibSpeed = (ch->Eff >> 4) * 4;
    }

    Vibrato2(ch);
}

static void DoEffects(StmTyp *ch)
{
    int8_t note;
    uint8_t tmpEff;
    uint8_t tremorData;
    uint8_t tremorSign;
    uint8_t tmpTrem;
    uint16_t i;
    uint16_t tick;

    /* *** VOLUME COLUMN EFFECTS (TICKS >0) *** */

    /* volume slide down */
    if ((ch->VolKolVol & 0xF0) == 0x60)
    {
        ch->RealVol -= (ch->VolKolVol & 0x0F);
        if (ch->RealVol < 0) ch->RealVol = 0;

        ch->OutVol  = ch->RealVol;
        ch->Status |= IS_Vol;
    }

    /* volume slide up */
    else if ((ch->VolKolVol & 0xF0) == 0x70)
    {
        ch->RealVol += (ch->VolKolVol & 0x0F);
        if (ch->RealVol > 64) ch->RealVol = 64;

        ch->OutVol  = ch->RealVol;
        ch->Status |= IS_Vol;
    }

    /* vibrato (+ set vibrato depth) */
    else if ((ch->VolKolVol & 0xF0) == 0xB0)
    {
        if (ch->VolKolVol != 0xB0)
            ch->VibDepth = ch->VolKolVol & 0x0F;

        Vibrato2(ch);
    }

    /* pan slide left */
    else if ((ch->VolKolVol & 0xF0) == 0xD0)
    {
        ch->OutPan -= (ch->VolKolVol & 0x0F);
        if (ch->OutPan < 0) ch->OutPan = 0;

        ch->Status |= IS_Pan;
    }

    /* pan slide right */
    else if ((ch->VolKolVol & 0xF0) == 0xE0)
    {
        ch->OutPan += (ch->VolKolVol & 0x0F);
        if (ch->OutPan > 255) ch->OutPan = 255;

        ch->Status |= IS_Pan;
    }

    /* tone portamento */
    else if ((ch->VolKolVol & 0xF0) == 0xF0) TonePorta(ch);

    /* *** MAIN EFFECTS (TICKS >0) *** */

    if (((ch->Eff == 0) && (ch->EffTyp == 0)) || (ch->EffTyp >= 36)) return;

    /* 0xy - Arpeggio */
    if (ch->EffTyp == 0)
    {
        tick = Song.Timer;
        note = 0;

        /* FT2 'out of boundary' arp LUT simulation */
             if (tick  > 16) tick  = 2;
        else if (tick == 16) tick  = 0;
        else                 tick %= 3;

        /*
        ** this simulation doesn't work properly for >=128 tick arps,
        ** but you'd need to hexedit the initial speed to get >31
        */

        if (!tick)
        {
            ch->OutPeriod = ch->RealPeriod;
        }
        else
        {
                 if (tick == 1) note = ch->Eff >> 4;
            else if (tick == 2) note = ch->Eff & 0x0F;

            ch->OutPeriod = RelocateTon(ch->RealPeriod, note, ch);
        }

        ch->Status |= IS_Period;
    }

    /* 1xx - period slide up */
    else if (ch->EffTyp == 1)
    {
        tmpEff = ch->Eff;
        if (!tmpEff)
            tmpEff = ch->PortaUpSpeed;

        ch->PortaUpSpeed = tmpEff;

        ch->RealPeriod -= (tmpEff * 4);
        if (ch->RealPeriod < 1) ch->RealPeriod = 1;

        ch->OutPeriod = ch->RealPeriod;
        ch->Status   |= IS_Period;
    }

    /* 2xx - period slide up */
    else if (ch->EffTyp == 2)
    {
        tmpEff = ch->Eff;
        if (!tmpEff)
            tmpEff = ch->PortaUpSpeed;

        ch->PortaUpSpeed = tmpEff;

        ch->RealPeriod += (tmpEff * 4);
        if (ch->RealPeriod > (32000 - 1)) ch->RealPeriod = 32000 - 1;

        ch->OutPeriod = ch->RealPeriod;
        ch->Status   |= IS_Period;
    }

    /* 3xx - tone portamento */
    else if (ch->EffTyp == 3) TonePorta(ch);

    /* 4xy - vibrato */
    else if (ch->EffTyp == 4) Vibrato(ch);

    /* 5xy - tone portamento + volume slide */
    else if (ch->EffTyp == 5)
    {
        TonePorta(ch);
        Volume(ch);
    }

    /* 6xy - vibrato + volume slide */
    else if (ch->EffTyp == 6)
    {
        Vibrato2(ch);
        Volume(ch);
    }

    /* 7xy - tremolo */
    else if (ch->EffTyp == 7)
    {
        tmpEff = ch->Eff;
        if (tmpEff)
        {
            if (tmpEff & 0x0F) ch->TremDepth = tmpEff & 0x0F;
            if (tmpEff & 0xF0) ch->TremSpeed = (tmpEff >> 4) * 4;
        }

        tmpTrem = (ch->TremPos / 4) & 0x1F;

        switch ((ch->WaveCtrl >> 4) & 3)
        {
            /* 0: sine */
            case 0:
            {
                tmpTrem = VibTab[tmpTrem];
            }
            break;

            /* 1: ramp */
            case 1:
            {
                tmpTrem *= 8;
                if (ch->VibPos >= 128) tmpTrem ^= 0xFF; /* FT2 bug, should've been TremPos */
            }
            break;

            /* 2/3: square */
            default:
            {
                tmpTrem = 255;
            }
            break;
        }

        tmpTrem = (tmpTrem * ch->TremDepth) / 64;

        if (ch->TremPos >= 128)
        {
            ch->OutVol = ch->RealVol - tmpTrem;
            if (ch->OutVol < 0) ch->OutVol = 0;
        }
        else
        {
            ch->OutVol = ch->RealVol + tmpTrem;
            if (ch->OutVol > 64) ch->OutVol = 64;
        }

        ch->TremPos += ch->TremSpeed;

        ch->Status |= IS_Vol;
    }

    /* Axy - volume slide */
    else if (ch->EffTyp == 10) Volume(ch); /* actually volume slide */

    /* Exy - E effects */
    else if (ch->EffTyp == 14)
    {
        /* E9x - note retrigger */
        if ((ch->Eff & 0xF0) == 0x90)
        {
            if (ch->Eff != 0x90) /* E90 is handled in GetNewNote() */
            {
                if (!((Song.Tempo - Song.Timer) % (ch->Eff & 0x0F)))
                {
                    StartTone(0, 0, 0, ch);
                    RetrigEnvelopeVibrato(ch);
                }
            }
        }

        /* ECx - note cut */
        else if ((ch->Eff & 0xF0) == 0xC0)
        {
            if (((Song.Tempo - Song.Timer) & 0x00FF) == (ch->Eff & 0x0F))
            {
                ch->OutVol  = 0;
                ch->RealVol = 0;
                ch->Status |= IS_Vol;
            }
        }

        /* EDx - note delay */
        else if ((ch->Eff & 0xF0) == 0xD0)
        {
            if (((Song.Tempo - Song.Timer) & 0x00FF) == (ch->Eff & 0x0F))
            {
                StartTone(ch->ToneType & 0x00FF, 0, 0, ch);

                if (ch->ToneType & 0xFF00)
                    RetrigVolume(ch);

                RetrigEnvelopeVibrato(ch);

                if ((ch->VolKolVol >= 0x10) && (ch->VolKolVol <= 0x50))
                {
                    ch->OutVol  = ch->VolKolVol - 16;
                    ch->RealVol = ch->OutVol;
                }
                else if ((ch->VolKolVol >= 0xC0) && (ch->VolKolVol <= 0xCF))
                {
                    ch->OutPan = (ch->VolKolVol & 0x0F) * 16;
                }
            }
        }
    }

    /* Hxy - global volume slide */
    else if (ch->EffTyp == 17)
    {
        tmpEff = ch->Eff;
        if (!tmpEff)
            tmpEff = ch->GlobVolSlideSpeed;

        ch->GlobVolSlideSpeed = tmpEff;

        if (!(tmpEff & 0xF0))
        {
            Song.GlobVol -= tmpEff;
            if (Song.GlobVol < 0) Song.GlobVol = 0;
        }
        else
        {
            Song.GlobVol += (tmpEff >> 4);
            if (Song.GlobVol > 64) Song.GlobVol = 64;
        }

        for (i = 0; i < Song.AntChn; ++i)
            Stm[i].Status |= IS_Vol;
    }

    /* Kxx - key off */
    else if (ch->EffTyp == 20)
    {
        if (((Song.Tempo - Song.Timer) & 31) == (ch->Eff & 0x0F))
            KeyOff(ch);
    }

    /* Pxy - panning slide */
    else if (ch->EffTyp == 25)
    {
        tmpEff = ch->Eff;
        if (!tmpEff)
            tmpEff = ch->PanningSlideSpeed;

        ch->PanningSlideSpeed = tmpEff;

        if (!(ch->Eff & 0xF0))
        {
            ch->OutPan += (ch->Eff >> 4);
            if (ch->OutPan > 255) ch->OutPan = 255;
        }
        else
        {
            ch->OutPan -= (ch->Eff & 0x0F);
            if (ch->OutPan < 0) ch->OutPan = 0;
        }

        ch->Status |= IS_Pan;
    }

    /* Rxy - multi note retrig */
    else if (ch->EffTyp == 27) MultiRetrig(ch);

    /* Txy - tremor */
    else if (ch->EffTyp == 29)
    {
        tmpEff = ch->Eff;
        if (!tmpEff)
            tmpEff = ch->TremorSave;

        ch->TremorSave = tmpEff;

        tremorSign = ch->TremorPos & 0x80;
        tremorData = ch->TremorPos & 0x7F;

        tremorData--;
        if (tremorData & 0x80)
        {
            if (tremorSign == 0x80)
            {
                tremorSign = 0x00;
                tremorData = tmpEff & 0x0F;
            }
            else
            {
                tremorSign = 0x80;
                tremorData = tmpEff >> 4;
            }
        }

        ch->TremorPos = tremorData | tremorSign;

        ch->OutVol  = tremorSign ? ch->RealVol : 0;
        ch->Status |= IS_Vol;
    }
}

void P_StartTone(uint8_t chNr)
{
    VOICE *v;
    VOICE *v2;
    StmTyp *ch;
    SampleTyp s;

    if (chNr >= MAX_VOICES)
        return;

    ch = &Stm[chNr];
    v = &voice[chNr];
    s = ch->InstrOfs;

	ch->noteTriggered = true;

    if (volumeRamping)
    {
        v2 = &voice[SPARE_OFFSET + chNr];

        // hackish, don't change this unless you know the struct inside out!
        memcpy(&v2->loopEnabled, &v->loopEnabled, sizeof (VOICE) - sizeof (int8_t *));

        v2->faderDest = 0.0f;
        v2->faderDelta = (0.0f - v2->fader) * f_samplesPerFrame010;
        v2->samplePositionFrac = 0.0f;
        v2->sampleData = v->sampleData;
    }

    voiceSetSource(chNr, s.Pek, s.Len, s.RepL, s.RepS + s.RepL, s.Typ & 3, s.Typ & 16, s.Typ & 32, ch->SmpStartPos);

    if (volumeRamping)
    {
        voice[chNr].fader      = 0.0f;
        voice[chNr].faderDest  = 1.0f;
        voice[chNr].faderDelta = (voice[chNr].faderDest - voice[chNr].fader) * f_samplesPerFrame005;
    }
}

static void Mix_UpdateChannelVolPanFrq(void)
{
    uint8_t i;
    uint8_t status;

    StmTyp *ch;

    for (i = 0; i < MAX_VOICES; ++i)
    {
        ch = &Stm[i];

        status = ch->Status;
        if (status != 0)
        {
            ch->Status = 0;

            /* this order is carefully selected, modification will result in unwanted behavior */

            if (status & IS_NyTon)
                P_StartTone(i);

            if (status & IS_Period)
                voiceSetSamplingFrequency(i, GetFrequenceValue(ch->FinalPeriod));

            if (status & IS_Vol)
                voiceSetVolume(i, ch->FinalVol);

            if (status & IS_Pan)
                voiceSetPanning(i, ch->FinalPan);
        }
    }
}

void MainPlayer(void) /* periodically called from mixer */
{
    int8_t tickzero;
    uint8_t i;

    if (!MusicPaused && Playing)
    {
        tickzero = 0;

        if (--Song.Timer == 0)
        {
            Song.Timer = Song.Tempo;
            tickzero   = 1;
        }

        if (tickzero)
        {
            if (!Song.PattDelTime2)
            {
                for (i = 0; i < Song.AntChn; ++i)
                {
                    if (Patt[Song.PattNr])
                        GetNewNote(&Stm[i], &Patt[Song.PattNr][(Song.PattPos * MAX_VOICES) + i]);
                    else
                        GetNewNote(&Stm[i], &NilPatternLine[(Song.PattPos * MAX_VOICES) + i]);

                    FixaEnvelopeVibrato(&Stm[i]);
                }
            }
            else
            {
                for (i = 0; i < Song.AntChn; ++i)
                {
                    DoEffects(&Stm[i]);
                    FixaEnvelopeVibrato(&Stm[i]);
                }
            }
        }
        else
        {
            for (i = 0; i < Song.AntChn; ++i)
            {
                DoEffects(&Stm[i]);
                FixaEnvelopeVibrato(&Stm[i]);
            }
        }

        if (Song.Timer == 1)
        {
            Song.PattPos++;

            if (Song.PattDelTime)
            {
                Song.PattDelTime2 = Song.PattDelTime;
                Song.PattDelTime  = 0;
            }

            if (Song.PattDelTime2)
            {
                Song.PattDelTime2--;
                if (Song.PattDelTime2) Song.PattPos--;
            }

            if (Song.PBreakFlag)
            {
                Song.PBreakFlag = 0;
                Song.PattPos    = Song.PBreakPos;
            }

            if ((Song.PattPos >= Song.PattLen) || Song.PosJumpFlag)
            {
                Song.PattPos     = Song.PBreakPos;
                Song.PBreakPos   = 0;
                Song.PosJumpFlag = 0;

                Song.SongPos++;
                if (Song.SongPos >= Song.Len)
                    Song.SongPos  = Song.RepS;

                Song.PattNr  = Song.SongTab[Song.SongPos];
                Song.PattLen = PattLens[Song.PattNr];
            }
        }
    }
    else
    {
        for (i = 0; i < Song.AntChn; ++i)
            FixaEnvelopeVibrato(&Stm[i]);
    }
}

void StopVoices(void)
{
    uint8_t i;
    StmTyp *ch;

    memset(voice, 0, sizeof (voice));
    memset(Stm,   0, sizeof (Stm));

    for (i = 0; i < MAX_VOICES; ++i)
    {
        ch = &Stm[i];

        ch->Nr       = i;
        ch->ToneType = 0;
        ch->RelTonNr = 0;
        ch->InstrNr  = 0;

        if (Instr[0] != NULL)
            ch->InstrSeg = *Instr[0];

        ch->Status   = IS_Vol;
        ch->RealVol  = 0;
        ch->OutVol   = 0;
        ch->OldVol   = 0;
        ch->FinalVol = 0.0f;
        ch->OldPan   = 128;
        ch->OutPan   = 128;
        ch->FinalPan = 0.5f;
        ch->VibDepth = 0;
    }
}

void SetPos(int16_t SongPos, int16_t PattPos)
{
    if (SongPos > -1)
    {
        Song.SongPos = SongPos;
        if ((Song.Len > 0) && (Song.SongPos >= Song.Len))
            Song.SongPos = Song.Len - 1;

        Song.PattNr  = Song.SongTab[SongPos];
        Song.PattLen = PattLens[Song.PattNr];
    }

    if (PattPos > -1)
    {
        Song.PattPos = PattPos;
        if (Song.PattPos >= Song.PattLen)
            Song.PattPos = Song.PattLen - 1;
    }

    Song.Timer = 1;
}

static void FreeInstr(uint16_t ins)
{
    uint8_t i;

    if (Instr[ins] != NULL)
    {
        for (i = 0; i < 32; ++i)
        {
            if (Instr[ins]->Samp[i].Pek != NULL)
            {
                free(Instr[ins]->Samp[i].Pek);
                Instr[ins]->Samp[i].Pek = NULL;
            }
        }

        free(Instr[ins]);
        Instr[ins] = NULL;
    }
}

static void FreeMusic(void)
{
    uint16_t i;

    for (i = 1; i < (255 + 1); ++i)
        FreeInstr(i);

    for (i = 0; i < 256; ++i)
    {
        if (Patt[i] != NULL)
        {
            free(Patt[i]);
            Patt[i] = NULL;
        }

        PattLens[i] = 64;
    }

    memset(&Song, 0, sizeof (Song));

    Song.Len     = 1;
    Song.Tempo   = 6;
    Song.Speed   = 125;
    Song.Timer   = 1;
    Song.AntChn  = MAX_VOICES;
    LinearFrqTab = 1;

    StopVoices();
    SetPos(0, 0);
}

static void Delta2Samp(int8_t *p, uint32_t len, uint8_t typ)
{
    uint32_t i;

    int16_t *p16;
    int16_t news16;
    int16_t olds16L;
    int16_t olds16R;

    int8_t *p8;
    int8_t news8;
    int8_t olds8L;
    int8_t olds8R;

    if (typ & 16) len /= 2; /* 16-bit */
    if (typ & 32) len /= 2; /* stereo */

    if (typ & 32)
    {
        if (typ & 16)
        {
            p16 = (int16_t *)(p);

            olds16L = 0;
            olds16R = 0;

            for (i = 0; i < len; ++i)
            {
                news16  = p16[i] + olds16L;
                p16[i]  = news16;
                olds16L = news16;

                news16 = p16[len + i] + olds16R;
                p16[len + i] = news16;
                olds16R = news16;
            }
        }
        else
        {
            p8 = (int8_t *)(p);

            olds8L = 0;
            olds8R = 0;

            for (i = 0; i < len; ++i)
            {
                news8  = p8[i] + olds8L;
                p8[i]  = news8;
                olds8L = news8;

                news8 = p8[len + i] + olds8R;
                p8[len + i] = news8;
                olds8R = news8;
            }
        }
    }
    else
    {
        if (typ & 16)
        {
            p16 = (int16_t *)(p);

            olds16L = 0;

            for (i = 0; i < len; ++i)
            {
                news16  = p16[i] + olds16L;
                p16[i]  = news16;
                olds16L = news16;
            }
        }
        else
        {
            p8 = (int8_t *)(p);

            olds8L = 0;

            for (i = 0; i < len; ++i)
            {
                news8  = p8[i] + olds8L;
                p8[i]  = news8;
                olds8L = news8;
            }
        }
    }
}

static void FreeAllInstr(void)
{
    uint16_t i;
    for (i = 1; i < (255 + 1); ++i)
        FreeInstr(i);
}

static int8_t AllocateInstr(uint16_t i)
{
    uint8_t j;

    InstrTyp *p;

    if (Instr[i] == NULL)
    {
        p = (InstrTyp *)(calloc(1, sizeof (InstrTyp)));
        if (p == NULL) return (0);

        for (j = 0; j < 32; ++j)
        {
            p->Samp[j].Pan = 128;
            p->Samp[j].Vol = 64;
        }

        Instr[i] = p;

        return (1);
    }

    return (0);
}

static int8_t LoadInstrHeader(MEM *f, uint16_t i)
{
    uint8_t j;

    InstrHeaderTyp ih;

    memset(&ih, 0, InstrHeaderSize);

    mread(&ih.InstrSize, 4, 1, f);

    if ((ih.InstrSize <= 0) || (ih.InstrSize > InstrHeaderSize))
        ih.InstrSize = InstrHeaderSize;

    mread(ih.Name, ih.InstrSize - 4, 1, f);

    if (meof(f) || (ih.AntSamp > 32)) return (0);

    if (ih.AntSamp)
    {
        if (!AllocateInstr(i)) return (0);

        memcpy(Instr[i]->TA, ih.TA, ih.InstrSize);
        Instr[i]->AntSamp = ih.AntSamp;

        mread(ih.Samp, ih.AntSamp * sizeof (SampleHeaderTyp), 1, f);
        if (meof(f)) return (0);

        for (j = 0; j < ih.AntSamp; ++j)
        {
            memcpy(&Instr[i]->Samp[j].Len, &ih.Samp[j].Len, 12 + 4 + 24);
            if (Instr[i]->Samp[j].Vol > 64) // non-FT2 fix: Don't blow our eardrums on malicious XMs!
                Instr[i]->Samp[j].Vol = 64;
        }
    }

    return (1);
}

static int8_t LoadInstrSample(MEM *f, uint16_t i)
{
    uint16_t j;
    int32_t l;

    SampleTyp *s;

    if (Instr[i] != NULL)
    {
        for (j = 1; j <= Instr[i]->AntSamp; ++j)
        {
            Instr[i]->Samp[j - 1].Pek = NULL;

            l = Instr[i]->Samp[j - 1].Len;
            if (l > 0)
            {
                Instr[i]->Samp[j - 1].Pek = (int8_t *)(malloc(l));
                if (Instr[i]->Samp[j - 1].Pek == NULL)
                {
                    for (j = i; j <= Song.AntInstrs; ++j)
                        FreeInstr(j);

                    return (0);
                }

                mread(Instr[i]->Samp[j - 1].Pek, l, 1, f);
                Delta2Samp(Instr[i]->Samp[j - 1].Pek, l, Instr[i]->Samp[j - 1].Typ);
            }

            s = &Instr[i]->Samp[j - 1];
            if (s->Pek == NULL)
            {
                s->Len  = 0;
                s->RepS = 0;
                s->RepL = 0;
            }
            else
            {
                if (s->RepS < 0) s->RepS = 0;
                if (s->RepL < 0) s->RepL = 0;
                if (s->RepS > s->Len) s->RepS = s->Len;
                if ((s->RepS + s->RepL) > s->Len) s->RepL = s->Len - s->RepS;
            }

            if (s->RepL == 0) s->Typ &= 0xFC; /* non-FT2 fix: force loop off if looplen is 0 */
        }
    }

    return (1);
}

static void UnpackPatt(TonTyp *patdata, uint16_t length, uint16_t packlen, uint8_t *packdata)
{
    uint32_t patofs;

    uint16_t i;
    uint16_t packindex;

    uint8_t j;
    uint8_t packnote;

    packindex = 0;
    for (i = 0; i < length; ++i)
    {
        for (j = 0; j < Song.AntChn; ++j)
        {
            if (packindex >= packlen) return;

            patofs   = (i * MAX_VOICES) + j;
            packnote = packdata[packindex++];

            if (packnote & 0x80)
            {
                if (packnote & 0x01) patdata[patofs].Ton    = packdata[packindex++];
                if (packnote & 0x02) patdata[patofs].Instr  = packdata[packindex++];
                if (packnote & 0x04) patdata[patofs].Vol    = packdata[packindex++];
                if (packnote & 0x08) patdata[patofs].EffTyp = packdata[packindex++];
                if (packnote & 0x10) patdata[patofs].Eff    = packdata[packindex++];
            }
            else
            {
                patdata[patofs].Ton    = packnote;
                patdata[patofs].Instr  = packdata[packindex++];
                patdata[patofs].Vol    = packdata[packindex++];
                patdata[patofs].EffTyp = packdata[packindex++];
                patdata[patofs].Eff    = packdata[packindex++];
            }
        }
    }
}

static int8_t PatternEmpty(uint16_t nr)
{
    uint32_t patofs;
    uint16_t i;
    uint8_t j;

    if (Patt[nr] == NULL)
    {
        return (1);
    }
    else
    {
        for (i = 0; i < PattLens[nr]; ++i)
        {
            for (j = 0; j < Song.AntChn; ++j)
            {
                patofs = (i * MAX_VOICES) + j;

                if (Patt[nr][patofs].Ton)    return (0);
                if (Patt[nr][patofs].Instr)  return (0);
                if (Patt[nr][patofs].Vol)    return (0);
                if (Patt[nr][patofs].EffTyp) return (0);
                if (Patt[nr][patofs].Eff)    return (0);
            }
        }
    }

    return (1);
}

static int8_t LoadPatterns(MEM *f)
{
    uint8_t *patttmp;
    uint16_t i;
    uint8_t tmpLen;

    PatternHeaderTyp ph;

    for (i = 0; i < Song.AntPtn; ++i)
    {
        mread(&ph.PatternHeaderSize, 4, 1, f);
        mread(&ph.Typ, 1, 1, f);

        ph.PattLen = 0;
        if (Song.Ver == 0x0102)
        {
            mread(&tmpLen, 1, 1, f);
            ph.PattLen = (uint16_t)(tmpLen) + 1; /* +1 in v1.02 */
        }
        else
        {
            mread(&ph.PattLen, 2, 1, f);
        }

        mread(&ph.DataLen, 2, 1, f);

        if (Song.Ver == 0x0102)
        {
            if (ph.PatternHeaderSize > 8)
                mseek(f, ph.PatternHeaderSize - 8, SEEK_CUR);
        }
        else
        {
            if (ph.PatternHeaderSize > 9)
                mseek(f, ph.PatternHeaderSize - 9, SEEK_CUR);
        }

        if (meof(f))
        {
            mclose(&f);
            return (0);
        }

        PattLens[i] = ph.PattLen;
        if (ph.DataLen)
        {
            Patt[i] = (TonTyp *)(calloc(sizeof (TonTyp), ph.PattLen * MAX_VOICES));
            if (Patt[i] == NULL)
            {
                mclose(&f);
                return (0);
            }

            patttmp = (uint8_t *)(malloc(ph.DataLen));
            if (patttmp == NULL)
            {
                mclose(&f);
                return (0);
            }

            mread(patttmp, ph.DataLen, 1, f);
            UnpackPatt(Patt[i], ph.PattLen, ph.DataLen, patttmp);
            free(patttmp);
        }

        if (PatternEmpty(i))
        {
            if (Patt[i] != NULL)
            {
                free(Patt[i]);
                Patt[i] = NULL;
            }

            PattLens[i] = 64;
        }
    }

    return (1);
}

int8_t ft2play_LoadModule(const uint8_t *moduleData, uint32_t dataLength)
{
    uint16_t i;

    MEM *f;
    SongHeaderTyp h;

    if (ModuleLoaded)
        ft2play_FreeSong();

    ModuleLoaded = 0;

    /* instr 0 is a placeholder for invalid instruments */
    AllocateInstr(0);
    Instr[0]->Samp[0].Vol = 0;
    /* ------------------------------------------------ */

    FreeMusic();
    LinearFrqTab = 0;

    f = mopen(moduleData, dataLength);
    if (f == NULL) return (0);

    /* start loading */
    mread(&h, sizeof (h), 1, f);

    if ((memcmp(h.Sig, "Extended Module: ", 17) != 0) || (h.Ver < 0x0102) || (h.Ver > 0x104))
    {
        mclose(&f);
        return (0);
    }

    if ((h.AntChn < 1) || (h.AntChn > MAX_VOICES) || (h.AntPtn > 256))
    {
        mclose(&f);
        return (0);
    }

    mseek(f, 60 + h.HeaderSize, SEEK_SET);
    if (meof(f))
    {
        mclose(&f);
        return (0);
    }

    Song.Len       = h.Len;
    Song.RepS      = h.RepS;
    Song.AntChn    = (uint8_t)(h.AntChn);
    Song.Speed     = h.DefSpeed ? h.DefSpeed : 125;
    Song.Tempo     = h.DefTempo ? h.DefTempo : 6;
    Song.InitSpeed = Song.Speed;
    Song.InitTempo = Song.Tempo;
    Song.AntInstrs = h.AntInstrs;
    Song.AntPtn    = h.AntPtn;
    Song.Ver       = h.Ver;
    LinearFrqTab   = h.Flags & 1;

    memcpy(Song.SongTab, h.SongTab, 256);

    if (Song.Ver < 0x0104)
    {
        for (i = 1; i <= h.AntInstrs; ++i)
        {
            if (!LoadInstrHeader(f, i))
            {
                FreeAllInstr();
                mclose(&f);
                return (0);
            }
        }

        if (!LoadPatterns(f))
        {
            FreeAllInstr();
            mclose(&f);
            return (0);
        }

        for (i = 1; i <= h.AntInstrs; ++i)
        {
            if (!LoadInstrSample(f, i))
            {
                FreeAllInstr();
                mclose(&f);
                return (0);
            }
        }
    }
    else
    {
        if (!LoadPatterns(f))
        {
            mclose(&f);
            return (0);
        }

        for (i = 1; i <= h.AntInstrs; ++i)
        {
            if (!LoadInstrHeader(f, i))
            {
                FreeInstr((uint8_t)(i));
                mclose(&f);
                break;
            }

            if (!LoadInstrSample(f, i))
            {
                mclose(&f);
                break;
            }
        }
    }

    mclose(&f);

    if (LinearFrqTab)
        Note2Period = linearPeriods;
    else
        Note2Period = amigaPeriods;

    if (Song.RepS > Song.Len) Song.RepS = 0;

    StopVoices();
    SetPos(0, 0);

    ModuleLoaded = 1;

    return (1);
}

void setSamplesPerFrame(uint32_t val)
{
    SpeedVal = val;
}

void voiceSetSource(uint8_t i, const int8_t *sampleData,
    int32_t sampleLength,  int32_t sampleLoopLength,
    int32_t sampleLoopEnd, int8_t loopFlag,
    int8_t sixteenbit, int8_t stereo, int32_t position)
{
    VOICE *v;

    if (i >= MAX_VOICES)
        return;

    v = &voice[i];

    if (sixteenbit)
    {
        sampleLength     /= 2;
        sampleLoopEnd    /= 2;
        sampleLoopLength /= 2;
    }

    if (position >= sampleLength)
    {
        position = 0;
        v->sampleData = NULL;
    }
    else
    {
        v->sampleData = sampleData;
    }

    v->samplePosition = position;
    v->samplePositionFrac = 0.0;
    v->sampleLength = sampleLength;
    v->sampleLoopBegin = sampleLoopEnd - sampleLoopLength;
    v->sampleLoopEnd = sampleLoopEnd;
    v->sampleLoopLength = sampleLoopLength;
    v->loopEnabled = loopFlag ? 1 : 0;
    v->loopBidi = loopFlag ? ((loopFlag & 2) ? 1 : 0) : 0;
    v->sample16bit = sixteenbit ? 1 : 0;
    v->loopingBackwards = 0;

    if (loopFlag && (sampleLoopLength == 0))
    {
        v->loopEnabled = 0;
        v->loopBidi = 0;
    }
}

void voiceSetVolume(uint8_t i, float vol)
{
    VOICE *v;

    float volL_f;
    float volR_f;

    if (i >= MAX_VOICES)
        return;

    v = &voice[i];

    vol = CLAMP(vol, 0.0f, 1.0f);
    v->volume = vol;

    volL_f = vol * v->panL;
    volR_f = vol * v->panR;

    if (volumeRamping)
    {
        v->targetVolL = volL_f;
        v->targetVolR = volR_f;

        v->volDeltaL = (v->targetVolL - v->volumeL) * f_samplesPerFrame005;
        v->volDeltaR = (v->targetVolR - v->volumeR) * f_samplesPerFrame005;
    }
    else
    {
        v->volumeL = volL_f;
        v->volumeR = volR_f;
    }
}

void voiceSetPanning(uint8_t i, float pan)
{
    VOICE *v;

    float volL_f;
    float volR_f;

    if (i >= MAX_VOICES)
        return;

    v = &voice[i];

    pan  = CLAMP(pan, 0.0f, 1.0f);

    v->panL = sqrtf(1.0f - pan);
    v->panR = sqrtf(pan);

    // update volumes

    volL_f = v->volume * v->panL;
    volR_f = v->volume * v->panR;

    if (volumeRamping)
    {
        v->targetVolL = volL_f;
        v->targetVolR = volR_f;

        v->volDeltaL = (v->targetVolL - v->volumeL) * f_samplesPerFrame005;
        v->volDeltaR = (v->targetVolR - v->volumeR) * f_samplesPerFrame005;
    }
    else
    {
        v->volumeL = volL_f;
        v->volumeR = volR_f;
    }
}

void voiceSetSamplingFrequency(uint8_t i, uint32_t samplingFrequency)
{
    if ((i < MAX_VOICES) && (f_audioFreq != 0.0f))
        voice[i].samplePositionDelta = samplingFrequency / f_audioFreq;
}

static void mix8b(uint8_t ch, uint32_t samples)
{
    const int8_t *sampleData;
    int32_t intFrac;
    int32_t samplePosition;
    int32_t samplePosition2;
    uint32_t j;

    float sample;
    float sample2;
    float *bufL;
    float *bufR;

    VOICE *v;

    v = &voice[ch];

    bufL = masterBufferL;
    bufR = masterBufferR;

    for (j = 0; (v->sampleData != NULL) && (v->sampleLength > 0) && (j < samples); ++j)
    {
        samplePosition = v->samplePosition;

        if (samplingInterpolation)
        {
            samplePosition2 = samplePosition + 1;

            if (v->loopEnabled)
            {
                if (v->loopBidi)
                {
                    if (v->loopingBackwards)
                    {
                        samplePosition2 = samplePosition - 1;
                        if (samplePosition2 < v->sampleLoopBegin)
                            samplePosition2 = v->sampleLoopBegin;
                    }
                    else
                    {
                        if (samplePosition2 >= v->sampleLoopEnd)
                            samplePosition2  = v->sampleLoopEnd - 1;
                    }
                }
                else
                {
                    if (samplePosition2 >= v->sampleLoopEnd)
                        samplePosition2  = v->sampleLoopBegin;
                }
            }
            else
            {
                if (samplePosition2 >= v->sampleLength)
                    samplePosition2  = v->sampleLength - 1;
            }


            sampleData = v->sampleData;
            if (sampleData == NULL)
            {
                sample = 0.0f;
            }
            else
            {
                samplePosition  = CLAMP(samplePosition,  0, v->sampleLength - 1);
                samplePosition2 = CLAMP(samplePosition2, 0, v->sampleLength - 1);

                sample  = sampleData[samplePosition];
                sample2 = sampleData[samplePosition2];

                sample = LERP(sample, sample2, v->samplePositionFrac) * (1.0f / 128.0f);
            }
        }
        else
        {
            sampleData = v->sampleData;
            if (sampleData == NULL)
            {
                sample = 0.0f;
            }
            else
            {
                samplePosition = CLAMP(samplePosition,  0, v->sampleLength - 1);

                sample  = sampleData[samplePosition];
                sample *= (1.0f / 128.0f);
            }
        }

        if (volumeRamping)
        {
            v->fader += v->faderDelta;

            if ((v->faderDelta > 0.0f) && (v->fader > v->faderDest))
            {
                v->fader = v->faderDest;
            }
            else if ((v->faderDelta < 0.0f) && (v->fader < v->faderDest))
            {
                v->fader = v->faderDest;

                v->sampleData = NULL;
            }

            sample *= v->fader;
        }

        (*bufL++) += (sample * v->volumeL);
        (*bufR++) += (sample * v->volumeR);

        if (volumeRamping)
        {
            v->volumeL += v->volDeltaL;
            v->volumeR += v->volDeltaR;

                 if ((v->volDeltaL > 0.0f) && (v->volumeL > v->targetVolL)) v->volumeL = v->targetVolL;
            else if ((v->volDeltaL < 0.0f) && (v->volumeL < v->targetVolL)) v->volumeL = v->targetVolL;
                 if ((v->volDeltaR > 0.0f) && (v->volumeR > v->targetVolR)) v->volumeR = v->targetVolR;
            else if ((v->volDeltaR < 0.0f) && (v->volumeR < v->targetVolR)) v->volumeR = v->targetVolR;
        }

        v->samplePositionFrac += v->samplePositionDelta;
        while (v->samplePositionFrac >= 1.0f)
        {
            intFrac = (int32_t)(v->samplePositionFrac);
            v->samplePositionFrac -= intFrac;

            if (v->loopingBackwards)
                v->samplePosition -= intFrac;
            else
                v->samplePosition += intFrac;

            if (v->loopEnabled)
            {
                if (v->loopBidi)
                {
                    if (v->loopingBackwards)
                    {
                        if (v->samplePosition < v->sampleLoopBegin)
                        {
                            v->samplePosition = v->sampleLoopBegin + (v->sampleLoopBegin - v->samplePosition - 1);
                            if (v->samplePosition < 0)
                                v->samplePosition = 0;

                            v->loopingBackwards = 0;
                        }
                    }
                    else
                    {
                        if (v->samplePosition >= v->sampleLoopEnd)
                        {
                            v->samplePosition = v->sampleLoopEnd - (v->samplePosition - v->sampleLoopEnd + 1);
                            v->loopingBackwards = 1;
                        }
                    }
                }
                else
                {
                    if (v->samplePosition >= v->sampleLoopEnd)
                        v->samplePosition  = v->sampleLoopBegin + (v->samplePosition - v->sampleLoopEnd);
                }
            }
            else if (v->samplePosition >= v->sampleLength)
            {
                v->sampleData = NULL;
                break;
            }
        }
    }
}

static void mix8bstereo(uint8_t ch, uint32_t samples)
{
    const int8_t *sampleData;
    int32_t intFrac;
    int32_t samplePosition;
    int32_t samplePosition2;
    uint32_t j;

    float sampleL;
    float sampleR;
    float sampleL2;
    float sampleR2;
    float *bufL;
    float *bufR;

    VOICE *v;

    v = &voice[ch];

    bufL = masterBufferL;
    bufR = masterBufferR;

    for (j = 0; (v->sampleData != NULL) && (v->sampleLength > 0) && (j < samples); ++j)
    {
        samplePosition = v->samplePosition;

        if (samplingInterpolation)
        {
            samplePosition2 = samplePosition + 1;

            if (v->loopEnabled)
            {
                if (v->loopBidi)
                {
                    if (v->loopingBackwards)
                    {
                        samplePosition2 = samplePosition - 1;
                        if (samplePosition2 < v->sampleLoopBegin)
                            samplePosition2 = v->sampleLoopBegin;
                    }
                    else
                    {
                        if (samplePosition2 >= v->sampleLoopEnd)
                            samplePosition2  = v->sampleLoopEnd - 1;
                    }
                }
                else
                {
                    if (samplePosition2 >= v->sampleLoopEnd)
                        samplePosition2  = v->sampleLoopBegin;
                }
            }
            else
            {
                if (samplePosition2 >= v->sampleLength)
                    samplePosition2  = v->sampleLength - 1;
            }

            sampleData = v->sampleData;
            if (sampleData == NULL)
            {
                sampleL = 0.0f;
                sampleR = 0.0f;
            }
            else
            {
                samplePosition  = CLAMP(samplePosition,  0, v->sampleLength - 1);
                samplePosition2 = CLAMP(samplePosition2, 0, v->sampleLength - 1);

                sampleL  = sampleData[samplePosition];
                sampleL2 = sampleData[samplePosition2];
                sampleR  = sampleData[samplePosition  + v->sampleLength];
                sampleR2 = sampleData[samplePosition2 + v->sampleLength];

                sampleL = LERP(sampleL, sampleL2, v->samplePositionFrac) * (1.0f / 128.0f);
                sampleR = LERP(sampleR, sampleR2, v->samplePositionFrac) * (1.0f / 128.0f);
            }
        }
        else
        {
            sampleData = v->sampleData;
            if (sampleData == NULL)
            {
                sampleL = 0.0f;
                sampleR = 0.0f;
            }
            else
            {
                samplePosition = CLAMP(samplePosition,  0, v->sampleLength - 1);

                sampleL = sampleData[samplePosition                  ] * (1.0f / 128.0f);
                sampleR = sampleData[samplePosition + v->sampleLength] * (1.0f / 128.0f);
            }
        }

        if (volumeRamping)
        {
            v->fader += v->faderDelta;

            if ((v->faderDelta > 0.0f) && (v->fader > v->faderDest))
            {
                v->fader = v->faderDest;
            }
            else if ((v->faderDelta < 0.0f) && (v->fader < v->faderDest))
            {
                v->fader = v->faderDest;

                v->sampleData = NULL;
            }

            sampleL *= v->fader;
            sampleR *= v->fader;
        }

        (*bufL++) += (sampleL * v->volumeL);
        (*bufR++) += (sampleR * v->volumeR);

        if (volumeRamping)
        {
            v->volumeL += v->volDeltaL;
            v->volumeR += v->volDeltaR;

                 if ((v->volDeltaL > 0.0f) && (v->volumeL > v->targetVolL)) v->volumeL = v->targetVolL;
            else if ((v->volDeltaL < 0.0f) && (v->volumeL < v->targetVolL)) v->volumeL = v->targetVolL;
                 if ((v->volDeltaR > 0.0f) && (v->volumeR > v->targetVolR)) v->volumeR = v->targetVolR;
            else if ((v->volDeltaR < 0.0f) && (v->volumeR < v->targetVolR)) v->volumeR = v->targetVolR;
        }

        v->samplePositionFrac += v->samplePositionDelta;
        while (v->samplePositionFrac >= 1.0f)
        {
            intFrac = (int32_t)(v->samplePositionFrac);
            v->samplePositionFrac -= intFrac;

            if (v->loopingBackwards)
                v->samplePosition -= intFrac;
            else
                v->samplePosition += intFrac;

            if (v->loopEnabled)
            {
                if (v->loopBidi)
                {
                    if (v->loopingBackwards)
                    {
                        if (v->samplePosition < v->sampleLoopBegin)
                        {
                            v->samplePosition = v->sampleLoopBegin + (v->sampleLoopBegin - v->samplePosition - 1);
                            if (v->samplePosition < 0)
                                v->samplePosition = 0;

                            v->loopingBackwards = 0;
                        }
                    }
                    else
                    {
                        if (v->samplePosition >= v->sampleLoopEnd)
                        {
                            v->samplePosition = v->sampleLoopEnd - (v->samplePosition - v->sampleLoopEnd + 1);
                            v->loopingBackwards = 1;
                        }
                    }
                }
                else
                {
                    if (v->samplePosition >= v->sampleLoopEnd)
                        v->samplePosition  = v->sampleLoopBegin + (v->samplePosition - v->sampleLoopEnd);
                }
            }
            else if (v->samplePosition >= v->sampleLength)
            {
                v->sampleData = NULL;
                break;
            }
        }
    }
}

static void mix16b(uint8_t ch, uint32_t samples)
{
    const int16_t *sampleData;
    int32_t intFrac;
    int32_t samplePosition;
    int32_t samplePosition2;
    uint32_t j;

    float sample;
    float sample2;
    float *bufL;
    float *bufR;

    VOICE *v;

    v = &voice[ch];

    bufL = masterBufferL;
    bufR = masterBufferR;

    for (j = 0; (v->sampleData != NULL) && (v->sampleLength > 0) && (j < samples); ++j)
    {
        samplePosition = v->samplePosition;

        if (samplingInterpolation)
        {
            samplePosition2 = samplePosition + 1;

            if (v->loopEnabled)
            {
                if (v->loopBidi)
                {
                    if (v->loopingBackwards)
                    {
                        samplePosition2 = samplePosition - 1;
                        if (samplePosition2 < v->sampleLoopBegin)
                            samplePosition2 = v->sampleLoopBegin;
                    }
                    else
                    {
                        if (samplePosition2 >= v->sampleLoopEnd)
                            samplePosition2  = v->sampleLoopEnd - 1;
                    }
                }
                else
                {
                    if (samplePosition2 >= v->sampleLoopEnd)
                        samplePosition2  = v->sampleLoopBegin;
                }
            }
            else
            {
                if (samplePosition2 >= v->sampleLength)
                    samplePosition2  = v->sampleLength - 1;
            }

            sampleData = (int16_t *)(v->sampleData);
            if (sampleData == NULL)
            {
                sample = 0.0f;
            }
            else
            {
                samplePosition  = CLAMP(samplePosition,  0, v->sampleLength - 1);
                samplePosition2 = CLAMP(samplePosition2, 0, v->sampleLength - 1);

                sample  = sampleData[samplePosition];
                sample2 = sampleData[samplePosition2];

                sample = LERP(sample, sample2, v->samplePositionFrac) * (1.0f / 32768.0f);
            }
        }
        else
        {
            sampleData = (int16_t *)(v->sampleData);
            if (sampleData == NULL)
            {
                sample = 0.0f;
            }
            else
            {
                samplePosition = CLAMP(samplePosition, 0, v->sampleLength - 1);

                sample = sampleData[samplePosition];
                sample *= (1.0f / 32768.0f);
            }
        }

        if (volumeRamping)
        {
            v->fader += v->faderDelta;

            if ((v->faderDelta > 0.0f) && (v->fader > v->faderDest))
            {
                v->fader = v->faderDest;
            }
            else if ((v->faderDelta < 0.0f) && (v->fader < v->faderDest))
            {
                v->fader = v->faderDest;

                v->sampleData = NULL;
            }

            sample *= v->fader;
        }

        (*bufL++) += (sample * v->volumeL);
        (*bufR++) += (sample * v->volumeR);

        if (volumeRamping)
        {
            v->volumeL += v->volDeltaL;
            v->volumeR += v->volDeltaR;

                 if ((v->volDeltaL > 0.0f) && (v->volumeL > v->targetVolL)) v->volumeL = v->targetVolL;
            else if ((v->volDeltaL < 0.0f) && (v->volumeL < v->targetVolL)) v->volumeL = v->targetVolL;
                 if ((v->volDeltaR > 0.0f) && (v->volumeR > v->targetVolR)) v->volumeR = v->targetVolR;
            else if ((v->volDeltaR < 0.0f) && (v->volumeR < v->targetVolR)) v->volumeR = v->targetVolR;
        }

        v->samplePositionFrac += v->samplePositionDelta;
        while (v->samplePositionFrac >= 1.0f)
        {
            intFrac = (int32_t)(v->samplePositionFrac);
            v->samplePositionFrac -= intFrac;

            if (v->loopingBackwards)
                v->samplePosition -= intFrac;
            else
                v->samplePosition += intFrac;

            if (v->loopEnabled)
            {
                if (v->loopBidi)
                {
                    if (v->loopingBackwards)
                    {
                        if (v->samplePosition <= v->sampleLoopBegin)
                        {
                            v->samplePosition = v->sampleLoopBegin + (v->sampleLoopBegin - v->samplePosition - 1);
                            if (v->samplePosition < 0)
                                v->samplePosition = 0;

                            v->loopingBackwards = 0;
                        }
                    }
                    else
                    {
                        if (v->samplePosition >= v->sampleLoopEnd)
                        {
                            v->samplePosition = v->sampleLoopEnd - (v->samplePosition - v->sampleLoopEnd + 1);
                            v->loopingBackwards = 1;
                        }
                    }
                }
                else
                {
                    if (v->samplePosition >= v->sampleLoopEnd)
                        v->samplePosition  = v->sampleLoopBegin + (v->samplePosition - v->sampleLoopEnd);
                }
            }
            else if (v->samplePosition >= v->sampleLength)
            {
                v->sampleData = NULL;
                break;
            }
        }
    }
}

static void mix16bstereo(uint8_t ch, uint32_t samples)
{
    const int16_t *sampleData;
    int32_t intFrac;
    int32_t samplePosition;
    int32_t samplePosition2;
    uint32_t j;

    float sampleL;
    float sampleR;
    float sampleL2;
    float sampleR2;
    float *bufL;
    float *bufR;

    VOICE *v;

    v = &voice[ch];

    bufL = masterBufferL;
    bufR = masterBufferR;

    for (j = 0; (v->sampleData != NULL) && (v->sampleLength > 0) && (j < samples); ++j)
    {
        samplePosition = v->samplePosition;

        if (samplingInterpolation)
        {
            samplePosition2 = samplePosition + 1;

            if (v->loopEnabled)
            {
                if (v->loopBidi)
                {
                    if (v->loopingBackwards)
                    {
                        samplePosition2 = samplePosition - 1;
                        if (samplePosition2 < v->sampleLoopBegin)
                            samplePosition2 = v->sampleLoopBegin;
                    }
                    else
                    {
                        if (samplePosition2 >= v->sampleLoopEnd)
                            samplePosition2  = v->sampleLoopEnd - 1;
                    }
                }
                else
                {
                    if (samplePosition2 >= v->sampleLoopEnd)
                        samplePosition2  = v->sampleLoopBegin;
                }
            }
            else
            {
                if (samplePosition2 >= v->sampleLength)
                    samplePosition2  = v->sampleLength - 1;
            }

            sampleData = (int16_t *)(v->sampleData);
            if (sampleData == NULL)
            {
                sampleL = 0.0f;
                sampleR = 0.0f;
            }
            else
            {
                samplePosition  = CLAMP(samplePosition,  0, v->sampleLength - 1);
                samplePosition2 = CLAMP(samplePosition2, 0, v->sampleLength - 1);

                sampleL  = sampleData[samplePosition];
                sampleL2 = sampleData[samplePosition2];
                sampleR  = sampleData[samplePosition  + v->sampleLength];
                sampleR2 = sampleData[samplePosition2 + v->sampleLength];

                sampleL = LERP(sampleL, sampleL2, v->samplePositionFrac) * (1.0f / 32768.0f);
                sampleR = LERP(sampleR, sampleR2, v->samplePositionFrac) * (1.0f / 32768.0f);
            }
        }
        else
        {
            sampleData = (int16_t *)(v->sampleData);
            if (sampleData == NULL)
            {
                sampleL = 0.0f;
                sampleR = 0.0f;
            }
            else
            {
                samplePosition  = CLAMP(samplePosition,  0, v->sampleLength - 1);

                sampleL = sampleData[samplePosition                  ] * (1.0f / 32768.0f);
                sampleR = sampleData[samplePosition + v->sampleLength] * (1.0f / 32768.0f);
            }
        }

        if (volumeRamping)
        {
            v->fader += v->faderDelta;

            if ((v->faderDelta > 0.0f) && (v->fader > v->faderDest))
            {
                v->fader = v->faderDest;
            }
            else if ((v->faderDelta < 0.0f) && (v->fader < v->faderDest))
            {
                v->fader = v->faderDest;

                v->sampleData = NULL;
            }

            sampleL *= v->fader;
            sampleR *= v->fader;
        }

        (*bufL++) += (sampleL * v->volumeL);
        (*bufR++) += (sampleR * v->volumeR);

        if (volumeRamping)
        {
            v->volumeL += v->volDeltaL;
            v->volumeR += v->volDeltaR;

                 if ((v->volDeltaL > 0.0f) && (v->volumeL > v->targetVolL)) v->volumeL = v->targetVolL;
            else if ((v->volDeltaL < 0.0f) && (v->volumeL < v->targetVolL)) v->volumeL = v->targetVolL;
                 if ((v->volDeltaR > 0.0f) && (v->volumeR > v->targetVolR)) v->volumeR = v->targetVolR;
            else if ((v->volDeltaR < 0.0f) && (v->volumeR < v->targetVolR)) v->volumeR = v->targetVolR;
        }

        v->samplePositionFrac += v->samplePositionDelta;
        while (v->samplePositionFrac >= 1.0f)
        {
            intFrac = (int32_t)(v->samplePositionFrac);
            v->samplePositionFrac -= intFrac;

            if (v->loopingBackwards)
                v->samplePosition -= intFrac;
            else
                v->samplePosition += intFrac;

            if (v->loopEnabled)
            {
                if (v->loopBidi)
                {
                    if (v->loopingBackwards)
                    {
                        if (v->samplePosition <= v->sampleLoopBegin)
                        {
                            v->samplePosition = v->sampleLoopBegin + (v->sampleLoopBegin - v->samplePosition - 1);
                            if (v->samplePosition < 0)
                                v->samplePosition = 0;

                            v->loopingBackwards = 0;
                        }
                    }
                    else
                    {
                        if (v->samplePosition >= v->sampleLoopEnd)
                        {
                            v->samplePosition = v->sampleLoopEnd - (v->samplePosition - v->sampleLoopEnd + 1);
                            v->loopingBackwards = 1;
                        }
                    }
                }
                else
                {
                    if (v->samplePosition >= v->sampleLoopEnd)
                        v->samplePosition  = v->sampleLoopBegin + (v->samplePosition - v->sampleLoopEnd);
                }
            }
            else if (v->samplePosition >= v->sampleLength)
            {
                v->sampleData = NULL;
                break;
            }
        }
    }
}

static void mixChannel(uint8_t i, uint32_t sampleBlockLength)
{
    VOICE *v;

    v = &voice[i];

    if (v->samplePositionDelta <= 0.0f)
        return;

    if (v->stereo)
    {
        if (v->sample16bit)
            mix16bstereo(i, sampleBlockLength);
        else
            mix8bstereo(i, sampleBlockLength);
    }
    else
    {
        if (v->sample16bit)
            mix16b(i, sampleBlockLength);
        else
            mix8b(i, sampleBlockLength);
    }
}

static void PMPMix32Proc(int16_t *outputStream, uint32_t sampleBlockLength)
{
    // custom non-FT2 mixer

    int16_t *streamPointer;
    uint8_t i;
    uint32_t j;

    float outL;
    float outR;

    memset(masterBufferL, 0, sampleBlockLength * sizeof (float));
    memset(masterBufferR, 0, sampleBlockLength * sizeof (float));

    for (i = 0; i < MAX_VOICES; ++i)
    {
        mixChannel(i, sampleBlockLength);
        if (volumeRamping)
            mixChannel(i + SPARE_OFFSET, sampleBlockLength);
    }

    streamPointer = outputStream;
    for (j = 0; j < sampleBlockLength; ++j)
    {
        if (MusicPaused)
        {
            *streamPointer++ = 0;
            *streamPointer++ = 0;
        }
        else
        {
            outL = masterBufferL[j] * (32768.0f / 3.0f);
            outR = masterBufferR[j] * (32768.0f / 3.0f);

            *streamPointer++ = (int16_t)(CLAMP(outL, -32768.0f, 32767.0f));
            *streamPointer++ = (int16_t)(CLAMP(outR, -32768.0f, 32767.0f));
        }
    }
}

void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    int16_t *stream;
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
            memcpy(waveBlockHeader->lpData, MixingBuffer, soundBufferSize);

            waveOutPrepareHeader(hWaveOut, waveBlockHeader, sizeof (WAVEHDR));
            waveOutWrite(hWaveOut, waveBlockHeader, sizeof (WAVEHDR));

            stream = (int16_t *)(MixingBuffer);

            sampleBlock = soundBufferSize / 4;
            while (sampleBlock > 0)
            {
                samplesTodo = (sampleBlock < sampleCounter) ? sampleBlock : sampleCounter;
                if (samplesTodo == 0)
                {
                    MainPlayer();
                    Mix_UpdateChannelVolPanFrq();

                    sampleCounter = SpeedVal;
                }
                else
                {
                    PMPMix32Proc(stream, samplesTodo);
                    stream += (samplesTodo * 2);

                    sampleBlock   -= samplesTodo;
                    sampleCounter -= samplesTodo;
                }
            }
        }

        mixingMutex = 0;
    }
}

int8_t openMixer(uint32_t _outputFrequency, uint32_t _soundBufferSize, uint8_t _interpolation, uint8_t _volumeRamping)
{
    uint32_t i = 0;
    MMRESULT r = 0;

    if (!_hWaveOut)
    {
        audioFreq       = _outputFrequency;
        f_audioFreq     = (float)(audioFreq);
        soundBufferSize = _soundBufferSize;

        if (f_audioFreq > 0.0f)
        {
            f_samplesPerFrame010 = 1.0f / (f_audioFreq * 0.010f);
            f_samplesPerFrame005 = 1.0f / (f_audioFreq * 0.005f);
        }

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

        r = waveOutOpen(&_hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)(waveOutProc), (DWORD_PTR)(NULL), CALLBACK_FUNCTION);
        if (r != MMSYSERR_NOERROR)
            return (0);

        for (i = 0; i < MIX_BUF_NUM; ++i)
        { 
            waveBlocks[i].dwBufferLength = soundBufferSize;

            waveBlocks[i].lpData = (LPSTR)(calloc(soundBufferSize, 1));
            if (waveBlocks[i].lpData == NULL)
                return (0); /* gets free'd later */

            waveOutPrepareHeader(_hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
            waveOutWrite(_hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));
        }

        samplingInterpolation = _interpolation;
        volumeRamping = _volumeRamping;

        MixingBuffer = (int8_t *)(calloc(soundBufferSize, 1));
        if (MixingBuffer == NULL)
            return (0); /* gets free'd later */

        sampleCounter = 0;
        setSamplesPerFrame(((audioFreq * 5) / 2 / 125));

        isMixing = 1;
        return (1);
    }

    return (0);
}

void closeMixer(void)
{
    uint32_t i;

    if (isMixing)
    {
        isMixing = 0;
        while (mixingMutex);

        if (_hWaveOut)
        {
            for (i = 0; i < MIX_BUF_NUM; ++i)
                waveOutUnprepareHeader(_hWaveOut, &waveBlocks[i], sizeof (WAVEHDR));

            for (i = 0; i < MIX_BUF_NUM; ++i)
            {
                while (waveBlocks[i].dwFlags & WHDR_PREPARED);
                if (waveBlocks[i].lpData != NULL)
                {
                    free(waveBlocks[i].lpData);
                    waveBlocks[i].lpData = NULL;
                }
            }

            waveOutReset(_hWaveOut);
            waveOutClose(_hWaveOut);

            _hWaveOut = 0;

            if (MixingBuffer != NULL)
            {
                free(MixingBuffer);
                MixingBuffer = NULL;
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

void ft2play_Close(void)
{
    Playing     = 0;
    MusicPaused = 1;

    while (mixingMutex);

    if (LogTab != NULL)
    {
        free(LogTab);
        LogTab = NULL;
    }

    if (VibSineTab != NULL)
    {
        free(VibSineTab);
        VibSineTab = NULL;
    }

    if (amigaPeriods != NULL)
    {
        free(amigaPeriods);
        amigaPeriods = NULL;
    }

    if (linearPeriods != NULL)
    {
        free(linearPeriods);
        linearPeriods = NULL;
    }

    if (NilPatternLine != NULL)
    {
        free(NilPatternLine);
        NilPatternLine = NULL;
    }

    closeMixer();
}

int8_t ft2play_Init(uint32_t outputFreq, int8_t lerpMixFlag, int8_t volRampFlag)
{
    uint8_t j;
    uint16_t i;
    int16_t noteVal;
    uint16_t noteIndex;

    /* allocate memory for pointers */

    if (NilPatternLine == NULL)
        NilPatternLine = (TonTyp *)(calloc(sizeof (TonTyp), 256 * MAX_VOICES));

    if (linearPeriods == NULL)
        linearPeriods = (int16_t *)(malloc(sizeof (int16_t) * ((12 * 10 * 16) + 16)));

    if (amigaPeriods == NULL)
        amigaPeriods = (int16_t *)(malloc(sizeof (int16_t) * ((12 * 10 * 16) + 16)));

    if (VibSineTab == NULL)
        VibSineTab = (int8_t *)(malloc(256));

    if (LogTab == NULL)
        LogTab = (uint32_t *)(malloc(sizeof (uint32_t) * 768));

    if ((NilPatternLine == NULL) || (linearPeriods == NULL) || (amigaPeriods == NULL) ||
        (VibSineTab     == NULL) || (LogTab        == NULL))
    {
        ft2play_Close();
        return (0);
    }

    /* generate tables */

    /* generate log table (value-exact to FT2's table) */
    for (i = 0; i < 768; ++i)
        LogTab[i] = (uint32_t)(floor(((256.0 * 8363.0) * exp((double)(i) / 768.0 * log(2.0))) + 0.5));

    /* generate linear table (value-exact to FT2's table) */
    for (i = 0; i < ((12 * 10 * 16) + 16); ++i)
        linearPeriods[i] = (((12 * 10 * 16) + 16) * 4) - (i * 4);

    /* generate amiga period table (value-exact to FT2's table, except for last 17 entries) */
    noteIndex = 0;
    for (i = 0; i < 10; ++i)
    {
        for (j = 0; j < ((i == 9) ? (96 + 8) : 96); ++j)
        {
            noteVal = ((AmigaFinePeriod[j % 96] * 64) + (-1 + (1 << i))) >> (i + 1);
            /* NON-FT2: j % 96. added for safety. we're patching the values later anyways. */

            amigaPeriods[noteIndex++] = noteVal;
            amigaPeriods[noteIndex++] = noteVal;
        }
    }

    /* interpolate between points (end-result is exact to FT2's table, except for last 17 entries) */
    for (i = 0; i < (12 * 10 * 8) + 7; ++i)
        amigaPeriods[(i * 2) + 1] = (amigaPeriods[i * 2] + amigaPeriods[(i * 2) + 2]) / 2;

    /*
    ** the amiga linear period table has its 17 last entries generated wrongly.
    ** the content seem to be garbage because of an 'out of boundaries' read from AmigaFinePeriods.
    ** these 17 values were taken from a memdump of FT2 in DOSBox.
    ** they might change depending on what you ran before FT2, but let's not make it too complicated.
    */

    amigaPeriods[1919] = 22; amigaPeriods[1920] = 16; amigaPeriods[1921] =  8; amigaPeriods[1922] =  0;
    amigaPeriods[1923] = 16; amigaPeriods[1924] = 32; amigaPeriods[1925] = 24; amigaPeriods[1926] = 16;
    amigaPeriods[1927] =  8; amigaPeriods[1928] =  0; amigaPeriods[1929] = 16; amigaPeriods[1930] = 32;
    amigaPeriods[1931] = 24; amigaPeriods[1932] = 16; amigaPeriods[1933] =  8; amigaPeriods[1934] =  0;
    amigaPeriods[1935] =  0;

    /* generate auto-vibrato table (value-exact to FT2's table) */
    for (i = 0; i < 256; ++i)
        VibSineTab[i] = (int8_t)(floor((64.0 * sin(((double)(-i) * (2.0 * M_PI)) / 256.0)) + 0.5));

    if (!openMixer(outputFreq, MIX_BUF_LEN, lerpMixFlag, volRampFlag))
    {
        ft2play_Close();
        return (0);
    }

    return (1);
}

void ft2play_FreeSong(void)
{
    Playing     = 0;
    MusicPaused = 1;

    memset(voice, 0, sizeof (voice));

    while (mixingMutex);

    FreeMusic();

    ModuleLoaded = 0;
}

void ft2play_PauseSong(int8_t pause)
{
    MusicPaused = pause;
}

void ft2play_PlaySong(void)
{
    if (!ModuleLoaded) return;

    StopVoices();

    Song.GlobVol = 64;

    if (Song.Speed == 0)
        Song.Speed = 125;

    setSamplesPerFrame(((audioFreq * 5) / 2 / Song.Speed));
    if (MusicPaused)
        MusicPaused = 0;

    Playing  = 1;
    isMixing = 1;
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

    pcnt = ((uint32_t)(buf->_cnt) > wrcnt) ? wrcnt : buf->_cnt;
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
    if (buf == NULL) return (1);
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

/* EOF */