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

/* Pseudo-opaque types. */
typedef struct cw_rwl_s cw_rwl_t;

struct cw_rwl_s {
	cw_bool_t is_malloced;
	cw_mtx_t lock;
	cw_cnd_t read_wait;
	cw_cnd_t write_wait;
	cw_uint32_t num_readers;
	cw_uint32_t num_writers;
	cw_uint32_t read_waiters;
	cw_uint32_t write_waiters;
};

cw_rwl_t *rwl_new(cw_rwl_t *a_rwl);

void    rwl_delete(cw_rwl_t *a_rwl);

void    rwl_rlock(cw_rwl_t *a_rwl);

void    rwl_runlock(cw_rwl_t *a_rwl);

void    rwl_wlock(cw_rwl_t *a_rwl);

void    rwl_wunlock(cw_rwl_t *a_rwl);
