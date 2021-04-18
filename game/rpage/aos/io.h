/*  Resistance's Portable-Adventure-Game-Engine (R-PAGE), Copyright (C) 2019 François Gutherz, Resistance.no
    Released under MIT License, see license.txt for details.
*/

#ifndef _IO_ROUTINES_
#define _IO_ROUTINES_

#define DISKNAME_LEN 256

#ifdef LATTICE
#include <exec/types.h>

#define PLATFORM_MOUSE_LEFT_BUTTON 1
#define PLATFORM_MOUSE_RIGHT_BUTTON (1 << 1)

#define HACK_MOUSE_LEFT_UP (!((*(UBYTE *)0xBFE001) & (1 << 6)))
#define HACK_MOUSE_RIGHT_UP (!((*(UWORD *)0xDFF016) & (1 << 10)))

#define DISK_REQ_OK 0 // It's ok, let read that file!
#define DISK_REQ_TEST_DISK_AVAIL 1 // Is it OK? Let's see if there is a disk in the drive ?
#define DISK_REQ_DISK_AVAIL 2 // Good, we have the disk present!
#define DISK_REQ_DISK_NOT_AVAIL 3 // Not good, the disk we want is not there!
#define DISK_REQ_WAIT_FOR_DISK 4 // No disk is present, we'll retry 1000 ms later
#define DISK_REQ_WAIT_POST_OK 5 // It's almost OK, but let's wait a bit!

#define DISK_REQ_RETRY_DELAY 4000

UBYTE HackGetKeyboardCode(void);

UBYTE Keyboard(void);
int file_get_size(char *filename);
void input_window_init(struct Window *window);
void input_update(short *button, short *x, short *y, unsigned short *rawkey);

char *disk_get_logical_name(char *device_physical_name);

#endif
#endif