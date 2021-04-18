/* NRV2X
*/

#ifdef LATTICE
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <libraries/dos.h>
#include <dos/rdargs.h>
#include <utility/tagitem.h>
#include <devices/audio.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <string.h>
#include <hardware/cia.h>

extern __asm void nrv2s_unpack(	register __a0 UBYTE *Source, /* Input buffer to be decompressed */
								register __a1 UBYTE *Destination); /* Output buffer. */ 

 #endif