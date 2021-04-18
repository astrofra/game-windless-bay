/*  Athanor 2, Original game by Eric "Atlantis" Safar, (C) Safargames 2021  
Amiga version by Francois "Astrofra" Gutherz.
*/

#include <stdarg.h>
#include <stdio.h>
#include "rpage/frwk.h"
#include "rpage/aos/locale.h"
#include "rpage/aos/sound.h"
#include "rpage/utils.h"
#include "rpage/easing.h"
#include "game/gui.h"
#include "game/vue.h"
#include "game/world.h"
#include "game/game.h"
#include "game/text.h"
#include "game/aos/files.h"
// #include "game/aos/files.h"
#include "game/special.h"
#include "game/aos/assets.h"

#include "rpage/aos/color.h" // FIXME
#include "rpage/aos/ptracker.h"

extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Custom far custom;

/// Game State (see: game_state)
int g_state;
void (*special_callback)(short *button, vec2 *mouse_coords);

short current_loaded_world = -1;
short current_loaded_room = -1;
short current_action = -1;

extern short current_world;
extern short current_room;

short next_vue_dialog = GAME_DEFAULT_VUE_DIALOG;

short save_world;
short save_room;

vec2 poly_pt_list[4];

inventory_object inventory[INVENTORY_SIZE];
short object_in_hand;
BOOL inventory_open = FALSE;

travel_destination travel_menu_list[4];

system_metrics game_metrics;

// timers
unsigned int Timer1, Timer2;
BOOL timer1_enabled, timer2_enabled;
ULONG timers_prev_clock;
unsigned short t_frames, t_seconds, t_minutes, t_hours;
ULONG last_update_clock;
short update_game_clock_every;

BOOL vue_is_dirty;

// Generic unpacking buffer
UBYTE *unpacking_buffer = NULL;

// Sound
char generic_sfx_filename[DISKNAME_LEN];
SoundInfo *generic_sfx;
BYTE *sfx_memory_buffer = NULL;

// Music
short music_world_index;
ULONG music_play_clock;
BYTE *music_memory_buffer = NULL;
short music_manager_update_count;
short music_manager_state;
short sfx_manager_update_count;
short sfx_manager_next_sample = -1;
short sfx_manager_state;
ULONG sfx_play_duration;
ULONG sfx_play_clock;

// dialog
char *dialogs[GAME_MAX_DIALOG];
char *system_dialogs[SYS_MAX_DIALOG];
char *tooltip_dialogs[TOOLTIP_MAX_DIALOG];
char *credits_dialogs[CREDITS_MAX_DIALOG];
UBYTE *dialogs_textblock;
UBYTE *system_textblock;
UBYTE *tooltip_textblock;
UBYTE *credits_textblock;
char dialog_tag[DIALOG_TAG_SIZE];
char str_dynamic[GAME_DYNAMIC_DIALOG_MAX_LEN];

BOOL load_or_save_request;

/// Global dialog index. Incremented each time the player goes thru a dialog sequence. Reset each time the player enters a room.
short dialog_seq_index;

short stress_test_counter = 0;

/// Make all necessary initialization of the game :<br>
/// * inventory<br>
void game_init(game_internals *gi)
{
    game_vue_make_dirty();
    inventory_open = FALSE;
    game_reset_world();
    game_set_special_callback(NULL);
    game_init_travel_menu();

    dialogs_textblock = NULL;
    system_textblock = NULL;
    tooltip_textblock = NULL;
    credits_textblock = NULL;

    // generic_sfx_filename = NULL; 
    generic_sfx = NULL;
    sfx_memory_buffer = rpage_os_alloc(MAX_SFX_SIZE, MEMF_CHIP);
    music_memory_buffer = rpage_os_alloc(MAX_MOD_SIZE, MEMF_CHIP);

    gi->disk_change = FALSE;
    memset(gi->next_disk, 0, DISKNAME_LEN);

    game_update_frame_clock();
    
    // game_set_object_auto_inventory(go_POISSON);
    // game_set_object_auto_inventory(go_HACHE);
    // game_set_object_auto_inventory(go_TORCHE);
}

void game_reset_world(void)
{
    worldClearScenarioFlags();
    game_set_current_loaded_world(-1);
    worldResetSpritesState();
    game_reset_inventory();
    game_set_dialog_sequence_index(0);
    game_set_object_to_hand(OBJECT_NONE);
    game_reset_timers();
    game_set_state(game_state_play);
    game_set_dialog_tag(NULL); // Reset the dialog customizer tag

    game_update_frame_clock();
    update_game_clock_every = 0;
    t_frames = 0;
    t_seconds = 0;
    t_minutes = 0;
    t_hours = 0;

    music_world_index = -1;
    music_manager_update_count = 0;
    music_manager_state = AUDIO_MUSIC_OFF;
    sfx_manager_state = AUDIO_SFX_OFF;
    sfx_play_duration = 0;
    sfx_play_clock = 0;

    sfx_manager_update_count = 0;
    sfx_manager_next_sample = -1;

    load_or_save_request = FALSE;

#ifdef _STRESS_TEST_
    current_room = vue_get_room_index(25);
#endif
}

void game_uninit(void)
{
    game_free_locale();

    StopSound(LEFT0);
    StopSound(LEFT1);
    StopSound(RIGHT0);
    StopSound(RIGHT1);
 
    // if (generic_sfx != NULL)
    // {
    //     RemoveSound(generic_sfx);
    //     generic_sfx = NULL;
    //     memset(generic_sfx_filename, 0, DISKNAME_LEN);
    // }

    if (sfx_memory_buffer != NULL)
    {
        FreeMem(sfx_memory_buffer, MAX_SFX_SIZE);
        sfx_memory_buffer = NULL;
    }

    if (music_memory_buffer != NULL)
    {
        FreeMem(music_memory_buffer, MAX_MOD_SIZE);
        music_memory_buffer = NULL;
    }
    
}

#define LOCALE_MAX_LANG 2
#define LOCALE_SELECTOR_LINE_H 22

int game_locale_selector(void)
{
    rect r[LOCALE_MAX_LANG];
    char *lang[LOCALE_MAX_LANG] = {"Francais", "English"};
    short i, w = 0, w2, h, hb;
    int locale_index = -1;
    vec2 mouse_coords;
    short prev_button = 0;

    for(i = 0; i < LOCALE_MAX_LANG; i++)
    {
        w2 = rpage_video_get_text_width(lang[i]);
        w = max(w, w2);
    }

    w += 16;
    h = LOCALE_MAX_LANG * LOCALE_SELECTOR_LINE_H;
    hb = (LOCALE_SELECTOR_LINE_H - 4); // button height

    for(i = 0; i < LOCALE_MAX_LANG; i++)
    {
        short wt;
        r[i].sx = (SCREEN_WIDTH - w) / 2;
        r[i].ex = r[i].sx + w;

        r[i].sy = (i * LOCALE_SELECTOR_LINE_H) + ((SCREEN_HEIGHT - h) / 2);
        r[i].ey = r[i].sy + hb;

        gui_draw_3d_button(&r[i], TRUE);

        wt = r[i].sx + ((w - rpage_video_get_text_width(lang[i])) / 2);
        rpage_video_draw_text(lang[i], wt, r[i].sy + ((hb - 8) / 2), 0);        
        rpage_video_draw_text(lang[i], wt, r[i].sy + ((hb - 8) / 2) - 1, 30);
    }

    rpage_video_draw_text(ATARI_DATASET_VERSION AMIGA_VERSION_STRING, 4, SCREEN_HEIGHT - 10, 8);

    rpage_mouse_show();

    while (HACK_MOUSE_LEFT_UP)
        rpage_video_vsync();

    while(locale_index < 0)
    {
        short button;

        rpage_video_vsync();
        rpage_input_update();
  
        button = HACK_MOUSE_LEFT_UP;
        if (button && !prev_button)
        {
            rpage_mouse_get_values(NULL, &mouse_coords);
            for(i = 0; i < LOCALE_MAX_LANG; i++)
            {
                if (point_within_rect(&mouse_coords, &r[i]))
                {
                    locale_index = i;
                    gui_draw_button_selected(&r[locale_index]);
                    break;
                }
            }
        }

        prev_button = button;
    }

    gui_draw_button_selected(&r[locale_index]); // FIXME ?

    for(i = 0; i < 60; i++)
        rpage_video_vsync();

    gui_fadeout(16, rpage_video_get_front_palette());

    rpage_video_clear();

    return locale_index;
}

void game_free_locale(void)
{
    if (dialogs_textblock != NULL)
    {
        free(dialogs_textblock);
        dialogs_textblock = NULL;
    }

    if (system_textblock != NULL)
    {
        free(system_textblock);
        system_textblock = NULL;
    }

    if (tooltip_textblock != NULL)
    {
        free(tooltip_textblock);
        tooltip_textblock = NULL;
    }

    if (credits_textblock != NULL)
    {
        free(credits_textblock);
        credits_textblock = NULL;
    }
}

