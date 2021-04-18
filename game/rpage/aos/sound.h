#ifdef LATTICE
// EasySound.h       
//                   
// V2.00 1990-0-23   
//                   
// AMIGA C CLUB (ACC)
// Anders Bjerin     
// Tulevagen 22      
// 181 41  LIDINGO   
// SWEDEN            

#ifndef SOUND_ROUTINES
#define SOUND_ROUTINES
#define GLOBAL_ENABLE_AUDIO
#define SOUND_MAX_INFO_POOL 8

/* Sound channels: */
#define LEFT0         0
#define RIGHT0        1
#define RIGHT1        2
#define LEFT1         3

#define NONSTOP       0
#define ONCE          1
#define MAXVOLUME    64
#define MINVOLUME     0
#define NORMALRATE    0

/* Structure containing all necessary information about the sound: */
typedef struct
{
	BYTE *SoundBuffer; /* WaveForm Buffers */
	UWORD RecordRate;	 /* Record Rate */
	ULONG FileLength;	 /* WaveForm Lengths */
} SoundInfo;

typedef LONG Fixed;

typedef struct
{
	ULONG oneShotHiSamples;	 /* #samples in the high octave 1-shot part */
	ULONG repeatHiSamples;	 /* #samples in the high octave repeat part */
	ULONG samplesPerHiCycle; /* #samples/cycle in high octave, else 0   */
	UWORD samplesPerSec;		 /* Data sampling rate */
	UBYTE ctOctave;					 /* Number of octaves of waveforms */
	UBYTE sCompression;			 /* Data compression technique used */
	Fixed volume;						 /* Playback volume from 0 to 0x10000 */
} Voice8Header;

BOOL PrepareIOA(UWORD period, UWORD volume, UWORD cycles, UBYTE channel, SoundInfo *info);
struct IOAudio *SoundIOAllocate(void);
void SoundIOAFree(struct IOAudio *info);

void SoundInit(void);
extern BOOL PlaySound(SoundInfo *info, UWORD volume, UBYTE channel, WORD delta_rate, UWORD repeat);
extern void StopSound(UBYTE channel);
extern void RemoveSound(SoundInfo *info);
SoundInfo *SoundInfoAllocate(void);
extern void SoundInfoFree(SoundInfo *info);
extern SoundInfo *LoadPackedSound(char *filename, BYTE *packed_block, /* BYTE *encoded_block, */ BYTE *unpacked_block);
// void adpcm_decoder_init(void);
BYTE *adpcm_decode(UBYTE *input, int numSamples, BYTE *output);

#endif
#endif