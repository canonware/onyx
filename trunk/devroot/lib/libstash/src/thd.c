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

#include "../include/libstash/libstash.h"

#include <sys/time.h>
#include <errno.h>

#ifdef _CW_OS_FREEBSD
#include <pthread_np.h>
#endif

#ifdef _LIBSTASH_DBG
#define _CW_THD_MAGIC 0x5638638e
#endif

#ifdef _LIBSTASH_DBG
cw_bool_t	cw_g_thd_initialized = FALSE;
#endif

/* Special thd structure for initial thread, needed for critical sections. */
cw_thd_t	cw_g_thd;

/* For thd_self(). */
cw_tsd_t	cw_g_thd_self_key;

static void	*thd_p_start_func(void *a_arg);
static void	thd_p_suspend(cw_thd_t *a_thd);

#ifdef _CW_THD_GENERIC_SR
/*
 * The generic suspend/resume mechanism uses signals (using pthread_kill()).
 * This is rather expensive, depending on the OS, but it does not violate
 * portability.  The only issue with this mechanism is that it requires one
 * signal that cannot otherwise be used by the thread being suspended/resumed.
 * On most OSs, SIGUSR1 or SIGUSR2 is the logical choice.
 */
#ifdef _CW_OS_LINUX
/*
 * I don't whether this signal can be safely used, but SIGUSR[12] definitely
 * won't work with LinuxThreads.
 */
#define _CW_THD_SIGSR		SIGUNUSED
#elif (defined(_CW_OS_FREEBSD))
/*
 * SIGUSR[12] are used by the linuxthreads port on FreeBSD, so use another
 * signal to allow libstash to work even if linked with linuxthreads.
 */
#define _CW_THD_SIGSR		SIGXCPU
#else
#define _CW_THD_SIGSR		SIGUSR1
#endif

static void	thd_p_sr_handle(int a_signal);
#endif

void
thd_l_init(void)
{
	_cw_assert(cw_g_thd_initialized == FALSE);

#ifdef _CW_THD_GENERIC_SR
	int			error;
	struct sigaction	action;

	/*
	 * Install a signal handler for suspend and resume.
	 */
	action.sa_flags = 0;
	action.sa_handler = thd_p_sr_handle;
	sigemptyset(&action.sa_mask);
	error = sigaction(_CW_THD_SIGSR, &action, NULL);
	if (error == -1) {
		_cw_out_put_e("Error in sigaction(): [s]\n",
		    strerror(error));
		abort();
	}
#endif
	tsd_new(&cw_g_thd_self_key, NULL);
	
	/* Initialize the main thread's thd structure. */
	cw_g_thd.thread = pthread_self();
	cw_g_thd.start_func = NULL;
	cw_g_thd.start_arg = NULL;
#ifdef _CW_THD_GENERIC_SR
	error = sem_init(&cw_g_thd.sem, 0, 0);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in sem_init(): [s]\n", strerror(error));
		abort();
	}
#endif
	cw_g_thd.suspended = FALSE;
	mtx_new(&cw_g_thd.crit_lock);
#ifdef _LIBSTASH_DBG
	cw_g_thd.magic = _CW_THD_MAGIC;
#endif
	/* Make thd_self() work for the main thread. */
	tsd_set(&cw_g_thd_self_key, (void *)&cw_g_thd);
	
#ifdef _LIBSTASH_DBG
	cw_g_thd_initialized = TRUE;
#endif
}

void
thd_l_shutdown(void)
{
#ifdef _CW_THD_GENERIC_SR
	int	error;
#endif

	_cw_assert(cw_g_thd_initialized);
	mtx_delete(&cw_g_thd.crit_lock);
#ifdef _CW_THD_GENERIC_SR
	error = sem_destroy(&cw_g_thd.sem);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in sem_destroy(): [s]\n", strerror(error));
		abort();
	}
#endif
#ifdef _LIBSTASH_DBG
	memset(&cw_g_thd, 0x5a, sizeof(cw_thd_t));
	cw_g_thd_initialized = FALSE;
#endif
}

void
thd_new(cw_thd_t *a_thd, void *(*a_start_func)(void *), void *a_arg)
{
	int	error;

	_cw_check_ptr(a_thd);
	_cw_assert(cw_g_thd_initialized);

	a_thd->start_func = a_start_func;
	a_thd->start_arg = a_arg;
#ifdef _CW_THD_GENERIC_SR
	error = sem_init(&a_thd->sem, 0, 0);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in sem_init(): [s]\n", strerror(error));
		abort();
	}
#endif
	a_thd->suspended = FALSE;
	mtx_new(&a_thd->crit_lock);
#ifdef _LIBSTASH_DBG
	a_thd->magic = _CW_THD_MAGIC;
#endif

	error = pthread_create(&a_thd->thread, NULL, thd_p_start_func,
	    (void *)a_thd);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_create(): [s]\n", strerror(error));
		abort();
	}
}

void
thd_delete(cw_thd_t *a_thd)
{
	int	error;

	_cw_check_ptr(a_thd);
	_cw_assert(a_thd->magic == _CW_THD_MAGIC);
	_cw_assert(cw_g_thd_initialized);

	mtx_delete(&a_thd->crit_lock);
#ifdef _CW_THD_GENERIC_SR
	error = sem_destroy(&a_thd->sem);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in sem_destroy(): [s]\n", strerror(error));
		abort();
	}
