/*  Resistance's Portable-Adventure-Game-Engine (R-PAGE), Copyright (C) 2019 François Gutherz, Resistance.no
    Released under MIT License, see license.txt for details.
*/

#include <stdlib.h>

// #include <exec/types.h>
#include "rpage/aos/inc.prl"

#include "rpage/frwk.h"
#include "ext/tinfl.h"
#include "rpage/aos/locale.h"
#include "ext/aos/shrinkler.h"
#include "ext/aos/nrv2.h"
#include "rpage/err.h"
#include "rpage/aos/debug.h"
#include "rpage/utils.h"

extern struct DosLibrary *DOSBase;

char *locale_ext[3] = {"_fr.pak", "_en.pak", "_es.pak"};

/// Compressed Text loading
UBYTE *load_pak_locale_to_array(char *text_array[], UWORD array_size, UBYTE *packed_block, char *filename)
{
    BPTR fileHandle;
	char tag[4], packer_tag[4];
    UWORD unpacked_block_size, packed_block_size;
    UBYTE *unpacked_block = NULL, *str_ptr;

    if ((fileHandle = Open(filename, MODE_OLDFILE)))
    {
		Read(fileHandle, &tag, 4);
		if (strncmp(tag, "TXPK", 4) != 0)
		{
			printf("cannot found tag 'TXPK'!\n");
		}
		else
		{
            // Get the original size of the block (before it was packed)
            Read(fileHandle, &unpacked_block_size, 2);
            unpacked_block = (UBYTE *)rpage_c_alloc(unpacked_block_size, sizeof(UBYTE));

            // Get the compression method
            Read(fileHandle, &packer_tag, 4);

            // Look for the packed size
            Read(fileHandle, &tag, 4);
            if (strncmp(tag, "SIZE", 4) == 0)
            {
                // UBYTE *packed_block;
                // Get the size of the block after it was packed
                Read(fileHandle, &packed_block_size, 2);

                // packed_block = (UBYTE *)rpage_c_alloc(packed_block_size, sizeof(UBYTE));
                if (packed_block != NULL)
                {
                    short i;
                    Read(fileHandle, packed_block, packed_block_size);

                    if (strncmp(packer_tag, "MINZ", 4) == 0)
                    {
                        tinfl_decompress_mem_to_mem(unpacked_block, unpacked_block_size, packed_block, packed_block_size, 1);
                    }
                    else if (strncmp(packer_tag, "SHRK", 4) == 0)
                    {
                        ShrinklerDecompress(packed_block, unpacked_block, NULL, NULL);
                    }
                    else if (strncmp(packer_tag, "NRV2", 4) == 0)
                    {
                        nrv2s_unpack(packed_block, unpacked_block);
                    }                

                    // Transfer the content to an array
                    str_ptr = unpacked_block;
                    for(i = 0; i < array_size; i++)
                    {
                        UWORD str_len;
                        str_len = (*str_ptr) << 8 | (*(str_ptr + 1));
                        str_ptr += 2;
                        text_array[i] = str_ptr;
                        str_ptr += str_len;
                    }

                    // // Free the allocated memory
                    // if (packed_block != NULL)
                    //     free(packed_block);
                }
                else
                {
                    printf(err_packed_blk_null);
                }
                
            }
            else
            {
                printf(err_no_size_found);
                printf(", %s\n", tag);
            }
            
        }

        Close(fileHandle);
    }
    else
    {
        printf(err_cannot_open_file, filename);
		return FALSE;
    }
    

    return unpacked_block;
}