#include "game/vue.h"

#ifndef _GAME_SPECIAL_
#define _GAME_SPECIAL_

// Sand weight puzzle
#define SND_TXTFIELD_SIZE 8

// Secret Key dialog
#define MAX_MAGIC_CODE_LEN 10
#define KEY_TXTFIELD_SIZE MAX_MAGIC_CODE_LEN

#define MAX_FISH_SPRITES 3
#define MAX_FISH_ANGLE 1024
#define FISH_SPRITE_SIZE 32
#define FISH_Y 64 // Y origin of the fish sprites

// Beetle race game
#define BEETLE_RACE_LINES 4
#define BEETLE_MAX_FRAME 4
#define BEETLE_WIDTH 24
#define BEETLE_HEIGHT 24
#define BT_TRACK_X_RIGHT (256 - BEETLE_WIDTH - 4)
#define BT_TRACK_X_LEFT (BEETLE_WIDTH / 2)
#define BT_THROW_LEN 256    // DO NOT CHANGE THIS
#define BT_MAX_FORCE 200

#define BT_SHADOW_Y 64 //  Y Origin of the shadow sprites

#define BTLR_TRACK_Y 18
#define BTLR_TRACK_H 30 // Height of a track

#define BTLR_BALL_X 16
#define BTLR_BALL_Y 28
#define BTLR_BALL_SIZE 16

// Beetle Race game states
#define BT_STATE_RUN 0
#define BT_STATE_AIM 1
#define BT_STATE_SHOOT 2
#define BT_STATE_DROPPED 3

// Fishing game states
#define FS_STATE_INIT 0
#define FS_STATE_RUN 1
// #define FS_STATE_AIMX 1
// #define FS_STATE_AIMY 2
#define FS_STATE_SHOOT 2
#define FS_STATE_DRAW 3
#define FS_STATE_RESTART 4
#define FS_STATE_END 5

#define FS_FISH_X (VUE_X + VUE_WIDTH + 8)
#define FS_FISH_Y 148

#define FS_PHASE_DURATION 240 // approx 4 seconds
#define FS_PHASE_FLOAT_DURATION 120 // How long will the float remain ? Approx 2 seconds

#define FS_FLOAT_SIZE 16

#define FS_GUI_AIM_COLOR 31
#define FS_GUI_POWER_COLOR 25

#define FS_SPR_AIM_Y 154
#define FS_SPR_POWER_Y (FS_SPR_AIM_Y + 16)
#define FS_MAX_POWER (VUE_HEIGHT - FS_FLOAT_SIZE)

#define FS_PX0 32
#define FS_PX1 96
#define FS_PX2 160
#define FS_PX3 224

// Altos puzzle coordinates
#define SP_ALTOS_WIDTH 8
#define SP_ALTOS_HEIGHT 6
#define SP_ALTOS_POS_X 48
#define SP_ALTOS_POS_Y 10
#define SP_ALTOS_KEY_SIZE 20
#define SP_ALTOS_KEY_PAD 2
#define SP_ALTOS_END_BT_X 230
#define SP_ALTOS_END_BT_Y 10
#define SP_ALTOS_RST_BT_X 230
#define SP_ALTOS_RST_BT_Y 120

// Squid puzzle coordinates
//         +---+
//      <P | S | N>            S: SQUID DISPLAY, P: PREV, N: NEXT
//         +---+
//
//    +-+         +-+
// <P |0| N>   <P |1| N>       1: DIGIT #0 DISPLAY, 2: DIGIT #1 DISPLAY, 
//    +-+         +-+          P: PREV, N: NEXT
//                             RST : Reset button
//                             END : Quit button

#define SP_SQD_MAX_SOLUTION 22

// Button mask
#define SP_SQD_ALL_MASK 0xFF
#define SP_SQD_S_MSK (1)
#define SP_SQD_0_MSK (1 << 1)
#define SP_SQD_1_MSK (1 << 2)
#define SP_SQD_RST_MSK (1 << 3)
#define SP_SQD_END_MSK (1 << 4)

// SQUID DISPLAY
#define SP_SQD_CORRECT_SP 0 // Correct squid sprite (all the others have a slight flaw)
#define SP_SQD_LG_BT_SIZE 24 // large button size (w = h)
#define SP_SQD_SM_BT_SIZE 16 // small button size (w = h)

#define SP_SQD_S_DEST_X 112 // Squid sprite, destination position
#define SP_SQD_S_DEST_Y 24
#define SP_SQD_S_Y 64 // Squid sprite Y origin
#define SP_SQD_S_W 48 // Squid sprite width
#define SP_SQD_S_H 40 // Squid sprite height
// #define SP_SQD_S_SRC_Y 40 // Y origin of the first squid sprite in the sprite sheet

