/*  	Windless Bay, Mount Erebus.
	Original game by Francois "Astrofra" Gutherz. 
	Amiga version (C) RESISTANCE 2021 
*/

#include <stdarg.h>
#include <stdio.h>
#include <graphics/gfxmacros.h>
#include "rpage/frwk.h"
#include "rpage/utils.h"
#include "rpage/easing.h"
#include "rpage/aos/io.h"
#include "game/gui.h"
#include "game/vue.h"
#include "game/world.h"
#include "game/game.h"
#include "game/text.h"
#include "rpage/aos/sound.h"
#include "rpage/aos/ptracker.h"
#include "game/special.h"
#include "rpage/aos/screen_size.h"
#include "game/aos/assets.h"

// #include "game/aos/sprites.h"

extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Custom far custom;

extern BYTE *music_memory_buffer;
extern short locale_index;

extern void specific_disk_request(char *filename);

// hack i/o
static UBYTE io_mouse_l, io_prev_mouse_l;
static UBYTE io_mouse_r, io_prev_mouse_r;
static UBYTE io_key, io_prev_key;

// mini game background
static rpage_bitmap *bg_bitmap = NULL;

// // Fishing mini game
// static rpage_bitmap *fish_sprites_bitmap;
// static rpage_bitmap *fish_sprites_mask_bitmap;
// // static rpage_bitmap *fish_float_sprite_bitmap;
// // static rpage_bitmap *fish_float_sprite_mask_bitmap;
// static vec2 fish_traj_pos[3][1024]; // Trajectory of a fish
// static short fish_traj_angle[1024]; // Angle of a fish along this trajectory

// // static short fish_idx;
// static vec2 fish_pos[MAX_FISH_SPRITES];    // Position of each fish for the current frame
// static short fish_frame[MAX_FISH_SPRITES]; // Angle of each fish for the current frame
// static vec2 fish_frames[16];               // Position of the source sprite in the sprite sheet, for a given angle
// static vec2 fish_float_pos[2];
// static short fish_float_frames[4 * 16];
// static short fish_float_frames_shoot[4 * 16];
// static const short fishing_float_posx[4] = {FS_PX0, FS_PX1, FS_PX2, FS_PX3};
// static const short fishing_aim_posx[4] = {(FS_PX0 + FS_PX0 + FS_PX1) / 3, FS_PX1, FS_PX2, (FS_PX2 + FS_PX3 + FS_PX3) / 3};

// sand puzzle/dialog
static vec2 sand_puzzle_ui_pos;
gui_dialog_descriptor sand_puzzle_dialog;

short prev_key = 0;
short sand_text_field_input_size = 0;
char sand_text_field[SND_TXTFIELD_SIZE];
char sand_text_field_display[SND_TXTFIELD_SIZE + 2];
short char_digit_key[10] = {0xA, 0x1, 0x2, 0x3,
                            0x4, 0x5, 0x6, 0x7,
                            0x8, 0x9};

char *char_digit_value[10] = {"0", "1", "2", "3",
                              "4", "5", "6", "7",
                              "8", "9"};

// Secret key dialog
static vec2 key_puzzle_ui_pos;
gui_dialog_descriptor key_puzzle_dialog;

short key_text_field_input_size = 0;
char key_text_field[KEY_TXTFIELD_SIZE];
char key_text_field_display[KEY_TXTFIELD_SIZE + 2];

static char vig_table[10][MAX_MAGIC_CODE_LEN];

// Save/Load dialog
gui_dialog_descriptor save_dialog;
rect bt_save_slot[MAX_SAVE_SLOT];
BOOL save_write_mode;
short save_slot_selected;
char save_slot_name[MAX_SAVE_SLOT][32];

// /* ------------------------------- */
// /* --- sprite_fishing_scan (data) --- */
// /* Ensure that this data is within chip memory or you'll see nothing !!! */
// UWORD chip sprite_fishing_scan_img[2][36] =
//     {
//         /* Sprite #0 */
//         {
//             0x0, 0x0,
//             0x0, 0x0,
//             0x180, 0x80,
//             0x1c0, 0x180,
//             0x3c0, 0x1c0,
//             0x3e0, 0x3c0,
//             0x7e0, 0x360,
//             0x770, 0x720,
//             0xf30, 0x630,
//             0xe38, 0xe10,
//             0x1e18, 0xc18,
//             0x1c1c, 0x1c08,
//             0x3c0c, 0x180c,
//             0x380e, 0x3804,
//             0x7ffe, 0x3ffe,
//             0x7ffe, 0x7ffe,
//             0x0, 0x0,
//             0x0, 0x0},
//         /* Sprite #1 */
//         {
//             0x0, 0x0,
//             0x0, 0x0,
//             0x180, 0x0,
//             0x3c0, 0x180,
//             0x7e0, 0x3c0,
//             0xff0, 0x760,
//             0x1f78, 0xe30,
//             0x3e3c, 0x1c18,
//             0x7c1e, 0x380c,
//             0x7c1e, 0x380c,
//             0x3e3c, 0x1c18,
//             0x1f78, 0xe30,
//             0xff0, 0x760,
//             0x7e0, 0x3c0,
//             0x3c0, 0x180,
//             0x180, 0x0,
//             0x0, 0x0,
//             0x0, 0x0},
// };

