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
#define _CW_KASID_CHUNK_NOPS 64

/* Padding for chunks.  Ideally, chunk's should be a power of two in size to
 * improve cache performance. */
#define _CW_KASID_CHUNK_PAD 0

/* Defined in kasio.h, where it is first used. */
/*  typedef struct cw_kasid_s cw_kasid_t; */
typedef struct cw_kasid_chunk_spare_s cw_kasid_chunk_spare_t;
typedef struct cw_kasid_op_s cw_kasid_op_t;
typedef struct cw_kasid_chunk_s cw_kasid_chunk_t;

struct cw_kasid_s
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

struct cw_kasid_chunk_spare_s
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

struct cw_kasid_op_s
{
  /* The payload.  This must be first in the structure, since pointers are cast
   * between (cw_kasid_op_t *), (cw_kasid_chunk_spare_t *), and
   * (cw_kasio_t *). */
  union
  {
    cw_kasio_t op;
    cw_kasid_chunk_spare_t spare;
  } data;
  
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_a;
#endif
  
  /* chunk this op is contained in. */
  cw_kasid_chunk_t * chunk;

  /* Stack linkage. */
  cw_ring_t stack;

#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_b;
#endif
};

struct cw_kasid_chunk_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_a;
#endif

  /* kasid this chunk is part of. */
  cw_kasid_t * kasid;

  cw_uint32_t ref_count;
  
  cw_kasid_chunk_spare_t * spares;
  cw_kasid_op_t ops[_CW_KASID_CHUNK_NOPS];
  
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_b;
#endif
#if (0 != _CW_KASID_CHUNK_PAD)
  cw_uint8_t pad[_CW_KASID_CHUNK_PAD];
#endif
};

cw_kasid_t *
kasid_new(cw_kasid_t * a_kasid, cw_pezz_t * a_chunks);

void
kasid_delete(cw_kasid_t * a_kasid);

/*  kasid_def */
/*  kasid_lookup */
/*  kasid_save */
/*  kasid_restore */

cw_kasio_t *
kasid_push(cw_kasid_t * a_kasid);

cw_kasio_t *
kasid_push_anon(cw_kasid_t * a_kasid);

cw_bool_t
kasid_pop(cw_kasid_t * a_kasid, cw_uint32_t a_count);

cw_bool_t
kasid_roll(cw_kasid_t * a_kasid, cw_sint32_t a_count);

cw_bool_t
kasid_dup(cw_kasid_t * a_kasid, cw_uint32_t a_count, cw_uint32_t a_index);

cw_uint32_t
kasid_count(cw_kasid_t * a_kasid);

cw_kasio_t *
kasid_get(cw_kasid_t * a_kasid, cw_uint32_t a_index);

cw_kasio_t *
kasid_get_down(cw_kasid_t * a_kasid, cw_kasio_t * a_kasio);

cw_kasio_t *
kasid_get_up(cw_kasid_t * a_kasid, cw_kasio_t * a_kasio);

cw_bool_t
kasid_get_index(cw_kasid_t * a_kasid, cw_kasio_t * a_kasio);
