/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

/* Maintain counters used to get an idea of performance. */
/* #define CW_CH_COUNT */
#ifdef CW_CH_COUNT
/* Print counter values to stderr in ch_delete(). */
/* #define CW_CH_VERBOSE */
#endif

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
    uint32_t magic;
#endif
    /* If space for a chi wasn't passed into ch_insert(), this is true. */
    bool is_malloced;

    /* Key. */
    const void *key;

    /* Data. */
    const void *data;

    /* Link into the slot's list of chi's. */
    ql_elm(cw_chi_t) slot_link;

    /* Slot number. */
    uint32_t slot;
};

struct cw_ch_s
{
#ifdef CW_DBG
    uint32_t magic;
#endif

#ifdef CW_CH_COUNT
    /* Counters used to get an idea of performance. */
    uint64_t num_collisions;
    uint64_t num_inserts;
    uint64_t num_removes;
    uint64_t num_searches;
#endif

    /* Opaque allocation/deallocation pointers. */
    cw_mema_t *mema;

    /* true if we malloced this structure internally. */
    bool is_malloced;

    /* Total number of items. */
    uint32_t count;

    /* Number of table slots. */
    uint32_t table_size;

    /* Hashing and key comparison function pointers. */
    uint32_t (*hash)(const void *);
    bool (*key_comp)(const void *, const void *);

    /* Must be last field, since it is used for array indexing of chi's beyond
     * the end of the structure. */
    ql_head(cw_chi_t) table[1];
};

/* Typedefs to allow easy function pointer passing. */
typedef uint32_t cw_ch_hash_t (const void *);
typedef bool cw_ch_key_comp_t (const void *, const void *);

/* Calculates ch size, given the number of hash table slots.  Use this to
 * calculate space allocation when passing pre-allocated space to ch_new(). */
#define CW_CH_TABLE2SIZEOF(t)						\
    (sizeof(cw_ch_t) + (((t) - 1) * sizeof(cw_chi_t *)))

cw_ch_t *
ch_new(cw_ch_t *a_ch, cw_mema_t *a_mema, uint32_t a_table_size,
       cw_ch_hash_t *a_hash, cw_ch_key_comp_t *a_key_comp);

void
ch_delete(cw_ch_t *a_ch);

uint32_t
ch_count(cw_ch_t *a_ch);

void
ch_insert(cw_ch_t *a_ch, const void *a_key, const void *a_data,
	  cw_chi_t *a_chi);

bool
ch_remove(cw_ch_t *a_ch, const void *a_search_key, void **r_key, void **r_data,
	  cw_chi_t **r_chi);

void
ch_chi_remove(cw_ch_t *a_ch, cw_chi_t *a_chi);

bool
ch_search(cw_ch_t *a_ch, const void *a_key, void **r_data);

uint32_t
ch_string_hash(const void *a_key);

uint32_t
ch_direct_hash(const void *a_key);

bool
ch_string_key_comp(const void *a_k1, const void *a_k2);

bool
ch_direct_key_comp(const void *a_k1, const void *a_k2);