#define SP_SQD_S_P_DEST_X 80 // Prev squid, destination position
#define SP_SQD_S_P_DEST_Y 40
#define SP_SQD_S_N_DEST_X 168 // Next squid, destination position
#define SP_SQD_S_N_DEST_Y SP_SQD_S_P_DEST_Y
#define SP_SQD_S_P_SRC_X 0 // Prev squid, source position
#define SP_SQD_S_P_SRC_Y SP_SQD_LG_BT_SIZE
#define SP_SQD_S_N_SRC_X (SP_SQD_LG_BT_SIZE * 2) // Next squid, source position
#define SP_SQD_S_N_SRC_Y SP_SQD_LG_BT_SIZE
#define SP_SQD_S_MAX 6 // how many different sprites for the squid ?

// LATIN DIGIT #0
#define SP_SQD_D0_DEST_X 88 // Digit #0 sprite, destination position
#define SP_SQD_D0_DEST_Y 96
#define SP_SQD_D_W 24 // Digit sprite width
#define SP_SQD_D_H 24 // Digit sprite height
#define SP_SQD_D_SRC_Y 40 // Y origin of the first Digit sprite in the sprite sheet

#define SP_SQD_D_MAX 6 // how many different latin digits ?
#define SP_SQD_0_P_DEST_X 64 // Prev digit #0, destination position
#define SP_SQD_0_P_DEST_Y 104
#define SP_SQD_0_P_SRC_X (4 * SP_SQD_LG_BT_SIZE) // Prev digit #0, source position
#define SP_SQD_0_P_SRC_Y 0

#define SP_SQD_0_N_DEST_X 120 // Prev digit #0, destination position
#define SP_SQD_0_N_DEST_Y 104
#define SP_SQD_0_N_SRC_X (4 * SP_SQD_LG_BT_SIZE) // Next digit #0, source position
#define SP_SQD_0_N_SRC_Y SP_SQD_SM_BT_SIZE

// LATIN DIGIT #1
#define SP_SQD_D1_DEST_X 160
#define SP_SQD_D1_DEST_Y SP_SQD_D0_DEST_Y
#define SP_SQD_1_P_DEST_X 136
#define SP_SQD_1_P_DEST_Y SP_SQD_0_P_DEST_Y
#define SP_SQD_1_N_DEST_X 192
#define SP_SQD_1_N_DEST_Y SP_SQD_0_N_DEST_Y

// Exit, Validate
#define SP_SQD_END_BT_W 24 // Exit/Validate sprite width
#define SP_SQD_END_BT_H 24 // Exit/Validate sprite height
#define SP_SQD_END_BT_X 230
#define SP_SQD_END_BT_Y 10
#define SP_SQD_VAL_BT_X 230
#define SP_SQD_VAL_BT_Y 120

#define SAVE_EMPTY_SLOT "---"

enum game_special { TRAVEL, PUZZLE_PECHE, PUZZLE_ALTOS, PUZZLE_CADRE, PUZZLE_BOUSIER, 
                    INIT_PIEUVRE, PUZZLE_PIEUVRE, PUZZLE_SABLE, TAG_PIEUVRE, ENTER_CODE, 
                    COQUINE, GAME_OVER, THE_END, DEBUG, GAME_SAVE, GAME_LOAD};

void start_game_special(short game_special_idx);

void game_intro_screen(void);
void game_endscreen(void);

void special_empty_players_hand(void);

void special_debug_panel(short *button, vec2 *mouse_coords);

void special_travel_menu_init(short *button, vec2 *mouse_coords);
void special_travel_menu_update(short *button, vec2 *mouse_coords);

// void special_sand_refresh_textfield(void);
// void special_sand_puzzle_init(short *button, vec2 *mouse_coords);
// void special_sand_puzzle_update(short *button, vec2 *mouse_coords);

// void special_vi_genere(void);
// BOOL special_check_magic_code(char *str);
// void special_key_puzzle_init(short *button, vec2 *mouse_coords);
// void special_key_refresh_textfield(void);
// void special_key_puzzle_update(short *button, vec2 *mouse_coords);

void special_game_over(short *button, vec2 *mouse_coords);
void special_game_over_load_screen(short *button, vec2 *mouse_coords);
void special_game_over_update(short *button, vec2 *mouse_coords);

void special_open_save_dialog_init(short *button, vec2 *mouse_coords);
void special_open_save_dialog(short *button, vec2 *mouse_coords);

// Debug
void special_audio_test_init(short *button, vec2 *mouse_coords);
void special_audio_test_update(short *button, vec2 *mouse_coords);


#endif