/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Slate <Version = slate>
 *
 ******************************************************************************/

#include "../include/modslate.h"

typedef struct node_s node_t;

struct node_s
{
    rb_node(node_t) link;
    cw_uint32_t key;
};

void
tree_dump(node_t *a_node, node_t *a_nil)
{
    /* Self. */
    fprintf(stderr, "%u%c", a_node->key, a_node->link.rbn_red ? 'r' : 'b');

    /* Left subtree. */
    if (a_node->link.rbn_left != a_nil)
    {
	fprintf(stderr, "[");
	tree_dump(a_node->link.rbn_left, a_nil);
	fprintf(stderr, "]");
    }
    else
    {
	fprintf(stderr, ".");
    }

    /* Right subtree. */
    if (a_node->link.rbn_right != a_nil)
    {
	fprintf(stderr, "[");
	tree_dump(a_node->link.rbn_right, a_nil);
	fprintf(stderr, "]");
    }
    else
    {
	fprintf(stderr, ".");
    }
}

cw_sint32_t
node_comp(node_t *a_a, node_t *a_b)
{
    cw_sint32_t retval;

    if (a_a->key < a_b->key)
    {
	retval = -1;
    }
    else if (a_a->key == a_b->key)
    {
	retval = 0;
    }
    else
    {
	retval = 1;
    }

    return retval;
}

int
main()
{
    rb_tree(node_t) tree;
#define NNODES 16
    node_t nodes[NNODES], key, *node_a, *node_b;
#define NSETS 5
    cw_uint32_t set[NSETS][NNODES] =
	{
	    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
	    {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
	    {2, 4, 6, 8, 10, 12, 14, 16, 2, 4, 6, 8, 10, 12, 14, 16},
	    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	    {1, 2, 3, 4, 5, 6, 7, 8, 16, 15, 14, 13, 12, 11, 10, 9}
	};
    cw_uint32_t i, j;

    libonyx_init();
    fprintf(stderr, "Test begin\n");

    /* Initialize tree and nodes. */
    rb_tree_new(&tree, link);
    for (i = 0; i < NNODES; i++)
    {
	rb_node_new(&tree, &nodes[i], link);
	nodes[i].key = i;
    }

    /*
     * Empty tree.
     */
    fprintf(stderr, "Empty tree:\n");
    /* rb_root(). */
    if (rb_root(&tree) == rb_tree_nil(&tree, link))
    {
	fprintf(stderr, "rb_root() --> nil\n");
    }
    else
    {
	fprintf(stderr, "rb_root() --> %u\n", rb_root(&tree)->key);
    }

    /* rb_first(). */
    rb_first(&tree, rb_root(&tree), link, node_a);
    if (node_a == rb_tree_nil(&tree, link))
    {
	fprintf(stderr, "rb_first() --> nil\n");
    }
    else
    {
	fprintf(stderr, "rb_first() --> %u\n", node_a->key);
    }

    /* rb_last(). */
    rb_last(&tree, rb_root(&tree), link, node_a);
    if (node_a == rb_tree_nil(&tree, link))
    {
	fprintf(stderr, "rb_last() --> nil\n");
    }
    else
    {
	fprintf(stderr, "rb_last() --> %u\n", node_a->key);
    }

    /* rb_next(). */
    rb_first(&tree, rb_root(&tree), link, node_b);
    rb_next(&tree, node_b, node_t, link, node_a);
    if (node_a == rb_tree_nil(&tree, link))
    {
	fprintf(stderr, "rb_next() --> nil\n");
    }
    else
    {
	fprintf(stderr, "rb_next() --> %u\n", node_a->key);
    }

    /* rb_prev(). */
    rb_first(&tree, rb_root(&tree), link, node_b);
    rb_prev(&tree, node_b, node_t, link, node_a);
    if (node_a == rb_tree_nil(&tree, link))
    {
	fprintf(stderr, "rb_prev() --> nil\n");
    }
    else
    {
	fprintf(stderr, "rb_prev() --> %u\n", node_a->key);
    }

    /* rb_search(). */
    key.key = 0;
    rb_search(&tree, &key, node_comp, link, node_a);
    if (node_a == rb_tree_nil(&tree, link))
    {
	fprintf(stderr, "rb_search(0) --> nil\n");
    }
    else
    {
	fprintf(stderr, "rb_search(0) --> %u\n", node_a->key);
    }

    /* rb_insert(). */
    for (i = 0; i < NSETS; i++)
    {
	/* Initialize tree and nodes. */
	rb_tree_new(&tree, link);
	for (j = 0; j < NNODES; j++)
	{
	    rb_node_new(&tree, &nodes[j], link);
	    nodes[j].key = set[i][j];
	}

	fprintf(stderr, "Empty\n");

	/* Insert nodes. */
	for (j = 0; j < NNODES; j++)
	{
	    fprintf(stderr, "rb_insert(%2u) --> ", nodes[j].key);
	    rb_insert(&tree, &nodes[j], node_comp, node_t, link);

	    tree_dump(rb_root(&tree), rb_tree_nil(&tree, link));
	    fprintf(stderr, "\n");
	}

	/* Print in order. */
	fprintf(stderr, "rb_first()/rb_next():");
	rb_first(&tree, rb_root(&tree), link, node_b);
	do
	{
	    /* Test rb_search(). */
	    key.key = node_b->key;
	    rb_search(&tree, &key, node_comp, link, node_a);
	    cw_assert(node_a != rb_tree_nil(&tree, link));
	    cw_assert(node_a->key == key.key);

	    node_a = node_b;
	    fprintf(stderr, " %u", node_a->key);
	    rb_next(&tree, node_a, node_t, link, node_b);
	} while(node_b != rb_tree_nil(&tree, link));
	fprintf(stderr, "\n");

	/* Print in reverse order. */
	fprintf(stderr, "rb_last()/rb_prev():");
	rb_last(&tree, rb_root(&tree), link, node_b);
	do
	{
	    /* Test rb_search(). */
	    key.key = node_b->key;
	    rb_search(&tree, &key, node_comp, link, node_a);
	    cw_assert(node_a != rb_tree_nil(&tree, link));
	    cw_assert(node_a->key == key.key);

	    node_a = node_b;
	    fprintf(stderr, " %u", node_a->key);
	    rb_prev(&tree, node_a, node_t, link, node_b);
	} while(node_b != rb_tree_nil(&tree, link));
	fprintf(stderr, "\n");

	/* Remove nodes. */
	for (j = 0; j < NNODES; j++)
	{
	    fprintf(stderr, "rb_remove(%2u) --> ", nodes[j].key);
	    rb_remove(&tree, &nodes[j], node_t, link);

	    tree_dump(rb_root(&tree), rb_tree_nil(&tree, link));
	    fprintf(stderr, "\n");
	}
    }

    fprintf(stderr, "Test end\n");
    libonyx_shutdown();
    return 0;
}
