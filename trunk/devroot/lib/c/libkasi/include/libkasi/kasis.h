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

struct cw_kasis_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif
  
  cw_ring_t * top;

  cw_pezz_t * chunk_pezz;

  /* Number of stack elements. */
  cw_uint32_t count;
};

struct cw_kasiso_s
{
  /* The payload.  This must be first in the structure, since pointers are cast
   * between (cw_kasiso_t *) and (cw_kasio_t *). */
  union
  {
    cw_kasio_t kasio;
    cw_ring_t kasisc_spares;
  } data;
  
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif
  
  /* chunk this kasisco is contained in. */
  cw_kasisc_t * chunk;

  /* Stack linkage.  If a spare slot, this field is used to link into the
   * kasis-wide spares ring. */
  cw_ring_t stack;
};

struct cw_kasisc_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif
  
  /* kasis this chunk is part of. */
  cw_kasis_t * kasis;

  cw_uint32_t ref_count;
  
  cw_ring_t * spares;

  /* Must be last field, since it is used for array indexing of kasiso's beyond
   * the end of the structure. */
  cw_kasiso_t objects[1];
};

cw_kasis_t *
kasis_new(cw_kasis_t * a_kasis, cw_pezz_t * a_chunk_pezz);

void
kasis_delete(cw_kasis_t * a_kasis);

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
