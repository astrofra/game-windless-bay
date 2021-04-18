/*  Resistance's Portable-Adventure-Game-Engine (R-PAGE), Copyright (C) 2019 François Gutherz, Resistance.no
    Released under MIT License, see license.txt for details.
*/

#ifdef LATTICE
#include "rpage/aos/inc.prl"
#include <time.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>

#include "rpage/aos/sound.h"
#include "rpage/aos/ptreplay.h"
#include "rpage/aos/ptreplay_protos.h"
#include "rpage/aos/ptreplay_pragmas.h"

#include "rpage/aos/bitmap.h"
#include "rpage/aos/io.h"

#include "ext/tinfl.h"
#include "ext/aos/shrinkler.h"
#include "ext/aos/nrv2.h"

#include "rpage/frwk.h"
#include "rpage/aos/debug.h"
#include "rpage/aos/ptracker.h"
#include "rpage/err.h"
#include "rpage/utils.h"

/* Music */
struct Library *PTReplayBase = NULL;
struct Module *protracker_mod_playing = NULL;
UBYTE *protracker_mod_data = NULL;
ULONG protracker_mod_size = 0;
// BYTE protracker_SigBit = NULL;
ULONG protracker_SigMask;

short protracker_mod_volume = 0;
short protracker_fade_min_volume = 0;
short protracker_fade_max_volume = 0;
short protracker_fade_speed = 0;

// extern struct IntuitionBase *IntuitionBase;
// extern struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;
extern struct Custom far custom;

#define DEBUG_ENABLE_PT

BOOL init_protracker_player(void)
{
#ifdef DEBUG_ENABLE_PT	
	if (SysBase->LibNode.lib_Version >= 36)
		if (!AssignPath("Libs","Libs"))
			printf("/!\\Cannot assign local Libs: folder. The Ptreplay library might not load properly!\n");

	if (!(PTReplayBase = OpenLibrary((UBYTE *)"ptreplay.library", 0)))
	{
		rpage_system_alert("Cannot open ptreplay.library!");
		PTReplayBase = NULL;
		return FALSE;
	}

	protracker_fade_speed = 0;
	protracker_fade_min_volume = 0;
	protracker_fade_max_volume = 64;
	protracker_mod_volume = protracker_fade_max_volume;

	return TRUE;
#else
	return FALSE;
#endif
}

void protracker_pause(void )
{
	if (PTReplayBase)
	{
		if (protracker_mod_data != NULL && protracker_mod_playing != NULL)	
		{	
			PTPause(protracker_mod_playing);
		}
	}
}

void protracker_resume(void )
{
	if (PTReplayBase)
	{
		if (protracker_mod_data != NULL && protracker_mod_playing != NULL)	
		{	
			PTResume(protracker_mod_playing);
		}
	}
}

void protracker_enable_channel(short channel, BOOL flag)
{
#ifdef DEBUG_ENABLE_PT
	if (PTReplayBase)
	{
		if (protracker_mod_data != NULL && protracker_mod_playing != NULL)	
		{
			short chan_mask;
			chan_mask = 1 << channel;

			if (flag)
				PTOnChannel(protracker_mod_playing, chan_mask);
			else
				PTOffChannel(protracker_mod_playing, chan_mask);
		}
	}
#endif
}

void uninit_protracker_player(void)
{
#ifdef DEBUG_ENABLE_PT	
	unload_protracker_music();

	if (PTReplayBase)
	{
		CloseLibrary(PTReplayBase);
		PTReplayBase = NULL;
	}
#endif
}

void load_protracker_music(char *filename, int filesize)
{
#if 0
#ifdef DEBUG_ENABLE_PT
	if (filesize < 0)
	{
		filesize = file_get_size(filename);
		printf("load_protracker_music(), guessed module file size : %d\n", filesize);
	}

	protracker_mod_data = load_raw_to_mem((UBYTE *)filename, (ULONG)filesize, TRUE);
	protracker_mod_size = filesize;
	printf("load_protracker_music(%s) loaded at %x.\n", filename, (unsigned int)protracker_mod_data);
#endif
#endif
}

