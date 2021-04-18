/*
	Windless Bay, Mount Erebus. 
	A research station deep into Antarctica, this is where your journey begins...
	
	Original game by Francois "Astrofra" Gutherz. 
	Amiga version (C) RESISTANCE 2021 

	The source code of this game is based on:
	* R-PAGE (the Resistance's Portable-Adventure-Game-Engine, C language)
	* AmigaOS for all the access to the Amiga hardware
*/

#include <stdio.h>
#include "rpage/frwk.h"
#include "rpage/utils.h"
#include "rpage/aos/debug.h"
#include "rpage/aos/io.h"
#include "rpage/aos/ptracker.h"
#include "rpage/aos/locale.h"
#include "game/text.h"
#include "game/vue.h"
#include "game/world.h"
#include "game/game.h"
#include "game/aos/assets.h"
#include "game/special.h"
#include "game/gui.h"
#include "rpage/aos/sound.h"

#include <workbench/workbench.h>
#include <proto/icon.h>

int g_loop; /* Common interation counter */
gui_dialog_descriptor disk_change_gui_desc;
short locale_index = -1; // locale_fr;

void assets_manage_disk_request(UBYTE *disk_request, BOOL *disk_change, ULONG *disk_change_timer, char *next_disk);

extern BYTE *music_memory_buffer;

game_internals gi;
ULONG disk_change_timer;
UBYTE disk_request;

void specific_disk_request(char *filename)
{
	disk_request = DISK_REQ_OK;

	// Specific disk swapping
	if (!device_is_hdd())
	{
		// We want to make sure the second disk is available,
		// anw we know that the vue 25 is located on the second disk
		strncpy(gi.next_disk, asset_get_disk_name(filename, ".pak"), DISKNAME_LEN);
		gi.disk_change = TRUE;

		while (gi.disk_change)
		{
			rpage_video_vsync();
			if (gi.disk_change && !device_is_hdd())
				assets_manage_disk_request(&disk_request, &gi.disk_change, &disk_change_timer, gi.next_disk);				
			rpage_input_update();
		}

		disk_request = DISK_REQ_OK;
	}
}

