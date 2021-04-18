/*  Athanor 2, Original game by Eric "Atlantis" Safar, (C) Safargames 2021  
    Amiga version by Francois "Astrofra" Gutherz.
*/

#ifndef _WORLD_HEADER_
#define _WORLD_HEADER_

#include <stdarg.h>
#include "rpage/frwk.h"
#include "rpage/utils.h"
#include "game/world_const.h"
#include "game/game.h"

#define MAX_LINK_PER_ROOM 4
#define MAX_NAME_CHAR_LEN 32
#define MAX_FILENAME_CHAR_LEN 64
#define MAX_DESCRIPTION_CHAR_LEN 256
#define MAX_ZONE_PER_ROOM 8

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef ON
#define ON 1
#endif

#ifndef OFF
#define OFF 0
#endif

/// Actions performed by the player
enum game_action { act_take_drop, act_look, act_use, act_talk, act_save, act_load };

/// Link directions, CW then UP and DOWN then some other random exit
enum link_direction { dir_north, dir_east, dir_south, dir_west, dir_up, dir_down, dir_other};

// TODO: automatically generate this list ?
extern short Flag009, Flag022, Flag023, Flag031, Flag036, Flag069;

typedef struct {
	short vue_index;
	// char name[MAX_NAME_CHAR_LEN];
	char bitmap[MAX_FILENAME_CHAR_LEN];
	short links[MAX_LINK_PER_ROOM];
	short zone[MAX_ZONE_PER_ROOM];
	BOOL (*conditions)(BOOL, short, short);
} room;

typedef struct {
	poly zone_poly;
	short object;
	BOOL (*conditions)(BOOL, short, short);
} zone;

typedef struct {
	rect source;
	vec2 dest;
	short vue;
	BOOL enabled;
	BOOL enabled_default;
	short go_index;
} sheet_sprite;

// typedef struct {
// 	char *filename;
// 	UBYTE disk;
// } file_dispatch;

typedef struct {
	short object_index;
	// vec2 position;
} inventory_object;

typedef struct {
	short in[2];
	short out;
} combined_game_object;

// typedef struct {
// 	short vue;
// 	short sample_index;
// } vue_sample;

typedef struct {
	short world_index; // from what world will this sample be played ?
	short sample_index; // sample index (in the world)
	short name_index; // index of the filename string for this sample 
} sample_per_world;

// For preload only!
typedef struct {
	short vue_index; // index of the vue where the sample needs to be preloaded
	short sample_index; // from the SafarScript stand point (!= 'sample_index' of 'sample_per_world')
} sample_per_vue;

extern room rooms[MAX_ROOM];
extern zone zones[MAX_ZONE];
extern const char *world_sprites_name[MAX_WORLD];
extern combined_game_object combined_go[MAX_COMBINED_OBJECTS];
extern const short go_to_inventory_tooltip[MAX_GAME_OBJECTS];

extern sample_per_world world_samples[SAMPLE_WORLD_LEN];
extern sample_per_vue vue_samples[SAMPLE_VUES_LEN];
extern char *sample_names[SAMPLE_NAMES_LEN];

extern unsigned int Timer1, Timer2;

void condition_master(void);
void condition_timers(void);

short worldGetMaxRoom(void);
short worldGetCurrentRoom(void);
void worldSetCurrentRoom(short room_index);
void worldSetCurrentRoomByVue(short vue_index);
void worldSetCurrentRoomByVue_ex(short vue_index, short dialog_index);
short roomGetVueIndex(short room_index);
short vue_get_room_index(short vue_index);
// short roomGetIndexByName(char *room_name);
short roomGetLinkDirection(short room_index, short dir);
char *world_get_object_name(short object_index);

short getPreviousVue(void);
void setPreviousVue(short v);

short worldGetMaxSheetSprites(void);

void worldOpenExitNorth(short from_index, short to_index);
void worldOpenExitSouth(short from_index, short to_index);
void worldOpenExitEast(short from_index, short to_index);
void worldOpenExitWest(short from_index, short to_index);

void worldCloseExitNorth(short vue_index);
void worldCloseExitSouth(short vue_index);
void worldCloseExitEast(short vue_index);
void worldCloseExitWest(short vue_index);

void worldSetCurrentChapter(short chapter);
void world_set_current_index(short world);
short world_get_current_index(void);
void worldAbortLeavingRoom(short unknown_param);

void gameObjectGetSpriteSheetCoords(short go_idx, rect *source, vec2 *dest);
short gameObjectGetSpriteInventoryIndex(short go_idx);
short getSpriteSheetVueIndex(short idx);
BOOL getSpriteSheetEnabled(short idx);
void setSpriteSheetEnabled(short idx, BOOL state);
short getSpriteSheetGameObjectIndex(short idx);

void worldResetSpritesState(void);

void gameSaveSlot(short slot, BOOL is_on_hdd);
void gameLoadSlot(short slot, BOOL is_on_hdd);

short worldGetGameObjectFromCRC16(unsigned short crc_val);

void worldLoadScenarioFlags(rpage_file file);
void worldSaveScenarioFlags(rpage_file file);

void worldLoadSpritesState(rpage_file file);
void worldSaveSpritesState(rpage_file file);

#endif