void game_load_locale(short locale_idx)
{
    // Load dialogs
    dialogs_textblock = load_pak_locale_to_array(dialogs, GAME_MAX_DIALOG, game_get_unpacking_buffer(), asset_build_device_path("txt_dialog", locale_ext[locale_idx]));

    // Load system dialogs
    system_textblock = load_pak_locale_to_array(system_dialogs, SYS_MAX_DIALOG, game_get_unpacking_buffer(), asset_build_device_path("txt_system", locale_ext[locale_idx]));

    // Load tooltips
    tooltip_textblock = load_pak_locale_to_array(tooltip_dialogs, TOOLTIP_MAX_DIALOG, game_get_unpacking_buffer(), asset_build_device_path("txt_tooltip", locale_ext[locale_idx]));

    // Load credits
    credits_textblock = load_pak_locale_to_array(credits_dialogs, CREDITS_MAX_DIALOG, game_get_unpacking_buffer(), asset_build_device_path("txt_credits", locale_ext[locale_idx]));

#ifdef DEBUG_MACROS
    {
        short i;
        printf("--- dialogs --- \n");
        for(i = 0; i < GAME_MAX_DIALOG; i++)
            printf("%03d:%s\n", i, dialogs[i]);

        printf("--- system_dialogs --- \n");
        for(i = 0; i < SYS_MAX_DIALOG; i++)
            printf("%03d:%s\n", i, system_dialogs[i]);

        printf("--- tooltip_dialogs --- \n");
        for(i = 0; i < TOOLTIP_MAX_DIALOG; i++)
            printf("%03d:%s\n", i, tooltip_dialogs[i]);

        printf("--- credits_dialogs --- \n");
        for(i = 0; i < CREDITS_MAX_DIALOG; i++)
            printf("%03d:%s\n", i, credits_dialogs[i]);            
    }
#endif
}

void game_init_travel_menu(void)
{
	/* SYS19 "Voguons vers", */
	/* SYS20 "Rester � bord", */
	/* SYS21 "Cnossos city", */
	/* SYS22 "Indus valley", */
	/* SYS23 "Rapa Nui", */

    // travel_menu_list[0].world_index = world_cnossos;
    // travel_menu_list[0].vue_index = 25;
    // travel_menu_list[0].dialog_idx = SYS21; // Cnossos City

    // travel_menu_list[1].world_index = world_indus;
    // travel_menu_list[1].vue_index = 45;
    // travel_menu_list[1].dialog_idx = SYS22; // Indus Valley

    // travel_menu_list[2].world_index = world_rapanui;
    // travel_menu_list[2].vue_index = 1;
    // travel_menu_list[2].dialog_idx = SYS23; // Rapa Nui

    // travel_menu_list[3].world_index = 0;
    // travel_menu_list[3].vue_index = -1;
    // travel_menu_list[3].dialog_idx = SYS20; // Stay here

}

ULONG game_update_frame_clock(void)
{
    last_update_clock = rpage_get_clock();
    return last_update_clock;
}

ULONG game_get_frame_clock(void)
{
    return last_update_clock;
}

system_metrics *game_get_system_metrics(void)
{
#ifdef GAME_VISUAL_DEBUG
    sprintf(game_metrics.chipmem_avail,         "CHIP MEM : %dKb", rpage_get_avail_video_memory() >> 10);
    sprintf(game_metrics.largest_chipmem_avail, "LARG.CHIP: %dKb", rpage_get_avail_largest_video_memory() >> 10);
    sprintf(game_metrics.othermem_avail,        "OTHER MEM: %dKb", rpage_get_avail_non_video_memory() >> 10);
    sprintf(game_metrics.vuecache_size,         "VUE CACHE: %dKb", vue_get_cache_size() >> 10);
    sprintf(game_metrics.gui_size,              "GUI MEM  : %dKb", gui_get_memory_footprint() >> 10);
#else
    memset(&game_metrics, 0, sizeof(game_metrics));
#endif
    return &game_metrics;
}

UBYTE *game_get_unpacking_buffer(void)
{
    return unpacking_buffer;
}

void game_set_unpacking_buffer(UBYTE *addr)
{
    if (addr != NULL)
        unpacking_buffer = addr;
    else
        rpage_system_alert("game_set_unpacking_buffer(), buffer is NULL!") ;  
}

UBYTE *game_get_sound_buffer(void)
{
    return sfx_memory_buffer;
}

void game_vue_make_dirty(void)
{
    vue_is_dirty = TRUE;
}

void game_go_to_travel_destination(short idx)
{
    world_set_current_index(travel_menu_list[idx].world_index);
    worldSetCurrentRoom(vue_get_room_index(travel_menu_list[idx].vue_index));
}

travel_destination *game_get_travel_menu(void)
{
    return (travel_destination *)&travel_menu_list;
}

void game_set_special_callback(void (*func))
{
    special_callback = (void (*))func;
}

void *game_get_special_callback(void)
{
    return special_callback;
}

void game_set_state(int state)
{
    g_state = state;
}

int game_get_state(void)
{
    return g_state;
}

void game_reset_timers(void)
{
    Timer1 = 0;
    Timer2 = 0;
    timer1_enabled = FALSE;
    timer2_enabled = FALSE;
    timers_prev_clock = rpage_get_clock();
}

void update_timers(void)
{
    ULONG   timers_clock = game_get_frame_clock(), // rpage_get_clock(),
            timers_step;

    timers_step = max(timers_clock - timers_prev_clock, 21) / 20;
    // printf("%d, ", timers_step);

    if (timer1_enabled)
        Timer1 += timers_step; // ORIGINAL_TIMER_FREQ/50;
    if (timer2_enabled)
        Timer2 += timers_step; // ORIGINAL_TIMER_FREQ/50;

    timers_prev_clock = timers_clock;
}

/// Set the current action selected by the player. See ::game_action
void game_set_action(short _action)
{
    current_action = _action;
}

/// Get the current action selected by the player. See ::game_action
short game_get_action(void)
{
    return current_action;
}

/// Get the world index that is currently loaded (and not the world index that will be loaded on the next update).
/// See ::world
short game_get_current_loaded_world(void)
{
    return current_loaded_world;
}

/// Set the world index that was loaded during the most recent update.
/// See ::world
void game_set_current_loaded_world(short world_index)
{
    current_loaded_world = world_index;
}

/// Get the room index that is currently loaded (and not the room index that will be loaded on the next update).
short game_get_current_loaded_room(void)
{
    return current_loaded_room;
}

/// Set the room index that was loaded during the most recent update.
void game_set_current_loaded_room(short room)
{
    current_loaded_room = room;
}

/// Mouse click on the compass, return North = 0, East = 1, ... See ::link_direction
/// If no click on the compass, returns -1
short control_get_compass_directions(vec2 *mouse_coords)
{
    short direction;

    for (direction = 0; direction < 4; direction++)
    {
        poly_pt_list[0].x = compass_poly_per_direction[direction].p0.x;
        poly_pt_list[0].y = compass_poly_per_direction[direction].p0.y;
        poly_pt_list[1].x = compass_poly_per_direction[direction].p1.x;
        poly_pt_list[1].y = compass_poly_per_direction[direction].p1.y;
        poly_pt_list[2].x = compass_poly_per_direction[direction].p2.x;
        poly_pt_list[2].y = compass_poly_per_direction[direction].p2.y;
        poly_pt_list[3].x = compass_poly_per_direction[direction].p3.x;
        poly_pt_list[3].y = compass_poly_per_direction[direction].p3.y;

        if (point_within_polygon(mouse_coords, poly_pt_list, 4))
            return direction;

    }

    return -1;
}

short control_get_clicked_sprite(vec2 *mouse_coords)
{
    rect source;
    vec2 dest;
    short i, game_object_idx;
    for (i = 0; i < worldGetMaxSheetSprites(); i++)
    {
        if (getSpriteSheetVueIndex(i) == rooms[worldGetCurrentRoom()].vue_index && getSpriteSheetEnabled(i))
        {
            // compare coordinates
            game_object_idx = getSpriteSheetGameObjectIndex(i);
            gameObjectGetSpriteSheetCoords(game_object_idx, &source, &dest);
            source.ex = dest.x + (source.ex - source.sx) + VUE_X;
            source.ey = dest.y + (source.ey - source.sy) + VUE_Y;
            source.sx = dest.x + VUE_X;
            source.sy = dest.y + VUE_Y;

            if (point_within_rect(mouse_coords, &source))
                return game_object_idx;
        }
    }

    return -1;
}

short control_get_clicked_zone(vec2 *mouse_coords)
{
    short i;
    for (i = 0; i < MAX_ZONE_PER_ROOM; i++)
    {
        if (rooms[worldGetCurrentRoom()].zone[i] > -1)
        {
            if (point_within_quad(mouse_coords, &(zones[rooms[worldGetCurrentRoom()].zone[i]].zone_poly)))
                return i;
        }
        else
            return -1; /* the first -1 of the array means the end of the array */
    }

    return -1;
}

void game_draw_debug_flags(void)
{
#ifdef GAME_VISUAL_DEBUG    
    char *flags[4];
    short i;

    for(i = 0; i < 3; i++)
    {
        flags[i] = (char *)rpage_c_alloc(64, sizeof(char));
    }
    flags[3] = (char *)rpage_c_alloc(WRLD_DBG_FLAG_STRSIZE, sizeof(char));

    sprintf(flags[0], "Timer1 (%d) = %d", (int)timer1_enabled, (int)Timer1);
    sprintf(flags[1], "Timer2 (%d) = %d", (int)timer2_enabled, (int)Timer2);
    sprintf(flags[2], "World = %d, Vue = %d", world_get_current_index(), roomGetVueIndex(worldGetCurrentRoom()));
    worldDebugScenarioFlags(flags[3]);

    gui_show_debug_flags(flags, 4);

    for(i = 0; i < 4; i++)
        free(flags[i]);
#endif
}

void game_draw_debug_visuals(void)
{
#ifdef GAME_VISUAL_DEBUG    
    gui_show_debug_visuals();
#endif    
}

