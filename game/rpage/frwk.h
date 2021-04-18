/*  \mainpage My Personal Index Page
    Resistance's Portable-Adventure-Game-Engine (R-PAGE), Copyright (C) 2019 François Gutherz, Resistance.no
    Released under MIT License, see license.txt for details.
*/

#ifndef _FRAMEWORK_HEADER_
#define _FRAMEWORK_HEADER_

#include "rpage/utils.h"

#ifdef LATTICE
#include <exec/types.h>
#include <graphics/gfx.h>
#include "rpage/aos/color.h"
#include "rpage/aos/screen_size.h"
#include "rpage/aos/screen.h"

typedef BPTR rpage_file;
typedef struct BitMap rpage_bitmap;
#ifdef VGA_ENABLED
typedef unsigned long rpage_palette;
#else
typedef unsigned short rpage_palette;
#endif
typedef struct SimpleSprite rpage_hardware_sprite;

#define MAX_HARDWARE_SPRITES 8
#define PLATFORM_DRAW_TWICE(_EXPR_){rpage_video_flip_buffers(); _EXPR_; rpage_video_present_screen(); rpage_video_flip_buffers(); _EXPR_; rpage_video_present_screen(); }

/// An existing file is opened for reading or writing.
#define MODE_OPEN_FILE      MODE_OLDFILE
/// A new file is created for writing.
#define MODE_CREATE_FILE    MODE_NEWFILE
/// Opens a file with an shared lock, but creates it if it didn't exist.
#define MODE_OPEN_OR_CREATE MODE_READWRITE
#endif

#ifdef WIN32
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <Windows.h>
#include "rpage/win32/screen_size.h"
typedef FILE rpage_file;
typedef SDL_Texture rpage_bitmap;
typedef SDL_Color rpage_palette;
typedef SDL_Texture rpage_hardware_sprite;
#define PLATFORM_DRAW_TWICE(_EXPR_);
#endif

#ifdef LATTICE
// "Faster" blit using a macro
#define RPAGE_VIDEO_BLT_BMP_CLIP(__main_screen, __source_bitmap, ___source_x, ___source_y, ___width, ___height, ___x, ___y, __clipping_rect, __mask) \
{\
short __width = ___width, __height = ___height;\
short __source_x = ___source_x, __source_y = ___source_y;\
short __x = ___x, __y = ___y;\
if (__x < __clipping_rect->sx)\
{\
short __clip_x = __clipping_rect->sx - __x;\
__x += __clip_x;\
__source_x += __clip_x;\
__width -= __clip_x;\
}\
else\
{\
if (__x + __width > __clipping_rect->ex)\
__width -= (__x + __width - __clipping_rect->ex);\
}\
\
if (__y < __clipping_rect->sy)\
{\
short __clip_y = __clipping_rect->sy - __y;\
__y += __clip_y;\
__source_y += __clip_y;\
__height -= __clip_y;\
}\
else\
{\
if (__y + __height > __clipping_rect->ey)\
__height -= (__y + __height - __clipping_rect->ey);\
}\
\
BltBitMap(__source_bitmap, __source_x, __source_y, __main_screen->bitmaps[__main_screen->physical], __x, __y, __width, __height, 0xC0, __mask, NULL);\
}
#endif

/* Platform-agnostic game interface */

enum rpage_screen_modes
{
    mode_lowres,
    mode_medres,
    mode_hires
};

/// Initialize the framework.<br>
/// Open the required system libraries (Graphics, Intuition, DiskFont on the Amiga side), reset the global timer and get the current task ID. 
void rpage_init(void);
/// Un-initialize the framework, close the system libraries.
void rpage_uninit(void);

/// Set the multitasking priority of the current task.<BR>
/// On the Amiga side, this function takes -127 to +127 as a parameter, the higher the more CPU time is allocated to the task.
BYTE rpage_set_process_priority(BYTE new_priority);
/// Opens a GURU MEDITATION message.
void rpage_system_alert(char *alert_message);
/// Use the system function BELL/RING/FLASH to send a visual/audio alert.
void rpage_system_flash(void);
/// calloc wrapper
void *rpage_c_alloc(unsigned long size, unsigned long size_type);
/// allocmem wrapper
void *rpage_os_alloc(unsigned long size, unsigned long ram_mask);
/// Wrapper to the system-specific memory deallocator.
void rpage_free_os_alloc(BYTE *block_ptr, ULONG block_size);
/// Return how many free memory is available to store the graphics data (aka Chipram on the Amiga side).
ULONG rpage_get_avail_video_memory(void);
/// Return the largest free memory block available to store the graphics data (aka Chipram on the Amiga side).
ULONG rpage_get_avail_largest_video_memory(void);
/// Return how many general purpose free memory is available (aka Fastram/Slowram on the Amiga side).
ULONG rpage_get_avail_non_video_memory(void);
/// Return the total of available memory.
ULONG rpage_get_avail_memory(void);
/// Try to deallocate as many system resources as possible
void rpage_reclaim_system_memory(void);
/// Get the elapsed time, in milliseconds, since ::rpage_init was invoked.
ULONG rpage_get_clock(void);

