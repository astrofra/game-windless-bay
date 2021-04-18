/*  Resistance's Portable-Adventure-Game-Engine (R-PAGE), Copyright (C) 2019 François Gutherz, Resistance.no
    Released under MIT License, see license.txt for details.
*/

#ifdef LATTICE
#ifndef SCREEN_ROUTINES
#define SCREEN_ROUTINES

#include "rpage/aos/inc.prl"
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>

struct ViewBuffer
{
    struct View view;
    struct ViewPort viewport;
    struct RasInfo  rasinfo;
    struct BitMap   *bitmap;
};

typedef struct{
	struct ViewBuffer vbs[2];
	struct BitMap *bitmaps[2];
	amiga_color *palettes[2];
	struct Screen *screen;
	struct Window *window;
	BOOL double_buffer_enabled;
	struct RastPort *rp;
	USHORT physical;
} buffered_screen;

#define DBUFFER_ENABLED TRUE

/* Prototypes for our functions */

void WaitVBL(buffered_screen *screen);
void disableScreen(void);
void enableScreen(void);
// buffered_screen *openMainScreenCustom(USHORT _width, USHORT _height, USHORT _colors, BOOL _dbuffer);
buffered_screen *openMainScreen(void);
void closeMainScreen(buffered_screen *main_screen);
struct BitMap *setupBitMap(LONG, LONG, LONG);
void freeBitMap(struct BitMap *,LONG, LONG, LONG);
BOOL setupPlanes(struct BitMap *, LONG, LONG, LONG);
void freePlanes(struct BitMap *, LONG, LONG, LONG);
USHORT getLogicalBitmapIndex(buffered_screen *screen);
USHORT getPhysicalBitmapIndex(buffered_screen *screen);
void flipBuffers(buffered_screen *screen);
void prepareFastDoubleBuffer(buffered_screen *screen);
void presentScreenFast(buffered_screen *screen);
void presentPalette(buffered_screen *screen);
void presentScreen(buffered_screen *screen);
void synchronizeBuffers(buffered_screen *screen);
UWORD screenGetDepth(void);
/// https://aminet.net/package.php?package=util/misc/add36k.lzh by Alexander Rawass
VOID add36k(struct IntuitionBase *ibase);

#endif
#endif