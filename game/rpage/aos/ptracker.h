/*  Resistance's Portable-Adventure-Game-Engine (R-PAGE), Copyright (C) 2019 François Gutherz, Resistance.no
    Released under MIT License, see license.txt for details.
*/

#ifdef LATTICE
#ifndef AUDIO_ROUTINES
#define AUDIO_ROUTINES

#include "rpage/aos/ptreplay.h"
#include "rpage/aos/ptreplay_protos.h"
#include "rpage/aos/ptreplay_pragmas.h"

BOOL init_protracker_player(void);
void uninit_protracker_player(void);
void load_imploded_protracker_music(char *filename, UBYTE *unpacking_sample_buffer, char *asset_path);
void load_packed_protracker_music(char *filename, UBYTE *packed_block, BYTE **unpacked_block);
void save_protracker_music(char *filename);
void load_protracker_music(char *filename, int filesize);
void save_protracker_music(char *filename);
void unload_protracker_music(void);
void protracker_setup_mod(void);
void protracker_play(void);
void protracker_stop(void);
void protracker_fadeout_async(void);
void protracker_set_fade_speed(short fade_speed);
short protracker_get_volume(void);
void protracker_update_state(void);
void protracker_enable_channel(short channel, BOOL flag);
void protracker_pause(void);
void protracker_resume(void);
UBYTE protracker_get_song_pos(void);
void protracker_set_song_pos(UBYTE pos);

#endif // #ifndef AUDIO_ROUTINES
#endif