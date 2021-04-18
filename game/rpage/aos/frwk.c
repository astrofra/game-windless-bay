/*  Resistance's Portable-Adventure-Game-Engine (R-PAGE), Copyright (C) 2019 François Gutherz, Resistance.no
    Released under MIT License, see license.txt for details.
*/

#ifdef LATTICE
#include "rpage/frwk.h"

#include "rpage/aos/inc.prl"
#include <time.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <math.h>
#include <time.h>

/*
Common
*/
#include "rpage/aos/board.h"
#include "rpage/aos/ptreplay.h"
#include "rpage/aos/ptreplay_protos.h"
#include "rpage/aos/ptreplay_pragmas.h"

/*
Routines
*/
#include "rpage/aos/screen.h"
#include "rpage/aos/bitmap.h"
#include "rpage/aos/color.h"
#include "rpage/aos/helper.h"
#include "rpage/aos/ptracker.h"
#include "rpage/aos/io.h"
#include "rpage/aos/time.h"
#include "rpage/aos/sound.h"

/*
Graphic assets
*/
#include "rpage/aos/screen_size.h"
#include "rpage/aos/mouse_ptr.h"
#include "rpage/aos/debug.h"

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;
extern struct DiskfontBase *DiskfontBase;
extern struct Custom far custom;

struct Task *main_task = NULL;
BYTE oldPri;

/* Main ViewPort */
buffered_screen *main_screen = NULL;
short scr_x_offset = 0, scr_y_offset = 0;
BOOL sprites_enabled[MAX_HARDWARE_SPRITES];

struct TextFont *main_font = NULL;

/* Global clock */
struct timeval startTime;
struct timeval endTime;

/* Input System */
short input_mouse_button;
short prev_input_mouse_button;

vec2 input_mouse_position;
vec2 prev_input_mouse_position;

BOOL input_enabled = FALSE;

unsigned short input_rawkey;

unsigned int g_max_video_ram = 0;
unsigned int g_max_non_video_ram = 0;

int color_luma[1 << SCREEN_DEPTH];

BYTE *temp_raster;

/* platform interface Amiga implementation */

void rpage_init(void)
{
    // BYTE error_code;
    short i;

#ifdef DEBUG_MACROS
    printf("rpage_init()\n");
#endif
    /* Open the Intuition library: */
    if (IntuitionBase == NULL)
    {
        IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0);
        if (!IntuitionBase)
        {
            rpage_system_alert("Could NOT open the Intuition library!");
            rpage_uninit();
            exit(0);
        }
    }
    else
    {
        rpage_system_alert("Platform already initialized!");
        exit(0);
    }

    /* Open the Graphics library: */
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0);
    if (!GfxBase)
    {
        rpage_system_alert("Could NOT open the Graphics library!");
        rpage_uninit();
        exit(0);
    }

    /* Open the DiskFont library: */
    DiskfontBase = (struct DiskfontBase *)OpenLibrary("diskfont.library", 0);
    if (!DiskfontBase)
    {
        rpage_system_alert("Could NOT open the Diskfont library!");
        rpage_uninit();
        exit(0);
    }
    else
    {
        // Add the local fonts folder to the system path
        if (SysBase->LibNode.lib_Version >= 36)
        {
		    if (!AssignPath("Fonts","Fonts"))
                printf("/!\\Cannot assign local Fonts: folder. The game fonts might not load properly!\n");	
        }
    }
    

    main_task = FindTask(NULL);
    // oldPri = SetTaskPri(main_task, 16);

    /* Timestamp of the platform startup */
    init_timer_device();
    timer_device_get_system_time(&startTime);

    /* Init the sprites array */
    sprites_enabled[0] = TRUE; /* sprites 0 is known to be reserved for the mouse cursor */
    for(i = 1; i < MAX_HARDWARE_SPRITES; i++)
        sprites_enabled[i] = FALSE;

    input_enabled = TRUE;

    main_screen = NULL;

    g_max_video_ram = rpage_get_avail_video_memory();
	g_max_non_video_ram = rpage_get_avail_non_video_memory();

    // Execute("endcli", NULL, NULL);
    // CloseWorkBench();
}

void rpage_reclaim_system_memory(void)
{
    if ((rpage_get_avail_memory() < (1024 << 10)) && (SysBase->LibNode.lib_Version < 39))
        add36k(IntuitionBase);
}

void rpage_uninit(void)
{
    short i;
#ifdef DEBUG_MACROS
    printf("rpage_uninit()\n");
#endif
    StopSound(LEFT0);
    StopSound(LEFT1);
    StopSound(RIGHT0);
    StopSound(RIGHT1);

    /* Remove any allocated sprite */
    for(i = 1; i < MAX_HARDWARE_SPRITES; i++)
        if (sprites_enabled[i])
        {
            FreeSprite(i);
            sprites_enabled[i] = FALSE;
        }    

    if (main_task != NULL)
        SetTaskPri(main_task, oldPri);

    uninit_timer_device();

    /* Close the Graphics library: */
    if (GfxBase)
        CloseLibrary((struct Library *)GfxBase);

    /* C Close the Intuition library:  */
    if (IntuitionBase)
        CloseLibrary((struct Library *)IntuitionBase);

    if (DiskfontBase)
        CloseLibrary((struct Library *)DiskfontBase);        

    IntuitionBase = NULL;
    GfxBase = NULL;
}

/*
    SYSTEM (resources, memory, multitasking...)
    -------------------------------------------
*/

BYTE rpage_set_process_priority(BYTE new_priority)
{
    return SetTaskPri(main_task, new_priority);
}

