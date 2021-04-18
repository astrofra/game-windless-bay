/*  	Windless Bay, Mount Erebus.
	Original game by Francois "Astrofra" Gutherz. 
	Amiga version (C) RESISTANCE 2021 
*/

#ifndef _VUE_HEADERS_
#define _VUE_HEADERS_

#include "rpage/frwk.h"
#include "game/aos/config.h"

#define VUE_X 8
#define VUE_Y 4
#define VUE_WIDTH 256
#define VUE_HEIGHT 144
#define VUE_DEPTH 5
#define _VUE_CORNER_COLOR_ 10 // color of the pixels we draw on each corner of the vue bitmap

#define _STR_MAX_SIZE 256
#define _STR_VUE_KEY_SIZE 64
#define MAX_VUE_CACHE_SIZE -1

typedef struct
{
    char key[_STR_VUE_KEY_SIZE];
    char filename[_STR_MAX_SIZE];
    int timestamp;
    rpage_bitmap *bitmap;
    rpage_palette *palette;
    rpage_bitmap *mask;
} Vue;

extern const rect vue_area;

BOOL init_vue_cache(short cache_size, ULONG reserved_memory, ULONG reserved_othermem, UBYTE *unpacker_buffer);
void flush_vue_cache(void);
void uninit_vue_cache(void);
ULONG vue_get_cache_size(void);

void load_vue(char *vue_name);
void display_vue(char *vue_name);
rpage_bitmap *get_vue_bitmap(char *vue_name);
rpage_palette *get_vue_palette(char *vue_name);
BOOL unload_vue(char *vue_name);
void vue_set_palette(char *vue_name, rpage_palette *palette);

void unload_sprites_sheet(void);
void load_sprites_sheet(char *sprites_filename);
rpage_bitmap *getSpriteSheetBitmap(void);
rpage_bitmap *getSpriteSheetMask(void);
void draw_sprite_on_vue(short sprite_index);
BOOL vue_is_point_in_rect(vec2 *point);
#endif