/*  	Windless Bay, Mount Erebus.
	Original game by Francois "Astrofra" Gutherz. 
	Amiga version (C) RESISTANCE 2021 
*/

#include "rpage/aos/bitmap.h"
#include "rpage/aos/mouse_ptr.h"
#include "rpage/frwk.h"
#include "rpage/utils.h"
#include "rpage/easing.h"
#include "game/vue.h"
#include "game/gui.h"
#include "game/game.h"
#include "game/world.h"
#include "game/text.h"
#include "game/aos/assets.h"
#include "game/aos/sprites.h"
#include <stdio.h>

extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Custom far custom;

// Track how many chip mem is allocated to the GUI
ULONG gui_memory_footprint;

// extern UBYTE *unpacking_buffer;
extern rpage_bitmap *main_bitmap;

// Main GUI layout
rpage_bitmap *gui_bitmap;
extern UBYTE gui_tileset[];

BOOL gui_modal_is_open = FALSE;

// FX
short fx_shake;

// Inventory
rpage_bitmap *inventory_bitmap = NULL;
rpage_bitmap *inventory_dirty_bitmap = NULL;
rpage_bitmap *panel_back = NULL;
extern UBYTE inventory_tileset[];

rpage_bitmap *spr_inventory = NULL;
rpage_bitmap *spr_inventory_mask = NULL;

char *previous_tooltip;

// Travel menu
rpage_bitmap *travel_menu_bitmap = NULL;
extern UBYTE travel_menu_tileset[];

// Disk request
rpage_bitmap *disk_request_back = NULL;

// Compass
rpage_bitmap *gui_compass_hilite_bitmap, *gui_compass_hilite_bitmask;

// Action icons (take, drop, use, look...)
rpage_bitmap *icons_bitmap;

// NPC portraits
rpage_bitmap *character_bitmap, *character_bitmap_back;
rpage_bitmap *character_buffer;
BOOL gui_portrait_shown;

 // FIXME! this should be replaced by a proper sprite manager
rpage_hardware_sprite object_sprite;

const rect action_icons_area =
{
    ACTION_ICONS_AREA_X, ACTION_ICONS_AREA_Y, 
    ACTION_ICONS_AREA_X + ACTION_ICON_WIDTH * 3, ACTION_ICONS_AREA_Y + ACTION_ICON_HEIGHT * 3
};

const rect inventory_area =
{
    INVENTORY_POS_X, INVENTORY_POS_Y, 
    INVENTORY_POS_X + INVENTORY_WIDTH, INVENTORY_POS_Y + INVENTORY_HEIGHT
};

const rect compass_hilite_per_direction_src[4] =
{
    // box of each sprite in the source bitmap
    {0, 0, 16, 32},   // N 
    {16, 0, 48, 24},  // E
    {48, 0, 64, 32},  // S
    {64, 0, 96, 24}   // W
};

const vec2 compass_hilite_per_direction_dest[4] =
{
    // position of each sprite in the destination bitmap
    {288, 144},   // N 
    {288, 160},   // E
    {288, 168},   // S
    {272, 160}    // W
};

const poly compass_poly_per_direction[4] =
{
    {{FP16(296.25), FP16(173.75)}, {FP16(296.25 + 7.75), FP16(173.75 - 8.25 )}, {FP16(296.25 - 0.125), FP16(173.75 - 23.5)}, {FP16(296.25 - 8), FP16(173.75 - 8.5)}},               // N
    {{FP16(296.625), FP16(174)}, {FP16(296.625 + 7.375), FP16(174 - 8.375 )}, {FP16(296.625 + 23.625), FP16(174 - 0.5)}, {FP16(296.625 + 6.875), FP16(174 + 6.75)}},                // E
    {{FP16(296.375), FP16(174.25)}, {FP16(296.375 + 6.75), FP16(174.25 + 6.625)}, {FP16(296.375), FP16(174.25 + 23.375)}, {FP16(296.375 - 8.125), FP16(174.25 + 5.75)}},            // S
    {{FP16(296.375), FP16(173.875)}, {FP16(296.375 - 8.25), FP16(173.875 - 8.5)}, {FP16(296.375 - 23.375), FP16(173.875 - 0.75)}, {FP16(296.375 - 8.125), FP16(173.875 + 6.125)}}   // W
};

const rect blank_area = 
{
    COMPASS_X, (ACTION_ICONS_AREA_Y + ACTION_ICON_HEIGHT * 3) + 4,
    COMPASS_X + COMPASS_WIDTH, COMPASS_Y - 4
};

const rect compass_area = 
{
    COMPASS_X, COMPASS_Y,
    COMPASS_X + COMPASS_WIDTH, COMPASS_Y + COMPASS_HEIGHT
};

void gui_load(void)
{
    rect dest_rect = {0, 0, 0, 0};
    rpage_bitmap *tmp_bitmap;

    gui_memory_footprint = rpage_get_avail_video_memory();

    // Global game UI
    rpage_load_pak_to_new_bitmap(&gui_bitmap, NULL, game_get_unpacking_buffer(), asset_build_device_path("gui", ".pak"));

    // Compass hilited sprites (+ 1bit mask)
    rpage_load_pak_to_new_bitmap(&gui_compass_hilite_bitmap, NULL, game_get_unpacking_buffer(), asset_build_device_path("gui_compass_hilite", ".pak"));
    rpage_load_pak_to_new_bitmap(&gui_compass_hilite_bitmask, NULL, game_get_unpacking_buffer(), asset_build_device_path("gui_compass_hilite", "_mask.pak"));

    // Action icons
    rpage_load_pak_to_new_bitmap(&icons_bitmap, NULL, game_get_unpacking_buffer(), asset_build_device_path("icons", ".pak"));

    // Inventory panel
    rpage_load_pak_to_new_bitmap(&tmp_bitmap, NULL, game_get_unpacking_buffer(), asset_build_device_path("inventory_panel", ".pak"));
    inventory_bitmap = rpage_bitmap_new(INVENTORY_TILE_WIDTH << 3, INVENTORY_TILE_HEIGHT << 3, 5);
    dest_rect.ex = INVENTORY_TILE_WIDTH;
    dest_rect.ey = INVENTORY_TILE_HEIGHT;
    rpage_bitmap_draw_tileset(inventory_bitmap, tmp_bitmap, inventory_tileset, &dest_rect, INVENTORY_TILE_WIDTH);
    rpage_video_wait_dma();
    rpage_bitmap_free(tmp_bitmap);

    inventory_dirty_bitmap = rpage_bitmap_new(INVENTORY_TILE_WIDTH << 3, INVENTORY_TILE_HEIGHT << 3, SCREEN_DEPTH);

    // memory area to save & restore the bitmaps behind the inventory panel
    panel_back = rpage_bitmap_new(INVENTORY_TILE_WIDTH << 3, INVENTORY_TILE_HEIGHT << 3, SCREEN_DEPTH);

    // Inventory sprites
    rpage_load_pak_to_new_bitmap(&spr_inventory, NULL, game_get_unpacking_buffer(), asset_build_device_path("spr_inventory", ".pak"));
    rpage_load_pak_to_new_bitmap(&spr_inventory_mask, NULL, game_get_unpacking_buffer(), asset_build_device_path("spr_inventory", "_mask.pak"));

    // Travel menu
    rpage_load_pak_to_new_bitmap(&tmp_bitmap, NULL, game_get_unpacking_buffer(), asset_build_device_path("travel_menu", ".pak"));
    travel_menu_bitmap = rpage_bitmap_new(TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT, SCREEN_DEPTH);
    dest_rect.ex = TRAVEL_MENU_TILE_WIDTH;
    dest_rect.ey = TRAVEL_MENU_TILE_HEIGHT;    
    rpage_bitmap_draw_tileset(travel_menu_bitmap, tmp_bitmap, travel_menu_tileset, &dest_rect, TRAVEL_MENU_TILE_WIDTH);
    rpage_video_wait_dma();
    rpage_bitmap_free(tmp_bitmap);

    // Disk request
    disk_request_back = panel_back; // rpage_bitmap_new(TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT, SCREEN_DEPTH);

    // temporary buffer containing the bitmap of the character and the border decorations
    character_buffer = rpage_bitmap_new(NPC_SIZE + NPC_MARGIN_D, NPC_SIZE + NPC_MARGIN_D, SCREEN_DEPTH);
    gui_reset_portrait_shown();

    // NPC portrait display
    character_bitmap = rpage_bitmap_new(NPC_SIZE, NPC_SIZE, SCREEN_DEPTH);
    character_bitmap_back = rpage_bitmap_new(NPC_SIZE + NPC_MARGIN_D, NPC_SIZE + NPC_MARGIN_D, SCREEN_DEPTH);

    previous_tooltip = NULL;

    gui_set_modal_flag(FALSE); // Reset game

    // FX
    fx_shake = 0;

    gui_memory_footprint -= rpage_get_avail_video_memory();
}