void game_draw_debug_button(void)
{
#ifdef GAME_VISUAL_DEBUG    
    short i;
    char str[16];

    for (i = 0; i < 4; i++)
        if (roomGetLinkDirection(worldGetCurrentRoom(), i) > -1)
        {
            rpage_video_draw_polygon((poly *)&compass_poly_per_direction[i], 30);
        }
            // rpage_video_draw_rect(&compass_button_per_direction[i], 30);

    for (i = 0; i < MAX_ZONE_PER_ROOM; i++)
    {
        if (rooms[worldGetCurrentRoom()].zone[i] > -1)
        {
            rpage_video_draw_polygon(&(zones[rooms[worldGetCurrentRoom()].zone[i]].zone_poly), 30);
            sprintf(str, "%d", rooms[worldGetCurrentRoom()].zone[i]);
            rpage_video_draw_text(str, zones[rooms[worldGetCurrentRoom()].zone[i]].zone_poly.p0.x + 4, zones[rooms[worldGetCurrentRoom()].zone[i]].zone_poly.p0.y - 1, 1);
            rpage_video_draw_text(str, zones[rooms[worldGetCurrentRoom()].zone[i]].zone_poly.p0.x + 3, zones[rooms[worldGetCurrentRoom()].zone[i]].zone_poly.p0.y - 2, 30);
        }
    }
#endif
}

BOOL game_flush_generic_sample(void)
{
    StopSound(LEFT0);
    StopSound(LEFT1);
    StopSound(RIGHT0);
    StopSound(RIGHT1);

    if (generic_sfx != NULL)
    {
        // RemoveSound(generic_sfx);
        SoundInfoFree(generic_sfx);
        generic_sfx = NULL;
        memset(generic_sfx_filename, 0, DISKNAME_LEN);
        return TRUE;
    }

    return FALSE;
}

void game_play_sample_ex(short param1, short param2)
{
    game_play_sample(param1);
}

/// Private
BOOL game_load_sample_from_filename(char *sample_name)
{
    if (generic_sfx == NULL || (generic_sfx_filename != NULL && strncmp(generic_sfx_filename, sample_name, DISKNAME_LEN)))
    {
        // printf("game_load_sample_from_filename(): generic_sfx_filename, sample_name = %s, %s\n", generic_sfx_filename, sample_name);
        if (generic_sfx != NULL)
            game_flush_generic_sample();
        // generic_sfx = LoadPackedSound(asset_build_device_path(sample_name, ".pak"), game_get_unpacking_buffer(), (UBYTE *)(game_get_unpacking_buffer() + MAX_SFX_SIZE + 16), game_get_sound_buffer());
        generic_sfx = LoadPackedSound(asset_build_device_path(sample_name, ".pak"), game_get_unpacking_buffer(), game_get_sound_buffer());
        strncpy(generic_sfx_filename, sample_name, DISKNAME_LEN);
        return TRUE;
    }

    return FALSE;
}

BOOL game_preload_sample_from_vue(short vue_index)
{
    short i;
    // Search for the sample linked to this vue
    for(i = 0; i < SAMPLE_VUES_LEN; i++)
    {
        if (vue_samples[i].vue_index == vue_index)
        {
            short sample_to_load = -1;
            // Get the sample index from the SafarScript standpoint ("game_play_sample(1);") 
            sample_to_load = vue_samples[i].sample_index;
            {
                short i;
                game_flush_sfx_manager();

                // Find the global index for this sample in current world/vue
                for(i = 0; i < SAMPLE_WORLD_LEN; i++)
                {
                    if (world_samples[i].sample_index == sample_to_load && world_samples[i].world_index == world_get_current_index())
                    {
                        // Get the filename of the sample based on the global samples index
                        game_load_sample_from_filename(sample_names[world_samples[i].name_index]);
                        return TRUE;
                    }
                }    
            }
        }
    }

    return FALSE;
}

void game_play_sample(short sample_script_index)
{
    short i;
    // This is the global index that will tell
    // the "sfx manager" what sample to play next
    // -1 means "no sample"
    game_flush_generic_sample();

    // Find the global index for this sample in current world/vue
    for(i = 0; i < SAMPLE_WORLD_LEN; i++)
    {
        if (world_samples[i].sample_index == sample_script_index && world_samples[i].world_index == world_get_current_index())
        {
            sfx_manager_next_sample = world_samples[i].name_index;
            // sfx_manager_update_count += 8; // make sure the next update will play the sample
            break;
        }
    }

    // printf("sfx_manager_next_sample = %d\n", sfx_manager_next_sample);  
}

void game_flush_sfx_manager(void)
{
// printf("game_flush_sfx_manager(%d)\n", sfx_manager_next_sample);
    sfx_manager_next_sample = -1;
}

void game_sfx_force_stop(void)
{
    game_flush_sfx_manager();
    sfx_play_clock = 0;
    sfx_manager_update_count += 0xF;
}

void game_update_sfx_manager(void)
{
    sfx_manager_update_count++;
    if (sfx_manager_update_count > 8)
    {
        short sample_idx = -1;
        // UBYTE song_pos;

        switch (sfx_manager_state)
        {
            case AUDIO_SFX_OFF:
                sample_idx = sfx_manager_next_sample;
                if (sample_idx >= 0)
                {
                    char *sample_name = sample_names[sample_idx];
                    // ULONG fade_clock;

                    // printf("game_update_sfx_manager(%d, %s) : AUDIO_SFX_OFF\n", sfx_manager_next_sample, sample_name);

                    // if no sound is loaded
                    // or the loaded sound doesn't match the one we want to play
                    game_load_sample_from_filename(sample_name);
                    // printf("sample_name = %s\n", sample_name);

                    if (generic_sfx)
                    { 
                        if (music_manager_state == AUDIO_MUSIC_PLAY || music_manager_state == AUDIO_MUSIC_FADEOUT)
                        {
                            protracker_enable_channel(3, FALSE);
                            RethinkDisplay();
                            rpage_video_vsync();
                        }

                        StopSound(LEFT1);
                        rpage_video_vsync();        
                        PlaySound(generic_sfx, MAXVOLUME, LEFT1, 0, ONCE);
                        sfx_play_clock = rpage_get_clock(); // game_get_frame_clock(); // 
                        if (generic_sfx != NULL && generic_sfx->RecordRate > 0)
                            sfx_play_duration = (((generic_sfx->FileLength) * 1000) / generic_sfx->RecordRate) + 250; // 250 ms of trail
                        else
                            sfx_play_duration = 0;
                                      
                        sfx_manager_state = AUDIO_SFX_PLAYING;
                    }
                }
                break;

            case AUDIO_SFX_PLAYING:
                if (rpage_get_clock() - sfx_play_clock > sfx_play_duration)
                {
                    StopSound(LEFT1);
                    // printf("game_update_sfx_manager(%d) : AUDIO_SFX_PLAYING\n", sfx_manager_next_sample);

                    if (music_manager_state == AUDIO_MUSIC_PLAY || music_manager_state == AUDIO_MUSIC_FADEOUT)
                    {
                        rpage_video_vsync();
                        RethinkDisplay();                    
                        protracker_enable_channel(3, TRUE);
                    }

                    game_flush_sfx_manager();

                    sfx_manager_state = AUDIO_SFX_OFF;
                }
                break;
        }

        sfx_manager_update_count = 0;
    }
}

void game_clear_dialog(void)
{
    gui_display_tiles_block(1, 19, 33, 25);
}

void game_clear_compass(void)
{
    gui_display_tiles_block(33, 18, 40, 25);
}
// extern rect compass_area;
void game_update_compass(void)
{
    short i;
    // rpage_video_draw_rect(&compass_area, 30);
    for (i = 0; i < 4; i++)
        if (roomGetLinkDirection(worldGetCurrentRoom(), i) > -1)
            gui_compass_refresh_direction(i);
}

void game_display_character(short character_sprite)
{
    if (character_sprite < MAX_SPR_PORTRAITS)
    {
        gui_display_character((char *)portrait_sprites[character_sprite]);
    }
}

void game_set_dialog_tag(char *_tag)
{
    if (_tag)
        strncpy(dialog_tag, _tag, DIALOG_TAG_SIZE);
    else
        memset(dialog_tag, 0, DIALOG_TAG_SIZE * sizeof(char));    
}

void game_display_dialog_ex(short dialog_index, short character_sprite)
{
    game_display_character(character_sprite);
    game_display_dialog(dialog_index);
}

void game_set_next_vue_dialog(short dialog_index)
{
    next_vue_dialog = dialog_index;
}

void game_display_dialog(short dialog_index)
{
    game_display_vue_dialog(roomGetVueIndex(worldGetCurrentRoom()), dialog_index);
}