/// Standard file open, using the OS layer. 'mode' can be set to:<br>
/// * MODE_OPEN_FILE: an existing file is opened for reading or writing<br>
/// * MODE_CREATE_FILE: a new file is created for writing<br>
/// * MODE_OPEN_OR_CREATE: opens a file with an shared lock, but creates it if it didn't exist<br>
rpage_file rpage_file_open(char *filename, long mode);
/// Close a current file.
void rpage_file_close(rpage_file file);
/// Read data from file into a buffer.
long rpage_file_read(rpage_file file, void *buffer, long len);
/// Write a data buffer into the file.
long rpage_file_write(rpage_file file, void *buffer, long len);

/// Open the main video output by allocating a blank screen framebuffer.<br>
/// The screenmode is defined by ::rpage_screen_modes. The current enum is tied to the regular Amiga OCS specifications.
void rpage_video_open(int screen_mode);
/// Close the video output, deallocate the screen framebuffer.
void rpage_video_close(void);
/// Get bit depth of the screen
UWORD rpage_video_get_depth(void);
/// Clear the screen.
void rpage_video_clear(void);
/// Set the default font file/size for every next call to ::rpage_video_draw_text
void rpage_video_clear_bit_mask(UBYTE bit_mask);
void rpage_video_set_font(char *font_filename, short font_size);
/// Wait for the vertical blank.
void rpage_video_vsync(void);
/// Wait for the completion of all ongoing DMA transfert (including the blitter operation, on the Amiga side).
void rpage_video_wait_dma(void);
/// Tell R-PAGE to redirect every next graphic drawing operations to the logical buffer.<br>
/// If the function is called again, all the drawing operations will be performed in the physical buffer again.<br>
/// This is how double buffering is handled by R-PAGE.
void rpage_video_flip_buffers(void);
/// Tell R-PAGE to swap the logical and the physical buffers. Everything that was drawn into the logical buffer will become visible.
void rpage_video_present_screen(void);
void rpage_prepare_fast_dbuffer(void);
void rpage_video_present_screen_fast(void);
/// Animated transition while copying the logical buffer to the physical buffer
void rpage_video_wipe_rect_to_physical_screen(rect *r);

/// Swap logical & physical palettes
void rpage_video_present_palette(void);
/// Hard copy the content of the physical buffer to the logical buffer. On the Amiga side This function may use the Blitter.
void rpage_video_sync_buffers(void);
/// Bring the game screen to top (Amiga only)
void rpage_video_screen_to_front(void);
/// Send the game screen to the back (Amiga only)
void rpage_video_screen_to_back(void);
/// Hardware scrolling
void rpage_video_scroll(short x, short y);

