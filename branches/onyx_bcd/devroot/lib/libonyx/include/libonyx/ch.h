/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
******************************************************************************
*
* <Copyright = jasone>
* <License>
*
******************************************************************************
*
* Version: <Version>
*
******************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_ch_s cw_ch_t;
typedef struct cw_chi_s cw_chi_t;

/* Declared here to avoid a circular dependency. */
typedef struct cw_mem_s cw_mem_t;

/* Internal container used by ch, one per item.  chi's are internally linked to
 * multiple ql's in order to implement various LIFO/FIFO orderings. */
struct cw_chi_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#endif
    /* If space for a chi wasn't passed into ch_insert(), this is TRUE. */
    cw_bool_t is_malloced;
    
    /* Key. */
    const void *key;
    
    /* Data. */
    const void *data;

    /* Link into the ch-wide list of chi's. */
    ql_elm(cw_chi_t) ch_link;

    /* Link into the slot's list of chi's. */
    ql_elm(cw_chi_t) slot_link;
    
    /* Slot number. */
    cw_uint32_t slot;
};

struct cw_ch_s
{
#ifdef CW_DBG
    cw_uint32_t magic;

    /* Counters used to get an idea of performance. */
    cw_uint32_t num_collisions;
    cw_uint32_t num_inserts;
    cw_uint32_t num_removes;
#endif

    /* Opaque allocation/deallocation pointers. */
    cw_opaque_alloc_t *alloc;
    cw_opaque_dealloc_t *dealloc;
    void *arg;

    /* TRUE if we malloced this structure internally. */
    cw_bool_t is_malloced;

    /* Head of the list of chi's. */
    ql_head(cw_chi_t) chi_ql;

    /* Total number of items. */
    cw_uint32_t count;

    /* Number of table slots. */
    cw_uint32_t table_size;

    /* Hashing and key comparison function pointers. */
    cw_uint32_t (*hash)(const void *);
    cw_bool_t (*key_comp)(const void *, const void *);

    /* Must be last field, since it is used for array indexing of chi's beyond
     * the end of the structure. */
    ql_head(cw_chi_t) table[1];
};

/* Typedefs to allow easy function pointer passing. */
typedef cw_uint32_t cw_ch_hash_t (const void *);
typedef cw_bool_t cw_ch_key_comp_t (const void *, const void *);

/* Calculates ch size, given the number of hash table slots.  Use this to
 * calculate space allocation when passing pre-allocated space to ch_new(). */
#define CW_CH_TABLE2SIZEOF(t)						\
    (sizeof(cw_ch_t) + (((t) - 1) * sizeof(cw_chi_t *)))

cw_ch_t *
ch_new(cw_ch_t *a_ch, cw_opaque_alloc_t *a_alloc,
       cw_opaque_dealloc_t *a_dealloc, void *a_arg, cw_uint32_t a_table_size,
       cw_ch_hash_t *a_hash, cw_ch_key_comp_t *a_key_comp);

void
ch_delete(cw_ch_t *a_ch);

cw_uint32_t
ch_count(cw_ch_t *a_ch);

void
ch_insert(cw_ch_t *a_ch, const void *a_key, const void *a_data,
	  cw_chi_t *a_chi);

cw_bool_t
ch_remove(cw_ch_t *a_ch, const void *a_search_key, void **r_key, void **r_data,
	  cw_chi_t **r_chi);

cw_bool_t
ch_search(cw_ch_t *a_ch, const void *a_key, void **r_data);

cw_bool_t
ch_get_iterate(cw_ch_t *a_ch, void **r_key, void **r_data);

cw_bool_t
ch_remove_iterate(cw_ch_t *a_ch, void **r_key, void **r_data, cw_chi_t **r_chi);

cw_uint32_t
ch_string_hash(const void *a_key);

cw_uint32_t
ch_direct_hash(const void *a_key);

cw_bool_t
ch_string_key_comp(const void *a_k1, const void *a_k2);

cw_bool_t
ch_direct_key_comp(const void *a_k1, const void *a_k2);
