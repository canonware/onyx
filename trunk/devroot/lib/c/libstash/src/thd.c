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

struct cw_thd_s {
#ifdef _LIBSTASH_DBG
	cw_uint32_t	magic;
#endif
	pthread_t	thread;
	void		*(*start_func)(void *);
	void		*start_arg;
	cw_bool_t	suspendible:1;
#ifdef _CW_THD_GENERIC_SR
	sem_t		sem;		/* For suspend/resume. */
	cw_bool_t	suspended:1;
#endif
	cw_mtx_t	crit_lock;
	cw_bool_t	singled:1;	/* Suspended by thd_single_enter()? */
	qr(cw_thd_t)	link;
	cw_bool_t	delete:1;
};

#ifdef _LIBSTASH_DBG
static cw_bool_t cw_g_thd_initialized = FALSE;
#endif

/* Special thd structure for initial thread, needed for critical sections. */
static cw_thd_t	cw_g_thd;

/* Protects the ring of thd's in thd_single_{enter,leave}(). */
static cw_mtx_t	cw_g_thd_single_lock;

/* For thd_self(). */
cw_tsd_t	cw_g_thd_self_key;

static void	thd_p_delete(cw_thd_t *a_thd);
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
#ifdef _CW_THD_GENERIC_SR
	int			error;
	struct sigaction	action;

	/*
	 * Install a signal handler for suspend and resume.  Restart system
	 * calls in order to reduce the impact of signals on the application
	 */
	action.sa_flags = SA_RESTART;
	action.sa_handler = thd_p_sr_handle;
	sigemptyset(&action.sa_mask);
	error = sigaction(_CW_THD_SIGSR, &action, NULL);
	if (error == -1) {
		_cw_out_put_e("Error in sigaction(): [s]\n",
		    strerror(error));
		abort();
	}
#endif
	_cw_assert(cw_g_thd_initialized == FALSE);

	mtx_new(&cw_g_thd_single_lock);
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
	cw_g_thd.suspended = FALSE;
#endif
	mtx_new(&cw_g_thd.crit_lock);
	cw_g_thd.singled = FALSE;
	qr_new(&cw_g_thd, link);
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
	tsd_delete(&cw_g_thd_self_key);
	mtx_delete(&cw_g_thd_single_lock);
#ifdef _LIBSTASH_DBG
	memset(&cw_g_thd, 0x5a, sizeof(cw_thd_t));
	cw_g_thd_initialized = FALSE;
#endif
}

cw_thd_t *
thd_new(void *(*a_start_func)(void *), void *a_arg, cw_bool_t a_suspendible)
{
	cw_thd_t	*retval;
	int		error;

	_cw_assert(cw_g_thd_initialized);

	retval = (cw_thd_t *)_cw_malloc(sizeof(cw_thd_t));

	retval->start_func = a_start_func;
	retval->start_arg = a_arg;
	retval->suspendible = a_suspendible;
#ifdef _CW_THD_GENERIC_SR
	error = sem_init(&retval->sem, 0, 0);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in sem_init(): [s]\n", strerror(error));
		abort();
	}
	retval->suspended = FALSE;
#endif
	mtx_new(&retval->crit_lock);
	retval->singled = FALSE;
	retval->delete = FALSE;
#ifdef _LIBSTASH_DBG
	retval->magic = _CW_THD_MAGIC;
#endif

	error = pthread_create(&retval->thread, NULL, thd_p_start_func,
	    (void *)retval);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_create(): [s]\n", strerror(error));
		abort();
	}

	return retval;
}

void
thd_delete(cw_thd_t *a_thd)
{
	int		error;

	_cw_check_ptr(a_thd);
	_cw_assert(a_thd->magic == _CW_THD_MAGIC);
	_cw_assert(cw_g_thd_initialized);

	error = pthread_detach(a_thd->thread);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_detach(): [s]\n", strerror(error));
		abort();
	}

	thd_p_delete(a_thd);
}

void *
thd_join(cw_thd_t *a_thd)
{
	void	*retval;
	int	error;

	_cw_check_ptr(a_thd);
	_cw_assert(a_thd->magic == _CW_THD_MAGIC);
	_cw_assert(cw_g_thd_initialized);

	error = pthread_join(a_thd->thread, &retval);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_join(): [s]\n", strerror(error));
		abort();
	}

	mtx_delete(&a_thd->crit_lock);
#ifdef _CW_THD_GENERIC_SR
	error = sem_destroy(&a_thd->sem);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in sem_destroy(): [s]\n", strerror(error));
		abort();
	}
