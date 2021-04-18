/*  Resistance's Portable-Adventure-Game-Engine (R-PAGE), Copyright (C) 2019 François Gutherz, Resistance.no
    Released under MIT License, see license.txt for details.
*/

#ifndef _UTILS_HEADER_
#define _UTILS_HEADER_
#include <string.h>
#include <stdlib.h>
#ifdef LATTICE
#include <exec/types.h>
#include "rpage/aos/inc.prl"
#endif
#include "rpage/coroutine.h"

#ifndef LATTICE
#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef UBYTE
typedef unsigned char UBYTE;
#endif

#ifndef ULONG
typedef unsigned long ULONG;
#endif

#ifndef UWORD
typedef unsigned short UWORD;
#endif

#ifndef BOOL
typedef int BOOL;
#endif
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif

#ifndef max
#define max(x,y) ((x) > (y) ? (x) : (y))
#endif

#define PRINT_LARGEST_CHIP(A) // printf("%d, %s, %s, %d, %d, %d\n", rpage_get_clock(), A, __FILE__, __LINE__, rpage_get_avail_video_memory(), rpage_get_avail_largest_video_memory());

typedef struct
{
    short x, y;
} vec2;

typedef struct
{
    int x, y;
} vec2fp;

typedef struct
{
    vec2 p0, p1, p2, p3;
} poly;

typedef struct
{
    short sx, sy, ex, ey;
} rect;

int fixed_rand(void);

/// calloc wrapper
void *rpage_c_alloc(unsigned long size, unsigned long size_type);
/// allocmem wrapper
void *rpage_os_alloc(unsigned long size, unsigned long ram_mask);

int range_adjust(int val, int in_lower, int in_upper, int out_lower, int out_upper);
int clamp(int x, int in_lower, int in_upper);
BOOL point_within_rect(vec2 *pt, rect *r);
BOOL point_within_quad(vec2 *pt, poly *pl);
BOOL point_within_polygon(vec2 *pt, vec2 *pt_list, unsigned short n_pt);
void rect_grow(rect *r, short step);
void rect_shrink(rect *r, short step);
char* citoa(int num, char* str, int base);
short str_count_delimiters(char *str);
short str_find_delimiter(short start, char *str);
int qsqr(int i);
/// Automatically update the frame, second, minute, hour and day counters.
void update_time_from_frame(unsigned short *seconds, unsigned short *minutes, unsigned short *hours);
ULONG int_sqrt(ULONG s);
#endif