ULONG gui_get_memory_footprint(void)
{
    return gui_memory_footprint;
}

void gui_unload(void)
{
    gui_memory_footprint = rpage_get_avail_video_memory();
    rpage_bitmap_free(gui_bitmap);
    rpage_bitmap_free(gui_compass_hilite_bitmap);
    rpage_bitmap_free(gui_compass_hilite_bitmask);
    rpage_bitmap_free(icons_bitmap);
    rpage_bitmap_free(inventory_bitmap);
    rpage_bitmap_free(inventory_dirty_bitmap);
    if (disk_request_back != panel_back)
        rpage_bitmap_free(disk_request_back);
    rpage_bitmap_free(panel_back);
    rpage_bitmap_free(travel_menu_bitmap);
    rpage_bitmap_free(spr_inventory);
    rpage_bitmap_free(spr_inventory_mask);
    rpage_bitmap_free(character_buffer);
    rpage_bitmap_free(character_bitmap);
    rpage_bitmap_free(character_bitmap_back);
    gui_memory_footprint = rpage_get_avail_video_memory() - gui_memory_footprint;

    gui_bitmap = NULL;
    gui_compass_hilite_bitmap = NULL;
    gui_compass_hilite_bitmask = NULL;
    icons_bitmap = NULL;
    inventory_bitmap = NULL;
    inventory_dirty_bitmap = NULL;
    disk_request_back = NULL;
    panel_back = NULL;
    travel_menu_bitmap = NULL;
    spr_inventory = NULL;
    spr_inventory_mask = NULL;
    character_buffer = NULL;
    character_bitmap = NULL;
    character_bitmap_back = NULL;
}

void gui_set_modal_flag(BOOL flag)
{
    gui_modal_is_open = flag;
}

BOOL gui_get_modal_flag(void)
{
    return gui_modal_is_open;
}

void gui_fadeout(short fadeout_len, rpage_palette *source_palette)
{
    rpage_palette pal0[1 << SCREEN_DEPTH]; // faded palette
    short t,i;

    //  Fade out
    for (t = 0; t <= fadeout_len; t++)
    {
        short _fade;
        rpage_video_vsync();
        _fade = (t * 255) / fadeout_len;
        for (i = 0; i < (1 << SCREEN_DEPTH); i++)
            pal0[i] = darken_rgb4_colors(source_palette[i], _fade);
        rpage_video_set_palette(pal0, (1 << SCREEN_DEPTH));
        rpage_video_present_palette();
    }
}

void gui_fade_to_white(short fadeout_len, rpage_palette *source_palette, BOOL order)
{
    rpage_palette pal0[1 << SCREEN_DEPTH], // faded palette
                    pal_white[1 << SCREEN_DEPTH];
    short t,i;
    ULONG c;
    UWORD d;

    for (i = 0; i < (1 << SCREEN_DEPTH); i++)
    {
        c = (i * 256) / (1 << SCREEN_DEPTH);
        c = int_sqrt(c * 270);
        c = min(0xFF, ((c * 3) / 2) + 96);
        // printf("%d, ", c);
        pal_white[i] = components_to_rgb4((UWORD)c, (UWORD)c, (UWORD)c);
    }

    // Swap some of the colors (19 <-> 31, 18 <-> 4),
    d = pal_white[19];
    pal_white[19] = pal_white[31];
    pal_white[31] = d;

    d = pal_white[18];
    pal_white[18] = pal_white[4];
    pal_white[4] = d;

    // printf("\n");

    //  Fade
    for (t = 0; t <= fadeout_len; t++)
    {
        short _fade;
        rpage_video_vsync();
        _fade = (t * 255) / fadeout_len;
        if (order)
        {
            for (i = 0; i < (1 << SCREEN_DEPTH); i++)
                pal0[i] = mix_rgb4_colors(pal_white[i], source_palette[i], _fade);
        }
        else
        {
            for (i = 0; i < (1 << SCREEN_DEPTH); i++)
                pal0[i] = mix_rgb4_colors(source_palette[i], pal_white[i], _fade);
        }
        rpage_video_set_palette(pal0, (1 << SCREEN_DEPTH));
        rpage_video_present_palette();
    }
}

void gui_fadein(short fadein_len, rpage_palette *target_palette)
{
    rpage_palette pal0[1 << SCREEN_DEPTH]; // faded palette
    short t,i;
    
    for (t = 0; t <= fadein_len; t++)
    {
        short _fade;
        rpage_video_vsync();
        _fade = (t * 255) / fadein_len;
        for (i = 0; i < (1 << SCREEN_DEPTH); i++)
            pal0[i] = darken_rgb4_colors(target_palette[i], 255 - _fade);
        rpage_video_set_palette(pal0, (1 << SCREEN_DEPTH));
        rpage_video_present_palette();
    }
}

void gui_fade_to_custom_palette(UWORD fadein_len, rpage_palette *original_palette, rpage_palette *target_palette)
{
    rpage_palette pal0[1 << SCREEN_DEPTH]; // faded palette
    UWORD t, i, _fade;

    for (t = 0; t <= fadein_len; t++)
    {
        rpage_video_vsync();
        _fade = (t * 255) / fadein_len;
        for (i = 0; i < (1 << SCREEN_DEPTH); i++)
            pal0[i] = mix_rgb4_colors(original_palette[i], target_palette[i], _fade);
        rpage_video_set_palette(pal0, (1 << SCREEN_DEPTH));
        rpage_video_present_palette();
    }
}