/* -- (end of) sprite_fishing_scan -- */
/* ------------------------------- */

// // Beetle race mini game
// static rpage_bitmap *beetle_race_sprites, *beetle_race_sprites_mask;
// static rpage_hardware_sprite beetle_ball_sprite;
// static short bt_ball_track_idx;
// static short bt_ball_drop_track_timer[BEETLE_RACE_LINES];
// // static short bt_ball_drop_track_x[BEETLE_RACE_LINES];
// static vec2 beetle_ball_pos;
// static unsigned short bt_aim_timeout;
// static vec2 bt_pos[BEETLE_RACE_LINES];
// static short bt_frame[BEETLE_RACE_LINES];
// static USHORT beetle_frame[BEETLE_MAX_FRAME] = {0 * BEETLE_WIDTH, 1 * BEETLE_WIDTH, 0 * BEETLE_WIDTH, 2 * BEETLE_WIDTH};
// static UBYTE bt_rand[512];
// static UBYTE bt_parabolic_y[BT_THROW_LEN];
// static UBYTE bt_force_shift[BT_THROW_LEN];
// static UBYTE bt_game_state;
// static short bt_ball_force;
// static rect bt_r_load;

/* ------------------------------- */
/* --- beetle_ball_sprite (data) --- */
/* Ensure that this data is within chip memory or you'll see nothing !!! */
// UWORD chip beetle_ball_sprite_img[1][36] =
//     {
//         /* Sprite #0 */
//         {
//             0x0, 0x0,
//             0x0, 0x0,
//             0x0, 0x380,
//             0x100, 0x6e0,
//             0x7c0, 0xbb0,
//             0xfe0, 0x17d8,
//             0x1ff0, 0x2f8c,
//             0x1fd8, 0x3764,
//             0x1fe8, 0x2a9c,
//             0x1fd8, 0x3524,
//             0x1fa8, 0x2054,
//             0xcd0, 0x3368,
//             0x3e0, 0x1d98,
//             0x0, 0x7f0,
//             0x0, 0x0,
//             0x0, 0x0,
//             0x0, 0x0,
//             0x0, 0x0},
// };

/* -- (end of) beetle_ball_sprite -- */
/* ------------------------------- */

// common bitmaps handlers
static rpage_bitmap *puzzle_sprites_bitmap;
static rpage_bitmap *puzzle_sprites_bitmap_mask;

// Altos puzzle mini game
UBYTE altos_board[SP_ALTOS_HEIGHT][SP_ALTOS_WIDTH];
vec2 altos_key_selection[4];
UBYTE altos_key_count;
SoundInfo *sp_sfx_0, *sp_sfx_1, *sp_sfx_2;
short video_inverse[32];

// Intro screen
// Game End screen (victory)
static rpage_bitmap *endscreen_a;
static rpage_bitmap *endscreen_b;
// static rpage_palette *endscreen_palette, *endscreen_palette_2, *endscreen_fade_palette;
static short endscreen_scroll_y, prev_y;
static rect endscreen_clip_rect;
#define LOGO_POS_Y 144
#define LOGO_Y_ANIM 72

// GameOver screen
static ULONG sp_sfx_clock;
static ULONG sp_sfx_duration; 

void special_empty_players_hand(void)
{
    game_set_object_auto_inventory(game_get_object_in_hand());
    game_set_object_to_hand(OBJECT_NONE);
    gui_draw_object_in_hand(OBJECT_NONE, NULL);
}

void start_game_special(short game_special_idx)
{
    switch (game_special_idx)
    {
    case -1:
        game_set_special_callback(NULL);
        break;

    case TRAVEL:
        special_empty_players_hand();
        game_set_special_callback(&special_travel_menu_init);
        break;

    case GAME_OVER:
        special_empty_players_hand();
        worldSetCurrentRoom(game_get_current_loaded_room());
        // game_set_special_callback(&special_game_over);
        special_game_over(NULL, NULL);
        break;

    case THE_END:
        special_empty_players_hand();
        game_set_state(game_state_victory);
        break;

    case GAME_SAVE:
        special_empty_players_hand();
        save_write_mode = TRUE;
        game_set_special_callback(&special_open_save_dialog_init);
        break;

    case GAME_LOAD:
        special_empty_players_hand();
        save_write_mode = FALSE;
        game_set_special_callback(&special_open_save_dialog_init);
        break;

    // case ENTER_CODE:
    //     special_vi_genere();
    //     special_empty_players_hand();
    //     game_set_special_callback(&special_key_puzzle_init);
    //     break;

    case DEBUG:
        game_set_special_callback(&special_debug_panel);
        break;
    }
}

// Load / Save

