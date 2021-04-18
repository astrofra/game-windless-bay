#ifdef LATTICE
/*   
		 EASY-SOUND   V2.00   1990-09-23   ANDERS BJERIN
		 ADPCM Decoder by Kalms
*/

/* Include some important header files: */
#include "rpage/aos/inc.prl"
#include <exec/types.h>
#include <exec/memory.h>
#include <devices/audio.h>
#include <stdio.h>
#include <string.h>

#include "ext/tinfl.h"
#include "ext/aos/shrinkler.h"
#include "ext/aos/nrv2.h"

#include "rpage/aos/sound.h"
#include "rpage/err.h"
#include "rpage/aos/adpcm.h"
#include "rpage/aos/debug.h"
#include "rpage/utils.h"

#define CLOCK_CONSTANT 3579545
#define MUSIC_PRIORITY 0

extern struct DosLibrary *DOSBase;

/* An IOAudio pointer to each sound channel: */
struct IOAudio *IOA[4] = {NULL, NULL, NULL, NULL};
SoundInfo sound_info_pool[SOUND_MAX_INFO_POOL];
BOOL sound_info_is_allocated[SOUND_MAX_INFO_POOL];
struct IOAudio sound_ioa_pool[SOUND_MAX_INFO_POOL];
BOOL sound_ioa_is_allocated[SOUND_MAX_INFO_POOL];

void SoundInit(void)
{
	short i;
	for(i = 0; i < SOUND_MAX_INFO_POOL; i++)
	{
		sound_ioa_is_allocated[i] = FALSE;
		sound_info_is_allocated[i] = FALSE;
	}
}

struct IOAudio *SoundIOAllocate(void)
{
	short i;
	for(i = 0; i < SOUND_MAX_INFO_POOL; i++)
	{
		if (!sound_ioa_is_allocated[i])
		{
			// printf("SoundInfoAllocate(%d)\n", i);
			sound_ioa_is_allocated[i] = TRUE;
			return &sound_ioa_pool[i];
		}
	}

	// printf("SoundInfoAllocate(FAILED)\n");
	return NULL;
}

void SoundIOAFree(struct IOAudio *info)
{
#ifdef GLOBAL_ENABLE_AUDIO	
	/* IMPORTANT! The sound must have been */
	/* stopped before you may remove it!!! */
	/* Have we allocated a SoundInfo structure? */
	if (info != NULL)
	{
		short i;
		// /* Deallocate the SoundInfo structure: */
		// free(info);
		for(i = 0; i < SOUND_MAX_INFO_POOL; i++)
		{
			if (sound_ioa_is_allocated[i] && info == &sound_ioa_pool[i])
			{
				// printf("SoundInfoFree(%d)\n", i);
				sound_ioa_is_allocated[i] = FALSE;
				return;
			}
		}

	}
#endif
}

SoundInfo *SoundInfoAllocate(void)
{
	// return (SoundInfo *)rpage_c_alloc(1, sizeof(SoundInfo));
	short i;
	for(i = 0; i < SOUND_MAX_INFO_POOL; i++)
	{
		if (!sound_info_is_allocated[i])
		{
			// printf("SoundInfoAllocate(%d)\n", i);
			sound_info_is_allocated[i] = TRUE;
			return &sound_info_pool[i];
		}
	}

	// printf("SoundInfoAllocate(FAILED)\n");
	return NULL;
}

void SoundInfoFree(SoundInfo *info)
{
#ifdef GLOBAL_ENABLE_AUDIO	
	/* IMPORTANT! The sound must have been */
	/* stopped before you may remove it!!! */
	/* Have we allocated a SoundInfo structure? */
	if (info != NULL)
	{
		short i;
		// /* Deallocate the SoundInfo structure: */
		// free(info);
		for(i = 0; i < SOUND_MAX_INFO_POOL; i++)
		{
			if (sound_info_is_allocated[i] && info == &sound_info_pool[i])
			{
				// printf("SoundInfoFree(%d)\n", i);
				sound_info_is_allocated[i] = FALSE;
				return;
			}
		}

	}
#endif
}