void gui_fade_to_grey(UWORD fadein_len, rpage_palette *original_palette)
{
    rpage_palette   pal0[1 << SCREEN_DEPTH], // faded palette
                            target_palette[1 << SCREEN_DEPTH]; // grey palette
    UWORD t, i, _fade;

    for(i = 0; i < 1 << SCREEN_DEPTH; i++)
    {
        UWORD r, g, b, luma;
        rgb4_to_components(original_palette[i], &r, &g, &b);
        luma = (r + g + b);
        luma = (luma * 255) / (0xF * 3);

        r = min(255, (luma * 350) / 255);
        g =  (((luma * luma) / 255) + luma) / 2;
        b = (luma * luma) / 350;

        r = (r + luma) / 2;
        g = (g + luma) / 2;
        b = (b + luma) / 2;

        r = (r + luma) / 2;
        g = (g + luma) / 2;
        b = (b + luma) / 2;  

        target_palette[i] = components_to_rgb4(r, g, b);
    }

    // rpage_video_set_palette(target_palette, (1 << SCREEN_DEPTH));
    // rpage_video_present_palette();

    for (t = 0; t <= fadein_len; t++)
    {
        rpage_video_vsync();
        _fade = (t * 255) / fadein_len;
        for (i = 0; i < (1 << SCREEN_DEPTH); i++)
            pal0[i] = mix_rgb4_colors(original_palette[i], target_palette[i], _fade);
        rpage_video_set_palette(pal0, (1 << SCREEN_DEPTH));
        rpage_video_present_palette();
    }
}

void gui_display(void)
{
    gui_display_tiles_block(0, 0, GUI_TILE_WIDTH, GUI_TILE_HEIGHT);
}

void __inline gui_update_fx(void)
{
    // Shake
    if (fx_shake >= 0)
    {
        int x_screen;
        x_screen = (sintab32[(fx_shake << 7) & 0x3FF] * fx_shake) >> 14;
        rpage_video_scroll(x_screen, 0);
        fx_shake--;
        if (fx_shake == 0)
            rpage_input_enable(TRUE);
    }

}

void gui_fx_shake(void)
{
    fx_shake = 30;
    rpage_input_enable(FALSE);
}

void gui_compass_refresh_direction(short direction)
{
    // rect direction_rect;

    if (gui_compass_hilite_bitmap && gui_compass_hilite_bitmask)
    {
        rpage_video_blt_bmp_mask(gui_compass_hilite_bitmap, 
            compass_hilite_per_direction_src[direction].sx, compass_hilite_per_direction_src[direction].sy, // source_x, source_y
            compass_hilite_per_direction_src[direction].ex - compass_hilite_per_direction_src[direction].sx, // width
            compass_hilite_per_direction_src[direction].ey - compass_hilite_per_direction_src[direction].sy, // height
            compass_hilite_per_direction_dest[direction].x, compass_hilite_per_direction_dest[direction].y,
            gui_compass_hilite_bitmask); // dest_x, dest_y
    }    
}

BOOL gui_is_point_in_blank_area(vec2 *point)
{
    return point_within_rect(point, (rect *)&blank_area);
}

BOOL gui_is_point_in_compass(vec2 *point)
{
    return point_within_rect(point, (rect *)&compass_area);
}


void gui_display_tiles_block(short tile_sx, short tile_sy, short tile_ex, short tile_ey)
{
    if (gui_bitmap)
    {
        rect dest_rect;
        dest_rect.sx = tile_sx;
        dest_rect.sy = tile_sy;
        dest_rect.ex = tile_ex;
        dest_rect.ey = tile_ey;

        rpage_video_draw_tileset(gui_bitmap, gui_tileset, &dest_rect, GUI_TILE_WIDTH);
    }
}

void gui_swap_amiga_32_colors(rect *r)
{
    short i, j, k;
    // swap Amiga GUI colors (according to a hardware constraint)
    // 19 <-> 31
    // 18 <-> 4
    for(j = r->sy; j < r->ey; j++)
    {
        for(i = r->sx; i < r->ex; i++)
        {
            k = rpage_video_get_pixel(i, j);
            switch(k)
            {
                case 19:
                    rpage_video_set_pixel(i, j, 31);
                    break;
                case 31:
                    rpage_video_set_pixel(i, j, 19);
                    break;
                case 4:
                    rpage_video_set_pixel(i, j, 18);
                    break;
                case 18:
                    rpage_video_set_pixel(i, j, 4);
                    break;
            }
        }
    }
}

void gui_bitmap_swap_amiga_32_colors(rpage_bitmap *bmp, rect *r)
{
    short i, j, k;
    // swap Amiga GUI colors (according to a hardware constraint)
    // 19 <-> 31
    // 18 <-> 4
    for(j = r->sy; j < r->ey; j++)
    {
        for(i = r->sx; i < r->ex; i++)
        {
            k = rpage_bitmap_get_pixel(bmp, i, j);
            switch(k)
            {
                case 19:
                    rpage_bitmap_set_pixel(bmp, i, j, 31);
                    break;
                case 31:
                    rpage_bitmap_set_pixel(bmp, i, j, 19);
                    break;
                case 4:
                    rpage_bitmap_set_pixel(bmp, i, j, 18);
                    break;
                case 18:
                    rpage_bitmap_set_pixel(bmp, i, j, 4);
                    break;
            }
        }
    }
}

void gui_draw_object_in_hand(short object_index, vec2 *position)
{
#ifdef LATTICE
    if (object_index > OBJECT_NONE)
    {
        short i;
        vec2 sprite_pos;

        i = gameObjectGetSpriteInventoryIndex(object_index);
        if (i >= 0)
        {
            if (rpage_sprite_is_enabled(1) && object_sprite.posctldata != inventory_sprites_img[i])
                rpage_remove_sprite(1);

            if (!rpage_sprite_is_enabled(1))
            {
                object_sprite.height = 16;
                object_sprite.posctldata = inventory_sprites_img[i];
                object_sprite.x = 0;
                object_sprite.y = 0;
                object_sprite.num = 0;
            }            

            sprite_pos.x = position->x - 12;
            sprite_pos.y = position->y - 5;
            rpage_move_sprite(1, &object_sprite, &sprite_pos);
        }
    }
    else
        rpage_remove_sprite(1);
#endif
}

void gui_actions_mouse_refresh(short action)
{
    rpage_mouse_set_bitmap(game_cursors_img[action], &(game_cursors_hotspot[action]));
}

void gui_actions_refresh(short button, vec2 *mouse_coords)
{
    short i;
    short x = 0, y = 0;
    rect r;
    for(i = 0; i < ACTION_MAX_ICON; i++)
    {
        short hilite_offset = 0;
        r.sx = x + ACTION_ICONS_AREA_X;
        r.sy = y + ACTION_ICONS_AREA_Y;
        r.ex = r.sx + ACTION_ICON_WIDTH;
        r.ey = r.sy + ACTION_ICON_HEIGHT;
        if (point_within_rect(mouse_coords, &r))
        {
            if (button)
            {
                if (game_get_action() == i)
                {
                    game_set_action(-1);
                    rpage_mouse_show();
                }
                else
                {
                    game_set_action(i);
                    if (i < act_save)
                        gui_actions_mouse_refresh(i);
                }
            }                    
            else
                hilite_offset = ACTION_ICONS_OFFSET_ROLLOVER;
        }

        if (game_get_action() == i)
            hilite_offset = ACTION_ICONS_OFFSET_SELECTED;

        rpage_video_blt_bmp(icons_bitmap, x + hilite_offset, y, ACTION_ICON_WIDTH, ACTION_ICON_HEIGHT, x + ACTION_ICONS_AREA_X, y + ACTION_ICONS_AREA_Y);       
        x += ACTION_ICON_WIDTH;
        if (x > ACTION_ICON_WIDTH)
        {
            x = 0;
            y += ACTION_ICON_HEIGHT;
        }
    }
}

void gui_draw_text_shadowed(char *str, short x, short y, unsigned short text_color, unsigned short shadow_color)
{
    // rpage_video_draw_text(str, x + 1, y, shadow_color);
    // rpage_video_draw_text(str, x + 1, y + 1, shadow_color);
    // rpage_video_draw_text(str, x, y, text_color);
    rpage_video_draw_shadow_text(str, x, y, text_color, shadow_color);
}

