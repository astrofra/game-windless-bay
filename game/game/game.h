/*  	Windless Bay, Mount Erebus.
	Original game by Francois "Astrofra" Gutherz. 
	Amiga version (C) RESISTANCE 2021 
*/

#ifndef _GAME_HEADER_
#define _GAME_HEADER_

// #define GAME_VISUAL_DEBUG
// #define _STRESS_TEST_
// #define _ST_SIMULATE_CLICKS_
// #define _ST_PLAY_PUZZLES_

#include "rpage/aos/io.h"
#include "game/aos/files.h"

#define AMIGA_VERSION_STRING "-1.2"

#define MAX_MOD_SIZE (95 * 1024)

#define GAME_DEFAULT_VUE_DIALOG 0

#define CHAR_W 6
#define CHAR_H 7
#define LINE_H (CHAR_H + 1)
#define DIALOG_X 12
#define DIALOG_Y 152
#define DIALOG_SUBSTR_LEN 256
#define DIALOG_MAX_LINE 64

#define INVENTORY_GRID_W 10
#define INVENTORY_GRID_H 2
#define INVENTORY_SIZE (INVENTORY_GRID_W * INVENTORY_GRID_H)

#define ENDTEXT_LEN (4 * 50)
#define ENDTEXT_FADE_LEN 32

#define ENDSCROLL_LEN (6 * 50)
#define ENDSCROLL_EXPOSE_LEN (4 * 50)
#define SCREEN_FADEIN_LEN 32
#define SCREEN_FADEOUT_LEN 16

#define OBJECT_NONE 0
#define INVENTORY_AUTO_INDEX -1

#define ORIGINAL_TIMER_FREQ 200

#define DIALOG_TAG_SIZE 8 // max size of dialog customizer string

#define SAVE_HEADER "SAV2"
#define SAVE_HEADER_TIME "TIME"
#define SAVE_HEADER_VUE "WRLD"
#define SAVE_HEADER_INVENTORY "INVT"
#define SAVE_HEADER_LINKS "LINK"
#define SAVE_HEADER_PUZZLES "PUZL"
#define SAVE_HEADER_SPRITES "SPRT"
#define SAVE_HEADER_FLAGS "FLAG"

#define AUDIO_MUSIC_OFF 0
#define AUDIO_MUSIC_PLAY 1
#define AUDIO_MUSIC_FADEOUT 2
#define AUDIO_MUSIC_MUTE 3

#define AUDIO_SFX_OFF 0
#define AUDIO_SFX_PLAYING 1

#define SAVE_DISK_NAME "save:"

extern short save_room;
extern short save_world;

typedef struct {
	BOOL disk_change;
	char next_disk[DISKNAME_LEN];
} game_internals;

enum game_state
{
	game_state_intro,
	game_state_play,
	game_state_victory,
	game_state_quit
};

typedef struct {
	short world_index;
	short vue_index;
	short dialog_idx;
} travel_destination;

#define SYS_METRICS_STR_SIZE 64
typedef struct
{
    char chipmem_avail[SYS_METRICS_STR_SIZE];
    char largest_chipmem_avail[SYS_METRICS_STR_SIZE];
    char othermem_avail[SYS_METRICS_STR_SIZE];
    char vuecache_size[SYS_METRICS_STR_SIZE];
    char gui_size[SYS_METRICS_STR_SIZE];
} system_metrics;

void game_init(game_internals *gi);
void game_uninit(void);
ULONG game_update_frame_clock(void);
ULONG game_get_frame_clock(void);
void game_reset_world(void);
void game_free_locale(void);
int game_locale_selector(void);
void game_load_locale(short locale_idx);
char *game_get_system_dialog(short sys_index);
UBYTE *game_get_unpacking_buffer(void);
UBYTE *game_get_sound_buffer(void);
system_metrics *game_get_system_metrics(void);
void game_set_unpacking_buffer(UBYTE *addr);
void game_reset_timers(void);
void game_set_action(short _action);
short game_get_action(void);
void game_set_current_loaded_world(short world_index);
short game_get_current_loaded_world(void);
short game_get_current_loaded_room(void);
void game_set_current_loaded_room(short room);
void game_draw_debug_compass_button(void);
void game_update(game_internals *gi);
void game_vue_make_dirty(void);
void game_vue_refresh(void);
void game_play_sample(short sample_index);
void game_play_sample_ex(short sample_index, short unknown_param);
void game_load_world_sprites(short world_index);
void game_hide_sprite(short sprite_index);
void game_show_sprite(short sprite_index);
void game_set_palette(char *pal_name);
void game_fadeto_palette(char *pal_name);
short game_get_dialog_sequence_index(void);
void game_set_dialog_sequence_index(short _idx);
void game_set_next_vue_dialog(short dialog_index);
void game_set_dialog_tag(char *_tag);
void game_display_dialog(short dialog_index);
void game_display_vue_dialog(short vue_index, short dialog_index);
void game_display_dialog_ex(short dialog_index, short character_sprite);
void game_display_dialog_sequence(short first_dialog_index, short last_dialog_index);
void game_display_dialog_sequence_ex(short first_dialog_index, short last_dialog_index, short character_sprite);
void game_display_character(short character_sprite);
BOOL game_is_current_action(short action_index);
void game_reset_inventory(void);
BOOL game_is_object_in_hand(short object_index);
void game_set_object_to_hand(short object_index);
short game_get_object_in_hand(void);
short game_combine_objects(short go_a, short go_b);
BOOL game_is_object_in_inventory(short object_index);
BOOL game_set_object_auto_inventory(short object_index);
BOOL game_set_object_to_inventory(short object_index, short inventory_slot_index);
void game_remove_object_from_inventory(short object_index);
void game_print_inventory(void);
void game_abort_leaving_room(short enable);
void game_clear_compass(void);
void game_clear_dialog(void);
void game_draw_vue_sprites(short vue_index);
void game_endscreen(void);
short control_get_clicked_sprite(vec2 *mouse_coords);
short control_get_clicked_zone(vec2 *mouse_coords);
void game_wait_ticks(unsigned int ticks);
void game_fade_out(BOOL enable);
void game_fade_in(BOOL enable);
void game_over(BOOL enable);
void game_enable_timer1(BOOL enable);
void game_enable_timer2(BOOL enable);

void game_load_music(UWORD music_index);
void game_play_music(BOOL flag);
void game_stop_music(BOOL flag);
void game_play_music_by_world_index(short music_index);
void game_update_music_manager(void);
void game_flush_sfx_manager(void);
void game_update_sfx_manager(void);
void game_sfx_force_stop(void);

void game_set_state(int state);
int game_get_state(void);
void game_set_special_callback(void *func);
void *game_get_special_callback(void);
void game_init_travel_menu(void);
travel_destination *game_get_travel_menu(void);
void game_go_to_travel_destination(short idx);

void game_save_time(rpage_file file);
void gameSaveWorldState(rpage_file file_ptr);
void gameSaveInventoryState(rpage_file file_ptr);
void gameSaveRoomLinks(rpage_file file_ptr);
void gameSavePuzzles(rpage_file file);

BOOL game_load_slot_preview(short slot, short *world_index, USHORT *hours, USHORT *minutes, USHORT *seconds, BOOL is_on_hdd);
void game_load_time(rpage_file file);
void gameLoadWorldState(rpage_file file_ptr);
void gameLoadInventoryState(rpage_file file_ptr);
void gameLoadRoomLinks(rpage_file file_ptr);
void gameLoadPuzzles(rpage_file file);

void game_draw_debug_button(void);
void game_draw_debug_visuals(void);
void game_draw_debug_flags(void);

#endif
