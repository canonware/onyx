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

    bool is_malloced:1;

    /* Intial table size and high/low water marks.  These values are used in
     * proportion if the table grows. */
    size_t base_power;
    size_t cur_power;

    bool shrinkable:1;

    /* Cached for later ch creation during rehashes. */
    size_t (*hash)(const void *);
    bool (*key_comp)(const void *, const void *);

    /* Where all of the real work is done. */
    cw_ch_t *ch;
};

cw_dch_t *
dch_new(cw_dch_t *a_dch, cw_mema_t *a_mema, size_t a_base_count,
	cw_ch_hash_t *a_hash, cw_ch_key_comp_t *a_key_comp);

void
dch_delete(cw_dch_t *a_dch);

size_t
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
