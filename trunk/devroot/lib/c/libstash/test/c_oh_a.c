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
 * Simple oh test.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#define NUM_STRINGS 5000

int
main()
{
	cw_oh_t hash;
	char  **strings, *junk;

/*   oh_h1_t * h1_ptr; */
	int     i;
	cw_bool_t error;
	cw_sint32_t ins_error;

	libstash_init();
	_cw_out_put("Test begin\n");

	oh_new(&hash);
/*   dbg_register(cw_g_dbg, "oh_slot"); */

	strings = (char **)_cw_malloc(sizeof(char *) * NUM_STRINGS);

	for (i = 0; i < NUM_STRINGS; i++) {
		strings[i] = (char *)_cw_malloc(sizeof(char) * 50);
		_cw_out_put_s(strings[i], "([i]) This is string [i]", i,
		    i);
	}

/*   h1_ptr = oh_get_h1(hash); */
/*   oh_set_h1(hash, new_h1); */

/*   _cw_out_put("<<< Begin first insertion loop >>>\n"); */
	for (i = 0; i < NUM_STRINGS; i++) {
/*     _cw_out_put("<<< Iteration [i] >>>\n", i); */
		ins_error = oh_item_insert(&hash, (void *)strings[i],
		    (void *)&(strings[i]));
		if (ins_error == 1) {
			_cw_out_put("(1) Error at i == [i]\n", i);
			oh_dump(&hash, FALSE);
			exit(1);
		}
	}

/*   _cw_out_put("<<< Begin first deletion loop >>>\n"); */
	for (i = 0; i < (NUM_STRINGS / 2); i++) {
		error = oh_item_delete(&hash, (void *)strings[i],
		    (void **)&junk, (void **)&junk);
		if (error == TRUE) {
			_cw_out_put("(2) Error at i == [i]\n", i);
			oh_dump(&hash, FALSE);
			exit(1);
		}
	}

/*   _cw_out_put("<<< Begin second insertion loop >>>\n"); */
	for (i = 0; i < NUM_STRINGS / 2; i++) {
		ins_error = oh_item_insert(&hash, (void *)strings[i],
		    (void *)&(strings[i]));
		if (ins_error == 1) {
			_cw_out_put("(3) Error at i == [i]\n", i);
			oh_dump(&hash, FALSE);
			exit(1);
		}
	}

/*   _cw_out_put("<<< Begin second deletion loop >>>\n"); */
	for (i = 0; i < NUM_STRINGS; i++) {
		error = oh_item_delete(&hash, (void *)strings[i],
		    (void **)&junk, (void **)&junk);
		if (error == TRUE) {
			_cw_out_put("(4) Error at i == [i]\n", i);
			oh_dump(&hash, FALSE);
			exit(1);
		}
	}

/*   _cw_out_put("<<< Final insertion >>>\n"); */
	ins_error = oh_item_insert(&hash, (void *)strings[0],
	    (void *)&(strings[0]));

	{
		_cw_out_put("Table size: [q]\n",
		    oh_get_size(&hash));
		_cw_out_put("Number of items: [q]\n",
		    oh_get_num_items(&hash));
	}

/*   oh_dump(&hash, FALSE); */

	for (i = 0; i < NUM_STRINGS; i++) {
		_cw_free(strings[i]);
	}
	_cw_free(strings);

	oh_delete(&hash);
	_cw_out_put("Test end\n");
	libstash_shutdown();

	return 0;
}
