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

/* Defined in kasio.h, where it is first used. */
typedef struct cw_kasid_s cw_kasid_t;
typedef struct cw_kasido_s cw_kasido_t;

struct cw_kasid_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  /* kasido's are allocated from here. */
  cw_pezz_t * kasido_pezz;

  /* Name/value pairs.  The keys and values are both of type (cw_kasio_t *). */
  cw_oh_t hash;

  /* Number of definitions.  Does not include passthru objects (defined using
   * kasid_def_anon(). */
  cw_uint32_t count;
};

/* Defined here (rather than in kasio.h) in order to resolve a circular
 * dependency. */
struct cw_dictext_s
{
  cw_kasio_ext_t ext;
  cw_kasid_t dict;
};

struct cw_kasido_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif
  
  /* Ring of definitions with the same name, throughout a kasids. */
  cw_ring_t kasids_link;

  cw_kasio_t object;
};

cw_kasid_t *
kasid_new(cw_kasid_t * a_kasid, cw_pezz_t * kasido_pezz);

void
kasid_delete(cw_kasid_t * a_kasid);

cw_bool_t
kasid_def(cw_kasid_t * a_kasid, cw_kasio_t * a_key, cw_kasio_t * a_val);

cw_bool_t
kasid_def_anon(cw_kasid_t * a_kasid, cw_kasio_t * a_key, cw_kasio_t * a_val);

cw_bool_t
kasid_undef(cw_kasid_t * a_kasid, cw_kasio_t * a_key);

cw_kasio_t *
kasid_lookup(cw_kasid_t * a_kasid, cw_kasio_t * a_key);

cw_bool_t
kasid_copy(cw_kasid_t * a_from, cw_kasid_t * a_to);

cw_uint32_t
kasid_count(cw_kasid_t * a_kasid);

cw_kasio_t *
kasid_iterate(cw_kasid_t * a_kasid);
