/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
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

#define _LIBSTASH_USE_BHP
#include <libstash/libstash_r.h>

/* Make sure that nums is at least as large. */
#define _LIBSTASH_TEST_NUM_NODES 100

int
main()
{
  char buf[21];
  cw_bhpi_t * bhpi;
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
  log_printf(cw_g_log, "Test begin\n");

  h = bhp_new(NULL, bhp_priority_compare_sint32, TRUE);
  _cw_check_ptr(h);

  for (i = 0; i < _LIBSTASH_TEST_NUM_NODES; i++)
  {
    bhpi = bhpi_new(NULL, &(nums[i]), &(nums[i]), NULL, NULL);
    _cw_check_ptr(bhpi);
    bhp_insert(h, bhpi);
  }
  for (i = 0; i < _LIBSTASH_TEST_NUM_NODES; i++)
  {
    _cw_assert(FALSE == bhp_del_min(h, (void **) &a, (void **) &b));
  }

  for (i = 0; i < _LIBSTASH_TEST_NUM_NODES; i++)
  {
    bhpi = bhpi_new(NULL, &(nums[i]), &(nums[i]), NULL, NULL);
    _cw_check_ptr(bhpi);
    bhp_insert(h, bhpi);
  }
  for (i = 0; i < _LIBSTASH_TEST_NUM_NODES; i++)
  {
    _cw_assert(FALSE == bhp_del_min(h, (void **) &a, (void **) &b));
    log_printf(cw_g_log, "i == %d, size == %s: %d, %d\n",
	       i, log_print_uint64(bhp_get_size(h), 10, buf), *a, *b);
  }

  bhp_delete(h);
  log_printf(cw_g_log, "Test end\n");
  libstash_shutdown();
  return 0;
}