void special_open_save_dialog_init(short *button, vec2 *mouse_coords)
{
    short idx;
    short world_index;
    USHORT hours, minutes, seconds;
    char *world_names[5] = {"blank", "Cnossos", "Indus", "RapaNui", "Boat"};

    // Collect existing save files
    for (idx = 0; idx < MAX_SAVE_SLOT; idx++)
    {
        if (game_load_slot_preview(idx, &world_index, &hours, &minutes, &seconds, device_is_hdd()))
        {
            if (hours > 0)
                sprintf(save_slot_name[idx], "%dH%02dm, %s", hours, minutes, world_names[world_index]);
            else
                sprintf(save_slot_name[idx], "%dm%02ds, %s", minutes, seconds, world_names[world_index]);
        }
        else
        {
            sprintf(save_slot_name[idx], SAVE_EMPTY_SLOT);
        }
    }

    // Open dialog
    gui_show_save_panel(save_write_mode, &save_dialog);

    if (!*button)
    {
        short _pos_x, _pos_y, _width, _height;
        short i = 0, j = 0;
        _pos_x = save_dialog.dialog_rect.sx + TF_MARGIN;
        _pos_y = save_dialog.dialog_rect.sy + TF_MARGIN + (TF_MARGIN / 2);
        _width = (INVENTORY_WIDTH / 2) - TF_MARGIN;
        _height = (BT_SAVE_SLOT_SIZE * 2) - (TF_MARGIN / 2);

        for (idx = 0; idx < MAX_SAVE_SLOT; idx++)
        {
            short _txt_x, _txt_y;
            char _str[8];
            bt_save_slot[idx].sx = _pos_x + i * _width;
            bt_save_slot[idx].sy = _pos_y + j * _height;
            bt_save_slot[idx].ex = bt_save_slot[idx].sx + BT_SAVE_SLOT_SIZE;
            bt_save_slot[idx].ey = bt_save_slot[idx].sy + BT_SAVE_SLOT_SIZE;

            // clickable button
            gui_draw_3d_button(&bt_save_slot[idx], FALSE);
            _txt_x = bt_save_slot[idx].sx + (TF_MARGIN / 2) - 1;
            _txt_y = bt_save_slot[idx].sy - 1;
            sprintf(_str, "%d", idx + 1);
            rpage_video_draw_text(_str, _txt_x, _txt_y + 1, 2);
            rpage_video_draw_text(_str, _txt_x, _txt_y, 30);

            // save name
            _txt_x = bt_save_slot[idx].sx + BT_SAVE_SLOT_SIZE + (TF_MARGIN / 2);
            _txt_y = bt_save_slot[idx].sy - 1;
            rpage_video_draw_text(save_slot_name[idx], _txt_x + 1, _txt_y + 1, 2);
            rpage_video_draw_text(save_slot_name[idx], _txt_x, _txt_y, 30);

            i++;
            if (i >= 2)
            {
                i = 0;
                j++;
            }
        }

        save_slot_selected = -1;

        game_set_special_callback(&special_open_save_dialog);
    }
}

void special_open_save_dialog(short *button, vec2 *mouse_coords)
{
    if ((*button) == PLATFORM_MOUSE_LEFT_BUTTON)
    {
        short idx;
        for (idx = 0; idx < MAX_SAVE_SLOT; idx++)
        {
            if (point_within_rect(mouse_coords, &bt_save_slot[idx]))
            {
                short j;
                // Clear all previously selected buttons
                for (j = 0; j < MAX_SAVE_SLOT; j++)
                    gui_draw_button_unselected(&bt_save_slot[j]);
                // select button
                save_slot_selected = idx;
                gui_draw_button_selected(&bt_save_slot[idx]);
                rpage_video_vsync();
                break;
            }
        }

        if (idx >= MAX_SAVE_SLOT)
        {
            if (point_within_rect(mouse_coords, &save_dialog.button0)) // Clicked on load/save
            {
                if (save_slot_selected >= 0)
                {
                    gui_draw_button_selected(&save_dialog.button0);
                    if (save_write_mode) // We want to save
                        gameSaveSlot(save_slot_selected, device_is_hdd());
                    else
                    {
                        // We want to load
                        if (strncmp(save_slot_name[save_slot_selected], SAVE_EMPTY_SLOT, 4) == 0)
                            rpage_system_flash();
                        else
                        {
                            gameLoadSlot(save_slot_selected, device_is_hdd());
                            // special_squid_dialog_tag();
                        }
                    }
                    gui_set_modal_flag(FALSE); // Load/Save panel (proceed)
                    game_wait_ticks(30);
                    game_vue_make_dirty();
                    game_set_special_callback(NULL);
                }
                else
                {
                    rpage_system_flash();
                }
                return;
            }
            else if (point_within_rect(mouse_coords, &save_dialog.button1)) // Clicked on Cancel
            {
                gui_set_modal_flag(FALSE); // Load/Save panel (cancel)
                gui_draw_button_selected(&save_dialog.button1);
                game_wait_ticks(10);
                game_vue_make_dirty();
                game_set_special_callback(NULL);
                return;
            }
        }
    }
}

void special_debug_panel(short *button, vec2 *mouse_coords)
{
#ifdef GAME_VISUAL_DEBUG
    game_set_action(-1);
    rpage_mouse_show();
    gui_show_debug_metrics(game_get_system_metrics());
#endif
    game_set_special_callback(NULL);
}

void special_travel_menu_init(short *button, vec2 *mouse_coords)
{
    // Flag009 = 1;
    game_set_action(-1);
    rpage_mouse_show();
    gui_show_travel_menu(game_get_travel_menu(), Flag009);
    // printf("%d, ", *button);
    if (*button == 0)
        game_set_special_callback(&special_travel_menu_update);
}

