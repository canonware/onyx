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

cw_ring_t *
ring_new(cw_ring_t * a_ring,
	 void (*a_dealloc_func)(void * dealloc_arg, void * ring),
	 void * a_dealloc_arg);

void
ring_delete(cw_ring_t * a_ring);

void
ring_dump(cw_ring_t * a_ring, const char * a_prefix);

void *
ring_get_data(cw_ring_t * a_ring);

void
ring_set_data(cw_ring_t * a_ring, void * a_data);

cw_ring_t *
ring_next(cw_ring_t * a_ring);

cw_ring_t *
ring_prev(cw_ring_t * a_ring);

void
ring_meld(cw_ring_t * a_a, cw_ring_t * a_b);

cw_ring_t *
ring_cut(cw_ring_t * a_ring);

void
ring_split(cw_ring_t * a_a, cw_ring_t * a_b);