/// Load a packer sound file. <br>
/// The sound is encoded either in raw or mdpcm. <br>
/// The file is compressed either in tinfl, doynamite68k or shrinkler.
/// File structure:
/// SMPK (4 bytes) 'Sample Packer' Header
/// SIZE (4 bytes) 'Size information block' header
/// ____ (4 bytes) Size of the original uncompressed & decoded sample
/// ADPC|GLI2|8SVX (4 bytes) Encoder type. ADPC = ADPCM, GLI2 = MDPCM, 8SVX = Amiga 8bits IFF sample.
/// SIZE (4 bytes) 'Size information block' header
/// ____ (4 bytes) Size of the encoded sample
/// MINZ|SHRK|D68K (4 bytes) Compressor type. MINZ = Tinfl Miniz, SHRK = Shrinkler, D68K = Doynamite68k
/// SIZE (4 bytes) 'Size information block' header
/// ____ (4 bytes) Size of the compressed block
SoundInfo *LoadPackedSound(char *filename, BYTE *packed_block, /*BYTE *encoded_block, */ BYTE *unpacked_block)
{
#ifdef GLOBAL_ENABLE_AUDIO	
	ULONG encoded_block_size = 0, unpacked_block_size = 0, packed_block_size = 0;
	ULONG frequency = 8000;
	BPTR fileHandle;
	char encoder_tag[4], compressor_tag[4], tag[4];
	UWORD mod_size;
	// BYTE *encoded_block = NULL;
	SoundInfo *sound;

	if ((fileHandle = Open(filename, MODE_OLDFILE)))
	{
		Read(fileHandle, &tag, 4);
		if (strncmp(tag, "SMPK", 4) == 0)
		{
			// read the sample length
			Read(fileHandle, &tag, 4); // SIZE
			if (strncmp(tag, "SIZE", 4) == 0)
			{
				Read(fileHandle, &unpacked_block_size, 4);
				// read the replay frequency
				Read(fileHandle, &tag, 4); // FREQ
				Read(fileHandle, &frequency, 4);

				// read the encoder name
				Read(fileHandle, &encoder_tag, 4);

				Read(fileHandle, &tag, 4);
				if (strncmp(tag, "SIZE", 4) == 0)
				{
					Read(fileHandle, &encoded_block_size, 4);

					// read the compressor name
					Read(fileHandle, &compressor_tag, 4);
					Read(fileHandle, &tag, 4);
					if (strncmp(tag, "SIZE", 4) == 0)
					{
						Read(fileHandle, &packed_block_size, 4);
						Read(fileHandle, packed_block, packed_block_size);
					}
					else
						printf(err_no_size_found); // compressor
				}
				else
					printf(err_no_size_found); // encoder
			}
			else
				printf(err_no_size_found); // original sample

			if ((encoded_block_size > 0) && (unpacked_block_size > 0) && (packed_block_size > 0))
			{
				// printf("unpacked_block = %x\n", unpacked_block);
				if (unpacked_block == NULL)
					unpacked_block = rpage_os_alloc(unpacked_block_size, MEMF_CHIP);

				// encoded_block = (UBYTE *)rpage_c_alloc(encoded_block_size, sizeof(UBYTE));

				if (unpacked_block != NULL)
				{
					// printf("unpacked_block (post malloc) = %x\n", unpacked_block);

					// printf("compressor_tag = %c%c%c%c\n", compressor_tag[0], compressor_tag[1], compressor_tag[2], compressor_tag[3]);
					if (strncmp(compressor_tag, "MINZ", 4) == 0)
						tinfl_decompress_mem_to_mem(unpacked_block, encoded_block_size, packed_block, packed_block_size, 1);
					else if (strncmp(compressor_tag, "SHRK", 4) == 0)
						ShrinklerDecompress(packed_block, unpacked_block, NULL, NULL);
					else if (strncmp(compressor_tag, "NRV2", 4) == 0)
						nrv2s_unpack(packed_block, unpacked_block);

					// if (strncmp(encoder_tag, "ADPC", 4) == 0)
					// {
					// 	adpcm_decode(encoded_block, encoded_block_size, unpacked_block);
					// }
					// else if (strncmp(encoder_tag, "GLI2", 4) == 0)
					// {
					// 	printf("Gligli MDPCM not supported yet!\n");
					// }
					// else if (strncmp(encoder_tag, "8SVX", 4) == 0)
					// {
					// 	memcpy(unpacked_block, encoded_block, unpacked_block_size);
					// }

					sound = SoundInfoAllocate();
					sound->SoundBuffer = unpacked_block;
					sound->FileLength = unpacked_block_size & ~1; // make sure the length is even
					sound->RecordRate = frequency;

					// free(encoded_block);
					// encoded_block = NULL;

					Close(fileHandle);
					return sound;
				}
				else
				{
					printf("LoadPackedSound(), unpacked_block rpage_c_alloc() failed!\n");
					Close(fileHandle);
					return NULL;					
				}
			}
		}
		else
		{
			printf("!Not a SMPK File!\n");
		}

		Close(fileHandle);
	}

	return NULL;
#else
	return NULL;
#endif		
}

