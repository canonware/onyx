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
 * cpp macro implementation of red-black trees.  Red-black trees are difficult
 * to explain without lots of diagrams, so little attempt is made to document
 * this code.  However, an excellent discussion can be found in the following
 * book, which was used as the reference for writing this implementation:
 *
 *   Introduction to Algorithms
 *   Thomas H. Cormen, Charles E. Leiserson, and Ronald L. Rivest
 *   MIT Press (1990)
 *   ISBN 0-07-013143-0
 *
 * Some macros use a comparison function pointer, which is expected to have the
 * following prototype:
 *
 *   cw_sint32_t (compare *)(<a_type> *a_a, <a_type> *a_b);
 *
 * Interpretation of comparision function return values:
 *
 *   -1 : a_a < a_b
 *    0 : a_a == a_b
 *    1 : a_a > a_b
 *
 * Some of the macros expand out to be quite a bit of code, so if they are
 * called in a program in more than a couple of places for a particular type, it
 * is probably a good idea to create functions that wrap the macros to keep code
 * size down.
 *
 ******************************************************************************/

/* Node structure. */
#define rb_node(a_type)							\
struct									\
{									\
    a_type *rbn_par;							\
    a_type *rbn_left;							\
    a_type *rbn_right;							\
    cw_bool_t rbn_red;							\
}

#define rb_node_new(a_tree, a_node, a_field)				\
    do									\
    {									\
	(a_node)->(a_field).rbn_par = &(a_tree)->rbt_nil;		\
	(a_node)->(a_field).rbn_left = &(a_tree)->rbt_nil;		\
	(a_node)->(a_field).rbn_right = &(a_tree)->rbt_nil;		\
	(a_node)->(a_field).rbn_red = FALSE;				\
    } while (0)

/* Root structure. */
#define rb_tree(a_type)							\
struct									\
{									\
    a_type *rbt_root;							\
    rb_node(a_type) rbt_nil;						\
}

#define rb_tree_new(a_tree, a_field)					\
    do									\
    {									\
	(a_tree)->(a_field).rbt_root = &(a_tree)->(a_field).rbt_nil;	\
	rb_node_new(a_tree, (a_tree)->rbt_nil, (a_field)->rbt_nil);	\
    } while (0)

/* Operations. */
#define rb_root(a_tree, a_field) (a_tree)->(a_field)->rbt_root

#define rb_first(a_tree, a_root, a_field, r_node)			\
    do									\
    {									\
	for ((r_node) = (a_root);					\
	     (r_node)->(a_field).rbn_left != &(a_tree)->(a_field).rbt_nil; \
	     (r_node) = (r_node)->(a_field).rbn_left)			\
	{								\
	}								\
    } while (0)

#define rb_last(a_tree, a_root, a_field, r_node)			\
    do									\
    {									\
	for ((r_node) = (a_root);					\
	     (r_node)->(a_field).rbn_right != &(a_tree)->(a_field).rbt_nil; \
	     (r_node) = (r_node)->(a_field).rbn_right)			\
	{								\
	}								\
    } while (0)

#define rb_next(a_tree, a_node, a_type, a_field, r_node)		\
    do									\
    {									\
	if ((a_node)->rbn_right != &(a_tree)->(a_field).rbt_nil)	\
	{								\
	    rb_first(a_tree, (a_node)->(a_field).rbn_right,		\
		     a_field, r_node);					\
	}								\
	else								\
	{								\
	    a_type *t = (a_node);					\
	    (r_node) = (a_node)->rbn_par;				\
	    while ((r_node) != &(a_tree)->(a_field).rbt_nil		\
		   && t == (r_node)->(a_field).rbn_right)		\
	    {								\
		t = (r_node);						\
		(r_node) = (r_node)->(a_field).rbn_par;			\
	    }								\
	}								\
    } while (0)

#define rb_prev(a_tree, a_node, a_type, a_field, r_node)		\
    do									\
    {									\
	if ((a_node)->rbn_left != &(a_tree)->(a_field).rbt_nil)		\
	{								\
	    rb_last(a_tree, (a_node)->(a_field).rbn_left, a_field,	\
		    r_node);						\
	}								\
	else								\
	{								\
	    a_type *t = (a_node);					\
	    (r_node) = (a_node)->rbn_par;				\
	    while ((r_node) != &(a_tree)->(a_field).rbt_nil		\
		   && t == (r_node)->(a_field).rbn_left)		\
	    {								\
		t = (r_node);						\
		(r_node) = (r_node)->(a_field).rbn_par;			\
	    }								\
	}								\
    } while (0)