void game_display_vue_dialog(short vue_index, short dialog_index)
{
    /* char room_number[16]; */
    char *str_description, str_line[DIALOG_SUBSTR_LEN];
    short len_description, i, j, k, start_str;

    if (dialog_index >= 0)
    {
        // short dialog_master_idx = dialog_per_room[roomGetVueIndex(game_get_current_loaded_room())][dialog_index];
        short dialog_master_idx = dialog_per_room[vue_index][dialog_index]; // suspicious design, we shall not display the text based on the NEXT room the player will visit.
        if (dialog_master_idx >= 0 && dialog_index < GAME_MAX_DIALOG_PER_ROOM)
        {          
            // look if this is a dynamic dialog? (incl. '%s')
            for(i = 0; i < GAME_MAX_DYNAMIC_DIALOG; i++)
                if (dynamic_dialogs[i] == dialog_master_idx)
                    break;
            
            if (i < GAME_MAX_DYNAMIC_DIALOG) // we found a dynamic dialog
            {
                // Inject a dynamic tag in it
                sprintf(str_dynamic, (const char*)dialogs[dialog_master_idx], dialog_tag);
                str_description = str_dynamic;
            }
            else
            {
                str_description = dialogs[dialog_master_idx];
            }

            len_description = (short)strlen(str_description);
        }
        else
        {
            str_description = "No dialog found!";
            len_description = (short)strlen(str_description);
        }
    }
    else
    {
        /* System message */
        str_description = game_get_system_dialog(dialog_index);
        len_description = (short)strlen(str_description);
        // printf("dialog_index, str_description = %d, %s\n", iabs(dialog_index), str_description);
    }

    // rpage_video_flip_buffers();
    // for(j = 0; j < 2; j++)
    // {
    //     rpage_video_flip_buffers();
{
        short line_count;
        short line_start[DIALOG_MAX_LINE], line_end[DIALOG_MAX_LINE];

        // printf("dialog_index, str_description = %d,\n%s\n", iabs(dialog_index), str_description);

        game_clear_dialog();

        line_count = str_count_delimiters(str_description);
        if (line_count > DIALOG_MAX_LINE)
        {
            rpage_system_alert("game_display_vue_dialog() Error!\nline_count exceeds DIALOG_MAX_LINE!");
            line_count = min(DIALOG_MAX_LINE, DIALOG_MAX_LINE);
        }
        // printf("lines: %d\n", line_count);

        start_str = 0;
        for(i = 0; i < line_count; i++)
        {
            line_start[i] = start_str;
            line_end[i] = str_find_delimiter(start_str, str_description);
            start_str = line_end[i] + 1;
        }

        i = 0;
        j = line_count - GUI_MAX_LINE;
        k = 0;

        while(TRUE)
        {
            for(i = 0; i < min(line_count, GUI_MAX_LINE); i++)
            {
                memset(str_line, 0, DIALOG_SUBSTR_LEN);
                strncpy(str_line, str_description + line_start[i + k], line_end[i + k] - line_start[i + k]);
                gui_draw_text_shadowed(str_line, DIALOG_X, DIALOG_Y + i * LINE_H, 30, 1);
            }

            if (j > 0)
            {
                k++;
                j--;

#ifndef _STRESS_TEST_
                rpage_mouse_read();
                do
                {
                    rpage_input_update();
                } while (!rpage_mouse_button_left_is_down());

                do
                {
                    rpage_input_update();
                } while (rpage_mouse_button_left_is_down());
                rpage_mouse_show();
#endif
                game_clear_dialog();                
            }
            else
                break;            
        }
}        
    // }
    // rpage_video_sync_buffers();

    // rpage_video_present_screen();
}

short game_get_dialog_sequence_index(void)
{
    return dialog_seq_index;
}

void game_set_dialog_sequence_index(short _idx)
{
    dialog_seq_index = _idx;
}

void game_display_dialog_sequence(short first_dialog_index, short last_dialog_index)
{
    short _idx;
    BOOL is_system_message;

    is_system_message = first_dialog_index < 0?TRUE:FALSE;

    first_dialog_index = iabs(first_dialog_index);
    last_dialog_index = iabs(last_dialog_index);

    _idx = game_get_dialog_sequence_index();
    _idx = min(_idx, last_dialog_index);
    _idx = max(_idx, first_dialog_index);
    game_display_dialog(_idx * (is_system_message?-1:1));
    _idx++;
    game_set_dialog_sequence_index(_idx);
}

void game_display_dialog_sequence_ex(short first_dialog_index, short last_dialog_index, short character_sprite)
{
    short _idx;
    _idx = game_get_dialog_sequence_index();
    _idx = min(_idx, last_dialog_index);
    _idx = max(_idx, first_dialog_index);
    game_display_dialog_ex(_idx, character_sprite);
    _idx++;
    game_set_dialog_sequence_index(_idx);
}

void game_vue_refresh(void)
{
    rpage_video_sync_buffers();
    rpage_video_flip_buffers();
    display_vue(rooms[game_get_current_loaded_room()].bitmap);
    game_draw_vue_sprites(rooms[game_get_current_loaded_room()].vue_index);
    game_clear_compass();
    game_update_compass();
#ifdef GAME_VISUAL_DEBUG                
    gui_draw_text_shadowed(rooms[game_get_current_loaded_room()].bitmap, 12, 5, 30, 1); // DEBUG ONLY!
#endif
    rpage_video_wait_dma();
    rpage_video_vsync();
    rpage_video_present_screen();
    rpage_video_sync_buffers();

    gui_actions_refresh(0, NULL);

    vue_is_dirty = FALSE;    
}

void game_hand_cursor_refresh(void)
{
    if (game_get_object_in_hand() > OBJECT_NONE && game_get_action() != act_use)
    {
        game_set_action(act_take_drop);
        gui_actions_mouse_refresh(act_take_drop);
    }
}

void game_update(game_internals *gi)
{
    BOOL action_return = FALSE;
// rpage_video_vsync();
#ifdef _STRESS_TEST_
    {
        short _vue, _rand, click_spr, click_zone;
        short played_minigame = 0;
        vec2 _mouse;

        _rand = fixed_rand()%100;

        if (game_get_special_callback() == NULL)
        {

            if (_rand < 90) // 9 chance on 10 to play in the current vue
            {
#ifdef _ST_SIMULATE_CLICKS_            
                if (!inventory_open)
                {
                    if (fixed_rand()%100 < 10) // 1 chance on 10 to open the inventory
                    {
                        gui_show_inventory(inventory, INVENTORY_SIZE);
                        inventory_open = TRUE;
                    }
                    else
                    {
                        //  otherwise simulate mouse interactions
                        _mouse.x = (fixed_rand()%(VUE_WIDTH - (VUE_X << 2))) + (VUE_X << 1);
                        _mouse.y = (fixed_rand()%(VUE_HEIGHT - (VUE_Y << 2))) + (VUE_Y << 1);

                        rpage_video_set_pixel(_mouse.x, _mouse.y + 1, 2);
                        rpage_video_set_pixel(_mouse.x + 1, _mouse.y + 1, 2);
                        rpage_video_set_pixel(_mouse.x - 1, _mouse.y + 1, 2);
                        rpage_video_set_pixel(_mouse.x, _mouse.y + 1 + 1, 2);
                        rpage_video_set_pixel(_mouse.x, _mouse.y - 1 + 1, 2);

                        rpage_video_set_pixel(_mouse.x, _mouse.y, 30);
                        rpage_video_set_pixel(_mouse.x + 1, _mouse.y, 30);
                        rpage_video_set_pixel(_mouse.x - 1, _mouse.y, 30);
                        rpage_video_set_pixel(_mouse.x, _mouse.y + 1, 30);
                        rpage_video_set_pixel(_mouse.x, _mouse.y - 1, 30);

                        // random action
                        game_set_action(fixed_rand()%4);

                        click_spr = control_get_clicked_sprite(&_mouse);
                        click_zone = control_get_clicked_zone(&_mouse);
                        if (click_spr >= 0)
                            rooms[worldGetCurrentRoom()].conditions(FALSE, -1, click_spr);
                        else if (click_zone >= 0)
                            zones[rooms[worldGetCurrentRoom()].zone[click_zone]].conditions(FALSE, zones[rooms[worldGetCurrentRoom()].zone[click_zone]].object, -1);
                    }
                }
                else
                {
                    // Close the inventory
                    gui_hide_inventory();
                    inventory_open = FALSE;
                }
#endif
            }
            else
            {
                // printf("STRESS TEST, vue change #%d (vue_%d), chip = %d/%d, fast = %d\n", stress_test_counter, roomGetVueIndex(current_room),
                //     rpage_get_avail_video_memory(), rpage_get_avail_largest_video_memory(), rpage_get_avail_non_video_memory());
                _vue = roomGetVueIndex(current_room);
//                 if (played_minigame == 0 && (_vue == 14 || _vue == 27 || _vue == 39 || _vue == 59))
//                 {
//                     // let's see if there's a minigame to play first
//                     switch(_vue)
//                     {
//                         case 14:
// #ifdef _ST_PLAY_PUZZLES_
//                             // printf("puzzle squid!\n");
//                             special_squid_set_random();
//                             special_squid_dialog_tag();
//                             game_set_special_callback(&special_squid_init);
// #endif
//                             played_minigame++;
//                             current_room++;              
//                             break;

//                         case 27:
// #ifdef _ST_PLAY_PUZZLES_
//                             // printf("puzzle fishing!\n");
//                             game_set_special_callback(&special_fishing_init);
// #endif
//                             played_minigame++;
//                             current_room++;
//                             break;

//                         case 39:
// #ifdef _ST_PLAY_PUZZLES_
//                             // printf("puzzle altos!\n");
//                             game_set_special_callback(&special_altos_puzzle_init);
// #endif
//                             played_minigame++;
//                             current_room++;
//                             break;

//                         case 59:
// #ifdef _ST_PLAY_PUZZLES_
//                             // printf("puzzle beetle race!\n");
//                             game_set_special_callback(&special_beetle_race_init);
// #endif
//                             played_minigame++;
//                             current_room++;
//                             break;
//                     }

                }
                else
                {   
                    // Ok, now we can jump to the next vue         
                    do // change vue
                    {
                        current_room++;
                    } while ((char *)rooms[current_room].conditions == NULL);
                    // if (fixed_rand()%100 < 50)
                    //     current_room = vue_get_room_index(35);
                    // else
                    //     current_room = vue_get_room_index(72);
                    
                    if (current_room >= MAX_ROOM) // vue_get_room_index(30)) //MAX_ROOM)
                        current_room = 0;

                    played_minigame = 0;

                    // world
                    _vue = roomGetVueIndex(current_room);
                    if (_vue <= 19)
                        world_set_current_index(world_rapanui);
                    else if (_vue <= 42)
                        world_set_current_index(world_cnossos);
                    else if (_vue <= 61)
                        world_set_current_index(world_indus);            
                    else if (_vue <= 74)
                        world_set_current_index(world_boat);

                    special_empty_players_hand();

                    //  printf("next vue: %d\n", _vue);

                    stress_test_counter++;

                    if (stress_test_counter > 200)
                        game_set_state(game_state_victory); // game_state_victory // game_state_quit
                }
            }
        }
    }
