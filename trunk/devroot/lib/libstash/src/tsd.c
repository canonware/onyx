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

void
tsd_new(cw_tsd_t *a_tsd, void (*a_func)(void *))
{
	int	error;

	_cw_check_ptr(a_tsd);

	error = pthread_key_create(&a_tsd->key, a_func);
	if (error) {
		fprintf(stderr,
		    "%s:%u:%s(): Error in pthread_key_create(): %s\n",
		    __FILE__, __LINE__, __FUNCTION__, strerror(error));
		abort();
	}
}

void
tsd_delete(cw_tsd_t *a_tsd)
{
	int	error;

	_cw_check_ptr(a_tsd);

	error = pthread_key_delete(a_tsd->key);
	if (error) {
		fprintf(stderr,
		    "%s:%u:%s(): Error in pthread_key_delete(): %s\n",
		    __FILE__, __LINE__, __FUNCTION__, strerror(error));
		abort();
	}
}

void *
tsd_get(cw_tsd_t *a_tsd)
{
	void	*retval;

	_cw_check_ptr(a_tsd);

	retval = pthread_getspecific(a_tsd->key);

	return retval;
}

void
tsd_set(cw_tsd_t *a_tsd, void *a_val)
{
	int	error;

	_cw_check_ptr(a_tsd);

	error = pthread_setspecific(a_tsd->key, a_val);
	if (error) {
		fprintf(stderr,
		    "%s:%u:%s(): Error in pthread_setspecific(): %s\n",
		    __FILE__, __LINE__, __FUNCTION__, strerror(error));
		abort();
	}
}