/// FIXME!
void rpage_video_scroll_bit_mask(short x, short y, UBYTE bit_mask);
/// Blit a bitmap into the screen.
void rpage_video_blt_bmp(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y);
/// Blit a bitmap into the screen using a bitplan mask, each bit of the 'bt' parameter being the mask for each bitplan.
void rpage_video_blt_bmp_bt(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, UBYTE bit_mask);
/// Blit a bitmap into the screen.<br>
/// A bitmap mask is used to merge the source into the destination. The bitmap mask should be 1bit wide.<br>
void rpage_video_blt_bmp_mask(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *mask_bitmap);
/// Blit a bitmap into the screen.<br>
/// * A 2D clipping operation is done, based on the clipping ::rect provided
void rpage_video_blt_bmp_clip(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rect *clipping_rect);
/// Blit a bitmap into the screen.<br>
/// A bitmap mask is used to merge the source into the destination. The bitmap mask should be 1bit wide.<br>
/// * A 2D clipping operation is done, based on the clipping ::rect provided
void rpage_video_blt_bmp_clip_mask(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *mask_bitmap, rect *clipping_rect);
/// Blit a bitmap into the screen.<br>
/// * A 2D clipping operation is done, based on the clipping ::rect provided
/// * A bitmask is applied to every pixel during the blit.
void rpage_video_blt_bmp_clip_mask_bt(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *mask_bitmap, rect *clipping_rect, UBYTE bit_mask);
void rpage_video_blt_bmp_mask_bt(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *mask_bitmap, UBYTE bit_mask);
/// Set the current palette of the screen.<br>
/// The color format is RGB444.
void rpage_video_set_palette(rpage_palette *palette, short palette_size);
/// Blacken the current palette of the screen.<br>
void rpage_video_set_palette_to_black(short first_color, short last_color);
/// Set the current palette of the screen to a gradient of greys.<br>
void rpage_video_set_palette_to_grey(short first_color, short last_color);
/// Draw an empty polygon (quad only) to the screen.<br>
/// * Color index range is (0,31) on the Amiga side.
void rpage_video_draw_polygon(poly *p, short color);
/// Draw an empty rect to the screen.<br>
/// * Color index range is (0,31) on the Amiga side.
void rpage_video_draw_rect(rect *r, short color_index);
/// Draw a filled rect to the screen.<br>
/// * Color index range is (0,31) on the Amiga side.
/// * A 2D clipping operation is done, based on the clipping ::rect provided
void rpage_video_fill_rect(rect *r, short color);
void rpage_video_fill_rect_clip(rect *r, short color, rect *clipping_rect);
/// Draw a pixel to the screen.
void rpage_video_set_pixel(short x, short y, short color_index);
/// Get the color of a pixel on screen
short rpage_video_get_pixel(short x, short y);
/// Draw a pixel on a bitmap. EXTREMELLY SLOW.
void rpage_bitmap_set_pixel(rpage_bitmap *dest_bitmap, short x, short y, short color);
/// Get the color of a pixel from a bitmap. EXTREMELLY SLOW.
short rpage_bitmap_get_pixel(rpage_bitmap *dest_bitmap, short x, short y);
/// Draw a text string to the screen.<br>
/// * The font must be defined prior to this operation, see ::rpage_video_set_font
void rpage_video_draw_text(char *str, short x, short y, short color_index);
void rpage_video_draw_shadow_text(char *str, short x, short y, unsigned short text_color, unsigned short shadow_color);
void rpage_video_draw_text_bit_mask(char *str, short x, short y, short color_index, UBYTE bit_mask);
/// Get text width in pixels, using the current font.
short rpage_video_get_text_width(char *str);
/// Draw a tileset-based image to the current screen.
void rpage_video_draw_tileset(rpage_bitmap *tileset_bitmap, UBYTE *tileset, rect *tile_rect, short tileset_width);
/// Store a part of the current screen into a bitmap to the same position.
void rpage_video_save_to_bitmap(rpage_bitmap *dest_bitmap, short source_x, short source_y, short width, short height);
/// Store a part of the current screen into a bitmap to a specific position.
void rpage_video_save_to_bitmap_ex(rpage_bitmap *dest_bitmap, short source_x, short source_y, short width, short height, short dest_x, short dest_y);
/// Display the amount of free memory (DEBUG ONLY)
void rpage_video_show_freemem(short x, short y, short width, short height);
rpage_bitmap *rpage_video_get_front_bitmap(void);
rpage_bitmap *rpage_video_get_back_bitmap(void);
rpage_palette *rpage_video_get_front_palette(void);

/// Draw a tileset-based image into a bitmap.
void rpage_bitmap_draw_tileset(rpage_bitmap *dest_bitmap, rpage_bitmap *tileset_bitmap, UBYTE *tileset, rect *tile_rect, short tileset_width);
/// Blit the destination bitmap into the source bitmap.
void rpage_bitmap_blit(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *dest_bitmap);
/// Blit the destination bitmap into the source bitmap.
void rpage_bitmap_blit_mask(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *dest_bitmap, rpage_bitmap *mask_bitmap);