#endif

#ifndef DEBUG_MACROS
    if (assets_device_access_occurred())
        rpage_video_screen_to_front();
#endif

// #ifdef GAME_VISUAL_DEBUG
//     rpage_video_show_freemem(4, 8, 256 + 8, 8);
// #endif
    // update time (how many minutes, hours... did the player spent in the game)
    // this is only required by the name of a save game 
    update_game_clock_every = (update_game_clock_every + 1) & 0x3F; // 63
    if ((update_game_clock_every & 0xF) == 0)
        update_time_from_frame(&t_seconds, &t_minutes, &t_hours);

    // Changing world
    if (game_get_current_loaded_world() != world_get_current_index())
    {
        ULONG _load_clock;
        // Clear some of the memory
        game_flush_generic_sample();

        _load_clock = rpage_get_clock();

        // if (game_get_current_loaded_world() != -1 && world_get_current_index() != world_boat)
        // {
        //     travel_destination *destination_list;
        //     destination_list = game_get_travel_menu();
        //     gui_show_chapter_title(game_get_system_dialog(destination_list[world_get_current_index() - 1].dialog_idx));
        // }

        // Load world's data
        game_load_world_sprites(world_get_current_index());

        game_play_music_by_world_index(world_get_current_index());
 
// #ifndef _STRESS_TEST_
//         if (game_get_current_loaded_world() != -1 && world_get_current_index() != world_boat)
//         {
//             // Waits extra time if the file loading occured to fast
//             // for the player to read the world's name
//             while(rpage_get_clock() - _load_clock < 3000)
//             {
//                 short _i;
//                 for(_i = 0; _i < 5; _i++)
//                     rpage_video_vsync();
//             }
//         }
// #endif
        game_set_current_loaded_world(world_get_current_index());
        condition_master();
        game_reset_timers();
    }
    else
    {
        //
        // Vue loading
        //
        // SafarScript init for the next vue (if we are changing vue)
        if (game_get_current_loaded_room() != worldGetCurrentRoom())
        {
            game_sfx_force_stop();
            game_update_sfx_manager();            
            if (rooms[worldGetCurrentRoom()].conditions != NULL)
            {
                rooms[worldGetCurrentRoom()].conditions(TRUE, -1, -1); /* Calling the init sequence */
                sfx_manager_update_count += 0xF;
                game_update_sfx_manager();
            }
        }
        if (game_get_current_loaded_room() != worldGetCurrentRoom())
        {
            ULONG vue_loading_time;
#ifdef _STRESS_TEST_
            char vue_load_time_str[64];
#endif
            game_set_current_loaded_room(worldGetCurrentRoom());

            // Stop the ongoing audio (if any)
            // game_flush_sfx_manager();
            // game_flush_generic_sample(); // FIXME

            // Reset the dialog sequence index
            game_set_dialog_sequence_index(0);

            // // Reset the dialog customizer tag
            // game_set_dialog_tag(NULL);

            // Reset the current NPC portrait
            gui_reset_portrait_shown();

            if (game_get_current_loaded_room() >= MAX_ROOM)
                rpage_system_alert("game_get_current_loaded_room() >= MAX_ROOM !");

            // Loads vue bitmap, draw it, update the compass, update the sprites, synchronize double buffer
            vue_loading_time = rpage_get_clock();
            load_vue(rooms[game_get_current_loaded_room()].bitmap);

            // Preload a sample (if any)
            game_preload_sample_from_vue(roomGetVueIndex(worldGetCurrentRoom()));

            rpage_video_flip_buffers();
            game_clear_dialog();
            display_vue(rooms[game_get_current_loaded_room()].bitmap);

            // rpage_video_vsync();
            game_clear_compass();
            game_update_compass();
            gui_actions_refresh(0, NULL);
            game_draw_vue_sprites(rooms[game_get_current_loaded_room()].vue_index);

            vue_loading_time = rpage_get_clock() - vue_loading_time;
#ifdef _STRESS_TEST_
            {
                char vue_load_time_str[256];
                sprintf(vue_load_time_str, "LOADED IN %dms, %d iterations", (int)vue_loading_time, (int)stress_test_counter);
                gui_draw_text_shadowed(rooms[game_get_current_loaded_room()].bitmap, 12, 5, 30, 1); // DEBUG ONLY!
                gui_draw_text_shadowed(vue_load_time_str, 12, 5 + 8, 30, 1); // DEBUG ONLY!

                {
                    static rect _r;
                    _r.sx = VUE_X + VUE_WIDTH + 8;
                    _r.sy = 76;
                    _r.ex = SCREEN_WIDTH - 1;
                    _r.ey = _r.sy + 48;
                    rpage_video_fill_rect(&_r, 0);
                    sprintf(vue_load_time_str, "CHIP");
                    rpage_video_draw_text(vue_load_time_str, VUE_X + VUE_WIDTH + 9, 76, 25);
                    sprintf(vue_load_time_str, "%dKb", rpage_get_avail_video_memory() >> 10);
                    rpage_video_draw_text(vue_load_time_str, VUE_X + VUE_WIDTH + 9, 76 + 6, 25);
                    sprintf(vue_load_time_str, "%dKb", rpage_get_avail_largest_video_memory() >> 10);
                    rpage_video_draw_text(vue_load_time_str, VUE_X + VUE_WIDTH + 9, 76 + 12, 25);

                    sprintf(vue_load_time_str, "OTHER");
                    rpage_video_draw_text(vue_load_time_str, VUE_X + VUE_WIDTH + 9, 76 + 20, 30);
                    sprintf(vue_load_time_str, "%dKb", rpage_get_avail_non_video_memory() >> 10);
                    rpage_video_draw_text(vue_load_time_str, VUE_X + VUE_WIDTH + 9, 76 + 26, 30);
                }
            }
#endif
            rpage_video_wait_dma();
            rpage_video_vsync();
            // rpage_video_present_screen();
            // rpage_video_sync_buffers();
            rpage_video_wipe_rect_to_physical_screen((rect *)&vue_area);
            // rpage_video_flip_buffers();
            rpage_video_present_screen();
            rpage_video_sync_buffers();

            // game logic patch specific to Athanor 2!
            switch(roomGetVueIndex(worldGetCurrentRoom()))
            {
                case 73:
                    if (Flag022 == 1)
                        game_set_palette("pal_073");
                    break;

                case 74:
                    if (Flag023 == 1)
                        game_set_palette("pal_074");
                    break;
            }

            game_display_dialog(next_vue_dialog);

            next_vue_dialog = GAME_DEFAULT_VUE_DIALOG;
            vue_is_dirty = FALSE;
            inventory_open = FALSE;
        }
        else
        {
            // Timers update
            if (!gui_get_modal_flag())
            {
                update_timers();
                // rpage_video_set_immediate_RGB444(0, 0x080);
            }
            // else
            //     rpage_video_set_immediate_RGB444(0, 0x800);

            condition_timers();

            // Sound fx update
            // game_update_sfx_manager();

            //
            // Vue refresh
            //
            if (vue_is_dirty)
            {              
                game_vue_refresh();
                game_hand_cursor_refresh();
            }
            else
            {
                vec2 mouse_coords;
                short button = 0;
                /* Sound fx update */
                game_update_sfx_manager();

                /* Gui interaction */
                rpage_mouse_get_values(&button, &mouse_coords);

                gui_update_fx();

                if ((update_game_clock_every & 0xF) == 0)
                    game_hand_cursor_refresh();

                if (special_callback != NULL)
                {
                    PRINT_LARGEST_CHIP("special_callback");
                    (*special_callback)(&button, &mouse_coords);
                    PRINT_LARGEST_CHIP("post-special_callback");
                }
                else
                {                  
                    switch(game_get_action())
                    {
                        case  act_save:
                            start_game_special(GAME_SAVE);
                            game_set_action(-1);
                            rpage_mouse_show();
                            gui_actions_refresh(FALSE, &mouse_coords);
                            load_or_save_request = TRUE;
                            break;

                        case act_load:
                            start_game_special(GAME_LOAD);
                            game_set_action(-1);
                            rpage_mouse_show();
                            gui_actions_refresh(FALSE, &mouse_coords);
                            load_or_save_request = TRUE;
                            break;

                        case act_look:
                        case act_take_drop:
                        case act_use:
                            if (button)
                                gui_hide_character();
                            break;
                    }
               
                    // printf("%d,%d,%d\n", button, mouse_coords.x, mouse_coords.y);
                    if (inventory_open)
                    {
                        short inventory_slot, query_object;
                        inventory_slot = gui_inventory_get_mouse_slot_index(&mouse_coords);
                        query_object = inventory[inventory_slot].object_index;
                        if (query_object > OBJECT_NONE && gui_is_point_in_inventory(&mouse_coords))
                        {
                            char *object_name = tooltip_dialogs[go_to_inventory_tooltip[query_object]];
                            gui_update_inventory_tooltip(&mouse_coords, object_name);
                        }
                        else
                        {
                            gui_update_inventory_tooltip(&mouse_coords, NULL);
                        }
                    }
                    //
                    //Left Mouse Button Down
                    //
                    if (rpage_mouse_button_left_was_down())
                    {
                        if (gui_is_point_in_compass(&mouse_coords))
                        {
                            // You cannot click on the compass
                            // if an action is ongoing
                            // or if the inventory is open
                            if (!inventory_open)
                            {
                                short click_direction = control_get_compass_directions(&mouse_coords);

                                // Cancel current action
                                if (game_get_action() >= 0)
                                {
                                    special_empty_players_hand();
                                    game_set_action(-1);
                                    rpage_mouse_show();
                                }
                                
                                if (click_direction >= 0)
                                {
                                    //
                                    //The user clicked on the compass
                                    //
                                    short link;
                                    link = roomGetLinkDirection(worldGetCurrentRoom(), click_direction);
                                    if (link >= 0)
                                    {
                                        setPreviousVue(roomGetVueIndex(worldGetCurrentRoom()));
                                        worldSetCurrentRoom(link);
                                        game_set_current_loaded_room(-1);
                                    }
                                    else
                                        gui_fx_shake();
                                }
                            }
                            else
                                gui_fx_shake();

                        }
                        else if(!inventory_open && vue_is_point_in_rect(&mouse_coords))
                        {
                            short click_spr, click_zone;
                            //
                            // The user clicked on the main vue
                            // (might be a zone or a sprite...)
                            //
                            click_spr = control_get_clicked_sprite(&mouse_coords);
                            click_zone = control_get_clicked_zone(&mouse_coords);
                            if (click_spr >= 0)
                                action_return = rooms[worldGetCurrentRoom()].conditions(FALSE, -1, click_spr);
                            else if (click_zone >= 0)
                                action_return = zones[rooms[worldGetCurrentRoom()].zone[click_zone]].conditions(FALSE, zones[rooms[worldGetCurrentRoom()].zone[click_zone]].object, -1);

                            switch (game_get_action())
                            {
                                case act_take_drop:
                                    if (game_get_object_in_hand() == OBJECT_NONE)
                                    {
                                        if (!action_return)
                                            game_display_dialog_sequence(SYS28, SYS30);
                                        // game_set_action(-1);
                                        // rpage_mouse_show();
                                    }
                                    break;

                                case act_look:
                                        if (!action_return)
                                            game_display_dialog_sequence(SYS02, SYS05);
                                        // game_set_action(-1);
                                        // rpage_mouse_show();
                                    break;

                                case act_talk:
                                        if (!action_return)
                                        {
                                            game_set_action(-1);
                                            rpage_mouse_show();
                                        }
                                    break;                                    

                                case act_use:
                                    if (!action_return)
                                    {
                                        if (game_get_object_in_hand() == OBJECT_NONE)
                                        {
                                            game_set_action(-1);
                                            rpage_mouse_show();
                                        }
                                        else
                                        {
                                            game_set_action(-1);
                                            rpage_mouse_show();
                                            if (!game_is_object_in_inventory(game_get_object_in_hand()))
                                                game_set_object_to_inventory(game_get_object_in_hand(), -1);
                                            game_set_object_to_hand(OBJECT_NONE);
                                        }
                                                                            
                                    }
                                    else
                                    {
                                        if (game_get_object_in_hand() == OBJECT_NONE)
                                        {
                                            game_set_action(-1);
                                            rpage_mouse_show();
                                        }
                                        else
                                        {
                                            game_set_action(act_take_drop);
                                        }
                                        
                                    }
                                    break;
                            }
                            
                        }
                        else if(gui_is_point_in_inventory(&mouse_coords))
                        {
                            if (inventory_open)
                            {
                                short inventory_slot, query_object;
                                inventory_slot = gui_inventory_get_mouse_slot_index(&mouse_coords);
                                query_object = inventory[inventory_slot].object_index;

                                if (game_get_object_in_hand() > OBJECT_NONE)
                                {
                                    // if no object exists in this inventory slot
                                    if (query_object < OBJECT_NONE)
                                    {
                                        game_set_object_to_inventory(game_get_object_in_hand(), inventory_slot);
                                        game_set_object_to_hand(OBJECT_NONE);
                                        gui_refresh_inventory(inventory, INVENTORY_SIZE);
                                        game_set_action(-1);
                                        rpage_mouse_show();
                                    }
                                    else
                                    {
                                        // if an object already exists in this inventory slot, we try to combine them
                                        short new_object;
                                        new_object = game_combine_objects(game_get_object_in_hand(), query_object);
                                        if (new_object > OBJECT_NONE)
                                        {
                                            inventory[inventory_slot].object_index = new_object;
                                            game_set_object_to_hand(OBJECT_NONE);
                                            gui_refresh_inventory(inventory, INVENTORY_SIZE);
                                            game_set_action(-1);
                                            rpage_mouse_show();                                            
                                        }
                                        else
                                        {
                                            gui_fx_shake();
                                        }
                                    }                
                                }
                                else if (game_get_object_in_hand() == OBJECT_NONE)
                                {   
                                    if (query_object > OBJECT_NONE)
                                    {
                                        if (game_get_action() < 0 || game_get_action() == act_take_drop)
                                        {
                                            game_set_action(act_take_drop);
                                            gui_actions_mouse_refresh(act_take_drop);
                                            game_remove_object_from_inventory(query_object);
                                            game_set_object_to_hand(query_object);
                                            gui_refresh_inventory(inventory, INVENTORY_SIZE);
                                        }
                                    }
                                }
                            }
                        }
                        else if (gui_is_point_in_blank_area(&mouse_coords))
                        {
                            special_empty_players_hand();
                            game_set_action(-1);
                            rpage_mouse_show();
                        }

                    }
                    else if (rpage_mouse_button_right_was_down())
                    {
                        //
                        // Right Mouse Button Down
                        //
                        if (!inventory_open)
                        {
                            // Show inventory
                            switch(game_get_action())
                            {
                                case act_talk:
                                case act_look:
                                    game_set_action(-1);
                                    rpage_mouse_show();
                                    break;
                            }
                            gui_hide_character();
                            gui_show_inventory(inventory, INVENTORY_SIZE);
                        }
                        else
                        {
                            // Hide inventory
                            gui_hide_inventory();
                        }

                        inventory_open ^= 1;
                    }

                    gui_draw_object_in_hand(object_in_hand, &mouse_coords);
                    if (!inventory_open)
                        gui_actions_refresh(button, &mouse_coords);
                }
             
            }
        }
    }

    {
        void *special_func;
        special_func = game_get_special_callback();
        // Tries to anticipate what media will be required on the next update
        // (specific to the floppy version)
        if (game_get_current_loaded_room() != worldGetCurrentRoom())
        {
            // printf("%s, %s\n", rooms[worldGetCurrentRoom()].bitmap, asset_get_disk_name(rooms[worldGetCurrentRoom()].bitmap, ".pak"));
            strncpy(gi->next_disk, asset_get_disk_name(rooms[worldGetCurrentRoom()].bitmap, ".pak"), DISKNAME_LEN);
            gi->disk_change = TRUE;
        }
        // else if (special_func == &special_fishing_init) // special case for the fishing game
        // {
        //     strncpy(gi->next_disk, asset_get_disk_name("vue_27_game", ".pak"), DISKNAME_LEN);
        //     gi->disk_change = TRUE;
        // }
        // else if (special_func == &special_altos_puzzle_init) // special case for the altos puzzle
        // {
        //     strncpy(gi->next_disk, asset_get_disk_name("game_altos_board", ".pak"), DISKNAME_LEN);
        //     gi->disk_change = TRUE;
        // }
        // else if (special_func == &special_beetle_race_init) // special case for the altos puzzle
        // {
        //     strncpy(gi->next_disk, asset_get_disk_name("beetle_race_sprites", ".pak"), DISKNAME_LEN);
        //     gi->disk_change = TRUE;
        // }
        // else if (special_func == &special_squid_init) // special case for the altos puzzle
        // {
        //     strncpy(gi->next_disk, asset_get_disk_name("game_squid_sprites", ".pak"), DISKNAME_LEN);
        //     gi->disk_change = TRUE;
        // }
        else if (load_or_save_request) // the save/load of a game might require a disk change as well
        {
            strncpy(gi->next_disk, SAVE_DISK_NAME, DISKNAME_LEN);
            gi->disk_change = TRUE;
            load_or_save_request = FALSE;
        }
        else
        {
            gi->disk_change = FALSE;
        }
    }
}