void save_protracker_music(char *filename)
{
#ifdef DEBUG_ENABLE_PT	
	rpage_file file;
	if (protracker_mod_data != NULL)
	{
		file = rpage_file_open(filename, MODE_OPEN_OR_CREATE);
		rpage_file_write(file, protracker_mod_data, protracker_mod_size);
		rpage_file_close(file);
	}
#endif
}

void load_imploded_protracker_music(char *filename, UBYTE *unpacking_sample_buffer, char *asset_path)
{
#if 0
#ifdef DEBUG_ENABLE_PT		
	int unpacked_block_size, packed_block_size, i;
	BYTE *packed_block, *unpacked_block, *smpl_list_ptr, *smpl_ptr_save;
	BPTR fileHandle;
	char tag[4];
	char *spl_files[32];
	UWORD mod_size;	

	if ((fileHandle = Open(filename, MODE_OLDFILE)))
	{
		Read(fileHandle, &tag, 4);
		if (strncmp(tag, "IMPK", 4) == 0)
		{
			Read(fileHandle, &tag, 4);
			if (strncmp(tag, "SIZE", 4) == 0)
			{
				Read(fileHandle, &unpacked_block_size, 4);
				unpacked_block = rpage_os_alloc(unpacked_block_size, MEMF_CHIP);

				Read(fileHandle, &tag, 4);
				if (strncmp(tag, "MINZ", 4) == 0)
				{
					Read(fileHandle, &tag, 4);
					if (strncmp(tag, "SIZE", 4) == 0)
					{
						Read(fileHandle, &packed_block_size, 4);
						packed_block = rpage_c_alloc(packed_block_size, sizeof(char));
						Read(fileHandle, packed_block, packed_block_size);
						tinfl_decompress_mem_to_mem(unpacked_block, unpacked_block_size, packed_block, packed_block_size, 1);
						free(packed_block);
						// printf("FreeMem(), load_imploded_protracker_music()\n");

						// look for the "SMPL" tag
						smpl_list_ptr = unpacked_block;
						while((int)(smpl_list_ptr - unpacked_block) < unpacked_block_size && strncmp(smpl_list_ptr, "SMPL", 4))
							smpl_list_ptr++;

						smpl_ptr_save = smpl_list_ptr;
							
						// get the sample filenames
						for(i = 0; i < 32; i++)
						{
							smpl_list_ptr += 4;
							if (smpl_list_ptr[0] | smpl_list_ptr[1] | smpl_list_ptr[2] | smpl_list_ptr[3])
							{
								spl_files[i] = (char *)rpage_c_alloc(5, sizeof(char));
								strncpy(spl_files[i], smpl_list_ptr, 4);
								printf("Sample #%d: '%s.pak'.\n", i, spl_files[i]);
							}
							else
							{
								spl_files[i] = NULL;
							}
						}

						// back to the memory adress
						// where the samples should be located 
						smpl_list_ptr  = smpl_ptr_save;

						for(i = 0; i < 32; i++)
						{
							SoundInfo *sfx;

							// load each sample
							if (spl_files[i])
							{
								char _file[256];
								sprintf(_file, "%s%s.pak", asset_path, spl_files[i]);
								sfx = LoadPackedSound(_file, unpacking_sample_buffer, smpl_list_ptr);
								smpl_list_ptr += sfx->FileLength;
								printf("Sample #%d: '%s.pak, %X bytes'.\n", i, spl_files[i], sfx->FileLength);
								SoundInfoFree(sfx);
							}
						}

						protracker_mod_data = unpacked_block;
						protracker_mod_size = unpacked_block_size;
					}
					else
					{
						printf(err_no_size_found);
					}
				}
			}
		}

		Close(fileHandle);
	}
#endif					
#endif					
}