/// Return the size in bytes of a ::rpage_bitmap.
ULONG rpage_bitmap_calculate_bytesize(short width, short height, short depth);
/// Allocate a new ::rpage_bitmap and return its address in video memory.
rpage_bitmap *rpage_bitmap_new(short width, short height, short depth);
/// Load a .PAK bitmap file into an existing bitmap.<br>
/// * The ::rpage_bitmap and ::rpage_palette must be allocated before calling this function.<br>
/// * the packed_buffer must be allocated before calling this function.<br>
BOOL rpage_load_pak_into_bitmap(rpage_bitmap **bitmap, rpage_palette **palette, BYTE *packed_buffer, char *filename);
/// Return the width in pixels of a ::rpage_bitmap
short rpage_bitmap_get_width(rpage_bitmap *bitmap);
/// Return the height in pixels of a ::rpage_bitmap
short rpage_bitmap_get_height(rpage_bitmap *bitmap);
/// Return the bitwise depth of a ::rpage_bitmap
short rpage_bitmap_get_depth(rpage_bitmap *bitmap);
/// Clear the current bitmap (fill it with 0, aka index 0 color)
void rpage_bitmap_clear(rpage_bitmap *bitmap);
/// Load a .PAK bitmap file into a new bitmap, self allocated by the function.<br>
/// * The ::rpage_bitmap and ::rpage_palette will be automatically allocated by the function.<br>
/// * If packed_buffer equals ::NULL, it will be automatically allocated by the function. Otherwise, R-PAGE will assume it contains the address of an already allocated block of memory.<br>
BOOL rpage_load_pak_to_new_bitmap(rpage_bitmap **new_bitmap, rpage_palette **new_palette, BYTE *packed_buffer, char *bitmap_filename);
/// Free the memory allocated by a ::rpage_bitmap.<br>
/// The possibly related ::rpage_palette is not automatically freed.
void rpage_bitmap_free(rpage_bitmap *bitmap);

/// Refresh the image/position of an hardware sprite
void rpage_move_sprite(short sprite_index, rpage_hardware_sprite *sprite, vec2 *position);
void rpage_move_sprite_fast(short sprite_index, rpage_hardware_sprite *sprite, vec2 *position);
/// Remove the sprite based on his index.
void rpage_remove_sprite(short sprite_index);
/// Returns TRUE is the sprite with this index is currently enabled.
BOOL rpage_sprite_is_enabled(short sprite_index);

/// Initialize the input system.<br>
/// On the Amiga side, the input system needs a Window to be created. This is done automatically by R-PAGE when calling ::rpage_video_open. As a consequence, the input won't work if no screen was created first.
BOOL rpage_input_init(void);
/// Pull the mouse/keyboard update from the input system.
void rpage_input_update(void);
/// Enable or disable the input pooling (Enabled by default)
void rpage_input_enable(BOOL enabled);
/// Flush the latest mouse update from the input system.<br>
/// This maybe required on a multitasking system.
void rpage_mouse_button_flush(void);
/// Private function that sets a bitmap to the mouse cursor
void rpage_mouse_set_system_image(unsigned short img_index);
/// Show the mouse cursor.
void rpage_mouse_show(void);
/// Change the look of the mouse cursor to warn the end-user that the application is currently busy.
void rpage_mouse_wait(void);
/// Change the look of the mouse cursor to warn the end-user that s.he should read some text.
void rpage_mouse_read(void);
/// Hide the mouse cursor.
void rpage_mouse_hide(void);
/// Defines the new aspect of the mouse cursor. Bitmap format is the regular Amiga hardware sprite format.
void rpage_mouse_set_bitmap(UWORD *sprite_data, vec2 *hotspot);
/// Read the current mouse coordinates and button states.
void rpage_mouse_get_values(short *button, vec2 *mouse_coords);
/// Read the previous mouse coordinates and button states.<br>
/// This might be useful when you need to calculate the delta/velocity of the mouse, or check is the mouse buttons where previously pressed. 
void rpage_mouse_get_prev_values(short *button, vec2 *mouse_coords);
/// Test if the left mouse button is currently pressed.
BOOL rpage_mouse_button_left_is_down(void);
/// Test if the right mouse button is currently pressed.
BOOL rpage_mouse_button_right_is_down(void);
/// Test if the left mouse button was pressed but isn't anymore.
BOOL rpage_mouse_button_left_was_down(void);
/// Test if the right mouse button was pressed but isn't anymore.
BOOL rpage_mouse_button_right_was_down(void);
/// Raw read the keyboard input.
unsigned short rpage_keyboard_rawkey(void);

#ifdef LATTICE
/// PRIVATE
buffered_screen* rpage_video_get_ptr(void);
/// Set a color (DEBUG ONLY)
void rpage_video_set_immediate_RGB444(short color_idx, color444 A);
#endif

#endif