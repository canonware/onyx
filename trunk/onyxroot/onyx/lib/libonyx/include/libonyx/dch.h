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

#ifdef CW_CH_COUNT
/* Maintain counters used to get an idea of performance. */
/* #define CW_DCH_COUNT */
#ifdef CW_DCH_COUNT
/* Print counter values to stderr in dch_delete(). */
/* #define CW_DCH_VERBOSE */
#endif
#endif

/* Pseudo-opaque type. */
typedef struct cw_dch_s cw_dch_t;

struct cw_dch_s
{
#ifdef CW_DBG
    uint32_t magic;
#endif

#ifdef CW_DCH_COUNT
    /* Counters used to get an idea of performance. */
    uint64_t num_grows;
    uint64_t num_shrinks;
#endif

    /* Opaque allocation/deallocation pointers. */
    cw_mema_t *mema;

    bool is_malloced:2/*1*/;

    /* Intial table size and high/low water marks.  These values are used in
     * proportion if the table grows. */
    uint32_t base_table;
    uint32_t base_grow;
    uint32_t base_shrink;

    bool shrinkable:2/*1*/;

    /* (grow_factor * base_table) is the current table size. */
    uint32_t grow_factor;

    /* Cached for later ch creation during rehashes. */
    uint32_t (*hash)(const void *);
    bool (*key_comp)(const void *, const void *);

    /* Where all of the real work is done. */
    cw_ch_t *ch;
};

cw_dch_t *
dch_new(cw_dch_t *a_dch, cw_mema_t *a_mema, uint32_t a_base_table,
	uint32_t a_base_grow, uint32_t a_base_shrink,
	cw_ch_hash_t *a_hash, cw_ch_key_comp_t *a_key_comp);

void
dch_delete(cw_dch_t *a_dch);

uint32_t
dch_count(cw_dch_t *a_dch);

bool
dch_shrinkable_get(cw_dch_t *a_dch);

void
dch_shrinkable_set(cw_dch_t *a_dch, bool a_shrinkable);

void
dch_insert(cw_dch_t *a_dch, const void *a_key, const void *a_data,
	   cw_chi_t *a_chi);

bool
dch_remove(cw_dch_t *a_dch, const void *a_search_key, void **r_key,
	   void **r_data, cw_chi_t **r_chi);

void
dch_chi_remove(cw_dch_t *a_dch, cw_chi_t *a_chi);

bool
dch_search(cw_dch_t *a_dch, const void *a_key, void **r_data);