void special_travel_menu_update(short *button, vec2 *mouse_coords)
{
    gui_get_pointed_destination(mouse_coords, FALSE, Flag009);

    if (*button)
    {
        short travel_destination_idx;
        travel_destination_idx = gui_get_pointed_destination(mouse_coords, TRUE, Flag009);
        switch (travel_destination_idx)
        {
        case -1:            // clicked outside
            gui_fx_shake();
            gui_hide_travel_menu();
            game_set_special_callback(NULL);
            break;

        default:                                  // clicked inside
            if (travel_destination_idx < Flag009) // actual destination
                game_go_to_travel_destination(travel_destination_idx);
            // otherwise, just "stay here"
            gui_hide_travel_menu();
            game_set_special_callback(NULL);
            break;
        }
    }

#ifdef _STRESS_TEST_
    game_set_special_callback(NULL);
#endif
}

void special_game_over(short *button, vec2 *mouse_coords)
{
    // protracker_fadeout_async();
    gui_fade_to_grey(64, rpage_video_get_front_palette());
    game_wait_ticks(4 * 60);

    game_set_special_callback(&special_game_over_load_screen);
}

void special_game_over_load_screen(short *button, vec2 *mouse_coords)
{
    rpage_bitmap *game_over_bmp;
    short i, w, h, _x;
    rect r;
    char *_txt;

    r.sx = 0;
    r.sy = 0;
    r.ex = SCREEN_WIDTH - 1;
    r.ey = SCREEN_HEIGHT - 1;

    sp_sfx_0 = LoadPackedSound(asset_build_device_path("sfx_laugh", ".pak"), game_get_unpacking_buffer(), game_get_sound_buffer());

    load_vue("game_over");
    gui_fadeout(16, rpage_video_get_front_palette());

    rpage_mouse_hide();
    protracker_stop();
    unload_protracker_music();

    rpage_video_set_palette_to_black(0, 31);
    rpage_video_flip_buffers();
    rpage_video_set_palette_to_black(0, 31);
    rpage_video_clear();
    // rpage_video_present_palette();
    rpage_video_vsync();
    rpage_video_present_screen();

    game_over_bmp = get_vue_bitmap("game_over");
    w = rpage_bitmap_get_width(game_over_bmp);
    h = rpage_bitmap_get_height(game_over_bmp);

    rpage_video_flip_buffers();
    rpage_video_clear();
    rpage_video_blt_bmp(game_over_bmp, 0, 0, w, h, (SCREEN_WIDTH - w) >> 1, (SCREEN_HEIGHT - h) >> 1);

    _txt = game_get_system_dialog(SYS17);
    _x = (SCREEN_WIDTH - rpage_video_get_text_width(_txt)) >> 1;
    rpage_video_draw_text(_txt, _x - 1, SCREEN_HEIGHT - 12, 20);

    gui_fadein(32, get_vue_palette("game_over"));
    rpage_video_set_palette(get_vue_palette("game_over"), 1 << SCREEN_DEPTH);
    
    PlaySound(sp_sfx_0, MAXVOLUME, LEFT1, 0, ONCE);
    PlaySound(sp_sfx_0, MAXVOLUME, RIGHT1, 0, ONCE);

    for(i = 0; i < 8; i++)
        rpage_video_vsync();

    PlaySound(sp_sfx_0, MAXVOLUME >> 1, LEFT0, 0, ONCE);

    for(i = 0; i < 8; i++)
        rpage_video_vsync();

    PlaySound(sp_sfx_0, MAXVOLUME >> 2, RIGHT0, 0, ONCE);

    sp_sfx_clock = rpage_get_clock();
    if (sp_sfx_0 != NULL && sp_sfx_0->RecordRate > 0)
        sp_sfx_duration = ((sp_sfx_0->FileLength) * 1000) / sp_sfx_0->RecordRate;
    else
        sp_sfx_duration = 0;        

    rpage_video_wipe_rect_to_physical_screen(&r);
    rpage_video_present_screen();

    if (!*button)
        game_set_special_callback(&special_game_over_update);
}

void special_game_over_update(short *button, vec2 *mouse_coords)
{
    if ((*button) && (rpage_get_clock() - sp_sfx_clock > sp_sfx_duration))
    {
        gui_fadeout(32, rpage_video_get_front_palette());

        StopSound(LEFT1);
        StopSound(RIGHT1);
        StopSound(LEFT0);
        StopSound(RIGHT0);

        rpage_video_flip_buffers();
        rpage_video_set_palette_to_black(0, 31);
        rpage_video_clear();
        rpage_video_present_screen();

        rpage_video_flip_buffers();
        gui_display();
        rpage_video_present_screen();
        rpage_video_sync_buffers();

        flush_vue_cache();
        SoundInfoFree(sp_sfx_0);
        // unload_sprites_sheet();
        game_reset_world();
        // world_set_current_index(world_cnossos);
        game_set_current_loaded_room(99);
        worldSetCurrentRoom(vue_get_room_index(2));
        game_set_next_vue_dialog(GAME_DEFAULT_VUE_DIALOG);
        game_vue_make_dirty();
        rpage_mouse_wait();
        game_set_special_callback(NULL);
    }
}

