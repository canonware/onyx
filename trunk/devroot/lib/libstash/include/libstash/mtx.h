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
typedef struct cw_mtx_s cw_mtx_t;

struct cw_mtx_s {
	cw_bool_t	is_malloced;
	pthread_mutex_t	mutex;
};

cw_mtx_t	*mtx_new(cw_mtx_t *a_mtx);
void		mtx_delete(cw_mtx_t *a_mtx);
void		mtx_lock(cw_mtx_t *a_mtx);
cw_bool_t	mtx_trylock(cw_mtx_t *a_mtx);
void		mtx_unlock(cw_mtx_t *a_mtx);
