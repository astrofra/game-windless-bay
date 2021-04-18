#ifdef LATTICE
#define INTUI_V36_NAMES_ONLY

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <pragmas/dos_pragmas.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include "rpage/aos/screen_size.h"
#include "rpage/aos/color.h"
#include "rpage/aos/screen.h"
#include "rpage/aos/debug.h"
#include "rpage/utils.h"

extern struct DosLibrary *DOSBase;
extern struct Library *IntuitionBase;
extern struct Library *GfxBase;

PLANEPTR my_bit_plane;
struct TmpRas my_tmp_ras;

void __inline WaitVBL(buffered_screen *screen)
{
	WaitTOF();
}

USHORT __inline getLogicalBitmapIndex(buffered_screen *screen)
{
	if (screen->double_buffer_enabled)
		return (USHORT)((screen->physical)^1);
	else
		return 0;	
}

USHORT __inline getPhysicalBitmapIndex(buffered_screen *screen)
{
	return screen->physical;
}

void __inline flipBuffers(buffered_screen *screen)
{
	if (screen->double_buffer_enabled)
	{
		/* Swap the physical and logical bitmaps */
		screen->physical = (USHORT)(screen->physical)^1;
		screen->screen->RastPort.BitMap	= screen->bitmaps[screen->physical];
		screen->screen->ViewPort.RasInfo->BitMap = screen->bitmaps[screen->physical];
		// screen->rp = &(screen->screen->RastPort);
	}
}

void __inline presentPalette(buffered_screen *screen)
{
	set_palette(&(screen->screen->ViewPort), &(screen->palettes[getPhysicalBitmapIndex(screen)]), 0, 1 << screen->screen->BitMap.Depth);
}

void __inline presentScreen(buffered_screen *screen)
{
	/* Update the physical display to match the recently updated bitmap. */
	MakeScreen(screen->screen);
	RethinkDisplay();
	presentPalette(screen);
}

void __inline presentScreenFast(buffered_screen *screen)
{
	LoadView(&(screen->vbs[getPhysicalBitmapIndex(screen)].view));
	WaitTOF();
}

void synchronizeBuffers(buffered_screen *screen)
{
	if (screen->double_buffer_enabled)
	{
		short i;
		for(i = 0; i < (1 << screen->screen->BitMap.Depth); i++)
			screen->palettes[getLogicalBitmapIndex(screen)][i] = screen->palettes[getPhysicalBitmapIndex(screen)][i];
		BltBitMap(screen->bitmaps[screen->physical], 0, 0, screen->bitmaps[getLogicalBitmapIndex(screen)], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0xC0, 0xFF, NULL);
		WaitBlit();
	}
}

void prepareFastDoubleBuffer(buffered_screen *screen)
{
	struct ViewBuffer *vb;

	vb = &(screen->vbs[getPhysicalBitmapIndex(screen)]);
	InitView(&(vb->view));
	strcpy((char *)&vb->rasinfo, (char *)screen->screen->ViewPort.RasInfo);
	vb->rasinfo.BitMap      = screen->bitmaps[getPhysicalBitmapIndex(screen)];
	vb->rasinfo.RxOffset    = screen->screen->ViewPort.RasInfo->RxOffset;
	vb->rasinfo.RyOffset    = screen->screen->ViewPort.RasInfo->RyOffset;
	vb->rasinfo.Next        = NULL;
	InitVPort(&vb->viewport);
	vb->viewport.RasInfo    = &vb->rasinfo;
	vb->viewport.DWidth     = SCREEN_WIDTH;
	vb->viewport.DHeight    = SCREEN_HEIGHT;
	vb->viewport.Next       = NULL;
	vb->view.ViewPort       = &vb->viewport;
	vb->viewport.ColorMap   = GetColorMap(COLORS);
	LoadRGB4(&vb->viewport, screen->palettes[getPhysicalBitmapIndex(screen)], COLORS);
	MakeVPort(&vb->view, &vb->viewport);
	MrgCop(&vb->view);

	vb = &(screen->vbs[getLogicalBitmapIndex(screen)]);
	InitView(&(vb->view));
	strcpy((char *)&vb->rasinfo, (char *)screen->screen->ViewPort.RasInfo);
	vb->rasinfo.BitMap      = screen->bitmaps[getLogicalBitmapIndex(screen)];
	vb->rasinfo.RxOffset    = screen->screen->ViewPort.RasInfo->RxOffset;
	vb->rasinfo.RyOffset    = screen->screen->ViewPort.RasInfo->RyOffset;
	vb->rasinfo.Next        = NULL;
	InitVPort(&vb->viewport);
	vb->viewport.RasInfo    = &vb->rasinfo;
	vb->viewport.DWidth     = SCREEN_WIDTH;
	vb->viewport.DHeight    = SCREEN_HEIGHT;
	vb->viewport.Next       = NULL;
	vb->view.ViewPort       = &vb->viewport;
	vb->viewport.ColorMap   = GetColorMap(COLORS);
	LoadRGB4(&vb->viewport, screen->palettes[getLogicalBitmapIndex(screen)], COLORS);
	MakeVPort(&vb->view, &vb->viewport);
	MrgCop(&vb->view);	
}