BOOL gui_is_point_in_inventory(vec2 *point)
{
    return point_within_rect(point, (rect *)&inventory_area);
}

void gui_update_inventory_tooltip(vec2 *mouse_coords, char *object_name)
{
    short x, y;
    x = INVENTORY_POS_X + 16;
    y = INVENTORY_POS_Y - 2;

    // Is this a tooltip for a different game object (or no object) ?
    if (object_name != previous_tooltip)
    {
        // Clear the tooltip background
        rpage_video_vsync();
        rpage_video_blt_bmp(inventory_bitmap, 0, 0, (INVENTORY_WIDTH >> 1) + 16, 9, INVENTORY_POS_X, INVENTORY_POS_Y);
        //  Draw the tooltip (if any)
        if (object_name != NULL)
        {
            short w;
            rect _r;
            w = (rpage_video_get_text_width(object_name) >> 1) << 1;
            _r.sx = x - 2;
            _r.sy = INVENTORY_POS_Y + 1;
            _r.ex = x + w + 4;
            _r.ey = _r.sy + 7;
            rpage_video_fill_rect(&_r, 18);

            rpage_video_set_pixel(_r.sx - 1, _r.sy, 0);
            rpage_video_set_pixel(_r.sx - 1, _r.sy + 1, 18);
            rpage_video_set_pixel(_r.sx - 1, _r.sy + 2, 0);
            rpage_video_set_pixel(_r.sx - 1, _r.sy + 3, 18);

            rpage_video_set_pixel(_r.sx - 2, _r.sy, 19);
            rpage_video_set_pixel(_r.sx - 2, _r.sy + 1, 20);
            rpage_video_set_pixel(_r.sx - 2, _r.sy + 2, 19);

            rpage_video_set_pixel(_r.ex + 1, _r.sy, 0);
            rpage_video_set_pixel(_r.ex + 1, _r.sy + 1, 18);
            rpage_video_set_pixel(_r.ex + 1, _r.sy + 2, 0);
            rpage_video_set_pixel(_r.ex + 1, _r.sy + 3, 18);

            rpage_video_set_pixel(_r.ex + 2, _r.sy, 19);
            rpage_video_set_pixel(_r.ex + 2, _r.sy + 1, 20);
            rpage_video_set_pixel(_r.ex + 2, _r.sy + 2, 19);

            gui_draw_text_shadowed(object_name, x, y, 30, 0);
        }
    }

    previous_tooltip = object_name;
}

void gui_rebuild_inventory_dirty_bitmap(inventory_object *inventory, short inventory_len)
{
    short i, spr_idx, spr_x, spr_y;
    short x, y;
    // vec2 inventory_pos;

    // inventory_pos.x = INVENTORY_POS_X;
    // inventory_pos.y = INVENTORY_POS_Y;

    // draw the inventory panel
    // rpage_video_vsync();
    // rpage_video_blt_bmp(inventory_bitmap, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, inventory_pos.x, inventory_pos.y);
    rpage_bitmap_blit(inventory_bitmap, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, 0, 0, inventory_dirty_bitmap);
    rpage_video_wait_dma();

    for(i = 0; i < INVENTORY_SIZE; i++)
        if (inventory[i].object_index > -1)
        {
            spr_idx = gameObjectGetSpriteInventoryIndex(inventory[i].object_index);
            if (spr_idx >= 0)
            {
                x = ((i - ((i / INVENTORY_OBJECT_PER_LINE) * INVENTORY_OBJECT_PER_LINE)) * INVENT_SHEET_ICON_WIDTH) + INVENTORY_CONTENT_POS_X;
                y = ((i / INVENTORY_OBJECT_PER_LINE) * INVENT_SHEET_ICON_HEIGHT) + INVENTORY_CONTENT_POS_Y;
                spr_y = (spr_idx / INVENT_SHEET_ICON_PER_LINE) * INVENT_SHEET_ICON_HEIGHT;
                spr_x = (spr_idx % INVENT_SHEET_ICON_PER_LINE) * INVENT_SHEET_ICON_WIDTH;
                // rpage_video_vsync();
                // rpage_video_blt_bmp_mask(spr_inventory, 
                //                         spr_x, spr_y,
                //                         INVENT_SHEET_ICON_WIDTH, INVENT_SHEET_ICON_HEIGHT, 
                //                         x, y, 
                //                         spr_inventory_mask);
                // rpage_bitmap_blit(spr_inventory, 
                //                     spr_x, spr_y,
                //                     INVENT_SHEET_ICON_WIDTH, INVENT_SHEET_ICON_HEIGHT, 
                //                     x, y,
                //                     inventory_dirty_bitmap);

                rpage_bitmap_blit_mask(spr_inventory, 
                                    spr_x, spr_y,
                                    INVENT_SHEET_ICON_WIDTH, INVENT_SHEET_ICON_HEIGHT, 
                                    x, y,
                                    inventory_dirty_bitmap, spr_inventory_mask);
            }
        }  

        rpage_video_wait_dma();  
}

void gui_refresh_inventory(inventory_object *inventory, short inventory_len)
{
    gui_rebuild_inventory_dirty_bitmap(inventory, inventory_len);
    rpage_video_vsync();
    rpage_video_blt_bmp(inventory_dirty_bitmap, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, INVENTORY_POS_X, INVENTORY_POS_Y);    
}

void gui_show_inventory(inventory_object *inventory, short inventory_len)
{
    short i;
    buffered_screen *screen;

#ifdef LATTICE  
    screen = rpage_video_get_ptr();
#endif

    // make a copy of the background under the inventory panel
    rpage_video_save_to_bitmap(panel_back, INVENTORY_POS_X, INVENTORY_POS_Y, INVENTORY_WIDTH, INVENTORY_HEIGHT);
    gui_rebuild_inventory_dirty_bitmap(inventory, inventory_len);

    for(i = INVENTORY_HEIGHT - 1; i >= 0; i -= 4)
    {
#ifndef LATTICE    
        rpage_video_vsync();
        rpage_video_blt_bmp(inventory_dirty_bitmap, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT - i, INVENTORY_POS_X, INVENTORY_POS_Y + i);
#else
        WaitTOF();
        // WaitBOVP(&(screen->screen->ViewPort));
        BltBitMap(inventory_dirty_bitmap,
                0, 0,
                screen->bitmaps[screen->physical],
                INVENTORY_POS_X, INVENTORY_POS_Y + i,
                INVENTORY_WIDTH, INVENTORY_HEIGHT - i,
                0xC0, 0xFF, NULL);
#endif
    }

    rpage_video_vsync();
    rpage_video_blt_bmp(inventory_dirty_bitmap, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, INVENTORY_POS_X, INVENTORY_POS_Y);

    gui_set_modal_flag(TRUE); // Inventory
}