void game_intro_screen(void)
{
//     short k, w, save_h, _tmp;
//     // int i, _fade;
//     rect _r = {0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT};
//     ULONG loading_time;
//     rpage_bitmap *title_back, *backbuffer;
//     rpage_palette endscreen_palette[32], endscreen_palette_2[32], color_logo_palette[32];
//     SoundInfo *sfx_voice_intro = NULL;

//     //  Load the voice over
//     sfx_voice_intro = LoadPackedSound(asset_build_device_path("sfx_voice_intro", ".pak"), 
//                                         game_get_unpacking_buffer(),
// //                                        (UBYTE *)(game_get_unpacking_buffer() + MAX_SFX_SIZE + 16), 
//                                         game_get_sound_buffer());                                       

//     //  Load Safar Games Logo
//     load_pak_to_palette(asset_build_device_path("pal_logo_safar_games", ".pak"), endscreen_palette_2);
//     rpage_load_pak_to_new_bitmap(&endscreen_a, NULL, game_get_unpacking_buffer(), asset_build_device_path("logo_safar_games", ".pak"));

//     rpage_mouse_hide();
//     rpage_video_flip_buffers();
//     rpage_video_set_palette_to_black(0, 31);
//     rpage_video_present_screen();

//     // Display Safar Games Logo
//     rpage_video_flip_buffers();
//     rpage_video_set_palette_to_black(0, 31);
//     rpage_video_clear();
//     rpage_video_blt_bmp(endscreen_a, 0, 0, rpage_bitmap_get_width(endscreen_a), rpage_bitmap_get_height(endscreen_a), (DEFAULT_WIDTH - rpage_bitmap_get_width(endscreen_a)) >> 1, (DEFAULT_HEIGHT - rpage_bitmap_get_height(endscreen_a)) >> 1);
//     rpage_video_set_palette_to_black(0, 31);

//     rpage_video_present_screen();
//     rpage_video_sync_buffers();
//     rpage_bitmap_free(endscreen_a);
//     endscreen_a = NULL;

//     // Safar Game Voice Over
//     if (sfx_voice_intro)
//     {
//         PlaySound(sfx_voice_intro, MAXVOLUME, LEFT0, 0, ONCE);
//         PlaySound(sfx_voice_intro, MAXVOLUME, LEFT1, 0, ONCE);
//         rpage_video_vsync();
//         PlaySound(sfx_voice_intro, MAXVOLUME, RIGHT0, 0, ONCE);
//         PlaySound(sfx_voice_intro, MAXVOLUME, RIGHT1, 0, ONCE);
//     }
//     else
//     {
//         rpage_video_vsync();
//     }
    

//     // Fade in with blue logo palette
//     gui_fadein(SCREEN_FADEIN_LEN, endscreen_palette_2);

//     loading_time = rpage_get_clock();

//     //  Load colored logo
//     load_pak_to_palette(asset_build_device_path("pal_logo_safar_games_c", ".pak"), color_logo_palette);
//     rpage_load_pak_to_new_bitmap(&endscreen_a, NULL, game_get_unpacking_buffer(), asset_build_device_path("logo_safar_games_c", ".pak"));

//     //  Wait until the voice over is done
//     if (sfx_voice_intro)
//     {    
//         while(rpage_get_clock() - loading_time < (sfx_voice_intro->FileLength * 1000) / sfx_voice_intro->RecordRate)
//             rpage_video_vsync();
//     }

//     //  Free audio channels
//     StopSound(LEFT0);
//     StopSound(RIGHT0);
//     StopSound(LEFT1);
//     StopSound(RIGHT1);

//     if (sfx_voice_intro)
//         SoundInfoFree(sfx_voice_intro);

//     //  Super weird hack to "reset" the audio channels
//     RethinkDisplay();

//     //  Play music
//     protracker_setup_mod();
//     protracker_play();
//     loading_time = rpage_get_clock();

//     while(rpage_get_clock() - loading_time < 150)
//         rpage_video_vsync();    

//     gui_fade_to_white(16, endscreen_palette_2, FALSE);

//     //  Display (blit) the color logo
//     rpage_video_blt_bmp(endscreen_a, 0, 0, rpage_bitmap_get_width(endscreen_a), rpage_bitmap_get_height(endscreen_a), (DEFAULT_WIDTH - rpage_bitmap_get_width(endscreen_a)) >> 1, (DEFAULT_HEIGHT - rpage_bitmap_get_height(endscreen_a)) >> 1);
//     // rpage_video_set_palette(color_logo_palette, 32);
//     // rpage_video_present_palette();
//     // gui_fade_to_white(64, color_logo_palette, TRUE);
//     gui_fade_to_white(80, color_logo_palette, TRUE);

//     // rpage_video_flip_buffers();
//     // backbuffer = rpage_video_get_front_bitmap();
//     // backbuffer = rpage_video_get_back_bitmap();
//     backbuffer = rpage_bitmap_new(320, 200, 5);
//     load_pak_to_palette(asset_build_device_path("pal_intro_screen", ".pak"), endscreen_palette);
//     rpage_load_pak_into_bitmap(&backbuffer, NULL, game_get_unpacking_buffer(), asset_build_device_path("intro_screen", ".pak"));
//     rpage_bitmap_blit(backbuffer, 0, 0, 320, 200, 0, 0, rpage_video_get_back_bitmap());
//     rpage_video_wait_dma();
//     rpage_bitmap_free(backbuffer);

//     while(rpage_get_clock() - loading_time < 6800)
//         rpage_video_vsync();

//     //  Fade out Safar Game logo
//     gui_fadeout(SCREEN_FADEOUT_LEN, color_logo_palette);

//     rpage_video_flip_buffers();

//     // Title picture
//     // // print version number
//     // rpage_video_draw_text(ATARI_DATASET_VERSION AMIGA_VERSION_STRING, 3, 0, 4);
//     // rpage_video_draw_text(ATARI_DATASET_VERSION AMIGA_VERSION_STRING, 4, 0, 4);
//     // rpage_video_draw_text(ATARI_DATASET_VERSION AMIGA_VERSION_STRING, 3, 2, 4);
//     // rpage_video_draw_text(ATARI_DATASET_VERSION AMIGA_VERSION_STRING, 5, 2, 2);
//     // rpage_video_draw_text(ATARI_DATASET_VERSION AMIGA_VERSION_STRING, 4, 1, 24);

//     rpage_video_present_screen();

//     // Fade in
//     gui_fadein(SCREEN_FADEIN_LEN, endscreen_palette);

//     // Game title
//     rpage_video_sync_buffers();
//     rpage_video_flip_buffers();
//     rpage_load_pak_to_new_bitmap(&endscreen_a, NULL, game_get_unpacking_buffer(), asset_build_device_path("windless_title", ".pak"));
//     rpage_load_pak_to_new_bitmap(&endscreen_b, NULL, game_get_unpacking_buffer(), asset_build_device_path("windless_title_mask", ".pak"));

//     _tmp = rpage_bitmap_get_height(endscreen_a);
//     save_h = min(DEFAULT_HEIGHT, LOGO_POS_Y + _tmp + LOGO_Y_ANIM) - LOGO_POS_Y;
//     title_back = rpage_bitmap_new(rpage_bitmap_get_width(endscreen_a), save_h, SCREEN_DEPTH);
//     rpage_video_save_to_bitmap(title_back, 20, LOGO_POS_Y, rpage_bitmap_get_width(endscreen_a), save_h);

//     rpage_video_wait_dma();

//     // logo scroll
//     w = rpage_bitmap_get_width(endscreen_a);

//     for (k = 0; k <= LOGO_Y_ANIM; k++)
//     {
//         // rpage_video_vsync();
//         rpage_video_flip_buffers();

//         // title scroll
//         rpage_video_blt_bmp(title_back, 0, 0, w, save_h, 20, LOGO_POS_Y);
//         rpage_video_blt_bmp_clip_mask(endscreen_a, 0, 0, 280, 46, 20, LOGO_POS_Y - k + LOGO_Y_ANIM, endscreen_b, &_r);

//         rpage_video_present_screen();
//     }

//     rpage_bitmap_free(title_back);
//     title_back = NULL;
//     rpage_bitmap_free(endscreen_a);
//     endscreen_a = NULL;
//     rpage_bitmap_free(endscreen_b);
//     endscreen_b = NULL;

//     //  Blinking "press mouse" message
//     {
//         char *_txt = game_get_system_dialog(SYS17);
//         short _x;
//         const short _color[24] = {19, 30, 29, 28, 27, 26, 26, 27, 28, 29, 30, 19,
//                                   19, 30, 29, 28, 27, 26, 26, 27, 28, 29, 30, 19};
//         // 19 <-> 31
//         // 18 <-> 4
//         _x = (SCREEN_WIDTH - rpage_video_get_text_width(_txt)) >> 1;
//         rpage_video_draw_text(_txt, _x + 1, SCREEN_HEIGHT - 13, 5);
//         rpage_video_draw_text(_txt, _x, SCREEN_HEIGHT - 11, 0);
//         rpage_video_draw_text(_txt, _x - 1, SCREEN_HEIGHT - 12, 30);

//         while (HACK_MOUSE_LEFT_UP)
//             rpage_video_vsync();

//         k = 0;
//         while (!HACK_MOUSE_LEFT_UP)
//         {
//             rpage_video_vsync();
//             rpage_video_draw_text(_txt, _x - 1, SCREEN_HEIGHT - 12, _color[k >> 2]);
//             k = (k + 1) % (24 << 2);
//         }
//     }

//     //  Fade out
//     protracker_fadeout_async();
//     rpage_video_present_screen();
//     gui_fadeout(SCREEN_FADEOUT_LEN, endscreen_palette);

//     rpage_video_flip_buffers();
//     rpage_video_clear();
//     rpage_video_set_palette_to_black(0, 31);
//     rpage_video_present_screen();

//     rpage_mouse_show();
}

