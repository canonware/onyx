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

#ifdef CW_FTHREADS
#include <pthread_np.h>
#endif

#ifdef CW_STHREADS
#include <thread.h>
#endif

#ifdef CW_DBG
#define CW_THD_MAGIC 0x5638638e
#endif

struct cw_thd_s
{
#ifdef CW_DBG
    cw_uint32_t magic;
#endif
    void *(*start_func)(void *);
    void *start_arg;
    cw_mtx_t mtx;
#ifdef CW_PTH
    pth_t pth;
#endif
#ifdef CW_PTHREADS
    pthread_t pthread;
#endif
#ifdef CW_MTHREADS
    thread_t mthread;
#endif
    cw_bool_t suspendible:1;
    cw_bool_t suspended:1; /* Suspended by thd_suspend()? */
    cw_bool_t singled:1; /* Suspended by thd_single_enter()? */
    qr(cw_thd_t) link;
    cw_bool_t delete:1;
};

#ifdef CW_DBG
static cw_bool_t cw_g_thd_initialized = FALSE;
#endif

#ifdef CW_PTH
/* Thread attribute object used for all thread creations. */
static pth_attr_t cw_g_thd_attr;
#endif

#ifdef CW_PTHREADS
/* Thread attribute object used for all thread creations. */
static pthread_attr_t cw_g_thd_attr;
#endif

/* Special thd structure for initial thread, needed for critical sections. */
static cw_thd_t cw_g_thd;

/* Protects the ring of thd's in thd_single_{enter,leave}(). */
static cw_mtx_t cw_g_thd_single_lock;

#ifdef CW_THD_GENERIC_SR
/* For interlocking of suspend. */
static sem_t cw_g_thd_sem;
#endif

/* For thd_self(). */
static cw_tsd_t cw_g_thd_self_key;

static void
thd_p_delete(cw_thd_t *a_thd);
static void *
thd_p_start_func(void *a_arg);
static void
thd_p_suspend(cw_thd_t *a_thd);
static void
thd_p_resume(cw_thd_t *a_thd);

#ifdef CW_THD_GENERIC_SR
static void
thd_p_suspend_handle(int a_signal);
static void
thd_p_resume_handle(int a_signal);
#endif

