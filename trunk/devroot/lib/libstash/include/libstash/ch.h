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

/* Pseudo-opaque type. */
typedef struct cw_ch_s cw_ch_t;

typedef struct
{
  const void * key;
  const void * data;
  cw_ring_t ch_link;
  cw_ring_t slot_link;
  cw_uint32_t slot;
} cw_chi_t;

struct cw_ch_s
{
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
  
  /* Counters used to get an idea of performance. */
  cw_uint32_t num_collisions;
  cw_uint32_t num_inserts;
  cw_uint32_t num_deletes;
#endif

  cw_bool_t is_malloced;
  cw_ring_t * chi_ring;
  cw_uint32_t count;
  cw_uint32_t table_size;

  /* If NULL, use _cw_malloc() instead. */
  cw_pezz_t * chi_pezz;

  cw_uint32_t (*hash)(const void *);
  cw_bool_t (*key_comp)(const void *, const void *);

  /* Must be last field, since it is used for array indexing of chi's beyond the
   * end of the structure. */
  cw_ring_t * table[1];
};

/* Typedefs to allow easy function pointer passing. */
typedef cw_uint32_t ch_hash_t(const void *);
typedef cw_bool_t ch_key_comp_t(const void *, const void *);

/* Calculates ch size, given the number of hash table slots.  Use this to
 * calculate space allocation when passing pre-allocated space to ch_new(). */
#define _CW_CH_TABLE2SIZEOF(t) \
  (sizeof(cw_ch_t) + (((t) - 1) * sizeof(cw_chi_t *)))

cw_ch_t *
ch_new(cw_ch_t * a_ch, cw_uint32_t a_table_size, cw_pezz_t * a_chi_pezz,
       ch_hash_t * a_hash, ch_key_comp_t * a_key_comp);

void
ch_delete(cw_ch_t * a_ch);

cw_uint32_t
ch_count(cw_ch_t * a_ch);

cw_bool_t
ch_insert(cw_ch_t * a_ch, const void * a_key, const void * a_data);

cw_bool_t
ch_remove(cw_ch_t * a_ch, const void * a_search_key, void ** r_key,
	  void ** r_data);

cw_bool_t
ch_search(cw_ch_t * a_ch, const void * a_key, void ** r_data);

cw_bool_t
ch_get_iterate(cw_ch_t * a_ch, void ** r_key, void ** r_data);

cw_bool_t
ch_remove_iterate(cw_ch_t * a_ch, void ** r_key, void ** r_data);

void
ch_dump(cw_ch_t * a_ch, const char * a_prefix);

cw_uint32_t
ch_hash_string(const void * a_key);

cw_uint32_t
ch_hash_direct(const void * a_key);

cw_bool_t
ch_key_comp_string(const void * a_k1, const void * a_k2);

cw_bool_t
ch_key_comp_direct(const void * a_k1, const void * a_k2);