/* PlaySound()                                                          */
/* PlaySound() plays one already prepared sound effect. You can decide  */
/* what volume, which channel should, what rate, and how many times the */
/* sound should be played.                                              */
/*                                                                      */
/* Synopsis: ok = PlaySound( pointer, volume, channel, drate, times );  */
/* ok:       (BOOL) If the sound was played successfully TRUE is        */
/*           returned, else FALSE.                                      */
/* pointer:  (CPTR) Actually a pointer to a SoundInfo structure. This   */
/*           pointer was returned by PrepareSound().                    */
/* volume:   (UWORD) Volume, 0 to 64.                                   */
/* channel:  (UBYTE) Which channel should be used. (LEFT0, RIGHT0,      */
/*           RIGHT1 or LEFT1)                                           */
/* drate:    (WORD) Delta rate. When the sound is prepared, the record  */
/*           rate is automatically stored in the SoundInfo structure,   */
/*           so if you do not want to change the rate, write 0.         */
/* times:    (UWORD) How many times the sound should be played. If you  */
/*           want to play the sound forever, write 0. (To stop a sound  */
/*           call the function StopSound().)                            */

BOOL PlaySound(SoundInfo *info, UWORD volume, UBYTE channel,
							 WORD delta_rate, UWORD repeat)
{
#ifdef GLOBAL_ENABLE_AUDIO
	if (info)
	{
		/* Before we may play the sound, we must make sure that the sound is */
		/* not already being played. We will therefore call the function     */
		/* StopSound(), in order to stop the sound if it is playing:         */
		StopSound(channel);

		/* Call the PrepareIOA() function that will declare and initialize an */
		/* IOAudio structure:                                                 */
		if (PrepareIOA(CLOCK_CONSTANT / info->RecordRate + delta_rate, volume, repeat,
									channel, info))
		{
			/* We will now start playing the sound: */
			BeginIO((struct IORequest *)IOA[channel]);
			return TRUE; /* OK! */
		}
		else
		{
			return FALSE; /* ERROR! */
		}
	}	
#else
	return FALSE;
#endif
}

/* StopSound()                                                         */
/* StopSound() will stop the specified audio channel from continuing   */
/* to play the sound. It will also close all devices and ports that    */
/* have been opened, and deallocate some memory that have been         */
/* allocated.                                                          */
/*                                                                     */
/* Synopsis: StopSound( channel );                                     */
/* channel:  (UBYTE) The audio channel that should be stopped. (LEFT0, */
/*           LEFT1, RIGHT0 or RIGHT1.)                                 */