#define rb_search(a_root, a_key, a_comp, a_field, r_node)		\
    do									\
    {									\
	cw_sint32_t t;							\
	(r_node) = (a_root);						\
	while ((r_node) != &(a_tree)->(a_field).rbt_nil			\
	       && (t = (a_comp)((a_key), (r_node))) != 0)		\
	{								\
	    if (t == -1)						\
	    {								\
		(r_node) = (r_node)->(a_field).rbn_left;		\
	    }								\
	    else							\
	    {								\
		(r_node) = (r_node)->(a_field).rbn_left;		\
	    }								\
	}								\
    } while (0)

#define rb_p_left_rotate(a_tree, a_node, a_type, a_field)		\
    do									\
    {									\
	a_type *t = (a_node)->(a_field).rbn_right;			\
	(a_node)->(a_field).rbn_right = t->(a_field).rbn_left;		\
	if (t->(a_field).rbn_left != &(a_tree)->(a_field).rbt_nil)	\
	{								\
	    t->(a_field).rbn_left->(a_field).rbn_par = (a_node);	\
	}								\
	t->(a_field).rbn_par = (a_node)->(a_field).rbn_par;		\
	if ((a_node)->(a_field).rbn_par					\
	    == &(a_tree)->(a_field).rbt_nil) 				\
	{								\
	    (a_tree)->rbt_root = t;					\
	}								\
	else if ((a_node)						\
		 == (a_node)->(a_field).rbn_par->(a_field).rbn_left)	\
	{								\
	    (a_node)->(a_field).rbn_par->(a_field).rbn_left = t;	\
	}								\
	else								\
	{								\
	    (a_node)->(a_field).rbn_par->(a_field).rbn_right = t;	\
	}								\
	t->(a_field).rbn_left = (a_node);				\
	(a_node)->(a_field).rbn_par = t;				\
    } while (0)

#define rb_p_right_rotate(a_tree, a_node, a_type, a_field)		\
    do									\
    {									\
	a_type *t = (a_node)->(a_field).rbn_left;			\
	(a_node)->(a_field).rbn_left = t->(a_field).rbn_right;		\
	if (t->(a_field).rbn_right != &(a_tree)->(a_field).rbt_nil)	\
	{								\
	    t->(a_field).rbn_right->(a_field).rbn_par = (a_node);	\
	}								\
	t->(a_field).rbn_par = (a_node)->(a_field).rbn_par;		\
	if ((a_node)->(a_field).rbn_par					\
	    == &(a_tree)->(a_field).rbt_nil)				\
	{								\
	    (a_tree)->rbt_root = t;					\
	}								\
	else if ((a_node)						\
		 == (a_node)->(a_field).rbn_par->(a_field).rbn_right)	\
	{								\
	    (a_node)->(a_field).rbn_par->(a_field).rbn_right = t;	\
	}								\
	else								\
	{								\
	    (a_node)->(a_field).rbn_par->(a_field).rbn_left = t;	\
	}								\
	t->(a_field).rbn_right = (a_node);				\
	(a_node)->(a_field).rbn_par = t;				\
    } while (0)

