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

/* Number of op's per chunk. */
#define _CW_KASIS_CHUNK_NOPS 64

/* Padding for chunks.  Ideally, chunk's should be a power of two in size to
 * improve cache performance. */
#define _CW_KASIS_CHUNK_PAD 0

typedef struct cw_kasis_s cw_kasis_t;
typedef struct cw_kasis_chunk_spare_s cw_kasis_chunk_spare_t;
typedef struct cw_kasis_op_s cw_kasis_op_t;
typedef struct cw_kasis_chunk_s cw_kasis_chunk_t;

struct cw_kasis_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_a;
#endif
  cw_ring_t * top;

  cw_pezz_t * chunk_pezz;

  /* Number of stack elements.  Does not include anonymous op's. */
  cw_uint32_t count;

#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_b;
#endif
};

struct cw_kasis_chunk_spare_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_a;
#endif
  
  cw_ring_t stack_spares;
  cw_ring_t chunk_spares;
  
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_b;
#endif
};

struct cw_kasis_op_s
{
  /* The payload.  This must be first in the structure, since pointers are cast
   * between (cw_kasis_op_t *), (cw_kasis_chunk_spare_t *), and
   * (cw_kasio_t *). */
  union
  {
    cw_kasio_t op;
    cw_kasis_chunk_spare_t spare;
  } data;
  
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_a;
#endif
  
  /* chunk this op is contained in. */
  cw_kasis_chunk_t * chunk;

  /* Stack linkage. */
  cw_ring_t stack;

#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_b;
#endif
};

struct cw_kasis_chunk_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_a;
#endif

  /* kasis this chunk is part of. */
  cw_kasis_t * kasis;

  cw_uint32_t ref_count;
  
  cw_kasis_chunk_spare_t * spares;
  cw_kasis_op_t ops[_CW_KASIS_CHUNK_NOPS];
  
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_b;
#endif
#if (0 != _CW_KASIS_CHUNK_PAD)
  cw_uint8_t pad[_CW_KASIS_CHUNK_PAD];
#endif
};

cw_kasis_t *
kasis_new(cw_kasis_t * a_kasis, cw_pezz_t * a_chunks);

void
kasis_delete(cw_kasis_t * a_kasis);

cw_kasio_t *
kasis_push(cw_kasis_t * a_kasis);

cw_kasio_t *
kasis_push_anon(cw_kasis_t * a_kasis);

cw_bool_t
kasis_pop(cw_kasis_t * a_kasis, cw_uint32_t a_count);

cw_bool_t
kasis_roll(cw_kasis_t * a_kasis, cw_uint32_t a_count);

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