UWORD screenGetDepth(void)
{
	return SCREEN_DEPTH;
}

buffered_screen *openMainScreen(void)
{
	int i;
	static buffered_screen bscreen;
	struct NewScreen new_screen;
	struct NewWindow new_window;

	bscreen.double_buffer_enabled = DBUFFER_ENABLED;
	bscreen.physical = 0; /* the physical screen (front buffer) has index 0. */

	for(i = 0; i < (DBUFFER_ENABLED?2:1); i++)
	{
		bscreen.bitmaps[i] = setupBitMap(SCREEN_DEPTH, SCREEN_WIDTH, SCREEN_HEIGHT);
		bscreen.palettes[i] = (amiga_color *)rpage_c_alloc(COLORS, sizeof(amiga_color));
		memset(bscreen.palettes[i], 0x000, COLORS);
	}

	if (bscreen.bitmaps[i-1] != NULL)
	{
		new_screen.LeftEdge = 0;
		new_screen.TopEdge = 0;
		new_screen.Width = SCREEN_WIDTH;
		new_screen.Height = SCREEN_HEIGHT;
		new_screen.Depth = SCREEN_DEPTH;
		new_screen.ViewModes = SPRITES;
		new_screen.BlockPen = 0;
		new_screen.DetailPen = 0;

		if (SCREEN_WIDTH >= 640)
		{
			new_screen.ViewModes |= HIRES;
		}

		if (SCREEN_HEIGHT >= 512)
		{
			new_screen.ViewModes |= LACE;
		}

		new_screen.Type = CUSTOMSCREEN | CUSTOMBITMAP | SCREENQUIET;
		new_screen.Font = NULL;
		new_screen.DefaultTitle = NULL;
		new_screen.Gadgets = NULL;
		new_screen.CustomBitMap = bscreen.bitmaps[bscreen.physical];

		bscreen.screen = OpenScreen(&new_screen);
		if (bscreen.screen != NULL)
		{
			ShowTitle(bscreen.screen, FALSE);
			/* Blacken the palette */
			// set_palette_to_black(&(bscreen.screen->ViewPort), 0, COLORS);
			set_palette_to_grey(&(bscreen.screen->ViewPort), 0, COLORS);

			WaitTOF();

			/* Create a fullscreen window */
			new_window.LeftEdge = 0;
			new_window.TopEdge = 0;
			new_window.DetailPen = 0;
			new_window.BlockPen = 0;
			new_window.Title = NULL;
			new_window.Width = SCREEN_WIDTH; // - SCR_PAD_X;
			new_window.Height = SCREEN_HEIGHT;
			new_window.BlockPen = 0;
			new_window.DetailPen = 0;
			new_window.IDCMPFlags = IDCMP_MOUSEMOVE | IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY | IDCMP_INTUITICKS | IDCMP_DISKINSERTED | IDCMP_DISKREMOVED;
			new_window.Flags = WFLG_ACTIVATE | WFLG_OTHER_REFRESH | WFLG_RMBTRAP | WFLG_REPORTMOUSE | WFLG_WINDOWACTIVE | WFLG_GIMMEZEROZERO | WFLG_BORDERLESS | WFLG_NOCAREREFRESH;
			new_window.Screen = bscreen.screen;
			new_window.Type = CUSTOMSCREEN;
			new_window.FirstGadget = NULL;
			new_window.CheckMark = NULL;
			new_window.BitMap = NULL;

			bscreen.window = OpenWindow(&new_window);
			if (bscreen.window == NULL)
			{
				printf("openMainScreenCustom(), OpenWindow() failed!\n");
				closeMainScreen(&bscreen);
				return NULL;
			}

			bscreen.screen->FirstWindow = bscreen.window;
			// bscreen.rp = &(bscreen.screen->RastPort);

			/* Allocate some display memory: */
			my_bit_plane = AllocRaster(SCREEN_WIDTH, SCREEN_HEIGHT);
			/* Prepare the TmpRas structure: */
			InitTmpRas(&my_tmp_ras, my_bit_plane, RASSIZE(SCREEN_WIDTH, SCREEN_HEIGHT));
			bscreen.screen->RastPort.TmpRas = &my_tmp_ras;

			ScreenToFront(bscreen.screen);
			return &bscreen;
		}
		else
		{
			for(i = 0; i  < (DBUFFER_ENABLED?2:1); i++)
			{
				freeBitMap(bscreen.bitmaps[i], SCREEN_DEPTH, SCREEN_WIDTH, SCREEN_HEIGHT);
				bscreen.bitmaps[i] = NULL;
				free(bscreen.palettes[i]);
				bscreen.palettes[i] = NULL;
			}
			// printf("free(), openMainScreen()\n");
			printf("openMainScreenCustom(), OpenScreen() failed!\n");
			return NULL;
		}
	}
	else
	{
		printf("openMainScreenCustom(), setupBitMaps() failed!\n");
		return NULL;
	}
}