//
// Inventory
//
void game_reset_inventory(void)
{
    short i;
    for (i = 0; i < INVENTORY_SIZE; i++)
    {
        inventory[i].object_index = -1;
        // inventory[i].position.x = 0;
        // inventory[i].position.y = 0;
    }
}

BOOL game_is_object_in_hand(short object_index)
{
    if (object_in_hand == object_index)
        return TRUE;
    // return game_is_object_in_inventory(object_index);
    return FALSE;
}

void game_set_object_to_hand(short object_index)
{
    // game_set_object_to_inventory(object_index, -1);
    object_in_hand = object_index;
    // if (object_index != OBJECT_NONE)
    //     game_set_action(act_take_drop);
}

void game_remove_object_from_inventory(short object_index)
{
    short i;
    for (i = 0; i < INVENTORY_SIZE; i++)
        if (inventory[i].object_index == object_index)
        {
            inventory[i].object_index = -1;
            return;
        }
}

short game_get_object_in_hand(void)
{
    return object_in_hand;
}

BOOL game_is_object_in_inventory(short object_index)
{
    short i;
    for (i = 0; i < INVENTORY_SIZE; i++)
        if (inventory[i].object_index == object_index)
            return TRUE;

    return FALSE;
}

BOOL game_set_object_auto_inventory(short object_index)
{
    return game_set_object_to_inventory(object_index, -1);
}

