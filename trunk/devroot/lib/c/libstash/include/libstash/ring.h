/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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
 * Public interface for the doubly linked ring class.
 *
 * Rings have similar uses to doubly linked lists, but they are much faster and
 * simpler.  There is no concept of head and tail.  Every element in the ring
 * can be used in the same way.  That is, it is possible (and legitimate) to
 * simultaneously have pointers to more than one element in the ring and operate
 * on them in like ways.
 *
 ****************************************************************************/

/* Pseudo-opaque types. */
typedef struct cw_ring_s cw_ring_t;

struct cw_ring_s
{
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;

  cw_ring_t * prev;
  cw_ring_t * next;
  void * data;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_ring : Pointer to space for a ring, or NULL.
 *
 * a_dealloc_func : Pointer to a deallocation function, or NULL.  Ignored if
 *                  a_ring == NULL.
 *
 * a_dealloc_arg : First argument to a_dealloc_func.  Not used if a_ring == NULL
 *                 or a_dealloc_func == NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a ring, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_ring_t *
ring_new(cw_ring_t * a_ring,
	 void (*a_dealloc_func)(void * dealloc_arg, void * ring),
	 void * a_dealloc_arg);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_ring : Pointer to a ring.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
ring_delete(cw_ring_t * a_ring);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_ring : Pointer to a ring.
 *
 * a_prefix : String constant that is used as a prefix for each line of output.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Dump the internals of a_ring to cw_g_out.
 *
 ****************************************************************************/
void
ring_dump(cw_ring_t * a_ring, const char * a_prefix);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_ring : Pointer to a ring.
 *
 * <<< Output(s) >>>
 *
 * retval : Data pointer.
 *
 * <<< Description >>>
 *
 * Return the value of a_ring's data pointer.
 *
 ****************************************************************************/
void *
ring_get_data(cw_ring_t * a_ring);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_ring : Pointer to a ring.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set a_ring's data pointer to a_data.
 *
 ****************************************************************************/
void
ring_set_data(cw_ring_t * a_ring, void * a_data);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_ring : Pointer to a ring.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a ring.
 *
 * <<< Description >>>
 *
 * Return a pointer to the next element in a_ring.
 *
 ****************************************************************************/
cw_ring_t *
ring_next(cw_ring_t * a_ring);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_ring : Pointer to a ring.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a ring.
 *
 * <<< Description >>>
 *
 * Return a pointer to the previous element in a_ring.
 *
 ****************************************************************************/
cw_ring_t *
ring_prev(cw_ring_t * a_ring);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to a ring.
 *
 * a_b : Pointer to a ring.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Combine a_a and a_b into one ring.
 *
 ****************************************************************************/
void
ring_meld(cw_ring_t * a_a, cw_ring_t * a_b);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_ring : Pointer to a ring.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to new ring.
 *
 * <<< Description >>>
 *
 * Remove a_ring from the rest of the ring and return the new ring.
 *
 ****************************************************************************/
cw_ring_t *
ring_cut(cw_ring_t * a_ring);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to a ring.
 *
 * a_b : Pointer to a ring.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Given a_a and a_b in the same ring, split the ring such that a_a and a_b are
 * the heads of the resulting rings.  This function is safe even for a ring with
 * only one node, and even if a_a == a_b for a ring with multiple nodes.
 *
 ****************************************************************************/
void
ring_split(cw_ring_t * a_a, cw_ring_t * a_b);
