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

#include "../include/libstash/libstash.h"

#include <sys/time.h>
#include <errno.h>

#ifdef _CW_OS_FREEBSD
#include <pthread_np.h>
#endif

/* Control internal initialization. */
pthread_once_t	cw_g_thd_once = PTHREAD_ONCE_INIT;

/* For thd_self(). */
cw_tsd_t	cw_g_thd_self_key;

static void	thd_p_once(void);
static void	thd_p_suspend(cw_thd_t *a_thd);

#ifdef _CW_THD_GENERIC_SR
/*
 * The generic suspend/resume mechanism uses signals (using pthread_kill()).
 * This is rather expensive, depending on the OS, but it does not violate
 * portability.  The only issue with this mechanism is that it requires two
 * signals that cannot otherwise be used by the thread being suspended/resumed.
 * On most OSs, SIGUSR1 and SIGUSR2 are the logical choice.
 */
#ifdef _CW_OS_LINUX
/*
 * I don't whether these signals can be safely used, but SIGUSR[12] definitely
 * won't work with LinuxThreads.
 */
#define _CW_THD_SIGSUSPEND	SIGPWR
#define _CW_THD_SIGRESUME	SIGUNUSED
#else
#define _CW_THD_SIGSUSPEND	SIGUSR1
#define _CW_THD_SIGRESUME	SIGUSR2
#endif

static void	thd_p_suspend_handle(int a_signal);
static void	thd_p_resume_handle(int a_signal);
#endif

void
thd_new(cw_thd_t *a_thd, void *(*a_start_func)(void *), void *a_arg)
{
	int	error;

	_cw_check_ptr(a_thd);

	/* Initialize the thd_self() tsd key. */
	thd_p_once();
	tsd_set(&cw_g_thd_self_key, (void *)a_thd);

	error = pthread_create(&a_thd->thread, NULL, a_start_func, a_arg);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_create(): [s]\n", strerror(error));
		abort();
	}
#ifdef _CW_THD_GENERIC_SR
	error = sem_init(&a_thd->sem, 0, 0);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in sem_init(): [s]\n", strerror(error));
		abort();
	}
#endif
	mtx_new(&a_thd->crit_lock);
}

void
thd_delete(cw_thd_t *a_thd)
{
	int	error;

	_cw_check_ptr(a_thd);

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
}

cw_thd_t *
thd_self(void)
{
	return (cw_thd_t *)tsd_get(&cw_g_thd_self_key);
}

void *
thd_join(cw_thd_t *a_thd)
{
	void	*retval;
	int	error;

	_cw_check_ptr(a_thd);

	mtx_delete(&a_thd->crit_lock);
	error = pthread_join(a_thd->thread, &retval);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_join(): [s]\n", strerror(error));
		abort();
	}
	return retval;
}

void
thd_crit_enter(cw_thd_t *a_thd)
{
	_cw_check_ptr(a_thd);

	mtx_lock(&a_thd->crit_lock);
}

void
thd_crit_leave(cw_thd_t *a_thd)
{
	_cw_check_ptr(a_thd);

	mtx_unlock(&a_thd->crit_lock);
}


void
thd_suspend(cw_thd_t *a_thd)
{
	_cw_check_ptr(a_thd);

	mtx_lock(&a_thd->crit_lock);

	thd_p_suspend(a_thd);
}

cw_bool_t
thd_trysuspend(cw_thd_t *a_thd)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_thd);

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

#ifdef _CW_THD_GENERIC_SR
	error = pthread_kill(a_thd->thread, _CW_THD_SIGRESUME);
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

static void
thd_p_once(void)
{
#ifdef _CW_THD_GENERIC_SR
	int			error;
	struct sigaction	action;

	/*
	 * Install signal handlers for suspend and resume.
	 */
	action.sa_flags = 0;
	action.sa_handler = thd_p_suspend_handle;
	sigemptyset(&action.sa_mask);
	error = sigaction(_CW_THD_SIGSUSPEND, &action, NULL);
	if (error == -1) {
		_cw_out_put_e("Error in sigaction(): [s]\n",
		    strerror(error));
		abort();
	}

	action.sa_flags = 0;
	action.sa_handler = thd_p_resume_handle;
	sigemptyset(&action.sa_mask);
	error = sigaction(_CW_THD_SIGRESUME, &action, NULL);
	if (error == -1) {
		_cw_out_put_e("Error in sigaction(): [s]\n",
		    strerror(error));
		abort();
	}
#endif
	tsd_new(&cw_g_thd_self_key, NULL);
}

static void
thd_p_suspend(cw_thd_t *a_thd)
{
	int	error;

#ifdef _CW_THD_GENERIC_SR
	error = pthread_kill(a_thd->thread, _CW_THD_SIGSUSPEND);
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
thd_p_suspend_handle(int a_signal)
{
	sigset_t	set;
	cw_thd_t	*thd;

/*  	write(2, "suspend\n", 8); */

	/* Block all signals except _CW_THD_SIGRESUME while suspended. */
	sigfillset(&set);
	sigdelset(&set, _CW_THD_SIGRESUME);
	thd = (cw_thd_t *)tsd_get(&cw_g_thd_self_key);
	sem_post(&thd->sem);
	sigsuspend(&set);
}

static void
thd_p_resume_handle(int a_signal)
{
/*  	write(2, "resume\n", 7); */
}
#endif
