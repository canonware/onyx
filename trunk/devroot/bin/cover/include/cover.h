/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include <libstash/libstash.h>

#define _GENERATIONS 1000
#define _POP_SIZE 1000
#define _CROSSOVER_PROBABILITY 50
#define _MUTATE_PROBABILITY_INV 120
#ifndef _SEED
#  define _SEED 101
#endif

#define NUM_NODES 5

#include "gene.h"
#include "matrix.h"
#include "pack.h"
