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

void
thd_new(cw_thd_t *a_thd, void *(*a_start_func)(void *), void *a_arg)
{
	int	error;

	_cw_check_ptr(a_thd);

	error = pthread_create(&a_thd->thread, NULL, a_start_func, a_arg);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_create(): [s]\n", strerror(error));
		abort();
	}
	mtx_new(&a_thd->crit_lock);
}

void
thd_delete(cw_thd_t *a_thd)
{
	int	error;

	_cw_check_ptr(a_thd);

	mtx_delete(&a_thd->crit_lock);
	error = pthread_detach(a_thd->thread);
	if (error) {
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Error in pthread_detach(): [s]\n", strerror(error));
		abort();
	}
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
#ifdef _CW_OS_FREEBSD
	{
		int	error;

		error = pthread_suspend_np(a_thd->thread);
		if (error) {
			out_put_e(NULL, NULL, 0, __FUNCTION__,
			    "Error in pthread_suspend_np(): [s]\n",
			    strerror(error));
			abort();
		}
	}
#else
#error "OS not supported"
#endif
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
#ifdef _CW_OS_FREEBSD
	{
		int	error;

		error = pthread_suspend_np(a_thd->thread);
		if (error) {
			out_put_e(NULL, NULL, 0, __FUNCTION__,
			    "Error in pthread_suspend_np(): [s]\n",
			    strerror(error));
			abort();
		}
	}
#else
#error "OS not supported"
#endif

	retval = FALSE;
	RETURN:
	return retval;
}
void
thd_resume(cw_thd_t *a_thd)
{
	_cw_check_ptr(a_thd);

#ifdef _CW_OS_FREEBSD
	{
		int	error;

		error = pthread_resume_np(a_thd->thread);
		if (error) {
			out_put_e(NULL, NULL, 0, __FUNCTION__,
			    "Error in pthread_resume_np(): [s]\n",
			    strerror(error));
			abort();
		}
	}
#else
#error "OS not supported"
#endif
	mtx_unlock(&a_thd->crit_lock);
}
