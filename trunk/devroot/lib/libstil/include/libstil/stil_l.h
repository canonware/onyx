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

cw_stiloe_t *stil_l_ref_iter(cw_stil_t *a_stil, cw_bool_t a_reset);

#define	stil_l_thread_insert(a_stil, a_thread)				\
	stilo_dict_def(&(a_stil)->threadsdict, (a_stil), (a_thread), (a_thread))
#define	stil_l_thread_remove(a_stil, a_thread)				\
	stilo_dict_undef(&(a_stil)->threadsdict, (a_stil), (a_thread))

#define	stil_l_name_lock_get(a_stil) (&(a_stil)->name_lock)
#define	stil_l_name_hash_get(a_stil) (&(a_stil)->name_hash)

#define	stil_l_thread_init(a_stil) (a_stil)->thread_init