void gui_hide_inventory(void)
{
    short i;
    buffered_screen *screen;
    static const short step = 4;

#ifdef LATTICE  
    screen = rpage_video_get_ptr();
    Forbid();
#endif

    for(i = step; i <= INVENTORY_HEIGHT; i += step)
    {
#ifndef LATTICE        
        rpage_video_vsync();
        rpage_video_blt_bmp(panel_back, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT - (INVENTORY_HEIGHT - i), INVENTORY_POS_X, INVENTORY_POS_Y);
        rpage_video_blt_bmp(inventory_dirty_bitmap, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT - i, INVENTORY_POS_X, INVENTORY_POS_Y + i);
#else
        WaitTOF();
        // WaitBOVP(&(screen->screen->ViewPort));
        // WaitTOF();
        // WaitBOVP(&(screen->screen->ViewPort));
        if (i < INVENTORY_HEIGHT)
            BltBitMap(inventory_dirty_bitmap,
                    0, 0,
                    screen->bitmaps[screen->physical],
                    INVENTORY_POS_X, INVENTORY_POS_Y + i,
                    INVENTORY_WIDTH, INVENTORY_HEIGHT - i,
                    0xC0, 0xFF, NULL);
        WaitBlit();
        BltBitMap(panel_back,
                0, i - step,
                screen->bitmaps[screen->physical],
                INVENTORY_POS_X, INVENTORY_POS_Y + i - step,
                INVENTORY_WIDTH, step,
                0xC0, 0xFF, NULL);           
#endif        
    }

    gui_set_modal_flag(FALSE); // Inventory

#ifdef LATTICE  
    Permit();
#endif    

    // rpage_video_vsync();
    // rpage_video_blt_bmp(panel_back, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, INVENTORY_POS_X, INVENTORY_POS_Y);
}

int gui_inventory_get_mouse_slot_index(vec2 *mouse_coords)
{
    short idx;
    vec2 inventory_pos;

    inventory_pos.x = (mouse_coords->x - INVENTORY_POS_X - INVENTORY_CONTENT_POS_X) / INVENT_SHEET_ICON_WIDTH;
    inventory_pos.y = (mouse_coords->y - INVENTORY_POS_Y - INVENTORY_CONTENT_POS_Y) / INVENT_SHEET_ICON_HEIGHT;

    idx = (inventory_pos.y * (INVENTORY_WIDTH / INVENT_SHEET_ICON_WIDTH)) + inventory_pos.x;
    idx = min(idx, INVENTORY_OBJECT_PER_LINE * INVENTORY_OBJECT_PER_ROW - 1);
    return idx;
}

void gui_show_debug_flags(char **flags, short n)
{
#ifdef GAME_VISUAL_DEBUG    
    short i, j, x = 0, y = 8, w = 12;
    char str[255];

    rpage_video_vsync();
    rpage_video_blt_bmp(inventory_bitmap, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT - 16, INVENTORY_POS_X, y);
    for(i = 0; i < 2; i++)
        rpage_video_blt_bmp(inventory_bitmap, 0, 16, INVENTORY_WIDTH, INVENTORY_HEIGHT - 16, INVENTORY_POS_X, y + (i * 32) + INVENTORY_HEIGHT - 16);

    y += 3;
    gui_draw_text_shadowed("Game Flags Debug", INVENTORY_POS_X + INVENTORY_CONTENT_POS_X + 8, y, 30, 0);
    y += 10;

    x = INVENTORY_POS_X + 8;
    for(j = 0; j < n; j++)
    {
        if (str_find_delimiter(0, flags[j]) >= strlen(flags[j]))
        {
            gui_draw_text_shadowed(flags[j], x, y, 30, 0);
            y += 8;
        }
        else
        {
            short len_str, i, start_str, end_str;            
            y += 2;
            i = 0;
            start_str = 0;
            len_str = (short)strlen(flags[j]);
            end_str = str_find_delimiter(start_str, flags[j]);
            while (start_str < len_str)
            {
                memset(str, 0, 255);
                strncpy(str, flags[j] + start_str, end_str - start_str);
                start_str = end_str + 1;
                gui_draw_text_shadowed(str, x, y, 30, 0);
                end_str = str_find_delimiter(start_str, flags[j]);
                y += 8;
                i++;
            }
        }
        
    }

#endif 
}

void gui_show_debug_visuals(void)
{
#ifdef GAME_VISUAL_DEBUG    
    short i, j, x = 0, y = 32, w = 14;
    rect _r;
    char str[16];

    rpage_video_vsync();
    rpage_video_blt_bmp(inventory_bitmap, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT - 16, INVENTORY_POS_X, y);
    rpage_video_blt_bmp(inventory_bitmap, 0, 16, INVENTORY_WIDTH, INVENTORY_HEIGHT - 16, INVENTORY_POS_X, y + INVENTORY_HEIGHT - 16);

    y += 3;
    gui_draw_text_shadowed("Game Visual Debug", INVENTORY_POS_X + INVENTORY_CONTENT_POS_X + 8, y, 30, 0);
    y += 2;

    for(i = 0, j = 0; i < (1 << SCREEN_DEPTH); i++)
    {
        _r.sx = INVENTORY_POS_X + INVENTORY_CONTENT_POS_X + 1 + x;
        _r.sy = y + INVENTORY_CONTENT_POS_Y;
        _r.ex = _r.sx + w - 1;
        _r.ey = _r.sy + w - 1;
        rpage_video_fill_rect(&_r, i);

        if (i == (i / 5) * 5)
        {
            sprintf(str, "%d", i);
            gui_draw_text_shadowed(str, _r.sx, _r.sy - 3, 30, 0);
        }

        x += w;
        j++;

        if (j >= 16) // (x > INVENTORY_WIDTH - (w << 1))
        {
            x = 0;
            y += w;
            j = 0;
        }
    }
#endif    
}

void gui_show_debug_metrics(system_metrics *metrics)
{
#ifdef GAME_VISUAL_DEBUG    
    short i, ys;
    vec2 ui_pos;

    // game_draw_debug_button();

    ui_pos.x = TRAVEL_MENU_POS_X;
    ui_pos.y = TRAVEL_MENU_POS_Y;

    rpage_video_vsync();
    rpage_video_blt_bmp(travel_menu_bitmap, 0, 0, TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT, ui_pos.x, ui_pos.y);

    ys = 8;
    ui_pos.x += 8;
    ui_pos.y += 5;

    i = 0;
    rpage_video_draw_text("   Game Debug", ui_pos.x + 1, ui_pos.y + (i * ys) + 1, 2);
    rpage_video_draw_text("   Game Debug", ui_pos.x, ui_pos.y + (i * ys), 30);
    ui_pos.y += 2;
    i++;    
    rpage_video_draw_text(metrics->chipmem_avail, ui_pos.x + 1, ui_pos.y + (i * ys) + 1, 2);
    rpage_video_draw_text(metrics->chipmem_avail, ui_pos.x, ui_pos.y + (i * ys), 30);
    i++;
    rpage_video_draw_text(metrics->largest_chipmem_avail, ui_pos.x + 1, ui_pos.y + (i * ys) + 1, 2);
    rpage_video_draw_text(metrics->largest_chipmem_avail, ui_pos.x, ui_pos.y + (i * ys), 30);
    i++;    
    rpage_video_draw_text(metrics->othermem_avail, ui_pos.x + 1, ui_pos.y + (i * ys) + 1, 2);
    rpage_video_draw_text(metrics->othermem_avail, ui_pos.x, ui_pos.y + (i * ys), 30);
    i++;
    rpage_video_draw_text(metrics->vuecache_size, ui_pos.x + 1, ui_pos.y + (i * ys) + 1, 2);
    rpage_video_draw_text(metrics->vuecache_size, ui_pos.x, ui_pos.y + (i * ys), 30);
    i++;
    rpage_video_draw_text(metrics->gui_size, ui_pos.x + 1, ui_pos.y + (i * ys) + 1, 2);
    rpage_video_draw_text(metrics->gui_size, ui_pos.x, ui_pos.y + (i * ys), 30);
#endif    
}

