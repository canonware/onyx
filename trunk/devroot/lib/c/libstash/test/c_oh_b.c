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
 * Multi-threded oh test.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#define NUM_STRINGS 20
#define NUM_THREADS 50

struct foo_s {
	cw_oh_t *hash;
	cw_uint32_t thread_num;
};

void   *
insert_items(void *a_arg)
{
	cw_uint32_t i;
	char   *string;
	struct foo_s *foo_var = (struct foo_s *)a_arg;

	for (i = 0; i < NUM_STRINGS; i++) {
		string = (char *)_cw_malloc(40);
		_cw_out_put_s(string, "thread [i], string [i]",
		    foo_var->thread_num, i);
		_cw_assert(0 == oh_item_insert(foo_var->hash,
			(void *)string, (void *)string));
/*     out_put_e(cw_g_out, NULL, 0, "insert_items", */
/* 		"thread [i], end iteration [i]\n", foo_var->thread_num, i); */
	}

	_cw_free(a_arg);

	return NULL;
}

int
main()
{
	cw_oh_t *hash;
	cw_thd_t threads[NUM_THREADS];
	cw_uint32_t i;
	struct foo_s *foo_var;
	cw_rwl_t lock;

	libstash_init();
	_cw_out_put("Test begin\n");
	hash = oh_new_r(NULL);
	rwl_new(&lock);

	for (i = 0; i < NUM_THREADS; i++) {
		foo_var = (struct foo_s *)_cw_malloc(sizeof(struct foo_s));

		foo_var->hash = hash;
		foo_var->thread_num = i;

		thd_new(&threads[i], insert_items, (void *)foo_var);
/*     _cw_out_put("Got to end of for loop, i == [i]\n", i); */
	}

	/* Join on threads. */
	for (i = 0; i < NUM_THREADS; i++) {
		thd_join(&threads[i]);
	}

	_cw_out_put("Number of items in hash table: [q]\n",
	    oh_get_num_items(hash));

	/* Delete all the strings. */
	{
		cw_uint64_t i, num_strings;
		void   *string, *junk;

		for (i = 0, num_strings = oh_get_num_items(hash);
		    i < num_strings;
		    i++) {
			_cw_assert(FALSE == oh_item_delete_iterate(hash,
			    &string, &junk));
			_cw_free(string);
		}
	}

	rwl_delete(&lock);
	oh_delete(hash);
	_cw_out_put("Test end\n");
	libstash_shutdown();

	return 0;
}