void StopSound(UBYTE channel)
{
#ifdef GLOBAL_ENABLE_AUDIO	
	/* Check if the IOAudio structure exist: */
	if (IOA[channel])
	{
		/* 1. Stop the sound: */
		AbortIO((struct IORequest *)IOA[channel]);

		/* 2. If there exist a Sound Device, close it: */
		if (IOA[channel]->ioa_Request.io_Device)
			CloseDevice((struct IORequest *)IOA[channel]);

		/* 3. If there exist a Message Port, delete it: */
		if (IOA[channel]->ioa_Request.io_Message.mn_ReplyPort)
			DeletePort(IOA[channel]->ioa_Request.io_Message.mn_ReplyPort);

		if (IOA[channel] != NULL)
		{
			// free(IOA[channel]);
			SoundIOAFree(IOA[channel]);
			IOA[channel] = NULL;
		}
	}
#endif
}

/* RemoveSound()                                                        */
/* RemoveSound() will stop playing the sound, and deallocate all memory */
/* that was allocated by the PrepareSound() function. Before your       */
/* program terminates, all sound that has been prepared, MUST be        */
/* removed.                                                             */
/*                                                                      */
/* IMPORTANT! The each channel that is currently playing the sound must */
/* be stopped! (Use the StopSound() function.)                          */

/* Synopsis: RemoveSound( pointer );                                    */
/* pointer:  (CPTR) Actually a pointer to a SoundInfo structure.        */

void RemoveSound(SoundInfo *info)
{
#ifdef GLOBAL_ENABLE_AUDIO		
	/* IMPORTANT! The sound must have been */
	/* stopped before you may remove it!!! */

	/* Have we allocated a SoundInfo structure? */
	if (info != NULL)
	{
		/* Deallocate the sound buffer: */
		if (info->SoundBuffer != NULL)
		{
			FreeMem(info->SoundBuffer, info->FileLength);
			info->SoundBuffer = NULL;
		}

		/* Deallocate the SoundInfo structure: */
		SoundInfoFree(info);
	}
#endif
}

/* PrepareIOA()                                                           */
/* PrepareIOA() allocates and initializes an IOAudio structure.           */
/*                                                                        */
/* Synopsis: ok = PrepareIOA( period, volume, cycles, channel, pointer ); */
/*                                                                        */
/* ok:       (BOOL) If the IOAudio structure was allocated and            */
/*           initialized successfully, TRUE is returned, else FALSE.      */
/* period:   (UWORD) Period time.                                         */
/* volume:   (UWORD) Volume, 0 to 64.                                     */
/* cycles:   (UWORD) How many times the sound should be played.           */
/*           (0 : forever)                                                */
/* channel:  (UBYTE) Which channel should be used. (LEFT0, RIGHT0,        */
/*           RIGHT1 or LEFT1)                                             */
/* pointer:  (CPTR) Actually a pointer to a SoundInfo structure.          */