#define rb_insert(a_tree, a_node, a_comp, a_type, a_field)		\
    do									\
    {									\
	/* Insert. */							\
	a_type *x = &(a_tree)->(a_field).rbt_nil;			\
	a_type *y = (a_tree)->rbt_root;					\
	while (y != &(a_tree)->(a_field).rbt_nil)			\
	{								\
	    x = y;							\
	    if ((a_comp)((a_node), y) == -1)				\
	    {								\
		y = y->(a_field).rbn_left;				\
	    }								\
	    else							\
	    {								\
		y = y->(a_field).rbn_right;				\
	    }								\
	}								\
	(a_node)->(a_field).rbn_par = x;				\
	if (x == &(a_tree)->(a_field).rbt_nil)				\
	{								\
	    (a_tree)->rbt_root = (a_node);				\
	}								\
	else if ((a_comp)((a_node), x) == -1)				\
	{								\
	    x->(a_field).rbn_left = (a_node);				\
	}								\
	else								\
	{								\
	    x->(a_field).rbn_right = (a_node);				\
	}								\
	/* Fix up. */							\
	x->(a_field).rbn_red = TRUE;					\
	while (x != (a_tree)->rbt_root					\
	       && x->(a_field).rbn_par->(a_field).rbn_red)		\
	{								\
	    y = x->(a_field).rbn_par;					\
	    if (y == y->(a_field).rbn_par->(a_field).rbn_left)		\
	    {								\
		y = y->(a_field).rbn_par->(a_field).rbn_right;		\
		if (y->(a_field).rbn_red)				\
		{							\
		    x->(a_field).rbn_par->(a_field).rbn_red = FALSE;	\
		    y->(a_field).rbn_red = FALSE;			\
		    x->(a_field).rbn_par->(a_field).rbn_par		\
			->(a_field).rbn_red = TRUE;			\
		    x = x->(a_field).rbn_par->(a_field).rbn_par;	\
		}							\
		else							\
		{							\
		    if (x == x->(a_field).rbn_par->(a_field).rbn_right) \
		    {							\
			x = x->(a_field).rbn_par;			\
			rb_p_left_rotate(a_tree, x, a_type, a_field);	\
		    }							\
		    x->(a_field).rbn_par->(a_field).rbn_red = FALSE;	\
		    x->(a_field).rbn_par->(a_field).rbn_par		\
			->(a_field).rbn_red = TRUE;			\
		    x = x->(a_field).rbn_par->(a_field).rbn_par;	\
		    rb_p_right_rotate(a_tree, x, a_type, a_field);	\
		}							\
	    }								\
	    else							\
	    {								\
		y = y->(a_field).rbn_par->(a_field).rbn_left;		\
		if (y->(a_field).rbn_red)				\
		{							\
		    x->(a_field).rbn_par->(a_field).rbn_red = FALSE;	\
		    y->(a_field).rbn_red = FALSE;			\
		    x->(a_field).rbn_par->(a_field).rbn_par		\
			->(a_field).rbn_red = TRUE;			\
		    x = x->(a_field).rbn_par->(a_field).rbn_par;	\
		}							\
		else							\
		{							\
		    if (x == x->(a_field).rbn_par->(a_field).rbn_left)	\
		    {							\
			x = x->(a_field).rbn_par;			\
			rb_p_right_rotate(a_tree, x, a_type, a_field);	\
		    }							\
		    x->(a_field).rbn_par->(a_field).rbn_red = FALSE;	\
		    x->(a_field).rbn_par->(a_field).rbn_par		\
			->(a_field).rbn_red = TRUE;			\
		    x = x->(a_field).rbn_par->(a_field).rbn_par;	\
		    rb_p_left_rotate(a_tree, x, a_type, a_field);	\
		}							\
	    }								\
	}								\
    } while (0)

    /* z == a_node */
