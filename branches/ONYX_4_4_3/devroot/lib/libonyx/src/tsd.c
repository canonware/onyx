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

#include "../include/libonyx/libonyx.h"

#include <sys/time.h>
#include <errno.h>

void
tsd_new(cw_tsd_t *a_tsd, void (*a_func)(void *))
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_tsd);

#ifdef CW_PTH
    if (pth_key_create(&a_tsd->key, a_func) == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_key_create(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_key_create(&a_tsd->key, a_func);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pthread_key_create(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}

void
tsd_delete(cw_tsd_t *a_tsd)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_tsd);

#ifdef CW_PTH
    if (pth_key_delete(a_tsd->key) == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_key_delete(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_key_delete(a_tsd->key);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pthread_key_delete(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}

void *
tsd_get(cw_tsd_t *a_tsd)
{
    void *retval;

    cw_check_ptr(a_tsd);

#ifdef CW_PTH
    retval = pth_key_getdata(a_tsd->key);
#endif
#ifdef CW_PTHREADS
    retval = pthread_getspecific(a_tsd->key);
#endif

    return retval;
}

void
tsd_set(cw_tsd_t *a_tsd, void *a_val)
{
#ifdef CW_PTHREADS
    int error;
#endif

    cw_check_ptr(a_tsd);

#ifdef CW_PTH
    if (pth_key_setdata(a_tsd->key, a_val) == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_key_setdata(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    error = pthread_setspecific(a_tsd->key, a_val);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pthread_setspecific(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
}
