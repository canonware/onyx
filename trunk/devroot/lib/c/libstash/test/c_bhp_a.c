/****************************************************************************
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

#include "../include/libstash/libstash.h"

/* Make sure that nums is at least as large. */
#define _LIBSTASH_TEST_NUM_NODES 100

int
main()
{
	cw_bhpi_t	*bhpi;
	cw_bhp_t	*h;
	cw_sint32_t	i, *a, *b, nums[100] = {
		-944, -522, 410, 296, -523, 828, 573, 611, 878, 936, 604, 527,
		-893, -696, 716, 299, -327, -653, 614, 720, -527, 26, -786,
		-603, 74, -643, 912, -484, -960, -266, -516, -905, 210, 893,
		390, 687, 722, -35, 298, 600, -99, -97, 128, 7, 206, -155, -693,
		879, 190, 921, 599, 663, -52, 813, -939, -978, -830, 972, -462,
		-790, -294, 20, -696, 916, -85, 694, 603, -363, -340, -97, -762,
		559, 805, 365, -432, 11, -789, -125, -108, 401, -204, -509, 65,
		743, -696, 125, 765, -526, 98, -697, -317, 804, 323, -13, 720,
		-762, -318, 324, -125, 340
	};

	libstash_init();
	_cw_out_put("Test begin\n");

	h = bhp_new_r(NULL, cw_g_mem, bhp_sint32_priority_compare);
	_cw_check_ptr(h);

	for (i = 0; i < _LIBSTASH_TEST_NUM_NODES; i++) {
		bhpi = bhpi_new(NULL, cw_g_mem, &(nums[i]), &(nums[i]), NULL,
		    NULL);
		_cw_check_ptr(bhpi);
		bhp_insert(h, bhpi);
	}
	for (i = 0; i < _LIBSTASH_TEST_NUM_NODES; i++)
		_cw_assert(bhp_min_del(h, (void **)&a, (void **)&b) == FALSE);

	for (i = 0; i < _LIBSTASH_TEST_NUM_NODES; i++) {
		bhpi = bhpi_new(NULL, cw_g_mem, &(nums[i]), &(nums[i]), NULL,
		    NULL);
		_cw_check_ptr(bhpi);
		bhp_insert(h, bhpi);
	}
	for (i = 0; i < _LIBSTASH_TEST_NUM_NODES; i++) {
		_cw_assert(bhp_min_del(h, (void **)&a, (void **)&b) == FALSE);
		_cw_out_put("i == [i], size == [q]: [i|s:s], [i|s:s]\n", i,
		    bhp_size_get(h), *a, *b);
	}

	bhp_delete(h);
	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
