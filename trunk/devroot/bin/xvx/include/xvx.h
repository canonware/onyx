/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include <libstash/libstash_r.h>
#include <libsock/libsock.h>

typedef enum
{
  _XVX_EMPTY = 0,
  _XVX_P1 = 1,
  _XVX_P2 = 2,
  _XVX_P3 = 3,
  _XVX_P4 = 4,
  _XVX_P5 = 5,
  _XVX_P6 = 6
} cw_stone_t;

/* Convert between vertex index and (x, y) coordinates. */
#define _XVX_XY2I(x, y) ((x) + (y) * 19)
#define _XVX_I2X(i) ((i) % 19)
#define _XVX_I2Y(i) ((i) / 19)

#include "move.h"
#include "board.h"
