/****************************************************************************
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
typedef struct cw_dch_s cw_dch_t;

struct cw_dch_s {
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t	magic;

	/* Counters used to get an idea of performance. */
	cw_uint32_t	num_grows;
	cw_uint32_t	num_shrinks;
#endif

	cw_bool_t	is_malloced;

	/*
	 * Intial table size and high/low water marks.  These values are
	 * used in proportion if the table grows.
	 */
	cw_uint32_t	base_table;
	cw_uint32_t	base_grow;
	cw_uint32_t	base_shrink;

	/* (grow_factor * base_table) is the current table size. */
	cw_uint32_t	grow_factor;

	/* Cached for later ch creation during rehashes. */
	cw_uint32_t	(*hash)(const void *);
	cw_bool_t	(*key_comp)(const void *, const void *);

	/* Where all of the real work is done. */
	cw_ch_t		*ch;
};

cw_dch_t	*dch_new(cw_dch_t *a_dch, cw_uint32_t a_base_table, cw_uint32_t
    a_base_grow, cw_uint32_t a_base_shrink, cw_ch_hash_t *a_hash,
    cw_ch_key_comp_t *a_key_comp);

void		dch_delete(cw_dch_t *a_dch);

cw_uint32_t	dch_count(cw_dch_t *a_dch);

cw_bool_t	dch_insert(cw_dch_t *a_dch, const void *a_key, const void
    *a_data, cw_chi_t *a_chi);

cw_bool_t	dch_remove(cw_dch_t *a_dch, const void *a_search_key, void
    **r_key, void **r_data, cw_chi_t **r_chi);

cw_bool_t	dch_search(cw_dch_t *a_dch, const void *a_key, void **r_data);

cw_bool_t	dch_get_iterate(cw_dch_t *a_dch, void **r_key, void **r_data);

cw_bool_t	dch_remove_iterate(cw_dch_t *a_dch, void **r_key, void
    **r_data, cw_chi_t **r_chi);

void		dch_dump(cw_dch_t *a_dch, const char *a_prefix);