void load_packed_protracker_music(char *filename, UBYTE *packed_block, BYTE **unpacked_block)
{
#ifdef DEBUG_ENABLE_PT
	int unpacked_block_size, packed_block_size;
	BPTR fileHandle;
	char tag[4];

	// printf("load_packed_protracker_music(), size = %d\n", file_get_size(filename));

	fileHandle = Open(filename, MODE_OLDFILE);

	if (fileHandle)
	{
		Read(fileHandle, &tag, 4);
		if (strncmp(tag, "PTPK", 4) == 0)
		{
			// read original module size
			Read(fileHandle, &unpacked_block_size, 4);
			// unpacked_block = rpage_os_alloc(unpacked_block_size, MEMF_CHIP);
	
			Read(fileHandle, &tag, 4);
			if (strncmp(tag, "MINZ", 4) == 0)
			{
				Read(fileHandle, &tag, 4);
				if (strncmp(tag, "SIZE", 4) == 0)
				{
					Read(fileHandle, &packed_block_size, 4);
					Read(fileHandle, packed_block, packed_block_size);
					tinfl_decompress_mem_to_mem(*unpacked_block, unpacked_block_size, packed_block, packed_block_size, 1);
				}
				else
					printf(err_no_size_found);
			}
			else if (strncmp(tag, "SHRK", 4) == 0)
			{
				Read(fileHandle, &tag, 4);
				if (strncmp(tag, "SIZE", 4) == 0)
				{
					Read(fileHandle, &packed_block_size, 4);
					Read(fileHandle, packed_block, packed_block_size);
					ShrinklerDecompress(packed_block, *unpacked_block, NULL, NULL);
				}
				else
					printf(err_no_size_found);
			}
			else if (strncmp(tag, "NRV2", 4) == 0)
			{
				Read(fileHandle, &tag, 4);
				if (strncmp(tag, "SIZE", 4) == 0)
				{
					Read(fileHandle, &packed_block_size, 4);
					Read(fileHandle, packed_block, packed_block_size);
					nrv2s_unpack(packed_block, *unpacked_block);
				}
				else
					printf(err_no_size_found);
			}			
			else
			{
				printf(err_unknown_tag, tag);
				// FreeMem(unpacked_block, unpacked_block_size);
				// // printf("FreeMem(), load_packed_protracker_music()\n");
				// unpacked_block = NULL;
				unpacked_block_size = 0;
			}

			protracker_mod_data = *unpacked_block;
			protracker_mod_size = unpacked_block_size;
// printf("load_packed_protracker_music(%s), protracker_mod_size=%d\n", filename, protracker_mod_size);

		}
		else
		{
			printf("!Not a Protracker Packed File!\n");
		}

		Close(fileHandle);
	}
	else
	{
		printf("!load_packed_protracker_music(), cannot open file %s, error code %d!\n", filename, IoErr());
	}
	

#endif
}

void unload_protracker_music(void)
{
#ifdef DEBUG_ENABLE_PT
	if (protracker_mod_data != NULL)
	{
		//	Stop music
		if (protracker_mod_playing != NULL)
		{
			PTStop(protracker_mod_playing);
			PTFreeMod(protracker_mod_playing);
		}
		// else
		// 	rpage_system_alert(strcat("unload_protracker_music", (char *)err_mod_not_setup));

// printf("unload_protracker_music(), protracker_mod_size=%d\n", protracker_mod_size);
		// FreeMem(protracker_mod_data, protracker_mod_size);
		// printf("FreeMem(), load_packed_protracker_music()\n");
		// protracker_mod_data = NULL;
		protracker_mod_playing = NULL;
	}
#endif
}

void protracker_setup_mod(void)
{
#ifdef DEBUG_ENABLE_PT
	if (PTReplayBase)
	{
		if (protracker_mod_data != NULL)
		{
			if (protracker_mod_playing == NULL)
				protracker_mod_playing = PTSetupMod((APTR)protracker_mod_data);
		}
	}
	else
		rpage_system_alert("Ptreplay.library was not open!");
#endif	
}