BOOL game_set_object_to_inventory(short object_index, short inventory_slot_index)
{
    // nothing to drop in the inventory, we pass
    if (object_index == OBJECT_NONE)
        return FALSE;

    // an object cannot be added twice to the inventory
    if (game_is_object_in_inventory(object_index))
        return FALSE;

    // the object has no sprite
    if (gameObjectGetSpriteInventoryIndex(object_index) == -1)
        return FALSE;

    // if inventory_slot_index < 0, we automatically find the first free index
    if (inventory_slot_index < 0)
    {
        short i;
        for (i = 0; i < INVENTORY_SIZE; i++)
            if (inventory[i].object_index == -1)
                break;

        if (i < INVENTORY_SIZE)
            inventory_slot_index = i;
        else
            return FALSE;
    }

    inventory[inventory_slot_index].object_index = object_index;
    return TRUE;
}

short game_combine_objects(short go_a, short go_b)
{
    short i;
    if (go_a > go_b)
    {
        short tmp = go_a;
        go_a = go_b;
        go_b = tmp;
    }

    for(i = 0; i < MAX_COMBINED_OBJECTS; i++)
    {
        if (combined_go[i].in[0] == go_a)
            if (combined_go[i].in[1] == go_b)
                return combined_go[i].out;
    }

    return OBJECT_NONE;
}

void game_print_inventory(void)
{
#ifdef DEBUG_MACROS
    short i;
    printf("Inventory: ");
    for (i = 0; i < INVENTORY_SIZE; i++)
        if (inventory[i].object_index > -1)
            printf("%s, ", world_get_object_name(inventory[i].object_index));
    printf("\n");
#endif
}

///
void game_load_world_sprites(short world_index)
{
    // printf("game_load_world_sprites(%d,%s)\n", world_index, (char *)world_sprites_name[world_index]);
    if (world_index >= 0 && world_index < MAX_WORLD)
        load_sprites_sheet((char *)world_sprites_name[world_index]);
    else
    {
        rpage_system_alert("Illegal world_index!");
    }
}

/// Iterate on the whole sprite sheet (all worlds included)
/// and look for the sprites that are related to the current vue.<br>
/// In order to draw the sprite, this function needs the "game_object" enum of the sprite. See ::game_object
void game_draw_vue_sprites(short vue_index)
{
    short i;
    for (i = 0; i < worldGetMaxSheetSprites(); i++)
    {
        if (getSpriteSheetVueIndex(i) == vue_index && getSpriteSheetEnabled(i))
        {
            draw_sprite_on_vue(getSpriteSheetGameObjectIndex(i));
        }
    }
}

/// Set the flag "enabled" of sheet sprite to FALSE, based on his ::game_object reference.<br>
/// The sprite will be cleared from screen during the next update.
void game_hide_sprite(short sprite_index)
{
    short i;
    for (i = 0; i < worldGetMaxSheetSprites(); i++)
    {
        if (getSpriteSheetGameObjectIndex(i) == sprite_index)
        {
            setSpriteSheetEnabled(i, FALSE);
            game_vue_make_dirty();
            break;
        }
    }
}

/// Set the flag "enabled" of sheet sprite to TRUE, based on his ::game_object reference.
/// The sprite will be drawn during the next update.
void game_show_sprite(short sprite_index)
{
    short i;
    for (i = 0; i < worldGetMaxSheetSprites(); i++)
    {
        if (getSpriteSheetGameObjectIndex(i) == sprite_index)
        {
            setSpriteSheetEnabled(i, TRUE);
            game_vue_make_dirty();
            break;
        }
    }
}

void game_fadeto_palette(char *pal_name)
{
    rpage_palette palette[1 << SCREEN_DEPTH];
    load_pak_to_palette(asset_build_device_path(pal_name, ".pak"), palette);

    gui_fade_to_custom_palette(32, rpage_video_get_front_palette(), palette);

    rpage_video_set_palette(palette, 1 << SCREEN_DEPTH);
    rpage_video_present_palette();

    // make sure the vue keeps tracks of this new palette
    vue_set_palette(rooms[game_get_current_loaded_room()].bitmap, palette);
}

void game_set_palette(char *pal_name)
{
    rpage_palette palette[1 << SCREEN_DEPTH];
    load_pak_to_palette(asset_build_device_path(pal_name, ".pak"), palette);
    rpage_video_set_palette(palette, 1 << SCREEN_DEPTH);
    rpage_video_present_palette();

    // make sure the vue keeps tracks of this new palette
    vue_set_palette(rooms[game_get_current_loaded_room()].bitmap, palette);
}

BOOL game_is_current_action(short action_index)
{
    return (BOOL)(game_get_action() == action_index);
}

void game_abort_leaving_room(short enable)
{
    if (enable)
    {
        printf("vue %d : AbordLeavingRoom!",  roomGetVueIndex(game_get_current_loaded_room()));
        worldSetCurrentRoom(vue_get_room_index(getPreviousVue()));
        // worldSetCurrentRoom(game_get_current_loaded_room());
    }
}

// FIXME : this is a blocking call
void game_wait_ticks(unsigned int ticks)
{
    unsigned int i;
    rpage_mouse_wait();
    for(i = 0; i < ticks; i++)
        rpage_video_vsync();
    rpage_mouse_show();
}

void game_fade_out(BOOL enable)
{
    // TODO
}

void game_fade_in(BOOL enable)
{
    // TODO
}

void game_over(BOOL enable)
{
#ifndef _STRESS_TEST_
    if (enable)
    {
            rpage_mouse_wait();
            start_game_special(GAME_OVER);
    }
#endif
}

void game_enable_timer1(BOOL enable)
{
    timer1_enabled = enable;
    if (!enable)
        Timer1 = 0;
}

void game_enable_timer2(BOOL enable)
{
    timer2_enabled = enable;
    if (!enable)
        Timer2 = 0;
}

BOOL game_load_slot_preview(short slot, short *world_index, USHORT *hours, USHORT *minutes, USHORT *seconds, BOOL is_on_hdd)
{
	rpage_file file;
	char file_header[4];
	char filename[16];
    USHORT *_tmp;

    if (is_on_hdd)
    	sprintf(filename, "save.%03d", slot);
    else
    	sprintf(filename, "%ssave.%03d", SAVE_DISK_NAME, slot);

	file = rpage_file_open(filename, MODE_OPEN_FILE);
    if (!file)
        return FALSE;

    // GENUINE SAVE FILE
	rpage_file_read(file, &file_header, 4);
    if (strncmp(file_header, SAVE_HEADER, 4) != 0)
    {
        rpage_file_close(file);
        return FALSE;
    }

    // TIME
    rpage_file_read(file, &file_header, 4);
    if (strncmp(file_header, SAVE_HEADER_TIME, 4) != 0)
    {
        rpage_file_close(file);
        return FALSE;
    }

    rpage_file_read(file, seconds, 2);
    rpage_file_read(file, minutes, 2);
    rpage_file_read(file, hours, 2);
// printf("(h,m) = %d, %d\n", *hours, *minutes);

    // WORLD/VUE
    rpage_file_read(file, &file_header, 4);
    if (strncmp(file_header, SAVE_HEADER_VUE, 4) != 0)
    {
        rpage_file_close(file);
        return FALSE;
    }

    //  previous vue
    rpage_file_read(file, &_tmp, 2);
    //  current room
    rpage_file_read(file, &_tmp, 2);
    // current world
    rpage_file_read(file, world_index, 2);

    rpage_file_close(file);
    return TRUE;
}

void gameLoadWorldState(rpage_file file)
{
    char file_header[4];
    short s;
    unsigned int ui;

    //  Header
    rpage_file_read(file, &file_header, 4);
    if (strncmp(file_header, SAVE_HEADER_VUE, 4) != 0)
        return;

    //  previous vue
    rpage_file_read(file, &s, 2);
    setPreviousVue(s);

    //  current room
    rpage_file_read(file, &s, 2);
    worldSetCurrentRoom(s);

    // current world
    rpage_file_read(file, &s, 2);
    world_set_current_index(s);

    // timers
    rpage_file_read(file, &s, 2);
    timer1_enabled = s?TRUE:FALSE;
    rpage_file_read(file, &ui, 4);
    Timer1 = ui;
    
    rpage_file_read(file, &s, 2);
    timer2_enabled = s?TRUE:FALSE;
    rpage_file_read(file, &ui, 4);
    Timer2 = ui;
}

