/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

cw_stilt_t	*stil_l_ref_iter(cw_stil_t *a_stil, cw_bool_t a_reset);
#define	stil_l_stilt_insert(a_stil, a_stilt)				\
	ql_tail_insert(&(a_stil)->stilt_head, (a_stilt), link)
#define	stil_l_stilt_remove(a_stil, a_stilt)				\
	ql_remove(&(a_stil)->stilt_head, (a_stilt), link)

#define	stil_l_name_lock_get(a_stil) (&(a_stil)->name_lock)
#define	stil_l_name_hash_get(a_stil) (&(a_stil)->name_hash)