//  Game End (victory)
void game_endscreen(void)
{
//     short i, j;
//     int t, _fade, y;
//     UWORD grey_pal[32], half_pal[32], _r = 2, _b = 1;
//     rpage_palette endscreen_palette[32], endscreen_fade_palette[32];
//     rpage_bitmap *backbuffer, *frontbuffer;

//     rpage_video_set_font("mini6.font", 8);

//     // prepare half palette (simulate the loss of the first bitplane)
//     for (i = 1; i < 32; i += 2)
//     {
//         j = i >> 1;
//         if (j & 0x1)
//             half_pal[i] = components_to_rgb4(((j >> 1) + 1) << 4, (j >> 1) << 4, ((j >> 1) + 1) << 4);
//         else
//             half_pal[i] = components_to_rgb4((j >> 1) << 4, (j >> 1) << 4, ((j >> 1) + 1) << 4);
//     }

//     for (i = 0; i < 31; i += 2)
//     {
//         j = i >> 1;
//         // half_pal[i] = components_to_rgb4((j + _r) * 15, j * 15, (j + _b) * 15);
//         if (j & 0x1)
//             half_pal[i] = components_to_rgb4(((j >> 1) + 1) << 4, (j >> 1) << 4, ((j >> 1) + 1) << 4);
//         else
//             half_pal[i] = components_to_rgb4((j >> 1) << 4, (j >> 1) << 4, ((j >> 1) + 1) << 4);
//     }

//     // prepare grey palette (simulate 50% transparent hilite of the first bitplane)
//     t = 127;
//     for (i = 1; i < 32; i += 2)
//     {
//         j = (i - 1) >> 1;
//         grey_pal[i] = components_to_rgb4(((j + _r) << 4) + t, (j << 4) + t, ((j + _b) << 4) + t);
//     }

//     for (i = 0; i < 31; i += 2)
//     {
//         j = i >> 1;
//         // grey_pal[i] = components_to_rgb4((j + _r) * 15, j * 15, (j + _b) * 15);
//         if (j & 0x1)
//             grey_pal[i] = components_to_rgb4(((j >> 1) + 1) << 4, (j >> 1) << 4, ((j >> 1) + 1) << 4);
//         else
//             grey_pal[i] = components_to_rgb4((j >> 1) << 4, (j >> 1) << 4, ((j >> 1) + 1) << 4);
//     }

//     //
//     // Load everything
//     //
//     rpage_mouse_wait();
//     load_pak_to_palette(asset_build_device_path("pal_vue_19", ".pak"), endscreen_palette);
//     backbuffer = getSpriteSheetBitmap(); // rpage_video_get_back_bitmap();
//     rpage_load_pak_into_bitmap(&backbuffer, NULL, game_get_unpacking_buffer(), asset_build_device_path("vue_19", ".pak"));
//     rpage_mouse_hide();
//     rpage_video_flip_buffers();
//     rpage_video_blt_bmp(backbuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
//     rpage_video_set_palette_to_black(0, (1 << SCREEN_DEPTH) - 1);
//     rpage_video_wait_dma();
//     rpage_video_present_screen();
//     // game_draw_debug_visuals();
//     rpage_video_sync_buffers();

//     protracker_play();

//     // Fade in
//     for (t = 0; t < SCREEN_FADEIN_LEN; t++)
//     {
//         rpage_video_vsync();
//         rpage_video_flip_buffers();
//         _fade = (t * 255) / SCREEN_FADEIN_LEN;
//         for (i = 0; i < (1 << SCREEN_DEPTH); i++)
//             endscreen_fade_palette[i] = darken_rgb4_colors(endscreen_palette[i], 255 - _fade); // mix_rgb4_colors(0x000, endscreen_palette[i], _fade);
//         rpage_video_set_palette(endscreen_fade_palette, (1 << SCREEN_DEPTH));
//         rpage_video_present_screen();
//     }

//     // Exposure
//     for (t = 0; t < ENDSCROLL_EXPOSE_LEN * 2; t++)
//         rpage_video_vsync();

//     for (t = 0; t < SCREEN_FADEOUT_LEN; t++)
//     {
//         rpage_video_vsync();
//         rpage_video_flip_buffers();
//         _fade = (t * 255) / SCREEN_FADEOUT_LEN;
//         for (i = 0; i < (1 << SCREEN_DEPTH); i++)
//             endscreen_fade_palette[i] = mix_rgb4_colors(endscreen_palette[i], half_pal[i], _fade);
//         rpage_video_set_palette(endscreen_fade_palette, (1 << SCREEN_DEPTH));
//         rpage_video_present_screen();
//     }

//     for (t = 0; t < (ENDSCROLL_EXPOSE_LEN >> 1); t++)
//         rpage_video_vsync();

//     rpage_video_flip_buffers();
//     rpage_video_clear_bit_mask(0x1);
//     rpage_video_set_palette(grey_pal, 32);
//     rpage_video_present_screen();
//     rpage_video_sync_buffers();

//     // y = 64;

//     //  Gradient copper list
// #ifdef LATTICE
//     {
//         short i, j, c;
//         ULONG *pal;
//         struct UCopList *copper;
//         struct ViewPort *vp;

//         buffered_screen *bs;

//         bs = rpage_video_get_ptr();
//         vp = &(bs->screen->ViewPort);

//         copper = (struct UCopList *)AllocMem(sizeof(struct UCopList), MEMF_PUBLIC | MEMF_CHIP | MEMF_CLEAR);

//         pal = (ULONG *)malloc(sizeof(ULONG) * 256);
//         memset(pal, 0xFF00FF, 256);

//         CINIT(copper, SCREEN_HEIGHT * 2 * 16);

//         for (i = 0; i < SCREEN_HEIGHT; i++)
//         {
//             if ((i > 0) && (i < 24))
//             {
//                 j = (i * 255) / 24; // Map to (0, 255)
//                 CWAIT(copper, i, 0);
//                 for (c = 0; c < 32; c += 2)
//                     CMOVE(copper, custom.color[c + 1], mix_rgb4_colors(grey_pal[c], grey_pal[c + 1], j));
//             }
//             else if (i > SCREEN_HEIGHT - 24)
//             {
//                 j = (i - SCREEN_HEIGHT + 24) * 16; // Map to (0, 255)
//                 CWAIT(copper, i, 0);
//                 for (c = 0; c < 32; c += 2)
//                     CMOVE(copper, custom.color[c + 1], mix_rgb4_colors(grey_pal[c + 1], grey_pal[c], j));
//             }
//         }

//         CEND(copper);

//         vp->UCopIns = copper;

//         rpage_video_vsync();

//         RethinkDisplay();

//         free(pal);
//         // FreeMem(copper, sizeof(struct UCopList));
//     }
// #endif

//     rpage_video_sync_buffers();
//     rpage_video_present_screen();
//     frontbuffer = rpage_video_get_front_bitmap();

//     t = 0;
//     y = 0;
//     while (y < CREDITS_MAX_DIALOG + (SCREEN_HEIGHT / 0xF / 2) - 2)
//     {
//         rpage_video_vsync();
//         rpage_video_wait_dma();
//         rpage_video_vsync();

//         if ((t % 14) == 0)
//         {
//             if (y < CREDITS_MAX_DIALOG)
//                 if (strncmp(credits_dialogs[y], "STOP", 6))
//                     rpage_video_draw_text_bit_mask(credits_dialogs[y], -1, 186, 1, 0x1);
//             y++;
//         }

//         // rpage_video_vsync();
//         // rpage_video_wait_dma();

//         rpage_video_vsync();
// #ifdef LATTICE
// #define CREDIT_SCROLL_PAD 64
//         BltBitMap(frontbuffer, CREDIT_SCROLL_PAD, 2, frontbuffer, CREDIT_SCROLL_PAD, 1, SCREEN_WIDTH - (CREDIT_SCROLL_PAD * 2), SCREEN_HEIGHT - 2, 0xC0, 0x1, NULL);
// #else
//         rpage_video_scroll_bit_mask(0, -1, 0x1);
// #endif

//         t++;
//     }

//     rpage_video_sync_buffers();

//     {
//         BOOL easter_egg = FALSE;
//         while (HACK_MOUSE_LEFT_UP)
//             rpage_video_vsync();

//         while (!HACK_MOUSE_LEFT_UP)
//         {
//             rpage_video_vsync();

//             if (!easter_egg && HackGetKeyboardCode() == 0x54)
//             {
//                 rpage_mouse_wait();
//                 unload_protracker_music();
//                 protracker_set_fade_speed(0);		
//                 load_packed_protracker_music(asset_build_device_path("mus_ending_dmasc", ".pak"), game_get_unpacking_buffer(), &music_memory_buffer);
//                 protracker_setup_mod();
//                 protracker_update_state();
//                 protracker_play();
//                 rpage_mouse_hide();
//                 easter_egg = TRUE;
//             }
//         }
//     }

//         // Remove the gradient copper list
// #ifdef LATTICE
//     {
//         // short i, j, c;
//         ULONG *pal;
//         struct UCopList *copper;
//         struct ViewPort *vp;

//         buffered_screen *bs;

//         bs = rpage_video_get_ptr();
//         vp = &(bs->screen->ViewPort);

//         copper = (struct UCopList *)AllocMem(sizeof(struct UCopList), MEMF_PUBLIC | MEMF_CHIP | MEMF_CLEAR);

//         pal = (ULONG *)malloc(sizeof(ULONG) * 256);
//         memset(pal, 0xFF00FF, 256);

//         CINIT(copper, 4 * 2 * 16);
//         CWAIT(copper, 0, 0);
//         CEND(copper);

//         vp->UCopIns = copper;

//         rpage_video_vsync();

//         RethinkDisplay();

//         free(pal);
//         // FreeMem(copper, sizeof(struct UCopList));
//     }
// #endif

//     //  Fade out
//     for (t = 0; t < SCREEN_FADEOUT_LEN; t++)
//     {
//         rpage_video_vsync();
//         rpage_video_flip_buffers();
//         _fade = (t * 255) / SCREEN_FADEOUT_LEN;
//         for (i = 0; i < (1 << SCREEN_DEPTH); i++)
//             endscreen_fade_palette[i] = darken_rgb4_colors(grey_pal[i], _fade); // mix_rgb4_colors(endscreen_palette[i], 0x000, _fade);
//         rpage_video_set_palette(endscreen_fade_palette, (1 << SCREEN_DEPTH));
//         rpage_video_present_screen();
//     }

//     rpage_mouse_show();

//     // Unload everything
//     // rpage_bitmap_free(endscreen_a);
//     // rpage_bitmap_free(endscreen_b);
//     // endscreen_a = NULL;
//     // endscreen_b = NULL;
}