void
thd_l_init(void)
{
#ifdef CW_PTH
    unsigned stacksize;
#endif
#ifdef CW_PTHREADS
    size_t stacksize;
#endif

#ifdef CW_THD_GENERIC_SR
    int error;
    struct sigaction action;

    /* Install signal handlers for suspend and resume.  Restart system calls in
     * order to reduce the impact of signals on the application. */
    action.sa_flags = SA_RESTART;
    action.sa_handler = thd_p_suspend_handle;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, CW_THD_SIGRESUME);
    error = sigaction(CW_THD_SIGSUSPEND, &action, NULL);
    if (error == -1)
    {
	fprintf(stderr, "%s:%u:%s(): Error in sigaction(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }

    action.sa_flags = SA_RESTART;
    action.sa_handler = thd_p_resume_handle;
    sigemptyset(&action.sa_mask);
    error = sigaction(CW_THD_SIGRESUME, &action, NULL);
    if (error == -1)
    {
	fprintf(stderr, "%s:%u:%s(): Error in sigaction(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }

    /* Initialize the semaphore that is used for suspend interlocking. */
    error = sem_init(&cw_g_thd_sem, 0, 0);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in sem_init(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
    cw_assert(cw_g_thd_initialized == FALSE);

#ifdef CW_PTH
    if (pth_init() == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_init(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }

    /* Create a thread attribute object to be used for all thread creations.
     * Make sure that the thread stack size isn't too tiny. */
    cw_g_thd_attr = pth_attr_new();
    if (pth_attr_get(cw_g_thd_attr, PTH_ATTR_STACK_SIZE, &stacksize) == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_attr_get(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
    if (stacksize < CW_THD_MINSTACK)
    {
	if (pth_attr_set(cw_g_thd_attr, PTH_ATTR_STACK_SIZE, CW_THD_MINSTACK)
	    == FALSE)
	{
	    fprintf(stderr, "%s:%u:%s(): Error in pth_attr_set(): %s\n",
		    __FILE__, __LINE__, __func__, strerror(errno));
	    abort();
	}
    }
#endif
#ifdef CW_PTHREADS
    /* Create a thread attribute object to be used for all thread creations.
     * Make sure that the thread stack size isn't too tiny. */
    pthread_attr_init(&cw_g_thd_attr);
    pthread_attr_getstacksize(&cw_g_thd_attr, &stacksize);
    if (stacksize < CW_THD_MINSTACK)
    {
	pthread_attr_setstacksize(&cw_g_thd_attr, CW_THD_MINSTACK);
    }
#endif

    mtx_new(&cw_g_thd_single_lock);
    tsd_new(&cw_g_thd_self_key, NULL);

    /* Initialize the main thread's thd structure. */
    cw_g_thd.start_func = NULL;
    cw_g_thd.start_arg = NULL;
    mtx_new(&cw_g_thd.mtx);
    mtx_lock(&cw_g_thd.mtx);
#ifdef CW_PTH
    cw_g_thd.pth = pth_self();
#endif
#ifdef CW_PTHREADS
    cw_g_thd.pthread = pthread_self();
#endif
#ifdef CW_MTHREADS
    cw_g_thd.mthread = mach_thread_self();
#endif
    cw_g_thd.suspendible = TRUE;
    cw_g_thd.suspended = FALSE;
    cw_g_thd.singled = FALSE;
    qr_new(&cw_g_thd, link);
#ifdef CW_DBG
    cw_g_thd.magic = CW_THD_MAGIC;
#endif
    /* Make thd_self() work for the main thread. */
    tsd_set(&cw_g_thd_self_key, (void *) &cw_g_thd);
    mtx_unlock(&cw_g_thd.mtx);

#ifdef CW_DBG
    cw_g_thd_initialized = TRUE;
#endif
}

void
thd_l_shutdown(void)
{
#ifdef CW_THD_GENERIC_SR
    int error;
#endif

    cw_assert(cw_g_thd_initialized);

#ifdef CW_PTHREADS
    pthread_attr_destroy(&cw_g_thd_attr);
#endif
#ifdef CW_PTH
    if (pth_attr_destroy(cw_g_thd_attr) == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_attr_destroy(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }

    if (pth_kill() == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_kill(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif

    mtx_delete(&cw_g_thd.mtx);
#ifdef CW_THD_GENERIC_SR
    error = sem_destroy(&cw_g_thd_sem);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in sem_destroy(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
    tsd_delete(&cw_g_thd_self_key);
    mtx_delete(&cw_g_thd_single_lock);
#ifdef CW_DBG
    memset(&cw_g_thd, 0x5a, sizeof(cw_thd_t));
    cw_g_thd_initialized = FALSE;
#endif
}

cw_thd_t *
thd_new(void *(*a_start_func)(void *), void *a_arg, cw_bool_t a_suspendible)
{
    cw_thd_t *retval;
#ifdef CW_PTH
    pth_t pth;
#endif
#ifdef CW_PTHREADS
    pthread_t pthread;
    int error;
#endif

    cw_assert(cw_g_thd_initialized);

    retval = (cw_thd_t *) cw_malloc(sizeof(cw_thd_t));

    retval->start_func = a_start_func;
    retval->start_arg = a_arg;
    mtx_new(&retval->mtx);
    mtx_lock(&retval->mtx);
    retval->suspendible = a_suspendible;
    retval->suspended = FALSE;
    retval->singled = FALSE;
    retval->delete = FALSE;
#ifdef CW_DBG
    retval->magic = CW_THD_MAGIC;
#endif
    mtx_unlock(&retval->mtx);

    /* Thread creation and setting retval->pthread must be atomic with respect
     * to thread suspension if the new thread is suspendible.  There are
     * multiple ways of trying to write this code, and all of them end up
     * requiring that an interlock be used (to avoid race conditions and/or
     * deadlocks, depending on the approach).  Since an interlock
     * (cw_g_thd_single_lock; using retval->mtx could result in deadlock) is
     * mandatory anyway, the pthread field of thd's is universally protected by
     * cw_g_thd_single_lock. */
    mtx_lock(&cw_g_thd_single_lock);

#ifdef CW_PTH
    pth = pth_spawn(cw_g_thd_attr, thd_p_start_func, (void *) retval);
    if (pth == NULL)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_spawn(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }

    /* Set retval->pthread here rather than in thd_p_start_func(), since it's
     * possible to call something like thd_join() before the new thread even
     * gets as far as initializing itself. */
    retval->pth = pth;
#endif
#ifdef CW_PTHREADS
    error = pthread_create(&pthread, &cw_g_thd_attr,
			   thd_p_start_func, (void *) retval);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pthread_create(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }

    /* Set retval->pthread here rather than in thd_p_start_func(), since it's
     * possible to call something like thd_join() before the new thread even
     * gets as far as initializing itself. */
    retval->pthread = pthread;
#endif

    mtx_unlock(&cw_g_thd_single_lock);

    return retval;
}

void
thd_delete(cw_thd_t *a_thd)
{
#ifdef CW_PTH
    pth_t pth;
    pth_attr_t attr;
#endif
#ifdef CW_PTHREADS
    pthread_t pthread;
    int error;
#endif

    cw_check_ptr(a_thd);
    cw_dassert(a_thd->magic == CW_THD_MAGIC);
    cw_assert(cw_g_thd_initialized);

#ifdef CW_PTH
    mtx_lock(&cw_g_thd_single_lock);
    pth = a_thd->pth;
    mtx_unlock(&cw_g_thd_single_lock);

    attr = pth_attr_of(pth);
    if (attr == NULL)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_attr_of(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
    if (pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE) == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_attr_set(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }	
    if (pth_attr_destroy(attr) == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_attr_destroy(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    mtx_lock(&cw_g_thd_single_lock);
    pthread = a_thd->pthread;
    mtx_unlock(&cw_g_thd_single_lock);

    error = pthread_detach(pthread);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pthread_detach(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif

    thd_p_delete(a_thd);
}

void *
thd_join(cw_thd_t *a_thd)
{
    void *retval;
#ifdef CW_PTH
    pth_t pth;
#endif
#ifdef CW_PTHREADS
    pthread_t pthread;
    int error;
#endif

    cw_check_ptr(a_thd);
    cw_dassert(a_thd->magic == CW_THD_MAGIC);
    cw_assert(cw_g_thd_initialized);

#ifdef CW_PTH
    mtx_lock(&cw_g_thd_single_lock);
    pth = a_thd->pth;
    mtx_unlock(&cw_g_thd_single_lock);

    if (pth_join(pth, &retval) == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_join(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_PTHREADS
    mtx_lock(&cw_g_thd_single_lock);
    pthread = a_thd->pthread;
    mtx_unlock(&cw_g_thd_single_lock);

    error = pthread_join(pthread, &retval);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pthread_join(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
    mtx_delete(&a_thd->mtx);
    cw_free(a_thd);
    return retval;
}

cw_thd_t *
thd_self(void)
{
    cw_thd_t *retval;

    retval = (cw_thd_t *) tsd_get(&cw_g_thd_self_key);

    cw_check_ptr(retval);
    cw_dassert(retval->magic == CW_THD_MAGIC);
    cw_assert(cw_g_thd_initialized);

    return retval;
}

void
thd_sigmask(int a_how, const sigset_t *a_set, sigset_t *r_oset)
{
    cw_assert(a_how == SIG_BLOCK || a_how == SIG_UNBLOCK
	      || a_how == SIG_SETMASK);

#ifdef CW_PTH
    pth_sigmask(a_how, a_set, r_oset);
#endif
#ifdef CW_PTHREADS
#ifdef CW_THD_GENERIC_SR
    {
	sigset_t set;

	/* Make a local copy of the signal mask, then modify it appropriately to
	 * keep from breaking suspend/resume. */
	memcpy(&set, a_set, sizeof(sigset_t));
	sigdelset(&set, CW_THD_SIGSUSPEND);
	sigdelset(&set, CW_THD_SIGRESUME);
	pthread_sigmask(a_how, &set, r_oset);
    }
#else
    pthread_sigmask(a_how, a_set, r_oset);
#endif
#endif
}

void
thd_crit_enter(void)
{
    cw_thd_t *thd;

    cw_assert(cw_g_thd_initialized);

    thd = thd_self();
    cw_check_ptr(thd);
    cw_dassert(thd->magic == CW_THD_MAGIC);
    mtx_lock(&thd->mtx);
}

void
thd_crit_leave(void)
{
    cw_thd_t *thd;

    cw_assert(cw_g_thd_initialized);

    thd = thd_self();
    cw_check_ptr(thd);
    cw_dassert(thd->magic == CW_THD_MAGIC);
    mtx_unlock(&thd->mtx);
}

void
thd_single_enter(void)
{
    cw_thd_t *self, *thd;

    cw_assert(cw_g_thd_initialized);

    self = thd_self();
    cw_check_ptr(self);
    cw_dassert(self->magic == CW_THD_MAGIC);

    mtx_lock(&cw_g_thd_single_lock);
    qr_foreach(thd, &cw_g_thd, link)
    {
	if (thd != self && thd->suspended == FALSE)
	{
	    mtx_lock(&thd->mtx);
	    thd_p_suspend(thd);
	    thd->singled = TRUE;
	}
    }
}

void
thd_single_leave(void)
{
    cw_thd_t *thd;

    cw_assert(cw_g_thd_initialized);

    qr_foreach(thd, &cw_g_thd, link)
    {
	if (thd->singled)
	{
	    thd->singled = FALSE;
	    thd_p_resume(thd);
	}
    }
    mtx_unlock(&cw_g_thd_single_lock);
}

void
thd_suspend(cw_thd_t *a_thd)
{
    cw_check_ptr(a_thd);
    cw_dassert(a_thd->magic == CW_THD_MAGIC);
    cw_assert(cw_g_thd_initialized);

    /* Protect suspension so that we don't risk deadlocking with a thread
     * entering a single section. */
    mtx_lock(&cw_g_thd_single_lock);
    mtx_lock(&a_thd->mtx);
    thd_p_suspend(a_thd);
    mtx_unlock(&cw_g_thd_single_lock);
}

cw_bool_t
thd_trysuspend(cw_thd_t *a_thd)
{
    cw_bool_t retval;

    cw_check_ptr(a_thd);
    cw_dassert(a_thd->magic == CW_THD_MAGIC);
    cw_assert(cw_g_thd_initialized);

    mtx_lock(&cw_g_thd_single_lock);
    if (mtx_trylock(&a_thd->mtx))
    {
	retval = TRUE;
	goto RETURN;
    }
    thd_p_suspend(a_thd);

    retval = FALSE;
    RETURN:
    mtx_unlock(&cw_g_thd_single_lock);
    return retval;
}

void
thd_resume(cw_thd_t *a_thd)
{
    cw_check_ptr(a_thd);
    cw_dassert(a_thd->magic == CW_THD_MAGIC);
    cw_assert(cw_g_thd_initialized);

#ifdef CW_THD_GENERIC_SR
    mtx_lock(&cw_g_thd_single_lock);
#endif

    thd_p_resume(a_thd);

#ifdef CW_THD_GENERIC_SR
    mtx_unlock(&cw_g_thd_single_lock);
#endif
}

static void
thd_p_delete(cw_thd_t *a_thd)
{
    cw_bool_t delete;

    /* Determine whether to delete the object now. */
    mtx_lock(&a_thd->mtx);
    if (a_thd->delete)
    {
	delete = TRUE;
    }
    else
    {
	delete = FALSE;
	a_thd->delete = TRUE;
    }
    mtx_unlock(&a_thd->mtx);
	
    if (delete)
    {
	mtx_delete(&a_thd->mtx);
	cw_free(a_thd);
    }
}

static void *
thd_p_start_func(void *a_arg)
{
    void *retval;
    cw_thd_t *thd = (cw_thd_t *) a_arg;

    cw_assert(cw_g_thd_initialized);

    tsd_set(&cw_g_thd_self_key, (void *) thd);

    if (thd->suspendible)
    {
	/* Insert this thread into the thread ring. */
	mtx_lock(&cw_g_thd_single_lock);
#ifdef CW_MTHREADS
	thd->mthread = mach_thread_self();
#endif
	qr_before_insert(&cw_g_thd, thd, link);
	mtx_unlock(&cw_g_thd_single_lock);

	retval = thd->start_func(thd->start_arg);

	/* Remove this thread from the thread ring. */
	mtx_lock(&cw_g_thd_single_lock);
	qr_remove(thd, link);
	mtx_unlock(&cw_g_thd_single_lock);
    }
    else
    {
	retval = thd->start_func(thd->start_arg);
    }

    thd_p_delete(thd);

    return retval;
}

static void
thd_p_suspend(cw_thd_t *a_thd)
{
#ifdef CW_MTHREADS
    kern_return_t mach_error;
#endif
#if (defined(CW_THD_GENERIC_SR) || defined(CW_FTHREADS) || defined(CW_STHREADS))
    int error;
#endif

    a_thd->suspended = TRUE;
#ifdef CW_PTH
    if (pth_suspend(a_thd->pth) == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_suspend(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_THD_GENERIC_SR
    error = pthread_kill(a_thd->pthread, CW_THD_SIGSUSPEND);
    if (error != 0)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pthread_kill(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
    if (sem_wait(&cw_g_thd_sem) != 0)
    {
	fprintf(stderr, "%s:%u:%s(): Error in sem_wait(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_FTHREADS
    error = pthread_suspend_np(a_thd->pthread);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pthread_suspend_np(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
#ifdef CW_STHREADS
    error = thr_suspend(a_thd->pthread);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in thr_suspend(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
#ifdef CW_MTHREADS
    mach_error = thread_suspend(a_thd->mthread);
    if (mach_error != KERN_SUCCESS)
    {
	fprintf(stderr, "%s:%u:%s(): Error in thread_suspend(): %d\n",
		__FILE__, __LINE__, __func__, mach_error);
	abort();
    }
#endif
}

static void
thd_p_resume(cw_thd_t *a_thd)
{
#ifdef CW_PTH
    if (pth_resume(a_thd->pth) == FALSE)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pth_resume(): %s\n",
		__FILE__, __LINE__, __func__, strerror(errno));
	abort();
    }
#endif
#ifdef CW_THD_GENERIC_SR
    int error;

    error = pthread_kill(a_thd->pthread, CW_THD_SIGRESUME);
    if (error != 0)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pthread_kill(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
#ifdef CW_FTHREADS
    int error;

    error = pthread_resume_np(a_thd->pthread);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in pthread_resume_np(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
#ifdef CW_STHREADS
    int error;

    error = thr_continue(a_thd->pthread);
    if (error)
    {
	fprintf(stderr, "%s:%u:%s(): Error in thr_continue(): %s\n",
		__FILE__, __LINE__, __func__, strerror(error));
	abort();
    }
#endif
#ifdef CW_MTHREADS
    kern_return_t mach_error;

    mach_error = thread_resume(a_thd->mthread);
    if (mach_error != KERN_SUCCESS)
    {
	fprintf(stderr, "%s:%u:%s(): Error in thread_resume(): %d\n",
		__FILE__, __LINE__, __func__, mach_error);
	abort();
    }
#endif
    a_thd->suspended = FALSE;
    mtx_unlock(&a_thd->mtx);
}

#ifdef CW_THD_GENERIC_SR
static void
thd_p_suspend_handle(int a_signal)
{
    sigset_t set;

    /* Block all signals except CW_THD_SIGRESUME while suspended. */
    sigfillset(&set);
    sigdelset(&set, CW_THD_SIGRESUME);
    /* Tell suspender we're suspended. */
    sem_post(&cw_g_thd_sem);

    /* Suspend until CW_THD_SIGRESUME is delivered and handled. */
    sigsuspend(&set);
}

static void
thd_p_resume_handle(int a_signal)
{
    /* Do nothing.  This function only exists to allow the suspended thread to
     * return from the sigsuspend() call in thd_p_suspend_handle(). */
}
#endif