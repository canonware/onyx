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

cw_nxoe_t *nx_l_ref_iter(cw_nx_t *a_nx, cw_bool_t a_reset);

#define	nx_l_thread_insert(a_nx, a_thread)				\
	nxo_dict_def(&(a_nx)->threadsdict, (a_nx), (a_thread), (a_thread))
#define	nx_l_thread_remove(a_nx, a_thread)				\
	nxo_dict_undef(&(a_nx)->threadsdict, (a_nx), (a_thread))

#define	nx_l_name_lock_get(a_nx) (&(a_nx)->name_lock)
#define	nx_l_name_hash_get(a_nx) (&(a_nx)->name_hash)

#define	nx_l_thread_init(a_nx) (a_nx)->thread_init