void gui_show_chapter_title(char *world_title)
{
    vec2 ui_pos;

    ui_pos.x = (SCREEN_WIDTH - TRAVEL_MENU_WIDTH) / 2;
    ui_pos.y = (SCREEN_HEIGHT - TRAVEL_MENU_HEIGHT) / 2;
    
    rpage_video_flip_buffers();
    rpage_video_clear();

    rpage_video_blt_bmp(travel_menu_bitmap, 0, 0, TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT, ui_pos.x, ui_pos.y);
    rpage_video_draw_text(world_title, -1, ui_pos.y + (TRAVEL_MENU_HEIGHT / 2) - 5, 1);
    rpage_video_draw_text(world_title, -1, ui_pos.y + (TRAVEL_MENU_HEIGHT / 2) - 6, 30);

    rpage_video_present_screen();
}

void gui_show_disk_request(char *disk_name, gui_dialog_descriptor *desc, BOOL allow_abort)
{
    vec2 ui_pos;
    rect bt_cancel;
    short tx;
    char _str[16];    

    ui_pos.x = (SCREEN_WIDTH - TRAVEL_MENU_WIDTH) / 2;
    ui_pos.y = (SCREEN_HEIGHT - TRAVEL_MENU_HEIGHT) / 2;

    // make a copy of the background under the inventory panel
    rpage_video_save_to_bitmap(disk_request_back, ui_pos.x, ui_pos.y, TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT);

    rpage_video_vsync();
    rpage_video_blt_bmp(travel_menu_bitmap, 0, 0, TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT, ui_pos.x, ui_pos.y);

    ui_pos.y -= 4;
    rpage_video_draw_text(game_get_system_dialog(SYS15), -1, ui_pos.y + (TRAVEL_MENU_HEIGHT / 2) - 5, 1);
    rpage_video_draw_text(game_get_system_dialog(SYS15), -1, ui_pos.y + (TRAVEL_MENU_HEIGHT / 2) - 6, 30);

    ui_pos.y += 8;
    rpage_video_draw_text(disk_name, -1, ui_pos.y + (TRAVEL_MENU_HEIGHT / 2) - 5, 1);
    rpage_video_draw_text(disk_name, -1, ui_pos.y + (TRAVEL_MENU_HEIGHT / 2) - 6, 30);

    if (allow_abort)
    {        
        ui_pos.y -= 4;
        bt_cancel.sx = ui_pos.x + TRAVEL_MENU_WIDTH - ((TF_BUTTON_WIDTH * 3) / 2) - TF_MARGIN;
        bt_cancel.sy = ui_pos.y + TRAVEL_MENU_HEIGHT - TF_BUTTON_HEIGHT - TF_MARGIN;
        bt_cancel.ex = bt_cancel.sx + ((TF_BUTTON_WIDTH * 3) / 2);
        bt_cancel.ey = bt_cancel.sy + TF_BUTTON_HEIGHT;
        memcpy(&desc->button0, &bt_cancel, sizeof(rect)); // CANCEL goes into 'button0'
        gui_draw_3d_button(&bt_cancel, FALSE);

        sprintf(_str, "%s", "Cancel");
        tx = bt_cancel.sx + ((((TF_BUTTON_WIDTH * 3) / 2) - rpage_video_get_text_width(_str)) >> 1);
        rpage_video_draw_text(_str, tx, bt_cancel.sy + 1, 2);
        rpage_video_draw_text(_str, tx, bt_cancel.sy, 30);    
    }

    gui_set_modal_flag(TRUE); // Disk request
}

void gui_hide_disk_request(void)
{
    vec2 ui_pos;

    ui_pos.x = (SCREEN_WIDTH - TRAVEL_MENU_WIDTH) / 2;
    ui_pos.y = (SCREEN_HEIGHT - TRAVEL_MENU_HEIGHT) / 2;
    
    rpage_video_vsync();
    rpage_video_blt_bmp(disk_request_back, 0, 0, TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT, ui_pos.x, ui_pos.y);

    gui_set_modal_flag(FALSE); // Disk request
}

void gui_draw_3d_button(rect *_r, BOOL hilite)
{
    rect r, r2;
    short col_black = 0,
            col_med = 14,
            col_light = 20,
            col_white = 19;

    if (!hilite)
    {
        col_med = 12;
        col_light = 17;
        col_white = 29;
    }

    memcpy(&r, _r, sizeof(rect));

    rpage_video_fill_rect(&r, col_black);
    rect_shrink(&r, 1);
    rpage_video_fill_rect(&r, col_med);
    memcpy(&r2, &r, sizeof(rect));
    r2.sy = (r2.sy + r2.ey) / 2;
    rpage_video_fill_rect(&r2, col_med - 2);
    rpage_video_draw_rect(&r, col_light);
    rpage_video_set_pixel(r.sx, r.sy, col_white);
    rpage_video_set_pixel(r.ex, r.ey, col_white);
}

void gui_draw_button_selected(rect *_r)
{
    rect r;
    short col_white = 14;
    memcpy(&r, _r, sizeof(rect));
    rpage_video_draw_rect(&r, col_white);
    // rect_grow(&r, 1);
    // rpage_video_draw_rect(&r, col_white);
}

void gui_draw_button_unselected(rect *_r)
{
    rect r;
    short col_black = 0;
    memcpy(&r, _r, sizeof(rect));
    rpage_video_draw_rect(&r, col_black);
}

void gui_show_textfield_dialog(vec2 *ui_pos, gui_dialog_descriptor *desc, char *text_ok, char *text_cancel, short field_size)
{
    short i, tx;
    rect tfield, bt_ok, bt_cancel;

    // clear the return info structure
    memset(desc, 0, sizeof(*desc));

    ui_pos->x = TRAVEL_MENU_POS_X;
    ui_pos->y = TRAVEL_MENU_POS_Y;

    desc->dialog_rect.sx = ui_pos->x;
    desc->dialog_rect.sy = ui_pos->y;
    desc->dialog_rect.ex = ui_pos->x + TRAVEL_MENU_WIDTH;
    desc->dialog_rect.ey = ui_pos->y + TRAVEL_MENU_HEIGHT;

    // make a copy of the background under the inventory panel
    rpage_video_save_to_bitmap(panel_back, ui_pos->x, ui_pos->y, TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT);

    rpage_video_vsync();
    rpage_video_blt_bmp(travel_menu_bitmap, 0, 0, TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT, ui_pos->x, ui_pos->y);

    bt_ok.sx = ui_pos->x + TF_MARGIN;
    bt_ok.sy = ui_pos->y + TRAVEL_MENU_HEIGHT - TF_BUTTON_HEIGHT - TF_MARGIN;
    bt_ok.ex = bt_ok.sx + TF_BUTTON_WIDTH;
    bt_ok.ey = bt_ok.sy + TF_BUTTON_HEIGHT;
    memcpy(&desc->button0, &bt_ok, sizeof(rect)); // OK goes into 'button0'
    gui_draw_3d_button(&bt_ok, TRUE);

    tx = bt_ok.sx + ((TF_BUTTON_WIDTH - rpage_video_get_text_width(text_ok)) >> 1);
    rpage_video_draw_text(text_ok, tx, bt_ok.sy + 1, 2);
    rpage_video_draw_text(text_ok, tx, bt_ok.sy, 30);

    bt_cancel.sx = ui_pos->x + TRAVEL_MENU_WIDTH - ((TF_BUTTON_WIDTH * 3) / 2) - TF_MARGIN;
    bt_cancel.sy = ui_pos->y + TRAVEL_MENU_HEIGHT - TF_BUTTON_HEIGHT - TF_MARGIN;
    bt_cancel.ex = bt_cancel.sx + ((TF_BUTTON_WIDTH * 3) / 2);
    bt_cancel.ey = bt_cancel.sy + TF_BUTTON_HEIGHT;
    memcpy(&desc->button1, &bt_cancel, sizeof(rect)); // CANCEL goes into 'button1'
    gui_draw_3d_button(&bt_cancel, FALSE);

    tx = bt_cancel.sx + ((((TF_BUTTON_WIDTH * 3) / 2) - rpage_video_get_text_width(text_cancel)) >> 1);
    rpage_video_draw_text(text_cancel, tx, bt_cancel.sy + 1, 2);
    rpage_video_draw_text(text_cancel, tx, bt_cancel.sy, 30);

    i = (field_size * 2 + 1) * 6; // Text field width
    tfield.sx = ui_pos->x + (TRAVEL_MENU_WIDTH - i) / 2;
    tfield.ex = tfield.sx + i;
    tfield.sy = ui_pos->y + TF_MARGIN * 3;
    tfield.ey = tfield.sy + 6;

    memcpy(&desc->textfield, &tfield, sizeof(rect));
    desc->tf_color_index = 0;
    rpage_video_fill_rect(&tfield, 0);
    rect_grow(&tfield, 1);
    rpage_video_draw_rect(&tfield, 1);
    rect_grow(&tfield, 1);
    rpage_video_draw_rect(&tfield, 5);
    rpage_video_set_pixel(tfield.sx, tfield.sy, 6);
    rpage_video_set_pixel(tfield.ex, tfield.ey, 6);
    rpage_video_set_pixel(tfield.sx, tfield.ey, 6);
    rpage_video_set_pixel(tfield.ex, tfield.sy, 6);

    // rpage_video_draw_text(text_ok, tfield.sx + 2, tfield.sy - 1, 30);
}