#endif
	error = pthread_detach(a_thd->thread);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_detach(): [s]\n", strerror(error));
		abort();
	}
#ifdef _LIBSTASH_DBG
	memset(a_thd, 0x5a, sizeof(cw_thd_t));
#endif
}

void *
thd_join(cw_thd_t *a_thd)
{
	void	*retval;
	int	error;

	_cw_check_ptr(a_thd);
	_cw_assert(a_thd->magic == _CW_THD_MAGIC);
	_cw_assert(cw_g_thd_initialized);

	mtx_delete(&a_thd->crit_lock);
	error = pthread_join(a_thd->thread, &retval);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_join(): [s]\n", strerror(error));
		abort();
	}
#ifdef _LIBSTASH_DBG
	memset(a_thd, 0x5a, sizeof(cw_thd_t));
#endif
	return retval;
}

cw_thd_t *
thd_self(void)
{
	cw_thd_t	*retval;

	retval = (cw_thd_t *)tsd_get(&cw_g_thd_self_key);

	_cw_check_ptr(retval);
	_cw_assert(retval->magic == _CW_THD_MAGIC);
	_cw_assert(cw_g_thd_initialized);

	return retval;
}

void
thd_crit_enter(void)
{
	cw_thd_t	*thd;

	_cw_assert(cw_g_thd_initialized);
	
	thd = thd_self();
	_cw_check_ptr(thd);
	_cw_assert(thd->magic == _CW_THD_MAGIC);
	mtx_lock(&thd->crit_lock);
}

void
thd_crit_leave(void)
{
	cw_thd_t	*thd;

	_cw_assert(cw_g_thd_initialized);

	thd = thd_self();
	_cw_check_ptr(thd);
	_cw_assert(thd->magic == _CW_THD_MAGIC);
	mtx_unlock(&thd->crit_lock);
}


void
thd_suspend(cw_thd_t *a_thd)
{
	_cw_check_ptr(a_thd);
	_cw_assert(a_thd->magic == _CW_THD_MAGIC);
	_cw_assert(cw_g_thd_initialized);

	mtx_lock(&a_thd->crit_lock);

	thd_p_suspend(a_thd);
}

cw_bool_t
thd_trysuspend(cw_thd_t *a_thd)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_thd);
	_cw_assert(a_thd->magic == _CW_THD_MAGIC);
	_cw_assert(cw_g_thd_initialized);

	if (mtx_trylock(&a_thd->crit_lock)) {
		retval = TRUE;
		goto RETURN;
	}

	thd_p_suspend(a_thd);

	retval = FALSE;
	RETURN:
	return retval;
}

void
thd_resume(cw_thd_t *a_thd)
{
	int	error;

	_cw_check_ptr(a_thd);
	_cw_assert(a_thd->magic == _CW_THD_MAGIC);
	_cw_assert(cw_g_thd_initialized);

#ifdef _CW_THD_GENERIC_SR
	error = pthread_kill(a_thd->thread, _CW_THD_SIGSR);
	if (error != 0) {
		_cw_out_put_e("Error in pthread_kill(): [s]\n",
		    strerror(error));
		abort();
	}
#endif
#ifdef _CW_THD_FREEBSD_SR
	error = pthread_resume_np(a_thd->thread);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_resume_np(): [s]\n",
		    strerror(error));
		abort();
	}
#endif
	mtx_unlock(&a_thd->crit_lock);
}

static void *
thd_p_start_func(void *a_arg)
{
	cw_thd_t	*thd = (cw_thd_t *)a_arg;

	_cw_assert(cw_g_thd_initialized);

	tsd_set(&cw_g_thd_self_key, (void *)thd);
	
	return thd->start_func(thd->start_arg);
}

static void
thd_p_suspend(cw_thd_t *a_thd)
{
	int	error;

#ifdef _CW_THD_GENERIC_SR
	error = pthread_kill(a_thd->thread, _CW_THD_SIGSR);
	if (error != 0) {
		_cw_out_put_e("Error in pthread_kill(): [s]\n",
		    strerror(error));
		abort();
	}
	error = sem_wait(&a_thd->sem);
	if (error != 0) {
		_cw_out_put_e("Error in sem_wait(): [s]\n", strerror(error));
		abort();
	}
#endif
#ifdef _CW_THD_FREEBSD_SR
	error = pthread_resume_np(a_thd->thread);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_resume_np(): [s]\n",
		    strerror(error));
		abort();
	}
#endif
}

#ifdef _CW_THD_GENERIC_SR
static void
thd_p_sr_handle(int a_signal)
{
	sigset_t	set;
	cw_thd_t	*thd;

	thd = thd_self();
	if (thd->suspended == FALSE) {
		/* Block all signals except _CW_THD_SIGSR while
		 * suspended. */
		sigfillset(&set);
		sigdelset(&set, _CW_THD_SIGSR);
		thd->suspended = TRUE;
		sem_post(&thd->sem);
		sigsuspend(&set);
	} else
		thd->suspended = FALSE;
}
#endif
