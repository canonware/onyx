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
	_cw_out_put("Test begin\n");

	/* Initialize entries. */
	_cw_out_put("qs_new(), qs_elm_new()\n");
	qs_new(&head);
	for (i = 0; i < NENTRIES; i++) {
		qs_elm_new(&entries[i], link);
		entries[i].id = 'a' + i;
	}
	_cw_out_put("  qs_foreach():");
	qs_foreach(t, &head, link) {
		_cw_out_put(" [c]", t->id);
/*  		_cw_out_put("(this:0x[p] prev:0x[p] next:0x[p])", */
/*  		    t, t->link.qse_prev, t->link.qse_next); */
	}
	_cw_out_put("\n");
	if (qs_top(&head) != NULL) {
		_cw_out_put("  qs_top(): [c]\n", qs_top(&head)->id);
		if (qs_down(qs_top(&head), link) != NULL) {
			_cw_out_put("  qs_down(qs_top()): [c]\n",
			    qs_down(qs_top(&head), link)->id);
		} else
			_cw_out_put("  qs_down(qs_top()): NULL\n");
	} else
		_cw_out_put("  qs_top(): NULL\n");

	_cw_out_put("qs_push()\n");
	for (i = 0; i < NENTRIES; i++) {
		qs_push(&head, &entries[i], link);
		_cw_out_put("  -->Iteration [i]\n", i);
		_cw_out_put("  qs_foreach():");
		qs_foreach(t, &head, link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
		if (qs_top(&head) != NULL) {
			_cw_out_put("  qs_top(): [c]\n", qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				_cw_out_put("  qs_down(qs_top()): [c]\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				_cw_out_put("  qs_down(qs_top()): NULL\n");
		} else
			_cw_out_put("  qs_top(): NULL\n");
	}

	_cw_out_put("qs_pop()\n");
	for (i = 0; i < NENTRIES; i++) {
		qs_pop(&head, link);
		_cw_out_put("  -->Iteration [i]\n", i);
		_cw_out_put("  qs_foreach():");
		qs_foreach(t, &head, link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
		if (qs_top(&head) != NULL) {
			_cw_out_put("  qs_top(): [c]\n", qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				_cw_out_put("  qs_down(qs_top()): [c]\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				_cw_out_put("  qs_down(qs_top()): NULL\n");
		} else
			_cw_out_put("  qs_top(): NULL\n");
	}

	_cw_out_put("qs_under_push()\n");
	qs_push(&head, &entries[0], link);
	for (i = 1; i < NENTRIES; i++) {
		qs_under_push(&entries[0], &entries[i], link);
		_cw_out_put("  -->Iteration [i]\n", i);
		_cw_out_put("  qs_foreach():");
		qs_foreach(t, &head, link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
		if (qs_top(&head) != NULL) {
			_cw_out_put("  qs_top(): [c]\n", qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				_cw_out_put("  qs_down(qs_top()): [c]\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				_cw_out_put("  qs_down(qs_top()): NULL\n");
		} else
			_cw_out_put("  qs_top(): NULL\n");
	}

	_cw_out_put("qs_pop()\n");
	for (i = 0; i < NENTRIES; i++) {
		qs_pop(&head, link);
		_cw_out_put("  -->Iteration [i]\n", i);
		_cw_out_put("  qs_foreach():");
		qs_foreach(t, &head, link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
		if (qs_top(&head) != NULL) {
			_cw_out_put("  qs_top(): [c]\n", qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				_cw_out_put("  qs_down(qs_top()): [c]\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				_cw_out_put("  qs_down(qs_top()): NULL\n");
		} else
			_cw_out_put("  qs_top(): NULL\n");
	}

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