void gameSaveWorldState(rpage_file file)
{
	char *file_header = SAVE_HEADER_VUE;
    short s;
    unsigned int ui;
    short b;

    //  Header
	rpage_file_write(file, file_header, 4);

    //  previous vue
    s = getPreviousVue();
	rpage_file_write(file, &s, 2);

    //  current room
    s = worldGetCurrentRoom();
    rpage_file_write(file, &s, 2);

    // current world
    s = world_get_current_index();
	rpage_file_write(file, &s, 2);

    // timers
    b = (short)timer1_enabled;
	rpage_file_write(file, &b, 2);
    ui = Timer1;
	rpage_file_write(file, &ui, 4);

    b = (short)timer2_enabled;
	rpage_file_write(file, &b, 2);
    ui = Timer2;
	rpage_file_write(file, &ui, 4);    

}

void gameLoadInventoryState(rpage_file file)
{
    char file_header[4];
    short i, idx, size;
    unsigned short value;

    //  Header
    rpage_file_read(file, &file_header, 4);
    if (strncmp(file_header, SAVE_HEADER_INVENTORY, 4) != 0)
        return;

    // clear inventory
    game_reset_inventory();

    rpage_file_read(file, &size, 2);
    for (i = 0; i < size; i++)
    {
        rpage_file_read(file, &idx, 2);
        rpage_file_read(file, &value, 2);
        inventory[idx].object_index = worldGetGameObjectFromCRC16(value);
    }
}

/// Save sparse inventory.
void gameSaveInventoryState(rpage_file file)
{
    char *file_header = SAVE_HEADER_INVENTORY;
    short i, s;
    unsigned short us;

    rpage_file_write(file, file_header, 4);

    // Inventory size (short)
    for (i = 0, s = 0; i < INVENTORY_SIZE; i++)
        if (inventory[i].object_index > OBJECT_NONE)
            s++;
    rpage_file_write(file, &s, 2);

    for (i = 0; i < INVENTORY_SIZE; i++)
    {
        s = inventory[i].object_index;
        if (s > OBJECT_NONE)
        {
            // If there is an object in the slot
            // save it's position in the inventory
            // and the CRC16 of the object name.
            us = game_object_crc16[s];
            rpage_file_write(file, &i, 2);
            rpage_file_write(file, &us, 2);
        }
    }
}

void gameLoadRoomLinks(rpage_file file)
{
    char file_header[4];
    short i, j;
    short l, max_room, max_link_per_room;

    //  Header
    rpage_file_read(file, &file_header, 4);
    if (strncmp(file_header, SAVE_HEADER_LINKS, 4) != 0)
        return;

    rpage_file_read(file, &max_room, 2);
    rpage_file_read(file, &max_link_per_room, 2);

    for(i = 0; i < max_room; i++)
    {
        short room_index, vue_index;
        rpage_file_read(file, &vue_index, 2);
        room_index = vue_get_room_index(vue_index);

        for(j = 0; j < max_link_per_room; j++)
        {
            rpage_file_read(file, &l, 2);
            rooms[room_index].links[j] = l;
        }
    }
}

void gameSaveRoomLinks(rpage_file file)
{
    char *file_header = SAVE_HEADER_LINKS;
    short i, j;
    short s,l; // s = room index, l = link

    rpage_file_write(file, file_header, 4);

    // write how many links per room
    // this is a constant defined at compile time
    // but this might change in the future
    // and we want the save file to remain compatible
    // with future versions of the game :)
    s = MAX_ROOM;
    rpage_file_write(file, &s, 2);

    s = MAX_LINK_PER_ROOM;
    rpage_file_write(file, &s, 2);

    for(i = 0; i < MAX_ROOM; i++)
    {
        s = rooms[i].vue_index;
        rpage_file_write(file, &s, 2);
        for(j = 0; j < MAX_LINK_PER_ROOM; j++)
        {
            l = rooms[i].links[j];
             rpage_file_write(file, &l, 2);
        }
    }
}

void gameLoadPuzzles(rpage_file file)
{
    // char file_header[4];
    // short s;

    // //  Header
    // rpage_file_read(file, &file_header, 4);
    // if (strncmp(file_header, SAVE_HEADER_PUZZLES, 4) != 0)
    //     return;

    // rpage_file_read(file, &s, 2);
    // special_squid_set_icon_solution(s);
    // rpage_file_read(file, &s, 2);
    // special_squid_set_digit_solution(s);
}

void gameSavePuzzles(rpage_file file)
{
    // char *file_header = SAVE_HEADER_PUZZLES;
    // short s;

    // rpage_file_write(file, file_header, 4);

    // s = special_squid_get_icon_solution();
    // rpage_file_write(file, &s, 2);

    // s = special_squid_get_digit_solution();
    // rpage_file_write(file, &s, 2);
}

void game_load_time(rpage_file file)
{
    char file_header[4];
    short i;

    //  Header
    rpage_file_read(file, &file_header, 4);
    if (strncmp(file_header, SAVE_HEADER_TIME, 4) != 0)
        return;

    // rpage_file_read(file, &t_frames, 2);
    rpage_file_read(file, &t_seconds, 2);
    rpage_file_read(file, &t_minutes, 2);
    rpage_file_read(file, &t_hours, 2);

    for(i = 0; i < 30; i++)
        update_time_from_frame(&t_seconds, &t_minutes, &t_hours);

    t_seconds = t_seconds % 60;
    t_minutes = t_minutes % 60;

    // printf("(h,m,s)=(%d:%d:%d)\n", t_hours, t_minutes, t_seconds);
}

void game_save_time(rpage_file file)
{
    char *file_header = SAVE_HEADER_TIME;
    short i;

    t_seconds = t_seconds % 60;
    t_minutes = t_minutes % 60;

    rpage_file_write(file, file_header, 4);

    // rpage_file_write(file, &t_frames, 2);
    rpage_file_write(file, &t_seconds, 2);
    rpage_file_write(file, &t_minutes, 2);
    rpage_file_write(file, &t_hours, 2);

    for(i = 0; i < 30; i++)
        update_time_from_frame(&t_seconds, &t_minutes, &t_hours);
}

char *game_get_system_dialog(short sys_index)
{
    return system_dialogs[iabs(sys_index)];
}

void game_load_music(UWORD music_index)
{
    // TODO
    // printf("game_load_music(%d);\n", music_index);
}

void game_play_music(BOOL flag)
{
    // TODO
    if (flag)
    {
        // Play music
    }
    else
    {
        // Stop music
    }
    
}

void game_stop_music(BOOL flag)
{
    // TODO
    if (flag)
    {
        // Stop music
    }
}

void game_update_music_manager(void)
{
    static ULONG music_length[5] = {0, 4 * 60 * 1000, 3 * 60 * 1000, 2 * 60 * 1000, 2 * 60 * 1000};

    music_manager_update_count++;
    if (music_manager_update_count > 30 && !game_get_special_callback())
    {
        switch (music_manager_state)
        {
            case AUDIO_MUSIC_OFF:
            case AUDIO_MUSIC_PLAY:
                // rpage_video_set_immediate_RGB444(0, 0x080);
                if (music_world_index > -1)
                {
                    if (rpage_get_clock() - music_play_clock > music_length[music_world_index])
                    {
                        protracker_set_fade_speed(-1);
                        music_manager_state = AUDIO_MUSIC_FADEOUT;
                    }
                }
                break;
            
            case AUDIO_MUSIC_FADEOUT:
                // if fade out is done
                // stop the music player
                // rpage_video_set_immediate_RGB444(0, 0x040);
                if (protracker_get_volume() < 1)
                {
                    music_play_clock = rpage_get_clock(); // rpage_get_clock();
                    music_manager_state = AUDIO_MUSIC_MUTE;
                }
                break;

            case AUDIO_MUSIC_MUTE:
                // play ambient sfx for a random period of time
                // until a certain amount of time is elapsed
                // rpage_video_set_immediate_RGB444(0, 0x400);
                if (game_get_frame_clock() - music_play_clock > 2 * 60 * 1000)
                {
                    // ...and until we are sure that a sfx is not playing
                    if (sfx_manager_state == AUDIO_SFX_OFF)
                    {
                        StopSound(LEFT0);
                        StopSound(RIGHT0);
                        StopSound(RIGHT1);
                        StopSound(LEFT1);
                        music_play_clock = rpage_get_clock(); // rpage_get_clock();
                        protracker_set_fade_speed(0);
                        protracker_play();
                        music_manager_state = AUDIO_MUSIC_PLAY;
                    }
                }
                break;
        }

        music_manager_update_count = 0;
    }
}

void game_play_music_by_world_index(short _music_index)
{
    // if (_music_index != music_world_index)
    // {
    //     music_world_index = _music_index;
    //     unload_protracker_music();
    //     protracker_set_fade_speed(0);

    //     switch(music_world_index)
    //     {
    //         case world_cnossos:
    //             load_packed_protracker_music(asset_build_device_path("mus_cnossos", ".pak"), game_get_unpacking_buffer(), &music_memory_buffer);
    //             break;
            
    //         case world_indus:         
    //             load_packed_protracker_music(asset_build_device_path("mus_indus", ".pak"), game_get_unpacking_buffer(), &music_memory_buffer);             
    //             break;
            
    //         case world_rapanui:
    //             load_packed_protracker_music(asset_build_device_path("mus_rapanui", ".pak"), game_get_unpacking_buffer(), &music_memory_buffer);
    //             break;

    //         case world_boat:
    //             load_packed_protracker_music(asset_build_device_path("mus_boat", ".pak"), game_get_unpacking_buffer(), &music_memory_buffer);
    //             break;
    //     }

    // }

    // protracker_set_fade_speed(0);
    // protracker_setup_mod();
    // protracker_play();
    // music_play_clock = rpage_get_clock();
    // music_manager_update_count = 0;
    // music_manager_state = AUDIO_MUSIC_PLAY;
}