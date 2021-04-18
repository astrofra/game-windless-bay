/*  Athanor 2, Original game by Eric "Atlantis" Safar, (C) Safargames 2021  
    Amiga version by Francois "Astrofra" Gutherz.
*/

#include <stdarg.h>
#include <stdio.h>
#include "rpage/frwk.h"
#include "rpage/utils.h"
#include "rpage/aos/io.h"
#include "game/aos/assets.h"
#include "game/aos/files.h"
#include "game/vue.h"

short asset_hdd_install = ASSETS_HDD_MODE;
char str_asset_filename[_STR_MAX_SIZE]; /// Ex: vue_01.pak, sprites.pak, ...
char *aos_drive_names[AOS_MAX_DRIVE_NAMES] = {"df0:", "df1:", "df2:", "df3:"}; 

BOOL assets_device_access;

/// Init the asset dispatcher
void init_asset_dispatch(short media_install_mode)
{
    asset_hdd_install = media_install_mode;
    assets_device_access = FALSE;
}

BOOL device_is_hdd(void)
{
    return asset_hdd_install;
}

BOOL disk_is_available_by_logical_name(char *logical_name)
{
    short i;
    for(i = 0; i < AOS_MAX_DRIVE_NAMES; i++)
        if(!strcmp(disk_get_logical_name((char *)aos_drive_names[i]), logical_name))
            return TRUE;

    return FALSE;
}

/// Get the device name where the asset file is meant to be found
char *asset_get_disk_name(char *asset_name, char *ext_name)
{
    short i;
    static char _str[_STR_MAX_SIZE];

    sprintf(_str, "%s%s", asset_name, ext_name);
    for(i = 0; i < FILE_DISPATCH_LIST_SIZE; i++)
    {
        if (strcmp(_str, file_dispatch_list[i].filename) == 0)
        {
            switch(file_dispatch_list[i].disk)
            {
                case DATA_DISK_0:
                    return (DISK_NAME_0);
                    break;
                // case DATA_DISK_1:
                //     return (DISK_NAME_1);
                //     break;
                // case DATA_DISK_2:
                //     return (DISK_NAME_2);
                //     break;
                case DATA_DISK_ANY:
                    // is any of the required disk available in one of the disk drives ?
                    for(i = 0; i < AOS_MAX_DRIVE_NAMES; i++)
                    {
                        if(!strcmp(disk_get_logical_name(aos_drive_names[i]), DISK_NAME_0))
                        {
                            // printf("%s\n", DISK_NAME_0 "assets/");
                            return (DISK_NAME_0);
                        }
                        // if(!strcmp(disk_get_logical_name(aos_drive_names[i]), DISK_NAME_1))
                        // {
                        //     // printf("%s\n", DISK_NAME_1 "assets/");
                        //     return (DISK_NAME_1);
                        // }
                        // if(!strcmp(disk_get_logical_name(aos_drive_names[i]), DISK_NAME_2))
                        // {
                        //     // printf("%s\n", DISK_NAME_2 "assets/");
                        //     return (DISK_NAME_2);
                        // }                            
                    }
                    return (DISK_NAME_0);
                    break;
                default:
                    return "";
                    break;

            }
        }
    }

    return "";
}

char *asset_build_device_path(char *asset_name, char *ext_name)
{
    static char _str[_STR_MAX_SIZE];
    short i, j;
    if (device_is_hdd())
    {
#ifdef LATTICE
        memset(str_asset_filename, 0, _STR_MAX_SIZE);
        i = strlen("assets/");
        strncpy(str_asset_filename, "assets/", i);
        j = strlen(asset_name);
        strncpy(str_asset_filename + i, asset_name, j);
        i += j;
        j = strlen(ext_name);
        strncpy(str_asset_filename + i, ext_name, j);
#else
        sprintf(str_asset_filename, "assets/%s%s", asset_name, ext_name);
#endif
    }
    else
    {
        assets_device_access = TRUE;
#ifdef LATTICE
        memset(str_asset_filename, 0, _STR_MAX_SIZE);
        i = strlen(asset_get_disk_name(asset_name, ext_name));
        strncpy(str_asset_filename, asset_get_disk_name(asset_name, ext_name), i);
        j = strlen("assets/");
        strncpy(str_asset_filename + i, "assets/", j);
        i += j;
        j = strlen(asset_name);
        strncpy(str_asset_filename + i, asset_name, j);
        i += j;
        j = strlen(ext_name);
        strncpy(str_asset_filename + i, ext_name, j);
#else
        sprintf(str_asset_filename, "%sassets/%s%s\0", asset_get_disk_name(asset_name, ext_name), asset_name, ext_name);
#endif          
    }

    return str_asset_filename;
}

BOOL assets_device_access_occurred(void)
{
    if (!device_is_hdd())
    {
        BOOL flag = assets_device_access;
        assets_device_access = FALSE;
        return flag;
    }

    return FALSE;
}