int main(int argc, char *argv[])
{
	ULONG reserved_chipmem, reserved_othermem;
	SoundInfo *generic_sfx;

	init_asset_dispatch(ASSETS_HDD_MODE);

	if (argc == 0)
	{
		// from Workbench
		extern struct DosLibrary *DOSBase;
		struct Library *IconBase = NULL;
		struct DiskObject *disk_obj;
		char **toolarray;
		char *tool_type;
		char *exe_names[2] = {"main.exe", "WindlessBay"};
		
		// Open the icon library
		// to be able to read icon's tool types
		IconBase = OpenLibrary("icon.library", 33);

		// printf("IconBase(%X)\n", IconBase);

		for (g_loop = 0; g_loop < 2; g_loop++)
		{
			disk_obj = GetDiskObject(exe_names[g_loop]);

			if (disk_obj)
			{
				// printf("disk_obj = %X\n", disk_obj);

				toolarray = (char **)disk_obj->do_ToolTypes;

				tool_type = (char *)FindToolType(toolarray, "HDD");

				if (tool_type)
				{
					short _i;
					for(_i = 0; _i < strlen(tool_type); _i++)
						tool_type[_i] |= 0x20;
					// printf("%s\n", tool_type);
					if ((strncmp(tool_type, "false", 5) == 0) || (strncmp(tool_type, "0", 1) == 0))
						init_asset_dispatch(ASSETS_FLOPPY_MODE);
				}

				// if (tool_type)
				// 	printf("tool_type = %s\n", tool_type);
				// else
				// 	printf("no tool_type !\n");

				FreeDiskObject(disk_obj);
			}
			// else
			// 	printf("no disk object for %s!\n", exe_names[g_loop]);
		}
		// get the icon of the game binary

		CloseLibrary(IconBase);
	}
	else
	{
		// from CLI
		for(g_loop = 1; g_loop < argc; g_loop++)
		{
			if (!strncmp(argv[g_loop], "/FLOPPY", 8))
				init_asset_dispatch(ASSETS_FLOPPY_MODE);
			
			if (!strncmp(argv[g_loop], "/FR", 4))
				locale_index = locale_fr;

			if (!strncmp(argv[g_loop], "/EN", 4))
				locale_index = locale_en;
		}
	}

	rpage_init();
	SoundInit();

	if (!device_is_hdd())
		rpage_reclaim_system_memory();

	// Larger buffer where any packed image will be loaded
	game_set_unpacking_buffer(rpage_os_alloc(MAX_PAK_SIZE * sizeof(BYTE), MEMF_ANY));

	rpage_video_open(mode_lowres);
	rpage_video_set_font("mini6.font", 6);
	rpage_input_init();
	rpage_video_flip_buffers();
	rpage_video_clear();
	rpage_video_set_palette_to_grey(0, 31);
	rpage_video_present_screen();
#ifdef _STRESS_TEST_
	locale_index = 0;
#endif

	if (locale_index < 0)
		locale_index = game_locale_selector();

	rpage_mouse_wait();

	game_init(&gi);
	game_load_locale(locale_index);

	rpage_mouse_wait();
	rpage_video_draw_text(game_get_system_dialog(SYS18), -1, -1, 15);

	// Audio
	init_protracker_player();
	load_packed_protracker_music(asset_build_device_path("mus_intro", ".pak"), game_get_unpacking_buffer(), &music_memory_buffer);
#ifndef _STRESS_TEST_
	game_intro_screen();
#endif
	rpage_video_flip_buffers();
	rpage_video_clear();
	rpage_mouse_wait();
	rpage_video_draw_text(game_get_system_dialog(SYS18), -1, -1, 15);
	rpage_video_set_palette_to_grey(0, 31);
	rpage_video_present_screen();

	gui_load();

	unload_protracker_music();

	reserved_chipmem = 0;
	reserved_chipmem += rpage_bitmap_calculate_bytesize(48, 48, VUE_DEPTH); // npc_XXX_XX.png		
	reserved_chipmem += rpage_bitmap_calculate_bytesize(320, 200, VUE_DEPTH + 1); // spr_XXXX.png (Cnossos...)		
	reserved_chipmem += rpage_bitmap_calculate_bytesize(320, 24, VUE_DEPTH); // spr_inventory.png
	reserved_chipmem += 48000; // misc buffer

	reserved_othermem = 128 << 10; // The game needs that amount of non-video ram to run properly!

	if (init_vue_cache(MAX_VUE_CACHE_SIZE, reserved_chipmem, reserved_othermem, game_get_unpacking_buffer()))
	{
#ifdef LATTICE
		specific_disk_request("vue_25");
#endif

		game_load_world_sprites(world_get_current_index()); // sprite preload (for the Amiga 500)

		// rpage_video_set_palette_to_black(0, COLORS);
		rpage_video_flip_buffers();
		// rpage_video_set_palette_to_black(0, COLORS);
		rpage_video_clear();

		gui_display();

		rpage_video_present_screen();
		rpage_video_sync_buffers();
		rpage_mouse_button_flush();

		rpage_mouse_wait();

		// if the game is not able to allocate at least one video buffer
		// then it's a major failure, we quit. Otherwise, the game starts.
		// protracker_set_fade_speed(-1);
		// unload_protracker_music();
		while (game_get_state() == game_state_play)
		{
			game_update_frame_clock();

			// AllocMem(~0,MEMF_ANY);

			switch(rpage_keyboard_rawkey())
			{
				case 0x45: // ESC
					if (device_is_hdd())
						game_set_state(game_state_quit);
					break;
#if 0
				case 0x50: // F1
					game_set_state(game_state_victory);
					// special_empty_players_hand();
					// game_set_special_callback(&special_sand_puzzle_init);
					break;
				case 0x51: // F2
					if (game_get_special_callback() == NULL)
						special_squid_set_random();
						special_squid_dialog_tag();
						// game_set_special_callback(&special_altos_puzzle_init);
					break;
				case 0x52: // F3
					rpage_video_screen_to_back();
					break;
				case 0x53: // F4
					if (game_get_special_callback() == NULL)
						game_set_special_callback(&special_beetle_race_init);
					break;
				case 0x54: // F5
					if (game_get_special_callback() == NULL) {
						special_squid_set_random();
						special_squid_dialog_tag();
						game_set_special_callback(&special_squid_init);
						// game_set_special_callback(&special_fishing_init); // special_squid_init);
					}
					break;
#endif

#ifdef GAME_VISUAL_DEBUG
				case 0x56: // F7
					game_draw_debug_flags();
					break;
				case 0x57: // F8
					game_draw_debug_visuals();
					break;
				case 0x58: // F9
					game_draw_debug_button();
					break;
				case 0x59: // F10
					if (game_get_special_callback() == NULL)
						game_set_special_callback(&special_debug_panel);
					break;
				case 0x55: // F6
					// // Sand puzzle
					// game_set_special_callback(&special_sand_puzzle_init);
					game_set_special_callback(&special_fishing_init);
					// game_over(1);
					break;
#endif
			}

			// printf("Clock:%d -----\n", rpage_get_clock());		
			rpage_video_vsync();
			// NB: disk_request is always = DISK_REQ_OK when in HDD mode
			if (disk_request == DISK_REQ_OK || device_is_hdd())
				game_update(&gi);

			if (gi.disk_change && !device_is_hdd())
				assets_manage_disk_request(&disk_request, &gi.disk_change, &disk_change_timer, gi.next_disk);

			rpage_input_update();

			game_update_music_manager();
			protracker_update_state();
		}

	}

	switch(game_get_state())
	{
		case game_state_victory:

			// Specific disk swapping
			if (!device_is_hdd())
			{
				strncpy(gi.next_disk, asset_get_disk_name("vue_19", ".pak"), DISKNAME_LEN);
				gi.disk_change = TRUE;

				while (gi.disk_change)
				{
					rpage_video_vsync();
					if (gi.disk_change && !device_is_hdd())
						assets_manage_disk_request(&disk_request, &gi.disk_change, &disk_change_timer, gi.next_disk);				
					rpage_input_update();
				}

				disk_request = DISK_REQ_OK;
			}

			gui_unload();

			unload_protracker_music();
			protracker_set_fade_speed(0);		
			// load_packed_protracker_music(asset_build_device_path("mus_ending", ".pak"), game_get_unpacking_buffer());
			load_packed_protracker_music(asset_build_device_path("mus_ending", ".pak"), game_get_unpacking_buffer(), &music_memory_buffer);
			protracker_setup_mod();
			// protracker_set_fade_speed(64);
			protracker_update_state();
			game_endscreen();
			break;
	}

	gui_unload();
	uninit_vue_cache();

	unload_protracker_music();
	uninit_protracker_player();

	game_uninit();

	rpage_video_close();
	// rpage_set_process_priority(0);

	// Free the unpacking buffer
	if (game_get_unpacking_buffer() != NULL)
		rpage_free_os_alloc(game_get_unpacking_buffer(), MAX_PAK_SIZE * sizeof(BYTE)); // free(game_get_unpacking_buffer());

	rpage_uninit();

	return 0;
}