void protracker_play(void)
{
#ifdef DEBUG_ENABLE_PT
	if (PTReplayBase)
	{
		if (protracker_mod_data != NULL)
		{
			if (protracker_mod_playing != NULL)
			{
				PTPlay(protracker_mod_playing);
				protracker_mod_volume = 64;
				PTSetVolume(protracker_mod_playing, protracker_mod_volume);
				// printf("PTPlay(protracker_mod_data, protracker_mod_playing) = %x, %x\n", protracker_mod_data, protracker_mod_playing);
			}
			else
				rpage_system_alert(strcat("protracker_play", (char *)err_mod_not_setup));
		}
	}
	else
		rpage_system_alert("Ptreplay.library was not open!");
#endif
}

void protracker_stop(void)
{
#ifdef DEBUG_ENABLE_PT
	if (PTReplayBase)
	{	
		if (protracker_mod_data != NULL)
		{
			//	Stop music
			if (protracker_mod_playing != NULL)
			{
				PTStop(protracker_mod_playing);
				// PTFade(protracker_mod_playing, 1);
			}
			// else
			// 	rpage_system_alert((char *)err_mod_not_setup);			
		}
	}
	else
		rpage_system_alert("Ptreplay.library was not open!");
#endif		
}

void protracker_fadeout_async(void)
{
#ifdef DEBUG_ENABLE_PT
	if (PTReplayBase)
	{	
		if (protracker_mod_data != NULL)
		{
			//	Fade music
			if (protracker_mod_playing != NULL)
			{
				PTStartFade(protracker_mod_playing, 1);
			}
			else
				rpage_system_alert(strcat("protracker_fadeout_async", (char *)err_mod_not_setup));			
		}
	}
	else
		rpage_system_alert("Ptreplay.library was not open!");
#endif		
}

UBYTE protracker_get_song_pos(void)
{
#ifdef DEBUG_ENABLE_PT
	if (PTReplayBase)
	{	
		if (protracker_mod_data != NULL)
		{
			//	Stop music
			if (protracker_mod_playing != NULL)
			{
				return(PTSongPos(protracker_mod_playing));
			}
		}
	}
	else
		rpage_system_alert("Ptreplay.library was not open!");
#endif		
	return 0;
}

void protracker_set_song_pos(UBYTE pos)
{
#ifdef DEBUG_ENABLE_PT
	if (PTReplayBase)
	{	
		if (protracker_mod_data != NULL)
		{
			//	Stop music
			if (protracker_mod_playing != NULL)
			{
				PTSetPos(protracker_mod_playing, pos);
				// printf("PTSetPos(protracker_mod_data, protracker_mod_playing, pos) = %x, %x, %d\n", protracker_mod_data, protracker_mod_playing, pos);
			}
			else
				rpage_system_alert(strcat("protracker_set_song_pos", (char *)err_mod_not_setup));			
		}
	}
	else
		rpage_system_alert("Ptreplay.library was not open!");
#endif	
}

short protracker_get_volume(void)
{
	return protracker_mod_volume;
}

void protracker_update_state(void)
{
#ifdef DEBUG_ENABLE_PT
	if (PTReplayBase)
	{
		if (protracker_mod_data != NULL && protracker_mod_playing != NULL)
		{
			if (protracker_fade_speed != 0)
			{
				protracker_mod_volume = min((protracker_fade_max_volume << 1), max((protracker_fade_min_volume << 1), protracker_mod_volume + protracker_fade_speed));
				PTSetVolume(protracker_mod_playing, protracker_mod_volume >> 1);
				if (protracker_mod_volume <= 0)
					protracker_stop();
			}
		}
	}
#endif
}

void protracker_set_fade_speed(short fade_speed)
{
#ifdef DEBUG_ENABLE_PT	
	protracker_fade_speed = fade_speed;
#endif
}

#endif