void closeMainScreen(buffered_screen *screen)
{
	if (screen->screen->FirstWindow != NULL)
		CloseWindow(screen->screen->FirstWindow);

	if (screen->screen != NULL)
	{
		int i;
		struct BitMap *_bmp[2];
		for(i = 0; i  < ((!(screen->double_buffer_enabled))?1:2); i++)
			_bmp[i] = screen->bitmaps[i];

		WaitBlit();
		CloseScreen(screen->screen);

		for(i = 0; i  < ((!(screen->double_buffer_enabled))?1:2); i++)
		{
			freeBitMap(_bmp[i], SCREEN_DEPTH, SCREEN_WIDTH, SCREEN_HEIGHT);
			_bmp[i] = NULL;
			free(screen->palettes[i]);
			screen->palettes[i] = NULL;
		}

		for(i = 0; i  < ((!(screen->double_buffer_enabled))?1:2); i++)
			screen->bitmaps[i] = NULL;
		screen->screen = NULL;
		screen->window = NULL;
	}
	else
		printf("closeMainScreen(): new_screen is NULL!\n");
}

/// allocate the bit maps for a screen.
struct BitMap *setupBitMap(LONG depth, LONG width, LONG height)
{
	struct BitMap *main_bitmap = NULL;

	main_bitmap = (struct BitMap *)rpage_c_alloc(1, sizeof(struct BitMap));
	if (main_bitmap != NULL)
	{
		InitBitMap(main_bitmap, depth, width, height);

		if (setupPlanes(main_bitmap, depth, width, height) != NULL)
			return (main_bitmap);

		freePlanes(main_bitmap, depth, width, height);
		free(main_bitmap);
		// printf("FreeMem(), setupBitMap()\n");
	}

	return NULL;
}

/// free up the memory allocated by setupBitMap().
VOID freeBitMap(struct BitMap *main_bitmap, LONG depth, LONG width, LONG height)
{
	if (main_bitmap != NULL)
	{
		freePlanes(main_bitmap, depth, width, height);
		free(main_bitmap);
	}
}


/// allocate the bit planes for a screen bit map.
BOOL setupPlanes(struct BitMap *bitMap, LONG depth, LONG width, LONG height)
{
	SHORT plane_num;

	for (plane_num = 0; plane_num < depth; plane_num++)
	{
		bitMap->Planes[plane_num] = (PLANEPTR)AllocRaster(width, height);

		if (bitMap->Planes[plane_num] != NULL)
			BltClear(bitMap->Planes[plane_num], (width / 8) * height, 1);
		else
		{
			freePlanes(bitMap, depth, width, height);
			return FALSE;
		}
	}
	return TRUE;
}

/// free up the memory allocated by setupPlanes().
VOID freePlanes(struct BitMap *bitMap, LONG depth, LONG width, LONG height)
{
	SHORT plane_num;

	for (plane_num = 0; plane_num < depth; plane_num++)
	{
		if (bitMap->Planes[plane_num] != NULL)
			FreeRaster(bitMap->Planes[plane_num], width, height);
	}
}


/// Add36k does nearly the same as 'subbp' or 'add21k', exept that it does not
/// only free 21kb of memory (one std. workbench plane), but in fact 36kb of
/// memory (that is one plane and 250 lines of the others).
/// Original code by Alexander Rawass
#define ADD36K_HEIGHT 80
VOID add36k(struct IntuitionBase *ibase)
{
	long pl0,pl1,plb,ple;
	struct Screen	*scr;
	struct BitMap	*bm;
	struct Window	*win;
	ULONG		origheight;

	if (ibase != NULL)
	{
		scr = ibase->ActiveScreen;
		win = ibase->ActiveWindow;

		origheight=scr->Height;
		SizeWindow(win,0,-150);

		Delay(1*50);

		scr->Height=ADD36K_HEIGHT;
		scr->BitMap.Depth=1;
		bm=&scr->BitMap;
		bm->Rows=ADD36K_HEIGHT;
		bm->Depth=1;
		pl0=(long)bm->Planes[0];
		pl1=(long)bm->Planes[1];
		bm->Planes[1]=NULL;
		plb=pl0+(640*ADD36K_HEIGHT/8);
		ple=pl1+(640*origheight/8);

		RemakeDisplay();
		FreeMem((APTR)plb,ple-plb);
		// printf("FreeMem(), add36k()\n");
	}
}
#endif