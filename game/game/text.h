#ifndef _GAME_DATA_
#define _GAME_DATA_

/* THIS CODE WAS GENERATED */
/*    DON'T MODIFY IT !    */

#define GAME_MAX_ROOM 20
#define GAME_MAX_DIALOG_PER_ROOM 2
#define GAME_MAX_DIALOG 19
#define GAME_MAX_DYNAMIC_DIALOG 1
#define GAME_DYNAMIC_DIALOG_MAX_LEN 48

#define SYS_MAX_DIALOG 33
#define TOOLTIP_MAX_DIALOG 28
#define CREDITS_MAX_DIALOG 130

extern short dialog_per_room[GAME_MAX_ROOM][GAME_MAX_DIALOG_PER_ROOM];
extern char *dialogs[GAME_MAX_DIALOG];
extern char *system_dialogs[SYS_MAX_DIALOG];
extern char *tooltip_dialogs[TOOLTIP_MAX_DIALOG];
extern char *credits_dialogs[CREDITS_MAX_DIALOG];
extern short dynamic_dialogs[GAME_MAX_DYNAMIC_DIALOG];

#define SYS00 (-0)
#define SYS01 (-1)
#define SYS02 (-2)
#define SYS03 (-3)
#define SYS04 (-4)
#define SYS05 (-5)
#define SYS06 (-6)
#define SYS07 (-7)
#define SYS08 (-8)
#define SYS09 (-9)
#define SYS10 (-10)
#define SYS11 (-11)
#define SYS12 (-12)
#define SYS13 (-13)
#define SYS14 (-14)
#define SYS15 (-15)
#define SYS16 (-16)
#define SYS17 (-17)
#define SYS18 (-18)
#define SYS19 (-19)
#define SYS20 (-20)
#define SYS21 (-21)
#define SYS22 (-22)
#define SYS23 (-23)
#define SYS24 (-24)
#define SYS25 (-25)
#define SYS26 (-26)
#define SYS27 (-27)
#define SYS28 (-28)
#define SYS29 (-29)
#define SYS30 (-30)
#define SYS31 (-31)
#define SYS32 (-32)
#endif
