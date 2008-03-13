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
	out_put(out_err, "Test begin\n");

	/* Initialize entries. */
	out_put(out_err, "ql_new()\n");
	ql_new(&head);
	for (i = 0; i < NENTRIES; i++) {
		entries[i].id = 'a' + i;
		ql_elm_new(&entries[i], link);
	}
	if (ql_first(&head) != NULL)
		out_put(out_err, "  ql_first(): [c]\n", ql_first(&head)->id);
	else
		out_put(out_err, "  ql_first(): NULL\n");
	if (ql_last(&head, link) != NULL) {
		out_put(out_err, "  ql_last(): [c]\n", ql_last(&head,
		    link)->id);
	} else
		out_put(out_err, "  ql_last(): NULL\n");
	out_put(out_err, "  ql_foreach():");
	ql_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	out_put(out_err, "  ql_reverse_foreach():");
	ql_reverse_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	ql_foreach(t, &head, link) {
		if (ql_next(&head, t, link) != NULL) {
			out_put(out_err, "  ql_next([c]): [c]\n", t->id,
			    ql_next(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_next([c]): NULL\n", t->id);
	}
	ql_reverse_foreach(t, &head, link) {
		if (ql_prev(&head, t, link) != NULL) {
			out_put(out_err, "  ql_prev([c]): [c]\n", t->id,
			    ql_prev(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_prev([c]): NULL\n", t->id);
	}

	/* Link the entries together. */
	out_put(out_err, "ql_tail_insert()\n");
	for (i = 0; i < NENTRIES; i++)
		ql_tail_insert(&head, &entries[i], link);
	if (ql_first(&head) != NULL)
		out_put(out_err, "  ql_first(): [c]\n", ql_first(&head)->id);
	else
		out_put(out_err, "  ql_first(): NULL\n");
	if (ql_last(&head, link) != NULL) {
		out_put(out_err, "  ql_last(): [c]\n", ql_last(&head,
		    link)->id);
	} else
		out_put(out_err, "  ql_last(): NULL\n");
	out_put(out_err, "  ql_foreach():");
	ql_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	out_put(out_err, "  ql_reverse_foreach():");
	ql_reverse_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	ql_foreach(t, &head, link) {
		if (ql_next(&head, t, link) != NULL) {
			out_put(out_err, "  ql_next([c]): [c]\n", t->id,
			    ql_next(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_next([c]): NULL\n", t->id);
	}
	ql_reverse_foreach(t, &head, link) {
		if (ql_prev(&head, t, link) != NULL) {
			out_put(out_err, "  ql_prev([c]): [c]\n", t->id,
			    ql_prev(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_prev([c]): NULL\n", t->id);
	}

	out_put(out_err, "ql_tail_remove()\n");
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  --> Iteration [i]\n", i);
		if (ql_first(&head) != NULL) {
			out_put(out_err, "  ql_first(): [c]\n",
			    ql_first(&head)->id);
		} else
			out_put(out_err, "  ql_first(): NULL\n");
		if (ql_last(&head, link) != NULL) {
			out_put(out_err, "  ql_last(): [c]\n", ql_last(&head,
			    link)->id);
		} else
			out_put(out_err, "  ql_last(): NULL\n");
		out_put(out_err, "  ql_foreach():");
		ql_foreach(t, &head, link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
		ql_tail_remove(&head, list_t, link);
	}
	if (ql_first(&head) != NULL)
		out_put(out_err, "  ql_first(): [c]\n", ql_first(&head)->id);
	else
		out_put(out_err, "  ql_first(): NULL\n");
	if (ql_last(&head, link) != NULL) {
		out_put(out_err, "  ql_last(): [c]\n", ql_last(&head,
		    link)->id);
	} else
		out_put(out_err, "  ql_last(): NULL\n");
	out_put(out_err, "  ql_foreach():");
	ql_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	out_put(out_err, "  ql_reverse_foreach():");
	ql_reverse_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	ql_foreach(t, &head, link) {
		if (ql_next(&head, t, link) != NULL) {
			out_put(out_err, "  ql_next([c]): [c]\n", t->id,
			    ql_next(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_next([c]): NULL\n", t->id);
	}
	ql_reverse_foreach(t, &head, link) {
		if (ql_prev(&head, t, link) != NULL) {
			out_put(out_err, "  ql_prev([c]): [c]\n", t->id,
			    ql_prev(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_prev([c]): NULL\n", t->id);
	}

	/* Link the entries together. */
	out_put(out_err, "ql_head_insert()\n");
	for (i = 0; i < NENTRIES; i++)
		ql_head_insert(&head, &entries[i], link);
	if (ql_first(&head) != NULL)
		out_put(out_err, "  ql_first(): [c]\n", ql_first(&head)->id);
	else
		out_put(out_err, "  ql_first(): NULL\n");
	if (ql_last(&head, link) != NULL) {
		out_put(out_err, "  ql_last(): [c]\n", ql_last(&head,
		    link)->id);
	} else
		out_put(out_err, "  ql_last(): NULL\n");
	out_put(out_err, "  ql_foreach():");
	ql_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	out_put(out_err, "  ql_reverse_foreach():");
	ql_reverse_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	ql_foreach(t, &head, link) {
		if (ql_next(&head, t, link) != NULL) {
			out_put(out_err, "  ql_next([c]): [c]\n", t->id,
			    ql_next(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_next([c]): NULL\n", t->id);
	}
	ql_reverse_foreach(t, &head, link) {
		if (ql_prev(&head, t, link) != NULL) {
			out_put(out_err, "  ql_prev([c]): [c]\n", t->id,
			    ql_prev(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_prev([c]): NULL\n", t->id);
	}

	out_put(out_err, "ql_head_remove()\n");
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  --> Iteration [i]\n", i);
		if (ql_first(&head) != NULL) {
			out_put(out_err, "  ql_first(): [c]\n",
			    ql_first(&head)->id);
		} else
			out_put(out_err, "  ql_first(): NULL\n");
		if (ql_last(&head, link) != NULL) {
			out_put(out_err, "  ql_last(): [c]\n", ql_last(&head,
			    link)->id);
		} else
			out_put(out_err, "  ql_last(): NULL\n");
		out_put(out_err, "  ql_foreach():");
		ql_foreach(t, &head, link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
		ql_head_remove(&head, list_t, link);
	}
	if (ql_first(&head) != NULL)
		out_put(out_err, "  ql_first(): [c]\n", ql_first(&head)->id);
	else
		out_put(out_err, "  ql_first(): NULL\n");
	if (ql_last(&head, link) != NULL) {
		out_put(out_err, "  ql_last(): [c]\n", ql_last(&head,
		    link)->id);
	} else
		out_put(out_err, "  ql_last(): NULL\n");
	out_put(out_err, "  ql_foreach():");
	ql_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	out_put(out_err, "  ql_reverse_foreach():");
	ql_reverse_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	ql_foreach(t, &head, link) {
		if (ql_next(&head, t, link) != NULL) {
			out_put(out_err, "  ql_next([c]): [c]\n", t->id,
			    ql_next(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_next([c]): NULL\n", t->id);
	}
	ql_reverse_foreach(t, &head, link) {
		if (ql_prev(&head, t, link) != NULL) {
			out_put(out_err, "  ql_prev([c]): [c]\n", t->id,
			    ql_prev(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_prev([c]): NULL\n", t->id);
	}

	/*
	 * ql_remove(), ql_before_insert(), and ql_after_insert() are used
	 * internally by other macros that are already tested, so there's no
	 * need to test them completely.  However, insertion/deletion from the
	 * middle of lists is not tested, so do that here.
	 */
	out_put(out_err, "ql_tail_insert(), ql_before_insert(),"
	    " ql_after_insert()\n");
	ql_tail_insert(&head, &entries[0], link);
	ql_before_insert(&head, &entries[0], &entries[1], link);
	ql_before_insert(&head, &entries[0], &entries[2], link);
	ql_after_insert(&entries[0], &entries[3], link);
	ql_after_insert(&entries[0], &entries[4], link);
	ql_before_insert(&head, &entries[1], &entries[5], link);
	ql_after_insert(&entries[2], &entries[6], link);
	ql_before_insert(&head, &entries[3], &entries[7], link);

	if (ql_first(&head) != NULL)
		out_put(out_err, "  ql_first(): [c]\n", ql_first(&head)->id);
	else
		out_put(out_err, "  ql_first(): NULL\n");
	if (ql_last(&head, link) != NULL) {
		out_put(out_err, "  ql_last(): [c]\n", ql_last(&head,
		    link)->id);
	} else
		out_put(out_err, "  ql_last(): NULL\n");
	out_put(out_err, "  ql_foreach():");
	ql_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	out_put(out_err, "  ql_reverse_foreach():");
	ql_reverse_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
	}
	out_put(out_err, "\n");
	ql_foreach(t, &head, link) {
		if (ql_next(&head, t, link) != NULL) {
			out_put(out_err, "  ql_next([c]): [c]\n", t->id,
			    ql_next(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_next([c]): NULL\n", t->id);
	}
	ql_reverse_foreach(t, &head, link) {
		if (ql_prev(&head, t, link) != NULL) {
			out_put(out_err, "  ql_prev([c]): [c]\n", t->id,
			    ql_prev(&head, t, link)->id);
		} else
			out_put(out_err, "  ql_prev([c]): NULL\n", t->id);
	}

	out_put(out_err, "ql_remove()\n");
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  --> Iteration [i]\n", i);
		ql_remove(&head, &entries[i], link);
		if (ql_first(&head) != NULL) {
			out_put(out_err, "  ql_first(): [c]\n",
			    ql_first(&head)->id);
		} else
			out_put(out_err, "  ql_first(): NULL\n");
		if (ql_last(&head, link) != NULL) {
			out_put(out_err, "  ql_last(): [c]\n", ql_last(&head,
			    link)->id);
		} else
			out_put(out_err, "  ql_last(): NULL\n");
		out_put(out_err, "  ql_foreach():");
		ql_foreach(t, &head, link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