ULONG  rpage_get_avail_video_memory(void)
{
    return AvailMem(MEMF_CHIP);
}

ULONG  rpage_get_avail_largest_video_memory(void)
{
    return AvailMem(MEMF_CHIP|MEMF_LARGEST);
}

ULONG  rpage_get_avail_non_video_memory(void)
{
    return AvailMem(MEMF_FAST);
}

ULONG rpage_get_avail_memory(void)
{
    ULONG m_any, m_chip, m_fast;
 
    m_any = AvailMem(MEMF_ANY);
    m_chip = AvailMem(MEMF_CHIP);
    m_fast = AvailMem(MEMF_FAST);
    return max(m_any, m_chip + m_fast);
}

void rpage_free_os_alloc(BYTE *block_ptr, ULONG block_size)
{
    if (block_ptr != NULL)
        FreeMem(block_ptr, block_size);
    else
        rpage_system_alert("rpage_free_os_alloc(), pointer is NULL!");
}

void rpage_system_alert(char *alert_message)
{
#ifdef GAME_VISUAL_DEBUG
    char guru_format_message[128];
    short margin_x; 
#ifdef DEBUG_MACROS
    printf("/!\\%s\n", alert_message);
#endif
    memset(guru_format_message, 0, 128);
    strncpy(guru_format_message, "   ", 128);
    if (strlen(alert_message) > 76)
        alert_message[76] = 0x0;
    strcat(guru_format_message, alert_message);
    margin_x = ((640 - strlen(alert_message) * 8) / 2);
    guru_format_message[0] = (margin_x & 0xFF00) >> 8;
    guru_format_message[1] = margin_x & 0xFF;
    guru_format_message[2] = 0xF;
    DisplayAlert(RECOVERY_ALERT, guru_format_message, 32);
#else
    rpage_system_flash();
    printf("/!\\%s\n", alert_message);
    rpage_system_flash();
    rpage_video_vsync();
    rpage_system_flash();
    rpage_video_vsync();
    rpage_system_flash();
#endif
}

void rpage_system_flash(void)
{
    DisplayBeep(main_screen->screen);
}

ULONG rpage_get_clock(void)
{ 
    timer_device_get_system_time(&endTime);
    SubTime(&endTime, &startTime);
    return (endTime.tv_secs * 1000 + endTime.tv_micro / 1000);
}

/*
    FILE
    ----
*/

rpage_file rpage_file_open(char *filename, long mode)
{
    return (rpage_file)Open(filename, mode);
}

void rpage_file_close(rpage_file file)
{
    Close((BPTR)file);
}

long rpage_file_read(rpage_file file, void *buffer, long len)
{
    return Read((BPTR)file, buffer, len);
}

long rpage_file_write(rpage_file file, void *buffer, long len)
{
    return Write((BPTR)file, buffer, len);
}

/*
    VIDEO (video framebuffer access)
    --------------------------------
*/

void rpage_video_open(int rpage_video_open)
{
#ifdef DEBUG_MACROS
    printf("rpage_video_open()\n");
#endif
    if (main_screen == NULL)
    {
        // if (rpage_video_open == mode_lowres)
            main_screen = openMainScreen();
            scr_x_offset = 0;
            scr_y_offset = 0;
        // else
        //     main_screen = openMainScreenCustom(320, 512, 32, FALSE); /* Double buffer is DISABLED */
        temp_raster = NULL; // rpage_os_alloc(RASSIZE(SCREEN_WIDTH, SCREEN_HEIGHT), MEMF_CHIP);

        main_font = NULL;
    }
    else
    {
        rpage_system_alert("A screen is already open!");
        exit(0);
    }
}

buffered_screen *rpage_video_get_ptr(void)
{
    return main_screen;
}

void rpage_video_screen_to_front(void)
{
    ScreenToFront(main_screen->screen);
    // WindowToFront(main_screen->window);
}

void rpage_video_screen_to_back(void)
{
    ScreenToBack(main_screen->screen);
    // WindowToFront(main_screen->window);
}

UWORD __inline rpage_video_get_depth(void)
{
    return screenGetDepth();
}

void __inline rpage_video_wait_dma(void)
{
    WaitBlit();
}

void __inline rpage_video_vsync(void)
{
    WaitVBL(main_screen);
}

void __inline rpage_video_flip_buffers(void)
{
    flipBuffers(main_screen);
}

void __inline rpage_video_present_screen(void)
{
    presentScreen(main_screen);
}

void rpage_video_wipe_rect_to_physical_screen(rect *r)
{
    #define SCREEN_WIPE_STRIDE 8
    short i,j,k;
    short p,l;
    amiga_color p_pal[1 << SCREEN_DEPTH], l_pal[1 << SCREEN_DEPTH], fade_pal[1 << SCREEN_DEPTH];

    p = getPhysicalBitmapIndex(main_screen);
    l = getLogicalBitmapIndex(main_screen);

    memcpy(p_pal, main_screen->palettes[p], sizeof(amiga_color) * (1 << SCREEN_DEPTH));
    memcpy(l_pal, main_screen->palettes[l], sizeof(amiga_color) * (1 << SCREEN_DEPTH));

    for(i = 0; i < SCREEN_WIPE_STRIDE; i++)
    {
        rpage_video_vsync();
        for(k = 0; k < 1 << SCREEN_DEPTH; k++)
            fade_pal[k] = mix_rgb4_colors(l_pal[k], p_pal[k], i << 5);
        rpage_video_set_palette(fade_pal, 1 << SCREEN_DEPTH);

        presentPalette(main_screen);

        for(j = r->sy; j < r->ey; j += SCREEN_WIPE_STRIDE)
        {
            BltBitMap(main_screen->bitmaps[p], r->sx, j + i, main_screen->bitmaps[l], 
                        r->sx, j + i, r->ex - r->sx, 1 , 0xC0, 0xFF, temp_raster);
        }
    }

    rpage_video_set_palette(p_pal, 1 << SCREEN_DEPTH);
    presentPalette(main_screen);
}

