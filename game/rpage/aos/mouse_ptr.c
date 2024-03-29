/*  Athanor 2, Original game by Eric "Atlantis" Safar, (C) Safargames 2021  
    Amiga version by Francois "Astrofra" Gutherz.
*/

#ifdef LATTICE
#include <exec/types.h>
#include "rpage/utils.h"
#include "rpage/aos/mouse_ptr.h"

vec2 system_cursors_hotspot[3] = 
{
	{-1, -1},
	{-1, -1},
	{-1, -1}
};

/* ------------------------------- */
/* --- system_cursors (data) --- */
/* Ensure that this data is within chip memory or you'll see nothing !!! */
UWORD chip system_cursors_img[3][36]=
{
	/* Sprite #0 */
	{
		0x0,0x0,
		0x0,0xe000,
		0x4000,0xb000,
		0x6000,0x9800,
		0x7000,0xcc00,
		0x7800,0xc600,
		0x7c00,0xe300,
		0x7e00,0xe180,
		0x7f00,0xf0c0,
		0x7f80,0xf060,
		0x7e00,0xf9e0,
		0x7c00,0xf7e0,
		0x6600,0xdd80,
		0x200,0xff80,
		0x300,0x76c0,
		0x100,0x3c0,
		0x0,0x3c0,
		0x0,0x0
	},
	/* Sprite #1 */
	{
		0x0,0x0,
		0xf40,0x16a0,
		0x3fe0,0x5f50,
		0x61f0,0xbfe8,
		0x7bf8,0xfff4,
		0xf7f8,0xfff6,
		0xe10c,0xfff2,
		0x7fdc,0xfff3,
		0x7fbe,0xbfe1,
		0x3f0c,0x5ff3,
		0xff8,0x37c7,
		0x1e0,0xe1e,
		0x700,0xef8,
		0xfc0,0x1730,
		0x7b0,0xa68,
		0x38,0x7c4,
		0x10,0x2e,
		0x0,0x0
	},
	/* Sprite #2 */
	{
		0x0,0x0,
		0x0,0x3ff8,
		0x3fc0,0x403c,
		0x7f8c,0xd572,
		0x7f8c,0xff73,
		0x3c,0xffcb,
		0x7fc,0x781b,
		0x7fc,0xaab,
		0x7fc,0xd5b,
		0x7fc,0xaab,
		0x7fc,0xffb,
		0x7fc,0x81b,
		0x3c,0xffdb,
		0x7fbc,0xff4b,
		0x3fd8,0x4027,
		0xaa0,0x355e,
		0x0,0x1ffc,
		0x0,0x0
	},
};

/* -- (end of) system_cursors -- */
/* ------------------------------- */

vec2 game_cursors_hotspot[4] = 
{
	{-4, -4},
	{-4, -4},
	{-4, -4},
	{-8, -8}
};

/* ------------------------------- */
/* --- game_cursors (data) --- */
/* Ensure that this data is within chip memory or you'll see nothing !!! */
UWORD chip game_cursors_img[4][36]=
{
	/* Sprite #0 */
	{
		0x0,0x0,
		0x0,0x7f00,
		0x7e00,0xff80,
		0x700,0xfbc0,
		0x380,0x5e0,
		0x1c0,0x1ef8,
		0x18f0,0x376e,
		0x307c,0x7ffb,
		0x7e,0x70ff,
		0x7e,0xff,
		0x7e,0xe1ff,
		0x407e,0xbfff,
		0x78fe,0xb7ff,
		0x1ffe,0x6fe1,
		0x7fc,0x3bc3,
		0x80,0xf7f,
		0x0,0x3fe,
		0x0,0x0
	},
	/* Sprite #1 */
	{
		0x0,0x0,
		0x0,0x1f00,
		0xf00,0x3ec0,
		0x1180,0x7fe0,
		0x2ee0,0xf950,
		0x5f60,0xfcf0,
		0x5fa0,0xe870,
		0x6fa0,0xf070,
		0x3060,0xffd0,
		0x1fc0,0x7fa0,
		0xf80,0x3770,
		0x0,0xff8,
		0x30,0x6c,
		0x18,0x3e,
		0xc,0x1f,
		0x6,0xd,
		0x0,0x6,
		0x0,0x0
	},
	/* Sprite #2 */
	{
		0x0,0x0,
		0x0,0x3e00,
		0x1c00,0x3f00,
		0x600,0xfb00,
		0x4300,0xee80,
		0x4700,0xfa80,
		0x6f00,0xf680,
		0x3e00,0xffc0,
		0x1c80,0x6b60,
		0x1c0,0x3e70,
		0x1e0,0x6fe,
		0xfc,0x37f,
		0x62,0x1bf,
		0x22,0xf7,
		0x22,0x7f,
		0x1e,0x7d,
		0x0,0x3e,
		0x0,0x0
	},
	/* Sprite #3 */
	{
		0x0,0x0,
		0x0,0x7ffe,
		0x7ffe,0xbffd,
		0x7ffe,0xffff,
		0x7ffe,0xffff,
		0x7ffe,0xffff,
		0x7ffe,0xffff,
		0x6666,0xffff,
		0x6666,0xffff,
		0x7ffe,0xffff,
		0x7ffe,0xbffd,
		0xe0,0x7f7e,
		0x70,0x1e8,
		0x30,0xf8,
		0x10,0x78,
		0x0,0x38,
		0x0,0x18,
		0x0,0x0
	},
};

/* -- (end of) game_cursors -- */
/* ------------------------------- */

#endif