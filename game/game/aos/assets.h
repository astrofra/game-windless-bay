/*  Athanor 2, Original game by Eric "Atlantis" Safar, (C) Safargames 2021  
    Amiga version by Francois "Astrofra" Gutherz.
*/

#ifndef _ASSETS_DISPATCH_
#define _ASSETS_DISPATCH_

#include <stdarg.h>
#include <stdio.h>
#include "rpage/frwk.h"
#include "rpage/utils.h"

#define ASSETS_FLOPPY_MODE  0
#define ASSETS_HDD_MODE     1

#define AOS_MAX_DRIVE_NAMES 4

void init_asset_dispatch(short media_install_mode);
BOOL device_is_hdd(void);
BOOL disk_is_available_by_logical_name(char *logical_name);
char *asset_get_disk_name(char *asset_name, char *ext_name);
char *asset_build_device_path(char *asset_name, char *ext_name);
BOOL assets_device_access_occurred(void);

#endif