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

typedef struct ring_s ring_t;

struct ring_s {
	qr(ring_t) link;
	cw_uint8_t	id;
};

int
main()
{
	cw_uint32_t	i;
	ring_t		entries[NENTRIES], *t;

	libstash_init();
	_cw_out_put("Test begin\n");

	/* Initialize entries. */
	_cw_out_put("qr_new()\n");
	for (i = 0; i < NENTRIES; i++) {
		qr_new(&entries[i], link);
		entries[i].id = 'a' + i;
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
/*  			_cw_out_put("(this:0x[p] prev:0x[p] next:0x[p])", */
/*  			    t, t->link.qre_prev, t->link.qre_next); */
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach_reverse([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_next(&entries[i], link);
		_cw_out_put("  qr_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_prev(&entries[i], link);
		_cw_out_put("  qr_prev([c]): [c]\n", entries[i].id, t->id);
	}

	/* Link the entries together. */
	_cw_out_put("qr_after_insert()\n");
	for (i = 1; i < NENTRIES; i++)
		qr_after_insert(&entries[i - 1], &entries[i], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_next(&entries[i], link);
		_cw_out_put("  qr_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_prev(&entries[i], link);
		_cw_out_put("  qr_prev([c]): [c]\n", entries[i].id, t->id);
	}

	_cw_out_put("qr_remove()\n");
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):        ", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
		_cw_out_put("  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
		qr_remove(&entries[i], link);
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_next(&entries[i], link);
		_cw_out_put("  qr_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_prev(&entries[i], link);
		_cw_out_put("  qr_prev([c]): [c]\n", entries[i].id, t->id);
	}

	/* Link the entries together. */
	_cw_out_put("qr_before_insert()\n");
	for (i = 1; i < NENTRIES; i++)
		qr_before_insert(&entries[i - 1], &entries[i], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_next(&entries[i], link);
		_cw_out_put("  qr_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_prev(&entries[i], link);
		_cw_out_put("  qr_prev([c]): [c]\n", entries[i].id, t->id);
	}

	_cw_out_put("qr_remove()\n");
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):        ", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
		_cw_out_put("  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
		qr_remove(&entries[i], link);
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_next(&entries[i], link);
		_cw_out_put("  qr_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_prev(&entries[i], link);
		_cw_out_put("  qr_prev([c]): [c]\n", entries[i].id, t->id);
	}

	/* meld, split */
	_cw_out_put("qr_split(a, e)\n");
	for (i = 1; i < NENTRIES; i++)
		qr_after_insert(&entries[i - 1], &entries[i], link);
	qr_split(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("qr_meld(a, e)\n");
	qr_meld(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("qr_meld(a, e)\n");
	qr_meld(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("qr_split(a, e)\n");
	qr_split(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("qr_split(a, a)\n");
	qr_split(&entries[0], &entries[0], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("qr_meld(a, a)\n");
	qr_meld(&entries[0], &entries[0], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("qr_split(a, b)\n");
	qr_split(&entries[0], &entries[1], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	_cw_out_put("qr_meld(a, b)\n");
	qr_meld(&entries[0], &entries[1], link);
	for (i = 0; i < NENTRIES; i++) {
		_cw_out_put("  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			_cw_out_put(" [c]", t->id);
		}
		_cw_out_put("\n");
	}
	
	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
