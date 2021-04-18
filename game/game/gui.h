/*  Athanor 2, Original game by Eric "Atlantis" Safar, (C) Safargames 2021  
    Amiga version by Francois "Astrofra" Gutherz.
*/

#ifndef _GUI_HEADER_
#define _GUI_HEADER_

#include "rpage/frwk.h"
#include "game/world.h"

#define FP16(A) (short)(A)

#define GUI_TILE_WIDTH (320/8)
#define GUI_TILE_HEIGHT (200/8)

#define GUI_MAX_LINE 5

#define COMPASS_X 272
#define COMPASS_Y 150
#define COMPASS_WIDTH 48
#define COMPASS_HEIGHT 48

/// Text field panel
#define TF_MARGIN 8
#define TF_BUTTON_WIDTH 32
#define TF_BUTTON_HEIGHT 12

/// Inventory panel
#define INVENTORY_WIDTH 252
#define INVENTORY_HEIGHT 64
#define INVENTORY_OBJECT_PER_LINE 10
#define INVENTORY_OBJECT_PER_ROW 2
#define INVENTORY_TILE_WIDTH ((((INVENTORY_WIDTH + 4) >> 3) << 3)/8)
#define INVENTORY_TILE_HEIGHT ((((INVENTORY_HEIGHT + 4) >> 3) << 3)/8)
#define INVENTORY_POS_X 10
#define INVENTORY_POS_Y (200-64)
#define INVENTORY_CONTENT_POS_X 5
#define INVENTORY_CONTENT_POS_Y 11

/// Inventory sprite sheet
#define INVENT_SHEET_ICON_WIDTH 24
#define INVENT_SHEET_ICON_HEIGHT 24
#define INVENT_SHEET_ICON_PER_LINE 13

#define TRAVEL_MENU_WIDTH (128+16)
#define TRAVEL_MENU_HEIGHT 64
#define TRAVEL_MENU_TILE_WIDTH (TRAVEL_MENU_WIDTH/8)
#define TRAVEL_MENU_TILE_HEIGHT (TRAVEL_MENU_HEIGHT/8)
#define TRAVEL_MENU_POS_X (((256 - TRAVEL_MENU_WIDTH)/2) + 8)
#define TRAVEL_MENU_POS_Y (((144 - TRAVEL_MENU_HEIGHT)/2) + 4)

#define ACTION_MAX_ICON 6
#define ACTION_ICON_WIDTH 24
#define ACTION_ICON_HEIGHT 24
#define ACTION_ICONS_AREA_X 272
#define ACTION_ICONS_AREA_Y 2
#define ACTION_ICONS_OFFSET_SELECTED 48
#define ACTION_ICONS_OFFSET_ROLLOVER 96

#define NPC_SIZE 48
#define NPC_POS_X 22
#define NPC_POS_Y 91
#define NPC_MARGIN 5
#define NPC_MARGIN_D (NPC_MARGIN * 2)

// Save panel
#define MAX_SAVE_SLOT 8 // how many save slots maximum
#define BT_SAVE_SLOT_SIZE 10
#define SAVE_PANEL_HEIGHT (INVENTORY_HEIGHT + (INVENTORY_HEIGHT >> 1)) // the save panel is derived from the inventory panel

extern rect compass_hilite_per_direction_src[4];
extern vec2 compass_hilite_per_direction_dest[4];
extern const poly compass_poly_per_direction[4];

/// A GUI dialog box will need this structure
/// to return the specific positions of its textfield 
/// and buttons.
typedef struct {
    rect dialog_rect;
    rect textfield;
    short tf_color_index;
    rect button0;
    rect button1;
    rect button2;
} gui_dialog_descriptor;

void gui_load(void);
void gui_unload(void);
void gui_show_debug_metrics(system_metrics *metrics);
void gui_show_debug_visuals(void);
void gui_show_debug_flags(char **flags, short n);
ULONG gui_get_memory_footprint(void);
void gui_display(void);
void gui_fadeout(short fadeout_len, rpage_palette *target_palette);
void gui_fadein(short fadein_len, rpage_palette *target_palette);
void gui_fade_to_grey(UWORD fadein_len, rpage_palette *original_palette);
/// order is TRUE: fade to, order is FALSE: fade from.
void gui_fade_to_white(short fadeout_len, rpage_palette *source_palette, BOOL order);
void gui_fade_to_custom_palette(UWORD fadein_len, rpage_palette *original_palette, rpage_palette *target_palette);
void gui_update_fx(void);
void gui_fx_shake(void);
void gui_display_tiles_block(short tile_sx, short tile_sy, short tile_ex, short tile_ey);
void gui_swap_amiga_32_colors(rect *r);
void gui_bitmap_swap_amiga_32_colors(rpage_bitmap *bmp, rect *r);
void gui_compass_refresh_direction(short direction);
void gui_actions_refresh(short button, vec2 *mouse_coords);
void gui_actions_mouse_refresh(short action);
void gui_draw_text_shadowed(char *str, short x, short y, unsigned short text_color, unsigned short shadow_color);
void gui_update_inventory_tooltip(vec2 *mouse_coords, char *object_name);
void gui_rebuild_inventory_dirty_bitmap(inventory_object *inventory, short inventory_len);
void gui_refresh_inventory(inventory_object *inventory, short inventory_len);
int gui_inventory_get_mouse_slot_index(vec2 *mouse_coords);
void gui_hide_inventory(void);
void gui_show_inventory(inventory_object *inventory, short inventory_len);
BOOL gui_is_point_in_inventory(vec2 *point);
BOOL gui_is_point_in_blank_area(vec2 *point);
BOOL gui_is_point_in_compass(vec2 *point);
void gui_draw_object_in_hand(short object_index, vec2 *position);
void gui_hide_travel_menu(void);
void gui_show_travel_menu(travel_destination *destination_list, short list_size);
short gui_get_pointed_destination(vec2 *point, BOOL click, short list_size);
void gui_reset_portrait_shown(void);
void gui_display_character(char *portrait_filename);
void gui_hide_character(void);
void gui_show_chapter_title(char *world_title);
void gui_show_disk_request(char *disk_name, gui_dialog_descriptor *desc, BOOL allow_abort);
void gui_hide_disk_request(void);
void gui_draw_3d_button(rect *r, BOOL hilite);
void gui_draw_button_selected(rect *_r);
void gui_draw_button_unselected(rect *_r);
void gui_show_textfield_dialog(vec2 *ui_pos, gui_dialog_descriptor *desc, char *text_ok, char *text_cancel, short field_size);
void gui_hide_textfield_dialog(void);
void gui_show_save_panel(BOOL save_mode, gui_dialog_descriptor *desc);
void gui_set_modal_flag(BOOL flag);
BOOL gui_get_modal_flag(void);
// void gui_hide_save_panel(void);
#endif