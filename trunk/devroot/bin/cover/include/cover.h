/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include <libonyx/libonyx.h>

#define CW_GENERATIONS 1000
#define CW_POP_SIZE 1000
#define CW_CROSSOVER_PROBABILITY 50
#define CW_MUTATE_PROBABILITY_INV 120
#ifndef CW_SEED
#  define CW_SEED 101
#endif

#define NUM_NODES 5

#include "cover_defs.h"

#include "gene.h"
#include "matrix.h"
#include "pack.h"
