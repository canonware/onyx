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

#define _CW_KASID_ENTS2SIZEOF(e) \
  (sizeof(cw_kasid_t) - sizeof(cw_ch_t) + _CW_CH_TABLE2SIZEOF(5 / 4 * (e)))

typedef struct cw_kasid_s cw_kasid_t;
typedef struct cw_kasido_s cw_kasido_t;

struct cw_kasid_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic;
#endif

  /* kasido's are allocated from here. */
  cw_pezz_t * kasido_pezz;

  /* Ring of anonymous entries, which are used for kasids lookup caching.
   * Anonymous entries are not inserted into the hash table, since 1) they never
   * need to be looked up by name, and 2) it would swell the hash table
   * non-deterministically, which would mandate using some form of expandible
   * hashing.
   *
   * A kasid can be on the same dictionary stack multiple times.  The anon cache
   * is for the top-most insertion. */
  cw_ring_t * anon;

  /* Name/value pairs.  The keys are (cw_nameext_t *), and the values are
   * (cw_kasido_t *).  The kasio from which the key is filched resides in the
   * kasido structure.
   *
   * Must be the last field since the hash table is embedded and can vary in
   * size. */
  cw_ch_t hash;
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

  /* kasid this kasido is contained in. */
  cw_kasid_t * kasid;
  
  /* Ring of definitions with the same name, throughout a kasids. */
  cw_ring_t kasids_link;

  union
  {
    cw_kasio_t name;
    
    /* Linkage for anonymous entries within the kasid this entry is part of. */
    cw_ring_t anon_link;
  } key;
  cw_kasio_t value;
};

cw_kasid_t *
kasid_new(cw_kasid_t * a_kasid, cw_pezz_t * kasido_pezz);

void
kasid_delete(cw_kasid_t * a_kasid);

cw_bool_t
kasid_def(cw_kasid_t * a_kasid, cw_kasio_t * a_name, cw_kasio_t * a_val);

cw_bool_t
kasid_def_anon(cw_kasid_t * a_kasid, cw_kasio_t * a_name, cw_kasio_t * a_val);

cw_bool_t
kasid_undef(cw_kasid_t * a_kasid, cw_kasio_t * a_name);

cw_kasio_t *
kasid_lookup(cw_kasid_t * a_kasid, cw_kasio_t * a_name);

cw_bool_t
kasid_copy(cw_kasid_t * a_from, cw_kasid_t * a_to);

cw_uint32_t
kasid_count(cw_kasid_t * a_kasid);

cw_kasio_t *
kasid_iterate(cw_kasid_t * a_kasid);
