/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

/* Number of queue entries (<= 26, preferrably). */
#define NENTRIES 8

typedef struct list_s list_t;

struct list_s {
	ql_elm(list_t) link;
	cw_uint8_t	id;
};

int
main()
{
	cw_uint32_t	i;
	list_t		entries[NENTRIES], *t;
	ql_head(list_t) head;

	libstash_init();
	_cw_out_put("Test begin\n");

	/* Initialize entries. */
	_cw_out_put("ql_new()\n");
	ql_new(&head);
	for (i = 0; i < NENTRIES; i++) {
		entries[i].id = 'a' + i;
		ql_elm_new(&entries[i], link);
	}
	if (ql_first(&head) != NULL)
		_cw_out_put("  ql_first(): [c]\n", ql_first(&head)->id);
	else
		_cw_out_put("  ql_first(): NULL\n");
	if (ql_last(&head, link) != NULL)
		_cw_out_put("  ql_last(): [c]\n", ql_last(&head, link)->id);
	else
		_cw_out_put("  ql_last(): NULL\n");
	_cw_out_put("  ql_foreach():");
	ql_foreach(t, &head, link) {
		_cw_out_put(" [c]", t->id);
	}
	_cw_out_put("\n");
	_cw_out_put("  ql_reverse_foreach():");
	ql_reverse_foreach(t, &head, link) {
		_cw_out_put(" [c]", t->id);
	}
	_cw_out_put("\n");
	ql_foreach(t, &head, link) {
		if (ql_next(&head, t, link) != NULL) {
			_cw_out_put("  ql_next([c]): [c]\n", t->id,
			    ql_next(&head, t, link)->id);
		} else
			_cw_out_put("  ql_next([c]): NULL\n", t->id);
	}
	ql_reverse_foreach(t, &head, link) {
		if (ql_prev(&head, t, link) != NULL) {
			_cw_out_put("  ql_prev([c]): [c]\n", t->id,
			    ql_prev(&head, t, link)->id);
		} else
			_cw_out_put("  ql_prev([c]): NULL\n", t->id);
	}
	
	/* Link the entries together. */
	_cw_out_put("ql_tail_insert()\n");
	for (i = 0; i < NENTRIES; i++)
		ql_tail_insert(&head, &entries[i], link);
	if (ql_first(&head) != NULL)
		_cw_out_put("  ql_first(): [c]\n", ql_first(&head)->id);
	else
		_cw_out_put("  ql_first(): NULL\n");
	if (ql_last(&head, link) != NULL)
		_cw_out_put("  ql_last(): [c]\n", ql_last(&head, link)->id);
	else
		_cw_out_put("  ql_last(): NULL\n");
	_cw_out_put("  ql_foreach():");
	ql_foreach(t, &head, link) {
		_cw_out_put(" [c]", t->id);
	}
	_cw_out_put("\n");
	_cw_out_put("  ql_reverse_foreach():");
	ql_reverse_foreach(t, &head, link) {
		_cw_out_put(" [c]", t->id);
	}
	_cw_out_put("\n");
	ql_foreach(t, &head, link) {
		if (ql_next(&head, t, link) != NULL) {
			_cw_out_put("  ql_next([c]): [c]\n", t->id,
			    ql_next(&head, t, link)->id);
		} else
			_cw_out_put("  ql_next([c]): NULL\n", t->id);
	}
	ql_reverse_foreach(t, &head, link) {
		if (ql_prev(&head, t, link) != NULL) {
			_cw_out_put("  ql_prev([c]): [c]\n", t->id,
			    ql_prev(&head, t, link)->id);
		} else
			_cw_out_put("  ql_prev([c]): NULL\n", t->id);
	}

	_cw_out_put("ql_tail_remove()\n");
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("Iteration [i]\n", i);
		if (ql_first(&head) != NULL)
			_cw_out_put("  ql_first(): [c]\n", ql_first(&head)->id);
		else
			_cw_out_put("  ql_first(): NULL\n");
		if (ql_last(&head, link) != NULL) {
			_cw_out_put("  ql_last(): [c]\n", ql_last(&head,
			    link)->id);
		} else
			_cw_out_put("  ql_last(): NULL\n");
		_cw_out_put("  ql_foreach():");
		ql_foreach(t, &head, link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
		ql_tail_remove(&head, list_t, link);
	}
	if (ql_first(&head) != NULL)
		_cw_out_put("  ql_first(): [c]\n", ql_first(&head)->id);
	else
		_cw_out_put("  ql_first(): NULL\n");
	if (ql_last(&head, link) != NULL)
		_cw_out_put("  ql_last(): [c]\n", ql_last(&head, link)->id);
	else
		_cw_out_put("  ql_last(): NULL\n");
	_cw_out_put("  ql_foreach():");
	ql_foreach(t, &head, link) {
		_cw_out_put(" [c]", t->id);
	}
	_cw_out_put("\n");
	_cw_out_put("  ql_reverse_foreach():");
	ql_reverse_foreach(t, &head, link) {
		_cw_out_put(" [c]", t->id);
	}
	_cw_out_put("\n");
	ql_foreach(t, &head, link) {
		if (ql_next(&head, t, link) != NULL) {
			_cw_out_put("  ql_next([c]): [c]\n", t->id,
			    ql_next(&head, t, link)->id);
		} else
			_cw_out_put("  ql_next([c]): NULL\n", t->id);
	}
	ql_reverse_foreach(t, &head, link) {
		if (ql_prev(&head, t, link) != NULL) {
			_cw_out_put("  ql_prev([c]): [c]\n", t->id,
			    ql_prev(&head, t, link)->id);
		} else
			_cw_out_put("  ql_prev([c]): NULL\n", t->id);
	}
#if (0)
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach_reverse([c]):", entries[i].id);
		ql_foreach_reverse(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = ql_next(&entries[i], link);
		_cw_out_put("  ql_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = ql_prev(&entries[i], link);
		_cw_out_put("  ql_prev([c]): [c]\n", entries[i].id, t->id);
	}

	/* Link the entries together. */
	_cw_out_put("ql_insert_before()\n");
	for (i = 1; i < NENTRIES; i++)
		ql_insert_before(&entries[i - 1], &entries[i], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach_reverse([c]):", entries[i].id);
		ql_foreach_reverse(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = ql_next(&entries[i], link);
		_cw_out_put("  ql_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = ql_prev(&entries[i], link);
		_cw_out_put("  ql_prev([c]): [c]\n", entries[i].id, t->id);
	}

	_cw_out_put("ql_remove()\n");
	for (i = 0; i < NENTRIES; i++)
		ql_remove(&entries[i], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach_reverse([c]):", entries[i].id);
		ql_foreach_reverse(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = ql_next(&entries[i], link);
		_cw_out_put("  ql_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = ql_prev(&entries[i], link);
		_cw_out_put("  ql_prev([c]): [c]\n", entries[i].id, t->id);
	}

	/* meld, split */
	_cw_out_put("ql_split(a, e)\n");
	for (i = 1; i < NENTRIES; i++)
		ql_insert_after(&entries[i - 1], &entries[i], link);
	ql_split(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("ql_meld(a, e)\n");
	ql_meld(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("ql_meld(a, e)\n");
	ql_meld(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("ql_split(a, e)\n");
	ql_split(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("ql_split(a, a)\n");
	ql_split(&entries[0], &entries[0], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("ql_meld(a, a)\n");
	ql_meld(&entries[0], &entries[0], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("ql_split(a, b)\n");
	ql_split(&entries[0], &entries[1], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("ql_meld(a, b)\n");
	ql_meld(&entries[0], &entries[1], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  ql_foreach([c]):", entries[i].id);
		ql_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
#endif
	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
