/* Windless Bay for Amiga OCS -- (c) RESISTANCE.NO 2021 */
/* THIS CODE WAS GENERATED BY R-PAGE SCENARIO TOOLCHAIN */
/*                   DON'T MODIFY IT !                  */

#ifndef _WORLD_CONSTANTS_
#define _WORLD_CONSTANTS_

#define ATARI_DATASET_VERSION "V8.9.11"

/* World constants */
#define MAX_ROOM 19
#define MAX_ZONE 1
#define MAX_WORLD 2
#define MAX_SPR_PORTRAITS 2
#define MAX_GAME_OBJECTS 4
#define MAX_COMBINED_OBJECTS 1

/* musics enum */
#define mus_ville10b 0

/* world enum */
#define world_blank 0
#define world_iceland 1

#ifdef DEBUG_MACROS
extern const char *game_object_name[MAX_GAME_OBJECTS];
#endif

extern const unsigned short game_object_crc16[MAX_GAME_OBJECTS];

extern const char *portrait_sprites[MAX_SPR_PORTRAITS];

BOOL conditions_vue_1(BOOL, short, short);

void worldClearScenarioFlags(void);
void worldDebugScenarioFlags(char *_str);

/**********************/
/* Game Objects (go_) */
/**********************/
#define go_VIDE 0
#define go_CABLE 1
#define go_COMBINE 2
#define go_TELEPHONE 3

/* Samples */
#define SAMPLE_WORLD_LEN 2
#define SAMPLE_NAMES_LEN 1
#define SAMPLE_VUES_LEN 1

#define WRLD_DBG_FLAG_STRSIZE 720
#endif