void gui_hide_textfield_dialog(void)
{
    gui_hide_travel_menu();
}

void gui_show_travel_menu(travel_destination *destination_list, short list_size)
{
    short i;
    vec2 ui_pos;

    ui_pos.x = TRAVEL_MENU_POS_X;
    ui_pos.y = TRAVEL_MENU_POS_Y;

    // make a copy of the background under the inventory panel
    rpage_video_save_to_bitmap(panel_back, ui_pos.x, ui_pos.y, TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT);

    rpage_video_vsync();
    rpage_video_blt_bmp(travel_menu_bitmap, 0, 0, TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT, ui_pos.x, ui_pos.y);

    ui_pos.x += 16;
    ui_pos.y += 5;

    // list_size = 3;
    rpage_video_draw_text(game_get_system_dialog(SYS19), ui_pos.x + 1, ui_pos.y + 1, 2);
    rpage_video_draw_text(game_get_system_dialog(SYS19), ui_pos.x, ui_pos.y, 30);

    ui_pos.x += 10;
    ui_pos.y += 10;

    for(i = 0; i < list_size; i++)
    {
        rpage_video_draw_text(game_get_system_dialog(destination_list[i].dialog_idx), ui_pos.x + 1, ui_pos.y + (i * 9) + 1, 2);
        rpage_video_draw_text(game_get_system_dialog(destination_list[i].dialog_idx), ui_pos.x, ui_pos.y + (i * 9), 30);
    }

    rpage_video_draw_text(game_get_system_dialog(destination_list[3].dialog_idx), ui_pos.x + 1, ui_pos.y + (i * 9) + 1, 2);
    rpage_video_draw_text(game_get_system_dialog(destination_list[3].dialog_idx), ui_pos.x, ui_pos.y + (i * 9), 30);

    gui_set_modal_flag(TRUE); // Travel menu
}

void gui_hide_travel_menu(void)
{
    vec2 ui_pos;

    ui_pos.x = TRAVEL_MENU_POS_X;
    ui_pos.y = TRAVEL_MENU_POS_Y;
    
    rpage_video_vsync();
    rpage_video_blt_bmp(panel_back, 0, 0, TRAVEL_MENU_WIDTH, TRAVEL_MENU_HEIGHT, ui_pos.x, ui_pos.y);

    gui_set_modal_flag(FALSE); // Travel menu
}

void gui_show_save_panel(BOOL save_mode, gui_dialog_descriptor *desc)
{
    short tx, pos_x, pos_y;
    rect _r, bt_ok, bt_cancel;
    char _str[16];

    pos_x = INVENTORY_POS_X;
    pos_y = (SCREEN_HEIGHT - INVENTORY_HEIGHT) >> 1;

    // make a copy of the background under the save panel
    // rpage_video_save_to_bitmap(panel_back, pos_x, pos_y, INVENTORY_WIDTH, INVENTORY_HEIGHT);
    rpage_video_vsync();
    rpage_video_blt_bmp(inventory_bitmap, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, pos_x, pos_y);
    pos_y -= (INVENTORY_HEIGHT >> 1);
    rpage_video_blt_bmp(inventory_bitmap, 0, 0, INVENTORY_WIDTH, (INVENTORY_HEIGHT * 3) >> 2, pos_x, pos_y);

    desc->dialog_rect.sx = pos_x;
    desc->dialog_rect.sy = pos_y;
    desc->dialog_rect.ex = pos_x;
    desc->dialog_rect.ey = pos_y + (INVENTORY_HEIGHT >> 1);

    pos_x--;
    pos_y+=2;
    _r.sx = pos_x;
    _r.sy = pos_y;
    _r.ex = pos_x + INVENTORY_WIDTH;
    _r.ey = pos_y + SAVE_PANEL_HEIGHT;
    rect_shrink(&_r, 8);
    _r.sx-=2;
    _r.ex+=3;
    rpage_video_fill_rect(&_r, 18);

    rect_shrink(&_r, 8);
    _r.sy-=10;
    _r.ey = _r.sy + 10;
    rpage_video_fill_rect(&_r, 18);

    // sprintf(_str, "%s", (save_mode?"Save game":"Load game"));
    // rpage_video_draw_text(_str, pos_x, pos_y, 30);

    bt_ok.sx = pos_x + TF_MARGIN;
    bt_ok.sy = pos_y + SAVE_PANEL_HEIGHT - TF_BUTTON_HEIGHT - TF_MARGIN;
    bt_ok.ex = bt_ok.sx + TF_BUTTON_WIDTH;
    bt_ok.ey = bt_ok.sy + TF_BUTTON_HEIGHT;
    memcpy(&desc->button0, &bt_ok, sizeof(rect)); // OK goes into 'button0'
    gui_draw_3d_button(&bt_ok, TRUE);

    sprintf(_str, "%s", (save_mode?"Save":"Load"));
    tx = bt_ok.sx + ((TF_BUTTON_WIDTH - rpage_video_get_text_width(_str)) >> 1);
    rpage_video_draw_text(_str, tx, bt_ok.sy + 1, 2);
    rpage_video_draw_text(_str, tx, bt_ok.sy, 30);

    bt_cancel.sx = pos_x + INVENTORY_WIDTH - ((TF_BUTTON_WIDTH * 3) / 2) - TF_MARGIN;
    bt_cancel.sy = pos_y + SAVE_PANEL_HEIGHT - TF_BUTTON_HEIGHT - TF_MARGIN;
    bt_cancel.ex = bt_cancel.sx + ((TF_BUTTON_WIDTH * 3) / 2);
    bt_cancel.ey = bt_cancel.sy + TF_BUTTON_HEIGHT;
    memcpy(&desc->button1, &bt_cancel, sizeof(rect)); // CANCEL goes into 'button1'
    gui_draw_3d_button(&bt_cancel, FALSE);

    sprintf(_str, "%s", "Cancel");
    tx = bt_cancel.sx + ((((TF_BUTTON_WIDTH * 3) / 2) - rpage_video_get_text_width(_str)) >> 1);
    rpage_video_draw_text(_str, tx, bt_cancel.sy + 1, 2);
    rpage_video_draw_text(_str, tx, bt_cancel.sy, 30);

    gui_set_modal_flag(TRUE); // Load/Save panel
}

