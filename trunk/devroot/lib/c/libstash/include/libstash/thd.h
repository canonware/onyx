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

/* Opaque type. */
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

cw_thd_t *thd_new(void *(*a_start_func)(void *), void *a_arg, cw_bool_t
    a_suspendible);
void	thd_delete(cw_thd_t *a_thd);
void	*thd_join(cw_thd_t *a_thd);
cw_thd_t *thd_self(void);
#define	thd_yield() sched_yield()
void	thd_sigmask(int a_how, const sigset_t *a_set, sigset_t *r_oset);
void	thd_crit_enter(void);
void	thd_crit_leave(void);
void	thd_single_enter(void);
void	thd_single_leave(void);
void	thd_suspend(cw_thd_t *a_thd);
cw_bool_t thd_trysuspend(cw_thd_t *a_thd);
void	thd_resume(cw_thd_t *a_thd);
