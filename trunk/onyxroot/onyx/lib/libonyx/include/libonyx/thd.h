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

/* Opaque type. */
typedef struct cw_thd_s cw_thd_t;

/* Determine what mechanism to use for thread suspend/resume.  The generic
 * version relies on signals, which tends to be much slower than native
 * implementations of suspend/resume, so only use it as a last resort. */
#define CW_THD_GENERIC_SR

/* FreeBSD-specific extensions (pthread_{suspend,resume}_np()). */
#ifdef CW_FTHREADS
#undef CW_THD_GENERIC_SR
#endif

/* Mach threads. */
#ifdef CW_MTHREADS
#undef CW_THD_GENERIC_SR
#endif

/* Solaris threads. */
#ifdef CW_STHREADS
#undef CW_THD_GENERIC_SR
#endif

/* Minimum thread stack size. */
#define CW_THD_MINSTACK 524288

cw_thd_t *
thd_new(void *(*a_start_func)(void *), void *a_arg, cw_bool_t a_suspendible);

void
thd_delete(cw_thd_t *a_thd);

void *
thd_join(cw_thd_t *a_thd);

cw_thd_t *
thd_self(void);

#define thd_yield() sched_yield()

void thd_sigmask(int a_how, const sigset_t *a_set, sigset_t *r_oset);

void
thd_crit_enter(void);

void
thd_crit_leave(void);

void
thd_single_enter(void);

void
thd_single_leave(void);

void
thd_suspend(cw_thd_t *a_thd);

cw_bool_t
thd_trysuspend(cw_thd_t *a_thd);

void
thd_resume(cw_thd_t *a_thd);