BOOL PrepareIOA(UWORD period, UWORD volume, UWORD cycles, UBYTE channel, SoundInfo *info)
{
#ifdef GLOBAL_ENABLE_AUDIO	
	UBYTE ch;

	/* Declare a pointer to a MsgPort structure: */
	struct MsgPort *port;

	/* Allocate space for an IOAudio structure: */
	// IOA[channel] = (struct IOAudio *)rpage_c_alloc(1, sizeof(struct IOAudio));
	IOA[channel] = SoundIOAllocate();

	/* Could we allocate enough memory? */
	if (IOA[channel])
	{
		/* Create Message port: */
		if ((port = (struct MsgPort *)CreatePort("Sound Port", 0)) == NULL)
		{
			/* ERROR! Could not create message port! */
			/* Deallocate the IOAudio structure: */
			if (IOA[channel] != NULL)
			{
				// free(IOA[channel]);
				SoundIOAFree(IOA[channel]);
				IOA[channel] = NULL;
			}

			return (FALSE); /* ERROR! */
		}
		else
		{
			/* Port created successfully! */
			/* Initialize the IOAudion structure: */

			/* Priority: */
			IOA[channel]->ioa_Request.io_Message.mn_Node.ln_Pri = MUSIC_PRIORITY;

			/* Port: */
			IOA[channel]->ioa_Request.io_Message.mn_ReplyPort = port;

			/* Channel: */
			ch = 1 << channel;
			IOA[channel]->ioa_Data = &ch;

			/* Length: */
			IOA[channel]->ioa_Length = sizeof(UBYTE);

			/* Open Audio Device: */
			if (OpenDevice(AUDIONAME, 0, (struct IORequest *)IOA[channel], 0))
			{
				/* ERROR! Could not open the device! */
				/* Delete Sound Port: */
				DeletePort(port);

				/* Deallocate the IOAudio structure: */
				if (IOA[channel] != NULL)
				{
					// free(IOA[channel]);
					SoundIOAFree(IOA[channel]);
					IOA[channel] = NULL;
				}

				return (FALSE); /* ERROR! */
			}
			else
			{
				/* Device opened successfully! */
				/* Initialize the rest of the IOAudio structure: */
				IOA[channel]->ioa_Request.io_Flags = ADIOF_PERVOL;
				IOA[channel]->ioa_Request.io_Command = CMD_WRITE;
				IOA[channel]->ioa_Period = period;
				IOA[channel]->ioa_Volume = volume;
				IOA[channel]->ioa_Cycles = cycles;

				/* The Audio Chip can of some strange reason not play sampled  */
				/* sound that is longer than 131KB. So if the sound is too long, */
				/* we simply cut it off:                                        */
				if (info->FileLength > 131000)
					IOA[channel]->ioa_Length = 131000;
				else
					IOA[channel]->ioa_Length = info->FileLength;

				// printf("PrepareIOA() ioa_Length = %d\n", IOA[ channel ]->ioa_Length);

				IOA[channel]->ioa_Data = info->SoundBuffer;

				return (TRUE); /* OK! */
			}
		}
	}
	return FALSE; /* ERROR! */
#else
	return FALSE;
#endif	
}

// void adpcm_decode(CodecState *state, UBYTE *input, int numSamples, UBYTE *output)
BYTE *adpcm_decode(UBYTE *Source, int Length, BYTE *Destination)
{
// #ifdef GLOBAL_ENABLE_AUDIO
// 	const ULONG JoinCode = 0;
// 	WORD EstMax = (WORD)(JoinCode & 0xffff);
// 	UWORD Delta = (UWORD)((JoinCode & 0xffff0000) >> 16);
// 	ULONG lDelta = 0;
// 	const UBYTE Bits = 4;
	
// 	if(!Delta) Delta = 5;
	
// 	Length /= 3;

// 	while(Length--) {
// 		UBYTE sampleCount = 24/Bits;
// 		ULONG temp = (Source[0] << 16) | (Source[1] << 8) | Source[2];
// 		Source+=3;

// 		while(sampleCount--) {
// 			WORD newEstMax = (Delta >> 1);
// 			UBYTE Shifter  = (temp >> sampleCount*Bits);
// 			UBYTE b = (Shifter & bitmask[Bits-1]);

// 			if ((Bits == 4) && ((Shifter & 0xf) == 0))
// 				Delta = 4;

// 			while(b--) {
// 				newEstMax += Delta;
// 			}

// 			lDelta = Delta * Matrix[Bits-2][Shifter & bitmask[Bits-1]];

// 			if(Shifter & (1<<(Bits-1))) {	// SignBit
// 				newEstMax = -newEstMax;
// 			}
// 			EstMax = (EstMax + newEstMax) & 0xffff;
			
// 			Delta = (UWORD)((LONG)(lDelta + 8192) >> 14);
			
// 			if(Delta < 5) Delta = 5;

// 			newEstMax = EstMax >> 6;
// 			if(127 < newEstMax)
// 				*Destination++ = 127;
// 			else if( -128 > newEstMax) {
// 				*Destination++ = -128;
// 			}
// 			else
// 				*Destination++ = newEstMax;
// 		}
// 	}
// 	// return (Delta<<16|(EstMax&0xffff));
// 	return Destination;
// #else
// 	return NULL;
// #endif
	return NULL;
}

#endif