/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

/* Calculate kasisc size, given the number of kasio's. */
#define _CW_KASISC_O2SIZEOF(n)

/* Calculate number of kasio's per kasisc, given kasisc size. */
#define _CW_KASISC_SIZEOF2O(s)

typedef struct cw_kasis_s cw_kasis_t;
typedef struct cw_kasiso_s cw_kasiso_t;
typedef struct cw_kasisc_s cw_kasisc_t;

struct cw_kasiso_s
{
  /* The payload.  This must be first in the structure, since pointers are cast
   * between (cw_kasiso_t *) and (cw_kasio_t *). */
  cw_kasio_t kasio;
  
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif
  
  /* Stack linkage.  If a spare slot, this field is used to link into the
   * kasis-wide spares ring.  If this kasiso is empty/invalid, the least
   * significant bit of the data pointer is 1.  This allows the GC to deallocate
   * completely unused kasisc's by iteratively unlinking all kasiso's in an
   * empty kasisc. */
  cw_ring_t link;
};

struct cw_kasisc_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  /* Pointer to the kasis's spares linkage. */
  cw_ring_t * spares;

  /* Linkage into a ring of kasisc's in the same kasis.  The GC uses this ring
   * to iterate through the kasis, and potentially deallocate empty kasisc's.
   * */
  cw_ring_t link;
  
  /* kasis this kasisc is part of. */
  cw_kasis_t * kasis;

  /* Must be last field, since it is used for array indexing of kasiso's beyond
   * the end of the structure. */
  cw_kasiso_t objects[1];
};

struct cw_kasis_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  /* Linkage to the top of the stack. */
  cw_ring_t top;

  /* Linkage to the ring of spare slots. */
  cw_ring_t spares;

  cw_pezz_t * kasisc_pezz;

  /* Number of stack elements. */
  cw_uint32_t count;
};

cw_kasis_t *
kasis_new(cw_kasis_t * a_kasis, cw_pezz_t * a_chunk_pezz);

void
kasis_delete(cw_kasis_t * a_kasis);

void
kasis_collect(cw_kasis_t * a_kasis,
	      void (*a_add_root_func)(void * add_root_arg, cw_kasioe_t * root),
	      void * a_add_root_arg);

cw_kasio_t *
kasis_push(cw_kasis_t * a_kasis);

cw_bool_t
kasis_pop(cw_kasis_t * a_kasis, cw_uint32_t a_count);

cw_bool_t
kasis_roll(cw_kasis_t * a_kasis, cw_sint32_t a_count);

cw_bool_t
kasis_dup(cw_kasis_t * a_kasis, cw_uint32_t a_count, cw_uint32_t a_index);

cw_uint32_t
kasis_count(cw_kasis_t * a_kasis);

cw_kasio_t *
kasis_get(cw_kasis_t * a_kasis, cw_uint32_t a_index);

cw_kasio_t *
kasis_get_down(cw_kasis_t * a_kasis, cw_kasio_t * a_kasio);

cw_kasio_t *
kasis_get_up(cw_kasis_t * a_kasis, cw_kasio_t * a_kasio);

cw_bool_t
kasis_get_index(cw_kasis_t * a_kasis, cw_kasio_t * a_kasio);