void __inline rpage_prepare_fast_dbuffer(void)
{
    prepareFastDoubleBuffer(main_screen);
}

void __inline rpage_video_present_screen_fast(void)
{
    presentScreenFast(main_screen);
}

void __inline rpage_video_present_palette(void)
{
    presentPalette(main_screen);
}

void rpage_video_sync_buffers(void)
{
    synchronizeBuffers(main_screen);
}

void rpage_video_clear_bit_mask(UBYTE bit_mask)
{
    if (main_screen != NULL)
    {
        short i;
        for (i = 0; i < SCREEN_DEPTH; i++)
            if (bit_mask & (1 << i))
                BltClear(main_screen->screen->RastPort.BitMap->Planes[i], RASSIZE(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
    }
    else
    {
        rpage_system_alert("No screen was found open!");
        exit(0);
    } 
}

void rpage_video_clear(void)
{
    // SetRast(&(main_screen->RastPort), 0);
    // Move(&(main_screen->RastPort), 0, 0);
    // ClearScreen(&(main_screen->RastPort));
    if (main_screen != NULL)
    {
        short i;
        for (i = 0; i < SCREEN_DEPTH; i++)
        {
            // memset(main_screen->RastPort.BitMap->Planes[i], 0x0, RASSIZE(main_screen->Width, main_screen->Height));
            BltClear(main_screen->screen->RastPort.BitMap->Planes[i], RASSIZE(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
            // WaitBlit();
        }
    }
    else
    {
        rpage_system_alert("No screen was found open!");
        exit(0);
    }
}

// struct ViewPort *rpage_video_get_viewport(void)
// {
//     return &(main_screen->screen->ViewPort);
// }

void __inline rpage_video_scroll(short x, short y)
{
    if ((scr_x_offset != x) || (scr_y_offset != y))
    {
        main_screen->screen->ViewPort.DxOffset = x;
        main_screen->screen->ViewPort.DyOffset = y;
        ScrollVPort(&(main_screen->screen->ViewPort));
    }

    scr_x_offset = x;
    scr_y_offset = y;
}

void rpage_video_set_immediate_RGB444(short color_idx, color444 A)
{
    ULONG r,g,b;

	r = ((ULONG)(A & 0x0f00)) >> 8;
	g = (A & 0x00f0) >> 4;
	b = (A & 0x000f);

    SetRGB4(&(main_screen->screen->ViewPort), color_idx, r, g, b);
    rpage_video_vsync();
}

void __inline rpage_video_scroll_bit_mask(short x, short y, UBYTE bit_mask)
{
    short src_x, src_y, dst_x, dst_y;

    if (x < 0)
    {
        src_x = -x;
        dst_x = 0;
    }
    else
    {
        src_x = 0;
        dst_x = x;
    }

    if (y < 0)
    {
        src_y = -y;
        dst_y = 0;
    }
    else
    {
        src_y = 0;
        dst_y = y;
    }

    BltBitMap(main_screen->bitmaps[main_screen->physical], src_x, src_y, main_screen->bitmaps[main_screen->physical], dst_x, dst_y, SCREEN_WIDTH - abs(x), SCREEN_HEIGHT - abs(y), 0xC0, bit_mask, temp_raster);
}

rpage_bitmap *rpage_video_get_front_bitmap(void)
{
    return main_screen->bitmaps[getPhysicalBitmapIndex(main_screen)];
}

rpage_bitmap *rpage_video_get_back_bitmap(void)
{
    return main_screen->bitmaps[getLogicalBitmapIndex(main_screen)];
}

rpage_palette *rpage_video_get_front_palette(void)
{
    return main_screen->palettes[getPhysicalBitmapIndex(main_screen)];
}

void __inline rpage_bitmap_blit(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *dest_bitmap)
{
    BltBitMap(source_bitmap, source_x, source_y, dest_bitmap, x, y, width, height, 0xC0, 0xFF, temp_raster);
    // WaitBlit();
}

void __inline rpage_bitmap_blit_mask(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *dest_bitmap, rpage_bitmap *mask_bitmap)
{
    struct RastPort tmp_rp;
    memcpy(&tmp_rp, &(main_screen->screen->RastPort), sizeof(struct RastPort));
    tmp_rp.BitMap = dest_bitmap;
    BltMaskBitMapRastPort(source_bitmap, source_x, source_y, &tmp_rp, x, y, width, height, (ABC|ABNC|ANBC), mask_bitmap->Planes[0]);
}

void __inline rpage_video_save_to_bitmap(rpage_bitmap *dest_bitmap, short source_x, short source_y, short width, short height)
{
    BltBitMap(main_screen->bitmaps[main_screen->physical], source_x, source_y, dest_bitmap, 0, 0, width, height, 0xC0, 0xFF, temp_raster);
}

void __inline rpage_video_save_to_bitmap_ex(rpage_bitmap *dest_bitmap, short source_x, short source_y, short width, short height, short dest_x, short dest_y)
{
    BltBitMap(main_screen->bitmaps[main_screen->physical], source_x, source_y, dest_bitmap, dest_x, dest_y, width, height, 0xC0, 0xFF, temp_raster);
}

void __inline rpage_video_blt_bmp(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y)
{
    BltBitMap(source_bitmap, source_x, source_y, main_screen->bitmaps[main_screen->physical], x, y, width, height, 0xC0, 0xFF, temp_raster);
    // WaitBlit();
}

void __inline rpage_video_blt_bmp_bt(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, UBYTE bit_mask)
{
    BltBitMap(source_bitmap, source_x, source_y, main_screen->bitmaps[main_screen->physical], x, y, width, height, 0xC0, bit_mask, main_screen->rp->TmpRas->RasPtr);
}

void __inline rpage_video_blt_bmp_mask(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *mask_bitmap)
{
    BltMaskBitMapRastPort(source_bitmap, source_x, source_y, &(main_screen->screen->RastPort), x, y, width, height, (ABC|ABNC|ANBC), mask_bitmap->Planes[0]);
    // WaitBlit();
}

void __inline rpage_video_blt_bmp_clip(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rect *clipping_rect)
{
    if (x < clipping_rect->sx)
    {
        short clip_x = clipping_rect->sx - x;
        x += clip_x;
        source_x += clip_x;
        width -= clip_x;
    }
    else
    {
        if (x + width > clipping_rect->ex)
            width -= (x + width - clipping_rect->ex);
    }

    if (width <= 0)
        return;
    
    if (y < clipping_rect->sy)
    {
        short clip_y = clipping_rect->sy - y;
        y += clip_y;
        source_y += clip_y;
        height -= clip_y;
    }
    else
    {
        if (y + height > clipping_rect->ey)
            height -= (y + height - clipping_rect->ey);
    }
    
    if (height <= 0)
        return;

    BltBitMap(source_bitmap, source_x, source_y, main_screen->bitmaps[main_screen->physical], x, y, width, height, 0xC0, 0xFF, temp_raster);
    // WaitBlit();
}

void __inline rpage_video_blt_bmp_clip_mask(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *mask_bitmap, rect *clipping_rect)
{
    if (x < clipping_rect->sx)
    {
        short clip_x = clipping_rect->sx - x;
        x += clip_x;
        source_x += clip_x;
        width -= clip_x;
    }
    else
    {
        if (x + width > clipping_rect->ex)
            width -= (x + width - clipping_rect->ex);
    }

    if (width <= 0)
        return;
    
    if (y < clipping_rect->sy)
    {
        short clip_y = clipping_rect->sy - y;
        y += clip_y;
        source_y += clip_y;
        height -= clip_y;
    }
    else
    {
        if (y + height > clipping_rect->ey)
            height -= (y + height - clipping_rect->ey);
    }
    
    if (height <= 0)
        return;

    BltMaskBitMapRastPort(source_bitmap, source_x, source_y, &(main_screen->screen->RastPort), x, y, width, height, (ABC|ABNC|ANBC), mask_bitmap->Planes[0]);
    // WaitBlit();
}

void __inline rpage_video_blt_bmp_mask_bt(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *mask_bitmap, UBYTE bit_mask)
{
    // UBYTE tmp_mask;
    
    // tmp_mask = main_screen->screen->RastPort.Mask;
    main_screen->screen->RastPort.Mask = bit_mask;
    BltMaskBitMapRastPort(source_bitmap, source_x, source_y, &(main_screen->screen->RastPort), x, y, width, height, (ABC|ABNC|ANBC), mask_bitmap->Planes[0]);
    main_screen->screen->RastPort.Mask = 0xFF;
}

void __inline rpage_video_blt_bmp_clip_mask_bt(rpage_bitmap *source_bitmap, short source_x, short source_y, short width, short height, short x, short y, rpage_bitmap *mask_bitmap, rect *clipping_rect, UBYTE bit_mask)
{
    // UBYTE tmp_mask;
    
    if (x < clipping_rect->sx)
    {
        short clip_x = clipping_rect->sx - x;
        x += clip_x;
        source_x += clip_x;
        width -= clip_x;
    }
    else
    {
        if (x + width > clipping_rect->ex)
            width -= (x + width - clipping_rect->ex);
    }

    if (width <= 0)
        return;
    
    if (y < clipping_rect->sy)
    {
        short clip_y = clipping_rect->sy - y;
        y += clip_y;
        source_y += clip_y;
        height -= clip_y;
    }
    else
    {
        if (y + height > clipping_rect->ey)
            height -= (y + height - clipping_rect->ey);
    }
    
    if (height <= 0)
        return;

    // tmp_mask = main_screen->screen->RastPort.Mask;
    main_screen->screen->RastPort.Mask = bit_mask;
    BltMaskBitMapRastPort(source_bitmap, source_x, source_y, &(main_screen->screen->RastPort), x, y, width, height, (ABC|ABNC|ANBC), mask_bitmap->Planes[0]);
    main_screen->screen->RastPort.Mask = 0xFF;
    // WaitBlit();
}

void __inline rpage_video_set_palette(rpage_palette *palette, short palette_size)
{
    short i;
    for(i = 0; i < palette_size; i++)
        main_screen->palettes[main_screen->physical][i] = palette[i];
    // set_palette(main_screen->palettes[main_screen->physical], &palette, 0, palette_size - 1);
    // set_palette(&(main_screen->screen->ViewPort), &palette, 0, palette_size - 1);
}

void __inline rpage_video_set_palette_to_black(short first_color, short last_color)
{
	short i;
	for (i = first_color; i <= last_color; i++)
		main_screen->palettes[main_screen->physical][i] = 0x0;
}

void __inline rpage_video_set_palette_to_grey(short first_color, short last_color)
{
	int loop;
    int *luma, tmp_col;

    luma = color_luma;

    for (loop = first_color; loop <= last_color; loop++)
        luma[loop] = range_adjust(loop, first_color, last_color, 0, 255);

    // Swap some of the colors (19 <-> 31, 18 <-> 4),
    // so that the Amiga cursor looks ok.
    if (last_color >= 31)
    {
        tmp_col = luma[19];
        luma[19] = luma[31];
        luma[31] = tmp_col;

        tmp_col = luma[18];
        luma[18] = luma[4];
        luma[4] = tmp_col;
    }

    // Set the colors
    for (loop = first_color; loop <= last_color; loop++)
    {
        short _r, _g, _b;
#ifdef VGA_ENABLED
        main_screen->palettes[main_screen->physical][loop] = components_to_rgb8(luma[loop], luma[loop], luma[loop]);
#else
        _r = min(255, (luma[loop] * 350) / 255);
        _g =  (((luma[loop] * luma[loop]) / 255) + luma[loop]) / 2;
        _b = (luma[loop] * luma[loop]) / 350;

        _r = (_r + luma[loop]) / 2;
        _g = (_g + luma[loop]) / 2;
        _b = (_b + luma[loop]) / 2;

        _r = (_r + luma[loop]) / 2;
        _g = (_g + luma[loop]) / 2;
        _b = (_b + luma[loop]) / 2;            
        
        main_screen->palettes[main_screen->physical][loop] = components_to_rgb4(_r, _g, _b);
#endif
    }
}

void rpage_video_show_freemem(short x, short y, short width, short height)
{
    rect r;
    r.sx = x;
    r.sy = y;
    r.ex = x + width;
    r.ey = y + height;
    rpage_video_fill_rect(&r, 0);

    r.sx = x + 1;
    r.sy = y + 1;
    r.ex = x + width - 1;
    r.ey = y + height - 1;
    rpage_video_fill_rect(&r, 2);    

    r.sx = x + 1;
    r.sy = y + 1;
    r.ex = x + (width * rpage_get_avail_video_memory()) / (g_max_non_video_ram + g_max_video_ram);
    r.ey = y + height - 1;
    rpage_video_fill_rect(&r, 20);
    rpage_video_draw_text("CHIP", r.sx + 4, r.sy - 2, 0);
    rpage_video_draw_text("CHIP", r.sx + 4, r.sy - 3, 30);

    if (g_max_non_video_ram > 0)
    {
        r.sx = r.ex + 1;
        r.ex = r.sx - 1 + (width * rpage_get_avail_non_video_memory()) / (g_max_non_video_ram + g_max_video_ram);
        rpage_video_fill_rect(&r, 8);
        rpage_video_draw_text("OTHER", r.sx + 4, r.sy - 2, 0);
        rpage_video_draw_text("OTHER", r.sx + 4, r.sy - 3, 30);
    }
    
}

void rpage_video_draw_tileset(rpage_bitmap *tileset_bitmap, UBYTE *tileset, rect *tile_rect, short tileset_width)
{
    UBYTE x , y, tile_idx, prev_tile_idx = 0xFF;
    UBYTE tile_x, tile_y,  tileset_bitmap_width = rpage_bitmap_get_width(tileset_bitmap) >> 3;

    for(y = (UBYTE)(tile_rect->sy); y < tile_rect->ey; y++)
    {
        unsigned short y_w = y * tileset_width;
        for(x = (UBYTE)(tile_rect->sx); x < tile_rect->ex; x++)
        {
            tile_idx = tileset[x + y_w];
            if (tile_idx > 0)
            {
                if (prev_tile_idx != tile_idx) /* let's avoid 2 divide ops. if possible */ 
                {
                    tile_x = (tile_idx%tileset_bitmap_width) << 3;
                    tile_y = (tile_idx/tileset_bitmap_width) << 3;
                }
#ifdef LATTICE
                BltBitMap(tileset_bitmap, tile_x, tile_y, main_screen->bitmaps[main_screen->physical], x << 3, y << 3, 8, 8, 0xC0, 0xFF, temp_raster);
#else
                rpage_video_blt_bmp(tileset_bitmap, tile_x, tile_y, 8, 8, x << 3, y << 3);
#endif
            }
            prev_tile_idx = tile_idx;
        }
    }
}

void rpage_bitmap_draw_tileset(rpage_bitmap *dest_bitmap, rpage_bitmap *tileset_bitmap, UBYTE *tileset, rect *tile_rect, short tileset_width)
{
    UBYTE x , y, tile_idx, prev_tile_idx = 0xFF;
    UBYTE tile_x, tile_y,  tileset_bitmap_width = rpage_bitmap_get_width(tileset_bitmap) >> 3;

    for(y = (UBYTE)(tile_rect->sy); y < tile_rect->ey; y++)
    {
        unsigned short y_w = y * tileset_width;
        for(x = (UBYTE)(tile_rect->sx); x < tile_rect->ex; x++)
        {
            tile_idx = tileset[x + y_w];
            if (tile_idx > 0)
            {
                if (prev_tile_idx != tile_idx) /* let's avoid 2 divide ops. if possible */ 
                {
                    tile_x = (tile_idx%tileset_bitmap_width) << 3;
                    tile_y = (tile_idx/tileset_bitmap_width) << 3;
                }
                rpage_bitmap_blit(tileset_bitmap, tile_x, tile_y, 8, 8, x << 3, y << 3, dest_bitmap);
            }
            prev_tile_idx = tile_idx;
        }
    }
}

void __inline rpage_video_fill_rect(rect *r, short color)
{
    if (color < 0)
        color = (1 << SCREEN_DEPTH) - 1;
    SetAPen(&(main_screen->screen->RastPort), color);
    RectFill( &(main_screen->screen->RastPort), r->sx, r->sy, r->ex, r->ey);
}

void __inline rpage_video_fill_rect_clip(rect *r, short color, rect *clipping_rect)
{
    rect video_rect;
    // UBYTE tmp_mask;

    video_rect.sx = r->sx;
    video_rect.sy = r->sy;
    video_rect.ex = r->ex;
    video_rect.ey = r->ey;

    if (video_rect.sx < clipping_rect->sx)
        video_rect.sx = clipping_rect->sx;
    else
    {
        if (video_rect.ex > clipping_rect->ex)
            video_rect.ex = clipping_rect->ex;
    }

    if (video_rect.ex <= video_rect.sx)
        return;

    if (video_rect.sy < clipping_rect->sy)
        video_rect.sy = clipping_rect->sy;
    else
    {
        if (video_rect.ey > clipping_rect->ey)
            video_rect.ey = clipping_rect->ey;
    }
    
    if (video_rect.ey <= video_rect.sy)
        return;

    if (color < 0)
        color = (1 << SCREEN_DEPTH) - 1;
    SetAPen(&(main_screen->screen->RastPort), color);
    // tmp_mask = main_screen->screen->RastPort.Mask;
    main_screen->screen->RastPort.Mask = 0x8;    
    RectFill( &(main_screen->screen->RastPort), video_rect.sx, video_rect.sy, video_rect.ex, video_rect.ey);
    main_screen->screen->RastPort.Mask = 0xFF;
}

void rpage_video_draw_polygon(poly *p, short color)
{
    if (color < 0)
        color = (1 << SCREEN_DEPTH) - 1;
    SetAPen(&(main_screen->screen->RastPort), color);
    Move(&(main_screen->screen->RastPort), p->p0.x, p->p0.y);
    Draw(&(main_screen->screen->RastPort), p->p1.x, p->p1.y);          
    Draw(&(main_screen->screen->RastPort), p->p2.x, p->p2.y);          
    Draw(&(main_screen->screen->RastPort), p->p3.x, p->p3.y);          
    Draw(&(main_screen->screen->RastPort), p->p0.x, p->p0.y);          
}

void rpage_video_draw_rect(rect *r, short color)
{
    rect video_rect;

    video_rect.sx = 0;
    video_rect.sy = 0;
    video_rect.ex = SCREEN_WIDTH - 1;
    video_rect.ey = SCREEN_HEIGHT - 1;

    if (color < 0)
        color = (1 << SCREEN_DEPTH) - 1;
    SetAPen(&(main_screen->screen->RastPort), color);
    Move(&(main_screen->screen->RastPort), max(r->sx, video_rect.sx), max(r->sy, video_rect.sy));
    Draw(&(main_screen->screen->RastPort), min(r->ex, video_rect.ex), max(r->sy, video_rect.sy));
    Draw(&(main_screen->screen->RastPort), min(r->ex, video_rect.ex), min(r->ey, video_rect.ey));
    Draw(&(main_screen->screen->RastPort), max(r->sx, video_rect.sx), min(r->ey, video_rect.ey));
    Draw(&(main_screen->screen->RastPort), max(r->sx, video_rect.sx), max(r->sy, video_rect.sy));
}

void __inline rpage_video_set_pixel(short x, short y, short color)
{
    if (color < 0)
        color = (1 << SCREEN_DEPTH) - 1;    
    SetAPen(&(main_screen->screen->RastPort), color);
    WritePixel(&(main_screen->screen->RastPort), x, y);
}

void __inline rpage_bitmap_set_pixel(rpage_bitmap *dest_bitmap, short x, short y, short color)
{
    struct RastPort tmp_rp;
    memcpy(&tmp_rp, &(main_screen->screen->RastPort), sizeof(struct RastPort));
    tmp_rp.BitMap = dest_bitmap;
    if (color < 0)
        color = (1 << SCREEN_DEPTH) - 1;    
    SetAPen(&tmp_rp, color);
    WritePixel(&tmp_rp, x, y);    
}

short __inline rpage_video_get_pixel(short x, short y)
{
    return (short)ReadPixel(&(main_screen->screen->RastPort), x, y);
}

short __inline rpage_bitmap_get_pixel(rpage_bitmap *dest_bitmap, short x, short y)
{
    struct RastPort tmp_rp;
    memcpy(&tmp_rp, &(main_screen->screen->RastPort), sizeof(struct RastPort));
    tmp_rp.BitMap = dest_bitmap;    
    return (short)ReadPixel(&tmp_rp, x, y);
}

void rpage_video_set_font(char *font_filename, short font_size)
{
    struct TextAttr ta;

    if (main_font != NULL)
        CloseFont(main_font);

    ta.ta_Name = font_filename;
    ta.ta_YSize = font_size;
    ta.ta_Flags = FPB_DISKFONT | FPF_DESIGNED;
    ta.ta_Style = FS_NORMAL;

    main_font = OpenDiskFont(&ta);
    if (main_font)
        SetFont(&(main_screen->screen->RastPort), main_font);
    else
    {
#ifdef DEBUG_MACROS        
        printf("Cannot open font %s!", font_filename);
#endif
        main_font = NULL;
    }
}

short rpage_video_get_text_width(char *str)
{
    return  (short)TextLength(&(main_screen->screen->RastPort), str, strlen(str));
}

void rpage_video_draw_text_bit_mask(char *str, short x, short y, short color_index, UBYTE bit_mask)
{
    // UBYTE tmp_mask;
    // tmp_mask = main_screen->screen->RastPort.Mask;
    main_screen->screen->RastPort.Mask = bit_mask;
    rpage_video_draw_text(str, x, y, color_index);
    main_screen->screen->RastPort.Mask = 0xFF;
}

void rpage_video_draw_text(char *str, short x, short y, short color)
{
    if (color < 0)
        color = (1 << SCREEN_DEPTH) - 1;

    if (x < 0)
        x = (SCREEN_WIDTH - TextLength(&(main_screen->screen->RastPort), str, strlen(str))) >> 1;
    if (y < 0)
        y = (SCREEN_HEIGHT - 8) >> 1;

    SetAPen(&(main_screen->screen->RastPort), color);
    SetBPen(&(main_screen->screen->RastPort), 0);
    Move(&(main_screen->screen->RastPort), x, y + 8);
    SetDrMd(&(main_screen->screen->RastPort), 0);
    Text(&(main_screen->screen->RastPort), str, strlen(str));
}

void rpage_video_draw_shadow_text(char *str, short x, short y, unsigned short text_color, unsigned short shadow_color)
{
    short l = strlen(str);

    if (text_color < 0)
        text_color = (1 << SCREEN_DEPTH) - 1;

    if (x < 0)
        x = (SCREEN_WIDTH - TextLength(&(main_screen->screen->RastPort), str, l)) >> 1;
    if (y < 0)
        y = (SCREEN_HEIGHT - 8) >> 1;

    y += 8;

    SetBPen(&(main_screen->screen->RastPort), 0);
    SetDrMd(&(main_screen->screen->RastPort), 0);

    SetAPen(&(main_screen->screen->RastPort), shadow_color);
    Move(&(main_screen->screen->RastPort), x + 1, y);
    Text(&(main_screen->screen->RastPort), str, l);
    Move(&(main_screen->screen->RastPort), x + 1, y + 1);
    Text(&(main_screen->screen->RastPort), str, l);

    SetAPen(&(main_screen->screen->RastPort), text_color);
    Move(&(main_screen->screen->RastPort), x, y);
    Text(&(main_screen->screen->RastPort), str, l);
}

void rpage_video_close(void)
{
#ifdef DEBUG_MACROS
    printf("rpage_video_close()\n");
#endif
    if (main_font)
    {
        CloseFont(main_font);
        main_font = NULL;
    }

    closeMainScreen(main_screen);
    main_screen = NULL;

    // rpage_free_os_alloc(temp_raster, RASSIZE(SCREEN_WIDTH, SCREEN_HEIGHT));
    temp_raster = NULL;
}

/*
    BITMAP (direct bitmap access)
    -----------------------------
*/

ULONG rpage_bitmap_calculate_bytesize(short width, short height, short depth)
{
    return (RASSIZE(width, height) * depth);
}

rpage_bitmap *rpage_bitmap_new(short width, short height, short depth)
{
    return (rpage_bitmap *)allocate_new_bitmap(width, height, depth);
}

BOOL rpage_load_pak_into_bitmap(rpage_bitmap **bitmap, rpage_palette **palette, BYTE *packed_buffer, char *bitmap_filename)
{
    return load_pak_img_to_bitmap((struct BitMap **)bitmap, (UWORD **)palette, packed_buffer, bitmap_filename);
}


short rpage_bitmap_get_width(rpage_bitmap *bitmap)
{
    return(short)(bitmap->BytesPerRow << 3);
}

short rpage_bitmap_get_height(rpage_bitmap *bitmap)
{
    return(short)(bitmap->Rows);
}

short rpage_bitmap_get_depth(rpage_bitmap *bitmap)
{
    return(short)(bitmap->Depth);
}

void rpage_bitmap_clear(rpage_bitmap *bitmap)
{
    short i;
    for (i = 0; i < rpage_bitmap_get_depth(bitmap); i++)
    {
        // memset(main_screen->RastPort.BitMap->Planes[i], 0x0, RASSIZE(main_screen->Width, main_screen->Height));
        BltClear(bitmap->Planes[i], rpage_bitmap_calculate_bytesize(rpage_bitmap_get_width(bitmap), rpage_bitmap_get_height(bitmap), 1), 0);
    }    
}

BOOL rpage_load_pak_to_new_bitmap(rpage_bitmap **new_bitmap, rpage_palette **new_palette, BYTE *packed_buffer, char *bitmap_filename)
{
    return load_pak_img_to_new_bitmap((struct BitMap **)new_bitmap, (UWORD **)new_palette, packed_buffer, bitmap_filename);
}

rpage_bitmap *rpage_build_bitmap_mask(rpage_bitmap *source_bitmap)
{
    // TODO
    return NULL;
}

void rpage_bitmap_free(rpage_bitmap *bitmap)
{
    if (bitmap != NULL)
        free_allocated_bitmap(bitmap);
}

/*

    HARDWARE SPRITES (specific to the Amiga)
    ----------------------------------------
*/

void rpage_move_sprite(short sprite_index, rpage_hardware_sprite *sprite, vec2 *position)
{
    // If the sprite doesn't exist yet, we need to create it
    if (!sprites_enabled[sprite_index])
    {
        short pick; 
        // FreeSprite(sprite_index);
        pick = GetSprite((struct SimpleSprite *)sprite, sprite_index);
        if (pick > -1)
            sprites_enabled[sprite_index] = TRUE;
#ifdef DEBUG_MACROS            
        else
            printf("rpage_move_sprite() GetSprite() returned %d!\n", pick);
#endif
    }

    if (sprites_enabled[sprite_index])
        MoveSprite(&(main_screen->screen->ViewPort), (struct SimpleSprite *)sprite, position->x, position->y);
}

void __inline rpage_move_sprite_fast(short sprite_index, rpage_hardware_sprite *sprite, vec2 *position)
{
    MoveSprite(&(main_screen->screen->ViewPort), (struct SimpleSprite *)sprite, position->x, position->y);
}

void rpage_remove_sprite(short sprite_index)
{
    if (sprites_enabled[sprite_index])
        FreeSprite(sprite_index);
    sprites_enabled[sprite_index] = FALSE;
}

BOOL rpage_sprite_is_enabled(short sprite_index)
{
    return sprites_enabled[sprite_index];
}

/*
    INPUT (mouse, keyboard...)
    --------------------------
*/

BOOL rpage_input_init(void)
{
    if (main_screen->screen == NULL)
    {
        rpage_system_alert("Cannot init. input without a screen open !");
        return FALSE;
    }

    if (main_screen->window == NULL)
    {
        rpage_system_alert("Cannot init. input without a window open !");
        return FALSE;
    }

    input_window_init(main_screen->window);
}

void rpage_input_enable(BOOL enabled)
{
    input_enabled = enabled;
    prev_input_mouse_button = input_mouse_button;
    prev_input_mouse_position = input_mouse_position;
    input_mouse_button = 0;
}

void rpage_input_update(void)
{
    prev_input_mouse_button = input_mouse_button;
    prev_input_mouse_position = input_mouse_position;

    if (input_enabled)
        input_update(&input_mouse_button, &(input_mouse_position.x), &(input_mouse_position.y), &input_rawkey);
}

void rpage_mouse_button_flush(void)
{
    ActivateWindow(main_screen->window); // this patch helps the main window to get the focus after game init
    input_mouse_button = 0;
}

unsigned short rpage_keyboard_rawkey(void)
{
    return input_rawkey;
}

BOOL __inline rpage_mouse_button_left_is_down(void)
{
    if ((input_mouse_button & PLATFORM_MOUSE_LEFT_BUTTON))
        return TRUE;

    return FALSE;
}

BOOL __inline rpage_mouse_button_right_is_down(void)
{
    if ((input_mouse_button & PLATFORM_MOUSE_RIGHT_BUTTON))
        return TRUE;

    return FALSE;
}

BOOL __inline rpage_mouse_button_left_was_down(void)
{
    if ((input_mouse_button & PLATFORM_MOUSE_LEFT_BUTTON) && !(prev_input_mouse_button & PLATFORM_MOUSE_LEFT_BUTTON))
        return TRUE;

    return FALSE;
}

BOOL __inline rpage_mouse_button_right_was_down(void)
{
    if ((input_mouse_button & PLATFORM_MOUSE_RIGHT_BUTTON) && !(prev_input_mouse_button & PLATFORM_MOUSE_RIGHT_BUTTON))
        return TRUE;

    return FALSE;
}

void __inline rpage_mouse_get_prev_values(short *button, vec2 *mouse_coords)
{
    if (button) *button = prev_input_mouse_button;
    if (mouse_coords) *mouse_coords = prev_input_mouse_position;   
}

void __inline rpage_mouse_get_values(short *button, vec2 *mouse_coords)
{
    // input_update(button, &(mouse_coords->x), &(mouse_coords->y));
    if (button) *button = input_mouse_button;
    if (mouse_coords) *mouse_coords = input_mouse_position; 
}

void rpage_mouse_set_bitmap(UWORD *sprite_data, vec2 *hotspot)
{
    if (main_screen != NULL && main_screen->screen->FirstWindow != NULL)
    {
#ifdef DEBUG_MACROS
        printf("rpage_mouse_set_bitmap()\n");
#endif
        SetPointer(main_screen->screen->FirstWindow, sprite_data, 16, 16, hotspot->x, hotspot->y);
    }
    else
        rpage_system_alert("Cannot set cursor state without a window ownership!");
}

void rpage_mouse_set_system_image(unsigned short img_index)
{
    if (main_screen != NULL && main_screen->screen->FirstWindow != NULL)
    {
#ifdef DEBUG_MACROS
        printf("rpage_mouse_set_system_img(%d)\n", img_index);
#endif
        SetPointer(main_screen->screen->FirstWindow, system_cursors_img[img_index], 16, 16, system_cursors_hotspot[img_index].x, system_cursors_hotspot[img_index].y);
    }
    else
        rpage_system_alert("Cannot set cursor state without a window ownership!");   
}

void rpage_mouse_show(void)
{
    rpage_mouse_set_system_image(MOUSE_CURSOR_POINT);
}

void rpage_mouse_wait(void)
{
    rpage_mouse_set_system_image(MOUSE_CURSOR_WAIT);
}

void rpage_mouse_read(void)
{
    rpage_mouse_set_system_image(MOUSE_CURSOR_READ);
}

void rpage_mouse_hide(void)
{
    if (main_screen != NULL && main_screen->screen->FirstWindow != NULL)
    {
#ifdef DEBUG_MACROS
        printf("rpage_mouse_hide()\n");
#endif
        SetPointer(main_screen->screen->FirstWindow, system_cursors_img[0], 0, 0, 0, 0);
    }
    else
        rpage_system_alert("Cannot set cursor state without a window ownership!");
}

#endif