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
typedef struct cw_thd_s cw_thd_t;

struct cw_thd_s {
	cw_bool_t is_malloced;
	pthread_t thread;
};

cw_thd_t *thd_new(cw_thd_t *a_thd, void *(*a_start_func) (void *), void *a_arg);

void    thd_delete(cw_thd_t *a_thd);

void   *thd_join(cw_thd_t *a_thd);

#define thd_yield() sched_yield()

#define thd_sigmask(a, b) pthread_sigmask((a), (b), NULL)
