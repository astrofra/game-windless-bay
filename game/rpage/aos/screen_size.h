/*  Resistance's Portable-Adventure-Game-Engine (R-PAGE), Copyright (C) 2019 François Gutherz, Resistance.no
    Released under MIT License, see license.txt for details.
*/

#ifdef LATTICE
#ifndef _SCREEN_SIZE_
#define _SCREEN_SIZE_

// #define VGA_ENABLED
#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 200
#define DISPL_WIDTH (DEFAULT_WIDTH)
#define DISPL_HEIGHT (DEFAULT_HEIGHT)
#define SCREEN_WIDTH (DEFAULT_WIDTH)
#define SCREEN_HEIGHT (DEFAULT_HEIGHT)
#define SCR_PAD_X 16

#ifdef VGA_ENABLED
#define SCREEN_DEPTH    7
#else
#define SCREEN_DEPTH    5
#endif
#define COLORS  (1 << SCREEN_DEPTH)

#endif // #ifndef _SCREEN_SIZE_
#endif