void assets_manage_disk_request(UBYTE *disk_request, BOOL *disk_change, ULONG *disk_change_timer, char *next_disk)
{
	BOOL allow_abort = FALSE;

	// In case the requested disk is a save disk,
	// the user is allowed to cancel the disk request
	if (strncmp(next_disk, SAVE_DISK_NAME, DISKNAME_LEN) == 0)
		allow_abort = TRUE;

#ifdef LATTICE
    switch(*disk_request)
    {
        case DISK_REQ_OK:
            *disk_request = DISK_REQ_TEST_DISK_AVAIL;					
            break;

        case DISK_REQ_TEST_DISK_AVAIL:
            if (disk_is_available_by_logical_name(next_disk)) // if disk is available
                *disk_request = DISK_REQ_DISK_AVAIL;
            else
                *disk_request = DISK_REQ_DISK_NOT_AVAIL;
            break;

        case DISK_REQ_DISK_AVAIL:
            *disk_request = DISK_REQ_WAIT_POST_OK;
            *disk_change_timer = rpage_get_clock();
            break;

        case DISK_REQ_DISK_NOT_AVAIL:
			memset(&disk_change_gui_desc, 0, sizeof(disk_change_gui_desc));
            gui_show_disk_request(next_disk, &disk_change_gui_desc, allow_abort);
            *disk_change_timer = rpage_get_clock();
            *disk_request = DISK_REQ_WAIT_FOR_DISK;
            break;

        case DISK_REQ_WAIT_FOR_DISK:
            if (rpage_get_clock() - *disk_change_timer > DISK_REQ_RETRY_DELAY)
            {
#ifdef DEBUG_MACROS
                rpage_system_flash();
#endif
                *disk_change_timer = rpage_get_clock();
                if (disk_is_available_by_logical_name(next_disk))
                {
                    gui_hide_disk_request();
                    *disk_request = DISK_REQ_WAIT_POST_OK;
                    *disk_change_timer = rpage_get_clock();
                }
            }
            break;

		case DISK_REQ_WAIT_POST_OK:
				rpage_video_screen_to_front();
				*disk_request = DISK_REQ_OK;
				*disk_change = FALSE;
			break;
    }
#endif
	//	We want the player to be able to abort a disk request
	if (allow_abort)
	{
		short mouse_button;
		vec2 mouse_coords;

		rpage_mouse_get_values(&mouse_button, &mouse_coords);
		if (mouse_button)
		{
			if(point_within_rect(&mouse_coords, &(disk_change_gui_desc.button0)))
			{
				gui_hide_disk_request();
				*disk_request = DISK_REQ_WAIT_POST_OK;
				*disk_change_timer = rpage_get_clock();
				game_set_special_callback(NULL);
			}
		}
	}
}