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

#include "../include/libonyx/libonyx.h"

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

	libonyx_init();
	fprintf(stderr, "Test begin\n");

	/* Initialize entries. */
	fprintf(stderr, "qs_new(), qs_elm_new()\n");
	qs_new(&head);
	for (i = 0; i < NENTRIES; i++) {
		qs_elm_new(&entries[i], link);
		entries[i].id = 'a' + i;
	}
	fprintf(stderr, "  qs_foreach():");
	qs_foreach(t, &head, link) {
		fprintf(stderr, " %c", t->id);
	}
	fprintf(stderr, "\n");
	if (qs_top(&head) != NULL) {
		fprintf(stderr, "  qs_top(): %c\n", qs_top(&head)->id);
		if (qs_down(qs_top(&head), link) != NULL) {
			fprintf(stderr, "  qs_down(qs_top()): %c\n",
			    qs_down(qs_top(&head), link)->id);
		} else
			fprintf(stderr, "  qs_down(qs_top()): NULL\n");
	} else
		fprintf(stderr, "  qs_top(): NULL\n");

	fprintf(stderr, "qs_push()\n");
	for (i = 0; i < NENTRIES; i++) {
		qs_push(&head, &entries[i], link);
		fprintf(stderr, "  -->Iteration %u\n", i);
		fprintf(stderr, "  qs_foreach():");
		qs_foreach(t, &head, link) {
			fprintf(stderr, " %c", t->id);
		}
		fprintf(stderr, "\n");
		if (qs_top(&head) != NULL) {
			fprintf(stderr, "  qs_top(): %c\n",
			    qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				fprintf(stderr, "  qs_down(qs_top()): %c\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				fprintf(stderr, "  qs_down(qs_top()): NULL\n");
		} else
			fprintf(stderr, "  qs_top(): NULL\n");
	}

	fprintf(stderr, "qs_pop()\n");
	for (i = 0; i < NENTRIES; i++) {
		qs_pop(&head, link);
		fprintf(stderr, "  -->Iteration %u\n", i);
		fprintf(stderr, "  qs_foreach():");
		qs_foreach(t, &head, link) {
			fprintf(stderr, " %c", t->id);
		}
		fprintf(stderr, "\n");
		if (qs_top(&head) != NULL) {
			fprintf(stderr, "  qs_top(): %c\n",
			    qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				fprintf(stderr, "  qs_down(qs_top()): %c\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				fprintf(stderr, "  qs_down(qs_top()): NULL\n");
		} else
			fprintf(stderr, "  qs_top(): NULL\n");
	}

	fprintf(stderr, "qs_under_push()\n");
	qs_push(&head, &entries[0], link);
	for (i = 1; i < NENTRIES; i++) {
		qs_under_push(&entries[0], &entries[i], link);
		fprintf(stderr, "  -->Iteration %u\n", i);
		fprintf(stderr, "  qs_foreach():");
		qs_foreach(t, &head, link) {
			fprintf(stderr, " %c", t->id);
		}
		fprintf(stderr, "\n");
		if (qs_top(&head) != NULL) {
			fprintf(stderr, "  qs_top(): %c\n",
			    qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				fprintf(stderr, "  qs_down(qs_top()): %c\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				fprintf(stderr, "  qs_down(qs_top()): NULL\n");
		} else
			fprintf(stderr, "  qs_top(): NULL\n");
	}

	fprintf(stderr, "qs_pop()\n");
	for (i = 0; i < NENTRIES; i++) {
		qs_pop(&head, link);
		fprintf(stderr, "  -->Iteration %u\n", i);
		fprintf(stderr, "  qs_foreach():");
		qs_foreach(t, &head, link) {
			fprintf(stderr, " %c", t->id);
		}
		fprintf(stderr, "\n");
		if (qs_top(&head) != NULL) {
			fprintf(stderr, "  qs_top(): %c\n",
			    qs_top(&head)->id);
			if (qs_down(qs_top(&head), link) != NULL) {
				fprintf(stderr, "  qs_down(qs_top()): %c\n",
				    qs_down(qs_top(&head), link)->id);
			} else
				fprintf(stderr, "  qs_down(qs_top()): NULL\n");
		} else
			fprintf(stderr, "  qs_top(): NULL\n");
	}

	fprintf(stderr, "Test end\n");
	libonyx_shutdown();
	return 0;
}
