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

/* Number of ring entries (<= 26, preferrably). */
#define NENTRIES 8

typedef struct qstack_s qstack_t;

struct qstack_s {
	qs_elm(qstack_t) link;
	cw_uint8_t	id;
};

int
main()
{
	cw_uint32_t	i;
	qstack_t	entries[NENTRIES], *t;
	qs_head(qstack_t) head;

	libstash_init();
	out_put(out_err, "Test begin\n");

	/* Initialize entries. */
	out_put(out_err, "qs_new(), qs_elm_new()\n");
	qs_new(&head);
	for (i = 0; i < NENTRIES; i++) {
		qs_elm_new(&entries[i], link);
		entries[i].id = 'a' + i;
	}
	out_put(out_err, "  qs_foreach():");
	qs_foreach(t, &head, link) {
		out_put(out_err, " [c]", t->id);
/*  		out_put(out_err, "(this:0x[p] prev:0x[p] next:0x[p])", */
/*  		    t, t->link.qse_prev, t->link.qse_next); */
	}
	out_put(out_err, "\n");
	if (qs_top(&head) != NULL) {
		out_put(out_err, "  qs_top(): [c]\n", qs_top(&head)->id);
		if (qs_down(qs_top(&head), link) != NULL) {
			out_put(out_err, "  qs_down(qs_top()): [c]\n",
			    qs_down(qs_top(&head), link)->id);
		} else
			out_put(out_err, "  qs_down(qs_top()): NULL\n");
	} else
		out_put(out_err, "  qs_top(): NULL\n");

	out_put(out_err, "qs_push()\n");
	for (i = 0; i < NENTRIES; i++) {
		qs_push(&head, &entries[i], link);
		out_put(out_err, "  -->Iteration [i]\n", i);
		out_put(out_err, "  qs_foreach():");
		qs_foreach(t, &head, link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
		if (qs_top(&head) != NULL) {
			out_put(out_err, "  qs_top(): [c]\n",
			    qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				out_put(out_err, "  qs_down(qs_top()): [c]\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				out_put(out_err, "  qs_down(qs_top()): NULL\n");
		} else
			out_put(out_err, "  qs_top(): NULL\n");
	}

	out_put(out_err, "qs_pop()\n");
	for (i = 0; i < NENTRIES; i++) {
		qs_pop(&head, link);
		out_put(out_err, "  -->Iteration [i]\n", i);
		out_put(out_err, "  qs_foreach():");
		qs_foreach(t, &head, link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
		if (qs_top(&head) != NULL) {
			out_put(out_err, "  qs_top(): [c]\n",
			    qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				out_put(out_err, "  qs_down(qs_top()): [c]\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				out_put(out_err, "  qs_down(qs_top()): NULL\n");
		} else
			out_put(out_err, "  qs_top(): NULL\n");
	}

	out_put(out_err, "qs_under_push()\n");
	qs_push(&head, &entries[0], link);
	for (i = 1; i < NENTRIES; i++) {
		qs_under_push(&entries[0], &entries[i], link);
		out_put(out_err, "  -->Iteration [i]\n", i);
		out_put(out_err, "  qs_foreach():");
		qs_foreach(t, &head, link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
		if (qs_top(&head) != NULL) {
			out_put(out_err, "  qs_top(): [c]\n",
			    qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				out_put(out_err, "  qs_down(qs_top()): [c]\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				out_put(out_err, "  qs_down(qs_top()): NULL\n");
		} else
			out_put(out_err, "  qs_top(): NULL\n");
	}

	out_put(out_err, "qs_pop()\n");
	for (i = 0; i < NENTRIES; i++) {
		qs_pop(&head, link);
		out_put(out_err, "  -->Iteration [i]\n", i);
		out_put(out_err, "  qs_foreach():");
		qs_foreach(t, &head, link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
		if (qs_top(&head) != NULL) {
			out_put(out_err, "  qs_top(): [c]\n",
			    qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				out_put(out_err, "  qs_down(qs_top()): [c]\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				out_put(out_err, "  qs_down(qs_top()): NULL\n");
		} else
			out_put(out_err, "  qs_top(): NULL\n");
	}

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