#endif
	_cw_free(a_thd);
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
thd_sigmask(int a_how, const sigset_t *a_set, sigset_t *r_oset)
{
	_cw_assert(a_how == SIG_BLOCK || a_how == SIG_UNBLOCK || a_how ==
	    SIG_SETMASK);

#ifdef _CW_THD_GENERIC_SR
	{
		sigset_t	set;

		/*
		 * Make a local copy of the signal mask, then modify it
		 * appropriately to keep from breaking suspend/resume.
		 */
		memcpy(&set, a_set, sizeof(sigset_t));
		sigdelset(&set, _CW_THD_SIGSR);
		pthread_sigmask(a_how, &set, r_oset);
	}
#else
	pthread_sigmask(a_how, a_set, r_oset);
#endif
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
thd_single_enter(void)
{
	cw_thd_t	*self, *thd;

	_cw_assert(cw_g_thd_initialized);

	self = thd_self();
	_cw_check_ptr(self);
	_cw_assert(self->magic == _CW_THD_MAGIC);

	mtx_lock(&cw_g_thd_single_lock);
	qr_foreach(thd, &cw_g_thd, link) {
		if (thd != self) {
			mtx_lock(&thd->crit_lock);
			thd_p_suspend(thd);
			thd->singled = TRUE;
		}
	}
	mtx_unlock(&cw_g_thd_single_lock);
}

void
thd_single_leave(void)
{
	cw_thd_t	*self, *thd;

	_cw_assert(cw_g_thd_initialized);

	self = thd_self();
	_cw_check_ptr(self);
	_cw_assert(self->magic == _CW_THD_MAGIC);

	mtx_lock(&cw_g_thd_single_lock);
	qr_foreach(thd, &cw_g_thd, link) {
		if (thd->singled) {
			thd_resume(thd);
			thd->singled = FALSE;
		}
	}
	mtx_unlock(&cw_g_thd_single_lock);
}

void
thd_suspend(cw_thd_t *a_thd)
{
	_cw_check_ptr(a_thd);
	_cw_assert(a_thd->magic == _CW_THD_MAGIC);
	_cw_assert(cw_g_thd_initialized);

	mtx_lock(&a_thd->crit_lock);

	/*
	 * Protect suspension so that we don't risk deadlocking with a thread
	 * entering a single section.
	 */
	mtx_lock(&cw_g_thd_single_lock);
	thd_p_suspend(a_thd);
	mtx_unlock(&cw_g_thd_single_lock);
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

	mtx_lock(&cw_g_thd_single_lock);
	thd_p_suspend(a_thd);
	mtx_unlock(&cw_g_thd_single_lock);

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
	mtx_unlock(&a_thd->crit_lock);
}

static void
thd_p_delete(cw_thd_t *a_thd)
{
	cw_bool_t	delete;
#ifdef _CW_THD_GENERIC_SR
	int		error;
#endif

	/* Determine whether to delete the object now. */
	mtx_lock(&a_thd->crit_lock);
	if (a_thd->delete)
		delete = TRUE;
	else {
		delete = FALSE;
		a_thd->delete = TRUE;
	}
	mtx_unlock(&a_thd->crit_lock);
	
	if (delete) {
		mtx_delete(&a_thd->crit_lock);
#ifdef _CW_THD_GENERIC_SR
		error = sem_destroy(&a_thd->sem);
		if (error) {
			out_put_e(NULL, NULL, 0, __FUNCTION__,
			    "Error in sem_destroy(): [s]\n", strerror(error));
			abort();
		}
#endif
		_cw_free(a_thd);
	}
}

static void *
thd_p_start_func(void *a_arg)
{
	void		*retval;
	cw_thd_t	*thd = (cw_thd_t *)a_arg;

	_cw_assert(cw_g_thd_initialized);

	tsd_set(&cw_g_thd_self_key, (void *)thd);

	if (thd->suspendible) {
		/* Insert this thread into the thread ring. */
		mtx_lock(&cw_g_thd_single_lock);
		qr_before_insert(&cw_g_thd, thd, link);
		mtx_unlock(&cw_g_thd_single_lock);
	
		retval = thd->start_func(thd->start_arg);

		/* Remove this thread from the thread ring. */
		mtx_lock(&cw_g_thd_single_lock);
		qr_remove(thd, link);
		mtx_unlock(&cw_g_thd_single_lock);
	} else
		retval = thd->start_func(thd->start_arg);

	thd_p_delete(thd);

	return retval;
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
	error = pthread_suspend_np(a_thd->thread);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_suspend_np(): [s]\n",
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

	/*
	 * Only enter the following block of code if we're being suspended;
	 * this function also gets entered again when the resume signal is sent.
	 */
	if (thd->suspended == FALSE) {
		thd->suspended = TRUE;
		/*
		 * Block all signals except _CW_THD_SIGSR while suspended.
		 */
		sigfillset(&set);
		sigdelset(&set, _CW_THD_SIGSR);

		/* Tell suspender we're suspended. */
		sem_post(&thd->sem);

		/* Suspend. */
		sigsuspend(&set);

		/* Tell resumer we're resumed. */
		thd->suspended = FALSE;
		sem_post(&thd->sem);
	}
}
#endif
