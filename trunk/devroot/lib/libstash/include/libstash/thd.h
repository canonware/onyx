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

/* Pseudo-opaque type. */
typedef struct cw_thd_s cw_thd_t;

/*
 * Define whether to use the generic thread suspend/resume mechanism on
 * FreeBSD.
 */
#ifdef _CW_OS_FREEBSD
#define _CW_THD_FREEBSD_SR
#else
#define _CW_THD_GENERIC_SR
#endif

struct cw_thd_s {
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
	cw_uint32_t	magic;
#endif
	pthread_t	thread;
	void		*(*start_func)(void *);
	void		*start_arg;
#ifdef _CW_THD_GENERIC_SR
	sem_t		sem;	/* For suspend/resume. */
#endif
	cw_bool_t	suspended;
	cw_mtx_t	crit_lock;
};

void	thd_new(cw_thd_t *a_thd, void *(*a_start_func)(void *), void *a_arg);
void	thd_delete(cw_thd_t *a_thd);
void	*thd_join(cw_thd_t *a_thd);
cw_thd_t *thd_self(void);
#define	thd_yield() sched_yield()
#define	thd_sigmask(a, b) pthread_sigmask((a), (b), NULL)
void	thd_crit_enter(void);
void	thd_crit_leave(void);
void	thd_suspend(cw_thd_t *a_thd);
cw_bool_t thd_trysuspend(cw_thd_t *a_thd);
void	thd_resume(cw_thd_t *a_thd);