// void gui_hide_save_panel(void)
// {
//     short i, pos_x, pos_y;
//     pos_x = INVENTORY_POS_X;
//     pos_y = (SCREEN_HEIGHT - INVENTORY_HEIGHT) >> 1;

//     rpage_video_vsync();
//     rpage_video_blt_bmp(panel_back, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, pos_x, pos_y);
// }

short gui_get_pointed_destination(vec2 *point, BOOL click, short list_size)
{
    short i;
    vec2 ui_pos;
    rect r;

    ui_pos.x = TRAVEL_MENU_POS_X;
    ui_pos.y = TRAVEL_MENU_POS_Y;

    ui_pos.x += 4;
    ui_pos.y += 5;
    ui_pos.y += 10;

    for(i = 0; i <= list_size; i++)
    {
        rpage_video_draw_text(">", ui_pos.x + 1 + 12, ui_pos.y + (i * 9) + 1, 18);
        rpage_video_draw_text(">", ui_pos.x + 12, ui_pos.y + (i * 9), 18);

        r.sx = ui_pos.x;
        r.ex = ui_pos.x + TRAVEL_MENU_WIDTH - 5;
        r.sy = ui_pos.y + (i * 9);
        r.ey = ui_pos.y + (i * 9) + 9;

        if (point_within_rect(point, &r))
        {
            rpage_video_draw_text(">", ui_pos.x + 1 + 12, ui_pos.y + (i * 9) + 1, 2);
            rpage_video_draw_text(">", ui_pos.x + 12, ui_pos.y + (i * 9), 30);
        }
    }

    if (click)
    {
        r.sx = ui_pos.x;
        r.sy = ui_pos.y;
        r.ex = ui_pos.x + TRAVEL_MENU_WIDTH;
        r.ey = ui_pos.y + TRAVEL_MENU_HEIGHT;

        if (point_within_rect(point, &r))
        {
            for(i = 0; i <= list_size; i++)
            {
                r.sx = ui_pos.x;
                r.ex = ui_pos.x + TRAVEL_MENU_WIDTH - 9;
                r.sy = ui_pos.y + (i * 9);
                r.ey = ui_pos.y + (i * 9) + 9;

                if (point_within_rect(point, &r))
                    return i;
            }

            return i;
        }
        // else
        //     return -1;
    }

    return -1;
}

void gui_reset_portrait_shown(void)
{
    gui_portrait_shown = FALSE;
}

void gui_hide_character(void)
{
    if (gui_portrait_shown)
    {
        rect _r;
        gui_reset_portrait_shown();
        rpage_video_sync_buffers();
        rpage_video_flip_buffers();
        rpage_video_blt_bmp(character_bitmap_back, 0, 0,
                            NPC_SIZE + NPC_MARGIN_D, NPC_SIZE + NPC_MARGIN_D, NPC_POS_X + NPC_MARGIN, NPC_POS_Y);
        _r.sx = NPC_POS_X + NPC_MARGIN;
        _r.sy = NPC_POS_Y;
        _r.ex = _r.sx + NPC_SIZE + NPC_MARGIN_D;
        _r.ey = _r.sy + NPC_SIZE + NPC_MARGIN_D;
        rpage_video_wipe_rect_to_physical_screen((rect *)&_r);
        rpage_video_vsync();
        rpage_video_present_screen();
    }
}

void gui_display_character(char *portrait_filename)
{
    // if (character_bitmap != NULL)
    //     rpage_bitmap_free(character_bitmap);
    short x,y;
    x = 0; // NPC_POS_X;
    y = 0; // NPC_POS_Y;

    if (!gui_portrait_shown)
    {
        short i;
        gui_portrait_shown = TRUE;
        // Save the bg behind the portrait
        rpage_video_save_to_bitmap(character_bitmap_back, NPC_POS_X + NPC_MARGIN, NPC_POS_Y, NPC_SIZE + NPC_MARGIN_D, NPC_SIZE + NPC_MARGIN_D);

        // rpage_load_pak_to_new_bitmap(&character_bitmap, NULL, game_get_unpacking_buffer(), asset_build_device_path(portrait_filename, ".pak"));
        rpage_load_pak_into_bitmap(&character_bitmap, NULL, game_get_unpacking_buffer(), asset_build_device_path(portrait_filename, ".pak"));

        rpage_bitmap_blit(character_bitmap, 0, 0, NPC_SIZE, NPC_SIZE, x + NPC_MARGIN, y + NPC_MARGIN, character_buffer);
        rpage_bitmap_blit(inventory_bitmap, 0, 0, // TOP
                            NPC_SIZE + NPC_MARGIN_D, NPC_MARGIN, 
                            x, y,
                            character_buffer);
        rpage_bitmap_blit(inventory_bitmap, 0, INVENTORY_HEIGHT - 5, // BOTTOM 
                            NPC_SIZE + NPC_MARGIN_D, NPC_MARGIN, 
                            x, y + NPC_SIZE + NPC_MARGIN,
                            character_buffer);                        
        rpage_bitmap_blit(inventory_bitmap, 0, 0, // LEFT
                            NPC_MARGIN, NPC_SIZE + NPC_MARGIN, 
                            x, y,
                            character_buffer);
        rpage_bitmap_blit(inventory_bitmap, INVENTORY_WIDTH - NPC_MARGIN, 0, // RIGHT
                            NPC_MARGIN, NPC_SIZE + NPC_MARGIN, 
                            x + NPC_SIZE + NPC_MARGIN, y,
                            character_buffer);
        rpage_bitmap_blit(inventory_bitmap, 0, 0, // LEFT CORNER
                            NPC_MARGIN_D, NPC_MARGIN_D, 
                            x, y,
                            character_buffer);
        rpage_bitmap_blit(inventory_bitmap, INVENTORY_WIDTH - NPC_MARGIN_D, 0, // RIGHT CORNER
                            NPC_MARGIN_D, NPC_MARGIN_D, 
                            x + NPC_SIZE, y,
                            character_buffer);
        rpage_bitmap_blit(inventory_bitmap, INVENTORY_WIDTH - NPC_MARGIN, INVENTORY_HEIGHT - NPC_MARGIN, // RIGHT BOTTOM CORNER
                            NPC_MARGIN, NPC_MARGIN, 
                            x + NPC_SIZE + NPC_MARGIN, y + NPC_SIZE + NPC_MARGIN,
                            character_buffer);   

        // rpage_video_flip_buffers();
        for(i = 0; i < NPC_SIZE + NPC_MARGIN_D; i+=2)
        {
            rpage_video_vsync();
            rpage_video_blt_bmp_clip(character_buffer, 
                                0, 0, 
                                NPC_SIZE + NPC_MARGIN_D, NPC_SIZE + NPC_MARGIN_D, // i, 
                                NPC_POS_X + NPC_MARGIN, NPC_POS_Y + NPC_SIZE + NPC_MARGIN_D - i,
                                (rect *)&vue_area); 
        }

        // rpage_video_flip_buffers();
        rpage_video_blt_bmp(character_buffer, 0, 0, NPC_SIZE + NPC_MARGIN_D, NPC_SIZE + NPC_MARGIN_D, NPC_POS_X + NPC_MARGIN, NPC_POS_Y);

        // rpage_bitmap_free(character_bitmap);
    }
}