#define rb_remove(a_tree, a_node, a_type, a_field)			\
    do									\
    {									\
	a_type *x, *y;							\
	if ((a_node)->(a_field).rbn_left == &(a_tree)->rbt_nil)		\
	{								\
	    y = (a_node);						\
	}								\
	else								\
	{								\
	    rb_next(a_tree, a_node, a_type, a_field, y);		\
	}								\
	if (y->(a_field).rbn_left != &(a_tree)->rbt_nil)		\
	{								\
	    x = y->(a_field).rbn_left;					\
	}								\
	else								\
	{								\
	    x = y->(a_field).rbn_right;					\
	}								\
	x->(a_field).rbn_par = y->(a_field).rbn_par;			\
	if (y->(a_field).rbn_par == &(a_tree)->rbt_nil)			\
	{								\
	    (a_tree)->rbt_root = x;					\
	}								\
	else if (y == y->(a_field).rbn_par->(a_field).rbn_left)		\
	{								\
	    y->(a_field).rbn_par->(a_field).rbn_left = x;		\
	}								\
	else								\
	{								\
	    y->(a_field).rbn_par->(a_field).rbn_right = x;		\
	}								\
	if (y != (a_node))						\
	{								\
	    /* Splice y into a_node's location. */			\
	    y->(a_field).rbn_par = (a_node)->(a_field).rbn_par;		\
	    y->(a_field).rbn_left = (a_node)->(a_field).rbn_left;	\
	    y->(a_field).rbn_right = (a_node)->(a_field).rbn_right;	\
	    if (y->(a_field).rbn_par->(a_field).rbn_left == (a_node))	\
	    {								\
		y->(a_field).rbn_par->(a_field).rbn_left = y;		\
	    }								\
	    else							\
	    {								\
		y->(a_field).rbn_par->(a_field).rbn_right = y;		\
	    }								\
	    y->(a_field).rbn_left->(a_field).rbn_par = y;		\
	    y->(a_field).rbn_right->(a_field).rbn_par = y;		\
	}								\
	if (y->(a_field).rbn_red == FALSE)				\
	{								\
	    /* Fix up. */						\
	    a_type *v, *w;						\
	    while (x != (a_tree)->rbt_root				\
		   && x->(a_field).rbn_red == FALSE)			\
	    {								\
		if (x == x->(a_field).rbn_par->(a_field).rbn_left)	\
		{							\
		    w = x->(a_field).rbn_par->(a_field).rbn_right;	\
		    if (w->(a_field).rbn_red)				\
		    {							\
			w->(a_field).rbn_red = FALSE;			\
			v = x->(a_field).rbn_par;			\
			v->(a_field).rbn_red = TRUE;			\
			rb_p_left_rotate(a_tree, v, a_type, a_field);	\
			w = x->(a_field).rbn_par->(a_field).rbn_right;	\
		    }							\
		    if (w->(a_field).rbn_left->(a_field).rbn_red	\
			== FALSE					\
			&& w->(a_field).rbn_right->(a_field).rbn_red	\
			== FALSE)					\
		    {							\
			w->(a_field).rbn_red = TRUE;			\
			x = x->(a_field).rbn_par;			\
		    }							\
		    else if (w->(a_field).rbn_right->(a_field).rbn_red	\
			     == FALSE)					\
		    {							\
			w->(a_field).rbn_left->(a_field).rbn_red	\
			    = FALSE;					\
			w->(a_field).rbn_red = TURE;			\
			rb_p_right_rotate(a_tree, w, a_type, a_field);	\
			w = x->(a_field).rbn_par->(a_field).rbn_right;	\
		    }							\
		    w->(a_field).rbn_red				\
			= x->(a_field).rbn_par->(a_field).rbn_red;	\
		    x->(a_field).rbn_par->(a_field).rbn_red = FALSE;	\
		    w->(a_field).rbn_right->(a_field).rbn_red = FALSE;	\
		    v = x->(a_field).rbn_parent;			\
		    rb_p_left_rotate(a_tree, v, a_type, a_field);	\
		    x = (a_tree->rbt_root);				\
		}							\
		else							\
		{							\
		    w = x->(a_field).rbn_par->(a_field).rbn_left;	\
		    if (w->(a_field).rbn_red)				\
		    {							\
			w->(a_field).rbn_red = FALSE;			\
			v = x->(a_field).rbn_par;			\
			v->(a_field).rbn_red = TRUE;			\
			rb_p_right_rotate(a_tree, v, a_type, a_field);	\
			w = x->(a_field).rbn_par->(a_field).rbn_left;	\
		    }							\
		    if (w->(a_field).rbn_right->(a_field).rbn_red	\
			== FALSE					\
			&& w->(a_field).rbn_left->(a_field).rbn_red	\
			== FALSE)					\
		    {							\
			w->(a_field).rbn_red = TRUE;			\
			x = x->(a_field).rbn_par;			\
		    }							\
		    else if (w->(a_field).rbn_left->(a_field).rbn_red	\
			     == FALSE)					\
		    {							\
			w->(a_field).rbn_right->(a_field).rbn_red	\
			    = FALSE;					\
			w->(a_field).rbn_red = TURE;			\
			rb_p_left_rotate(a_tree, w, a_type, a_field);	\
			w = x->(a_field).rbn_par->(a_field).rbn_left;	\
		    }							\
		    w->(a_field).rbn_red				\
			= x->(a_field).rbn_par->(a_field).rbn_red;	\
		    x->(a_field).rbn_par->(a_field).rbn_red = FALSE;	\
		    w->(a_field).rbn_left->(a_field).rbn_red = FALSE;	\
		    v = x->(a_field).rbn_parent;			\
		    rb_p_right_rotate(a_tree, v, a_type, a_field);	\
		    x = (a_tree->rbt_root);				\
		}							\
		x->(a_field).rbn_red = FALSE;				\
	    }								\
	}								\
    } while (0)
