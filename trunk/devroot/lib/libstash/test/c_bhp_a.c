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
 * <<< Description >>>
 *
 * bhp test.
 *
 ****************************************************************************/

#define _STASH_USE_BHP
#include <libstash/libstash_r.h>

/* Make sure that nums is at least as large. */
#define _STASH_TEST_NUM_NODES 10

/****************************************************************************
 *
 * Priority comparison function that compares two signed 32 bit integers.
 *
 ****************************************************************************/
cw_sint32_t
prio_comp(void * a_a, void * a_b)
{
  cw_sint32_t retval,
    a = *(cw_sint32_t *) a_a,
    b = *(cw_sint32_t *) a_b;


  if (a > b)
  {
    retval = 1;
  }
  else if (a < b)
  {
    retval = -1;
  }
  else
  {
    retval = 0;
  }

/*    log_eprintf(g_log, NULL, 0, "prio_comp", */
/*  	      "%d %c %d (%d)\n", */
/*  	      a, !retval ? '=' : retval > 0 ? '>' : '<', b, retval); */
  
  return retval;
}

int
main()
{
  cw_bhp_t * h;
  cw_sint32_t i, * a, * b, nums[100] = 
  {
    -944, -522, 410, 296, -523, 828, 573, 611, 878, 936, 604, 527, -893, -696,
    716, 299, -327, -653, 614, 720, -527, 26, -786, -603, 74, -643, 912, -484,
    -960, -266, -516, -905, 210, 893, 390, 687, 722, -35, 298, 600, -99, -97,
    128, 7, 206, -155, -693, 879, 190, 921, 599, 663, -52, 813, -939, -978,
    -830, 972, -462, -790, -294, 20, -696, 916, -85, 694, 603, -363, -340, -97,
    -762, 559, 805, 365, -432, 11, -789, -125, -108, 401, -204, -509, 65, 743,
    -696, 125, 765, -526, 98, -697, -317, 804, 323, -13, 720, -762, -318, 324,
    -125, 340
  };
  
  libstash_init();

  h = bhp_new(NULL, TRUE);
  _cw_check_ptr(h);

  bhp_set_priority_compare(h, prio_comp);

  /* Insert all _STASH_TEST_NUM_NODES numbers into the heap. */
  for (i = 0; i < _STASH_TEST_NUM_NODES; i++)
  {
    bhp_insert(h, &(nums[i]), &(nums[i]));
    bhp_dump(h);
    log_printf(g_log, "===========================\n");
  }

  for (i = 0; i < _STASH_TEST_NUM_NODES; i++)
  {
    _cw_assert(FALSE == bhp_del_min(h, (void **) &a, (void **) &b));
    log_printf(g_log, "i == %d, size == %d: %d, %d\n",
	       i, bhp_get_size(h), *a, *b);
  }
  
  libstash_shutdown();
  return 0;
}
