/*  	Windless Bay, Mount Erebus.
	Original game by Francois "Astrofra" Gutherz. 
	Amiga version (C) RESISTANCE 2021 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpage/aos/debug.h"
#include "rpage/frwk.h"
#include "rpage/utils.h"
#include "game/vue.h"
#include "game/world.h"
#include "game/aos/assets.h"
#include "rpage/aos/debug.h"

short vue_cache_size = 0;
Vue *vue_cache[16];

/// Sprite cache data
Vue *sprite_cache[1];

/// Common unpacking buffer
BYTE *packed_vue;

const rect vue_area = 
{
    VUE_X, VUE_Y,
    VUE_X + VUE_WIDTH, VUE_Y + VUE_HEIGHT
};

/// Create a new cache for the vues. A vue is literally a view showing a distinct place in the game.<br>
/// The cache is made of a series of memory blocks, each block storing a vue in the bitmap format.
/// This function tries to allocate the largest amount of memory blocks, limited to 16.
/// All blocks have the same size, defined by ::rpage_bitmap_calculate_bytesize<br>
/// An additionnal block is reserved to unpack (decompress) each vue. The unpacking size if defined by ::MAX_PAK_SIZE (in bytes)
BOOL init_vue_cache(short cache_size, ULONG reserved_videomem, ULONG reserved_othermem, UBYTE *unpacker_buffer)
{
    short i;
    // Common unpacking buffer
    packed_vue = unpacker_buffer;

    // printf("chip(%d), largest_chip(%d), slow(%d)\n", 
    //         rpage_get_avail_video_memory() >> 10, 
    //         rpage_get_avail_largest_video_memory() >> 10, 
    //         rpage_get_avail_non_video_memory() >> 10);

    // printf("reserved_videomem = %d\n", reserved_videomem);

    // Vue cache init starts here
    if (cache_size == MAX_VUE_CACHE_SIZE)
    {
        LONG avail_mem;
		LONG vue_size = rpage_bitmap_calculate_bytesize(VUE_WIDTH, VUE_HEIGHT, VUE_DEPTH);

        // If there's not enough slow/fast ram
        // we should reserve LESS chip ram for the vue cache
        // thus a penalty added here to 'reserved_videomem'.
        if (rpage_get_avail_non_video_memory() < reserved_othermem)
            reserved_videomem += reserved_othermem - rpage_get_avail_non_video_memory();
        
        // printf("reserved_videomem = %d\n", reserved_videomem);
    
        avail_mem = (LONG)rpage_get_avail_largest_video_memory();
//         if (reserved_videomem > (ULONG)avail_mem)
//         {
//             char _str[256];
// #ifdef DEBUG_MACROS
//             printf("init_vue_cache(avail = %dBk, reserved = %dKb)\n", (avail_mem >> 10), (reserved_videomem >> 10));
// #endif

//             sprintf(_str, "NOT ENOUGH CHIP MEM! (%dKb AVAIL, %dKb NEEDED)", (int)(avail_mem >> 10), (int)((reserved_videomem + vue_size)>> 10));
//             rpage_system_alert(_str);
//             return FALSE;
//         }

        avail_mem -= (LONG)reserved_videomem;
        cache_size = (short)(max(1, min(16, avail_mem / vue_size)));
    }

    // printf("cache_size = %d, ", cache_size);

    vue_cache_size = cache_size;
#ifdef DEBUG_MACROS    
    printf("init_vue_cache(%d vues, reserved = %dKb)\n", vue_cache_size, (reserved_videomem >> 10));
#endif

    for (i = 0; i < vue_cache_size; i++)
    {
        vue_cache[i] = (Vue *)rpage_os_alloc(1 * sizeof(Vue), MEMF_ANY); // New vue struct

        if (vue_cache[i] != NULL)
        {
            vue_cache[i]->bitmap = rpage_bitmap_new(VUE_WIDTH, VUE_HEIGHT, VUE_DEPTH); // Bitmap in video memory for the unpacked image
            vue_cache[i]->mask = NULL; // Not used, for sprites only.
            vue_cache[i]->palette = (rpage_palette *)rpage_os_alloc((1 << VUE_DEPTH) * sizeof(UWORD), MEMF_ANY); // Palette
            if (vue_cache[i]->palette != NULL)
            {
                vue_cache[i]->timestamp = -1; // Timestamp (when the vue was loaded/accessed)
                vue_cache[i]->filename[0] = '\0';
                vue_cache[i]->key[0] = '\0';
            }
            else
            {
                rpage_system_alert("init_vue_cache(), palette malloc failed!");
            }
            
        }
        else
        {
            char str_err[256];
            sprintf(str_err, "init_vue_cache(), malloc failed on cache #%d.", i);
            rpage_system_alert(str_err);
            uninit_vue_cache();
            return FALSE;
        }
        
    }

    // printf("chip(%d), largest_chip(%d), slow(%d)\n", 
    //         rpage_get_avail_video_memory() >> 10, 
    //         rpage_get_avail_largest_video_memory() >> 10, 
    //         rpage_get_avail_non_video_memory() >> 10);

#ifdef DEBUG_MACROS
        printf("init_vue_cache(), vue_cache_size = %d.\n", vue_cache_size);
#endif

    // Sprite cache starts here
    sprite_cache[0] = (Vue *)rpage_os_alloc(1 * sizeof(Vue), MEMF_ANY);
    if (vue_cache[0] != NULL)
    {
        sprite_cache[0]->bitmap = rpage_bitmap_new(320, 200, VUE_DEPTH);
        sprite_cache[0]->mask = rpage_bitmap_new(320, 200, 1); 
        sprite_cache[0]->palette = NULL;
        sprite_cache[0]->timestamp = -1;
        sprite_cache[0]->filename[0] = '\0';
        sprite_cache[0]->key[0] = '\0';
    }
    else
    {
        rpage_system_alert("init_vue_cache(), malloc failed on sprite cache.");
        uninit_vue_cache();
        return FALSE;
    }

    return TRUE;
}

/// Return the total size of the cache in memory 
ULONG vue_get_cache_size(void)
{
    return vue_cache_size * rpage_bitmap_calculate_bytesize(VUE_WIDTH, VUE_HEIGHT, VUE_DEPTH);
}

/// Empty all vues
void flush_vue_cache(void)
{
    short i;
    for(i = 0; i < vue_cache_size; i++)
    {
        vue_cache[i]->timestamp = -1;
        memset(vue_cache[i]->filename, 0, _STR_MAX_SIZE);
        memset(vue_cache[i]->key, 0, _STR_VUE_KEY_SIZE);
    }
}

/// Destroy the vue cache, freeing each memory block one by one.
void uninit_vue_cache(void)
{
    short i;
    for (i = 0; i < vue_cache_size; i++)
    {
        rpage_bitmap_free(vue_cache[i]->bitmap); // Free the bitmap buffer
        if (vue_cache[i]->palette != NULL)
        {
            rpage_free_os_alloc((BYTE *)(vue_cache[i]->palette), (1 << VUE_DEPTH) * sizeof(UWORD)); // Free the palette
            vue_cache[i]->palette = NULL;
        }

        if (vue_cache[i] != NULL)
        {
            rpage_free_os_alloc((BYTE *)vue_cache[i], 1 * sizeof(Vue)); // Free the struct itself
            vue_cache[i] = NULL;
        }
    }

    unload_sprites_sheet(); // Unload sprite sheet (if any was loaded)
    if (sprite_cache[0] != NULL)
    {
        if (sprite_cache[0]->palette != NULL)
            free(sprite_cache[0]->palette); // Free the palette
        sprite_cache[0]->palette = NULL;
        rpage_free_os_alloc((BYTE *)sprite_cache[0], 1 * sizeof(Vue)); // Free the struct itself
        sprite_cache[0] = NULL;
    }
}

void _clear_vue_cache(short vue_index)
{
#ifdef DEBUG_MACROS    
    printf("_clear_vue_cache(%s) = %d, ", vue_cache[vue_index]->key, vue_index);
#endif  
    vue_cache[vue_index]->timestamp = -1;
    vue_cache[vue_index]->filename[0] = '\0';
    vue_cache[vue_index]->key[0] = '\0';
#ifdef DEBUG_MACROS    
    printf("Cleared key = %s\n", vue_cache[vue_index]->key);
#endif      
}

short _find_free_vue_cache(void)
{
    short oldest_vue = 0;

    if (vue_cache_size > 1)
    {
        // Find the oldest vue (== the vue with the smaller timestamp)
        short i;
        for(i = 0; i < vue_cache_size; i++)
            if (vue_cache[i]->timestamp < vue_cache[oldest_vue]->timestamp)
                oldest_vue = i;
    }
    
    // Recycle the vue by clearing its age (timestamp) and its content
    _clear_vue_cache(oldest_vue);

#ifdef DEBUG_MACROS
        printf("_find_free_vue_cache(), found vue #[%d].\n", oldest_vue);
#endif   

    return oldest_vue;
}

void vue_set_palette(char *vue_name, rpage_palette *palette)
{
    short i;
    for(i = 0; i < vue_cache_size; i++)
    {
        if (strncmp(vue_cache[i]->key, vue_name, _STR_VUE_KEY_SIZE) == 0)
        {
#ifdef DEBUG_MACROS            
            printf("get_vue_bitmap(%s) = %d\n", vue_name, i);
#endif
            // Set palette by COPY, in case the palette passed
            // to the function is deleted afterward
            memcpy(vue_cache[i]->palette, palette, (1 << vue_cache[i]->bitmap->Depth) * sizeof(rpage_palette));
            // vue_cache[i]->timestamp = -1; // this vue cannot be cached anymore because it was ALTERED
            break;
        }
    }
}

short _vue_is_already_cached(char *vue_name)
{
    if (vue_cache_size > 0)
    {
        short i;
        // Search if the vue was already loaded
        for(i = 0; i < vue_cache_size; i++)
            if (strncmp(vue_cache[i]->key, vue_name, _STR_VUE_KEY_SIZE) == 0)
            {
                // rpage_video_set_immediate_RGB444(0, 0x080);
                return i;
            }
    }

    // rpage_video_set_immediate_RGB444(0, 0x800);
    return -1;
}

void load_vue(char *vue_name)
{
    /// Vue cache data
    short i;
// PRINT_LARGEST_CHIP("begin-load_vue()");
    // Search if the vue was already loaded
    i = _vue_is_already_cached(vue_name);
#ifdef DEBUG_MACROS    
    printf("_vue_is_already_cached: %s, %d\n", vue_name, i);
#endif
    if (i >= 0)
    {
        // If we found this vue in the cache we don't load it again,
        // but we update the timestamp as it's the most recently accessed
        vue_cache[i]->timestamp = rpage_get_clock();
#ifdef DEBUG_MACROS
        printf("load_vue(), found '%s' in [%d].\n", vue_name, i);
#endif        
    }
    else
    {    
        // ULONG profiler;
        char str_buffer[_STR_MAX_SIZE];
        // Otherwise, we ask for a new vue cache
        // and load the new vue into it.
#ifdef DEBUG_MACROS
        printf("load_vue(), cache miss! Could not find '%s'.\n", vue_name, i);
#endif
        i = _find_free_vue_cache();
        str_buffer[0] = '\0'; 
        // strcat(str_buffer, asset_build_device_path(vue_name, ".pak"));
        sprintf(str_buffer, "%s", asset_build_device_path(vue_name, ".pak"));

        rpage_mouse_wait();
        rpage_set_process_priority(0);

        // profiler = rpage_get_clock();
        if (rpage_load_pak_into_bitmap(&(vue_cache[i]->bitmap), &(vue_cache[i]->palette), packed_vue, str_buffer))
        {
            strncpy(vue_cache[i]->key, vue_name, _STR_VUE_KEY_SIZE);
            strncpy(vue_cache[i]->filename, str_buffer, _STR_MAX_SIZE);
            vue_cache[i]->timestamp = rpage_get_clock();
        }
#ifdef DEBUG_MACROS
        else
        {
            printf("Could not load vue %s\n", str_buffer);
        }
#endif

        // profiler = rpage_get_clock() - profiler;
        // printf("loading time: %d\n", profiler);
        
        rpage_set_process_priority(127);
        rpage_mouse_show();
#ifdef DEBUG_MACROS
        printf("loadVue(%s)\n", str_buffer);
#endif
    }
// PRINT_LARGEST_CHIP("end-load_vue()");
}

rpage_bitmap *get_vue_bitmap(char *vue_name)
{
    short i;
    for(i = 0; i < vue_cache_size; i++)
    {
        if (strncmp(vue_cache[i]->key, vue_name, _STR_VUE_KEY_SIZE) == 0)
        {
#ifdef DEBUG_MACROS            
            printf("get_vue_bitmap(%s) = %d\n", vue_name, i);
#endif
            return vue_cache[i]->bitmap;
        }
    }

    return NULL;
}

rpage_palette *get_vue_palette(char *vue_name)
{
    short i;
    for(i = 0; i < vue_cache_size; i++)
    {
        if (strncmp(vue_cache[i]->key, vue_name, _STR_VUE_KEY_SIZE) == 0)
        {
            return vue_cache[i]->palette;
        }
    }

    return NULL;    
}

BOOL unload_vue(char *vue_name)
{
    short i;
    for(i = 0; i < vue_cache_size; i++)
    {
        if (strncmp(vue_cache[i]->key, vue_name, _STR_VUE_KEY_SIZE) == 0)
        {
#ifdef DEBUG_MACROS            
            printf("unload_vue(%s) = %d\n", vue_name, i);
#endif
            _clear_vue_cache(i);
            return TRUE;
        }

    }

    return FALSE;
}

BOOL vue_is_point_in_rect(vec2 *point)
{
    return point_within_rect(point, (rect *)&vue_area);
}

void display_vue(char *vue_name)
{
    short i;

    // Search for the vue that matches this name
    i = _vue_is_already_cached(vue_name);
#ifdef DEBUG_MACROS    
    printf("_vue_is_already_cached: %s, %d\n", vue_name, i);
#endif
    if (i >= 0)
    {
        Vue *new_vue = vue_cache[i];
        rpage_video_blt_bmp(new_vue->bitmap, 0, 0, rpage_bitmap_get_width(new_vue->bitmap), rpage_bitmap_get_height(new_vue->bitmap), VUE_X, VUE_Y);
        rpage_video_set_pixel(VUE_X, VUE_Y, _VUE_CORNER_COLOR_);
        rpage_video_set_pixel(rpage_bitmap_get_width(new_vue->bitmap) + VUE_X - 1, VUE_Y, _VUE_CORNER_COLOR_);
        rpage_video_set_pixel(VUE_X, rpage_bitmap_get_height(new_vue->bitmap) + VUE_Y - 1, _VUE_CORNER_COLOR_);
        rpage_video_set_pixel(rpage_bitmap_get_width(new_vue->bitmap) + VUE_X - 1, rpage_bitmap_get_height(new_vue->bitmap) + VUE_Y - 1, _VUE_CORNER_COLOR_);
        rpage_video_set_palette(new_vue->palette, 1 << rpage_video_get_depth());
#ifdef DEBUG_MACROS    
        printf("displayVue(%s)\n", vue_name);
#endif
    }
    else
    {
#ifdef DEBUG_MACROS
        printf("displayVue() Cannot find this vue! '%s'\n", vue_name);
#endif
        {
            static char _err[512];
            sprintf(_err, "!Cannot find '%s' in the vue cache :(", vue_name);
            rpage_system_alert(_err);
        }
    }
    
}

void unload_sprites_sheet(void)
{
#ifdef DEBUG_MACROS      
    printf("unload_sprites_sheet()\n");
    printf("bitmap = %x, mask = %x\n", sprite_cache[0]->bitmap, sprite_cache[0]->mask);
#endif

    if (sprite_cache[0]->bitmap != NULL)
    {
#ifdef DEBUG_MACROS      
    printf("rpage_bitmap_free(sprite_cache[0]->bitmap);\n");
#endif        
        rpage_bitmap_free(sprite_cache[0]->bitmap); // Free the bitmap buffer
        sprite_cache[0]->bitmap = NULL;
    }

    if (sprite_cache[0]->mask != NULL)
    {
#ifdef DEBUG_MACROS      
    printf("rpage_bitmap_free(sprite_cache[0]->mask);\n");
#endif        
        rpage_bitmap_free(sprite_cache[0]->mask); // Free the mask buffer
        sprite_cache[0]->mask = NULL;
    }
}

void load_sprites_sheet(char *sprites_filename)
{
    /// Vue cache data
#ifdef DEBUG_MACROS      
    printf("load_sprites_sheet(%s)\n", sprites_filename);
#endif

    // if this is a new sprite sheet
    if (strncmp(sprite_cache[0]->key, sprites_filename, _STR_VUE_KEY_SIZE) != 0)
    {
        char str_buffer[_STR_MAX_SIZE];

        // Load the sprite sheet itself
        // sprintf(str_buffer, "%s", asset_build_device_path(sprites_filename, ".pak"));
        memset(str_buffer, 0, _STR_MAX_SIZE);
        strncpy(str_buffer, asset_build_device_path(sprites_filename, ".pak"), _STR_MAX_SIZE);
#ifdef DEBUG_MACROS      
        printf("str_buffer = '%s'.\n", str_buffer);
#endif

        if (rpage_load_pak_into_bitmap(&(sprite_cache[0]->bitmap), NULL, packed_vue, str_buffer))
        {
            strncpy(sprite_cache[0]->key, sprites_filename, _STR_VUE_KEY_SIZE);
            strncpy(sprite_cache[0]->filename, str_buffer, _STR_MAX_SIZE);

            // Load the sprite sheet (1 bit) masks
            // sprintf(str_buffer, "%s", asset_build_device_path(sprites_filename, "_mask.pak"));
            memset(str_buffer, 0, _STR_MAX_SIZE);
            strncpy(str_buffer, asset_build_device_path(sprites_filename, "_mask.pak"), _STR_MAX_SIZE);

#ifdef DEBUG_MACROS      
        printf("str_buffer = '%s'.\n", str_buffer);
#endif

            if (!rpage_load_pak_into_bitmap(&(sprite_cache[0]->mask), NULL, packed_vue, str_buffer))
            {
#ifdef DEBUG_MACROS
                printf("Could not load sprite sheet mask %s\n", str_buffer);
#endif
            }
            sprite_cache[0]->timestamp = rpage_get_clock();
        }
    }

#ifdef DEBUG_MACROS      
    printf("bitmap = %x, mask = %x\n", sprite_cache[0]->bitmap, sprite_cache[0]->mask);
#endif    
}

rpage_bitmap *getSpriteSheetBitmap(void)
{
    return sprite_cache[0]->bitmap;
}

rpage_bitmap *getSpriteSheetMask(void)
{
    return sprite_cache[0]->mask;
}

void draw_sprite_on_vue(short sprite_index)
{
    rect source;
    vec2 dest;

#ifdef DEBUG_MACROS
    printf("game_show_sprite(%d)\n", sprite_index);
#endif

    // get the index of the sheet sprite, the source rect, the destination x,y
    gameObjectGetSpriteSheetCoords(sprite_index, &source, &dest);
#ifdef DEBUG_MACROS
    printf("source=(%d,%d,%d,%d)\n", source.sx, source.sy, source.ex - source.sx, source.ey - source.sy);
    printf("dest  =(%d,%d)\n", dest.x, dest.y);
#endif

    // blit the sprite!
    rpage_video_blt_bmp_mask(getSpriteSheetBitmap(), source.sx, source.sy, source.ex - source.sx, source.ey - source.sy, dest.x + VUE_X, dest.y + VUE_Y, getSpriteSheetMask());
}