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
	out_put(out_err, "Test begin\n");

	/* Initialize entries. */
	out_put(out_err, "qr_new()\n");
	for (i = 0; i < NENTRIES; i++) {
		qr_new(&entries[i], link);
		entries[i].id = 'a' + i;
	}
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
/*  			out_put(out_err, "(this:0x[p] prev:0x[p] next:0x[p])", */
/*  			    t, t->link.qre_prev, t->link.qre_next); */
		}
		out_put(out_err, "\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach_reverse([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_next(&entries[i], link);
		out_put(out_err, "  qr_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_prev(&entries[i], link);
		out_put(out_err, "  qr_prev([c]): [c]\n", entries[i].id, t->id);
	}

	/* Link the entries together. */
	out_put(out_err, "qr_after_insert()\n");
	for (i = 1; i < NENTRIES; i++)
		qr_after_insert(&entries[i - 1], &entries[i], link);
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_next(&entries[i], link);
		out_put(out_err, "  qr_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_prev(&entries[i], link);
		out_put(out_err, "  qr_prev([c]): [c]\n", entries[i].id, t->id);
	}

	out_put(out_err, "qr_remove()\n");
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):        ", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
		out_put(out_err, "  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
		qr_remove(&entries[i], link);
	}
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_next(&entries[i], link);
		out_put(out_err, "  qr_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_prev(&entries[i], link);
		out_put(out_err, "  qr_prev([c]): [c]\n", entries[i].id, t->id);
	}

	/* Link the entries together. */
	out_put(out_err, "qr_before_insert()\n");
	for (i = 1; i < NENTRIES; i++)
		qr_before_insert(&entries[i - 1], &entries[i], link);
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_next(&entries[i], link);
		out_put(out_err, "  qr_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_prev(&entries[i], link);
		out_put(out_err, "  qr_prev([c]): [c]\n", entries[i].id, t->id);
	}

	out_put(out_err, "qr_remove()\n");
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):        ", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
		out_put(out_err, "  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
		qr_remove(&entries[i], link);
	}
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_reverse_foreach([c]):", entries[i].id);
		qr_reverse_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_next(&entries[i], link);
		out_put(out_err, "  qr_next([c]): [c]\n", entries[i].id, t->id);
	}
	for (i = 0; i < NENTRIES; i++) {
		t = qr_prev(&entries[i], link);
		out_put(out_err, "  qr_prev([c]): [c]\n", entries[i].id, t->id);
	}

	/* meld, split */
	out_put(out_err, "qr_split(a, e)\n");
	for (i = 1; i < NENTRIES; i++)
		qr_after_insert(&entries[i - 1], &entries[i], link);
	qr_split(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	out_put(out_err, "qr_meld(a, e)\n");
	qr_meld(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	out_put(out_err, "qr_meld(a, e)\n");
	qr_meld(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	out_put(out_err, "qr_split(a, e)\n");
	qr_split(&entries[0], &entries[4], link);
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	out_put(out_err, "qr_split(a, a)\n");
	qr_split(&entries[0], &entries[0], link);
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	out_put(out_err, "qr_meld(a, a)\n");
	qr_meld(&entries[0], &entries[0], link);
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	out_put(out_err, "qr_split(a, b)\n");
	qr_split(&entries[0], &entries[1], link);
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	out_put(out_err, "qr_meld(a, b)\n");
	qr_meld(&entries[0], &entries[1], link);
	for (i = 0; i < NENTRIES; i++) {
		out_put(out_err, "  qr_foreach([c]):", entries[i].id);
		qr_foreach(t, &entries[i], link) {
			out_put(out_err, " [c]", t->id);
		}
		out_put(out_err, "\n");
	}
	
	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
