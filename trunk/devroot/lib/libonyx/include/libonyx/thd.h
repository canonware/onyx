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

/* GNU pth. */
#ifdef CW_PTH
#undef CW_THD_GENERIC_SR
#endif

#ifdef CW_THD_GENERIC_SR
/* The generic suspend/resume mechanism uses signals (using pthread_kill()).
 * This is rather expensive, depending on the OS, but it does not violate
 * portability.  The only issue with this mechanism is that it requires two
 * signals that cannot otherwise be used by the thread being suspended/resumed.
 * On most OSs, SIGUSR1 and SIGUSR2 are the logical choices. */
#define CW_THD_SIGSUSPEND SIGUSR1
#define CW_THD_SIGRESUME SIGUSR2
#endif

/* Minimum thread stack size. */
#define CW_THD_MINSTACK 524288

cw_thd_t *
thd_new(void *(*a_start_func)(void *), void *a_arg, bool a_suspendible);

void
thd_delete(cw_thd_t *a_thd);

void *
thd_join(cw_thd_t *a_thd);

cw_thd_t *
thd_self(void);

#ifdef CW_PTH
#define thd_yield() pth_yield(NULL)
#else
#define thd_yield() sched_yield()
#endif

void
thd_sigmask(int a_how, const sigset_t *a_set, sigset_t *r_oset);

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

bool
thd_trysuspend(cw_thd_t *a_thd);

void
thd_resume(cw_thd_t *a_thd);
