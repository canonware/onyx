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

#define CW_MEM_C_
#include "../include/libonyx/libonyx.h"

#ifdef cw_malloc
#undef cw_malloc
#endif
#define cw_malloc(a) malloc(a)

#ifdef cw_calloc
#undef cw_calloc
#endif
#define cw_calloc(a, b) calloc(a, b)

#ifdef cw_realloc
#undef cw_realloc
#endif
#define cw_realloc(a, b) realloc(a, b)

#ifdef cw_free
#undef cw_free
#endif
#define cw_free(a) free(a)

#ifdef CW_MEM_ERROR
struct cw_mem_item_s
{
    size_t size;
    char *filename;
    cw_uint32_t line_num;
    cw_chi_t chi; /* For internal ch linkage. */
};
#endif

/* mema. */
cw_mema_t *
mema_new(cw_mema_t *a_mema, cw_opaque_alloc_t *a_alloc,
	 cw_opaque_calloc_t *a_calloc, cw_opaque_realloc_t *a_realloc,
	 cw_opaque_dealloc_t *a_dealloc, void *a_arg)
{
    cw_mema_t *retval;

    if (a_mema != NULL)
    {
	retval = a_mema;
	retval->is_malloced = FALSE;
    }
    else
    {
	cw_check_ptr(a_alloc);
	cw_check_ptr(a_dealloc);
    	retval = (cw_mema_t *) cw_opaque_alloc(a_alloc, a_arg,
					       sizeof(cw_mema_t));
	retval->is_malloced = TRUE;
    }

    retval->alloc = a_alloc;
    retval->calloc = a_calloc;
    retval->realloc = a_realloc;
    retval->dealloc = a_dealloc;
    retval->arg = a_arg;

    return retval;
}

void
mema_delete(cw_mema_t *a_mema)
{
    cw_check_ptr(a_mema);

    if (a_mema->is_malloced)
    {
	cw_opaque_dealloc(a_mema->dealloc, a_mema->arg, a_mema,
			  sizeof(cw_mema_t));
    }
}

/* mem. */
cw_mem_t *
mem_new(cw_mem_t *a_mem, cw_mem_t *a_internal)
{
    cw_mem_t *retval;
    volatile cw_uint32_t try_stage = 0;

    xep_begin();
    volatile cw_mem_t *v_retval;
    xep_try
    {
	if (a_mem != NULL)
	{
	    v_retval = retval = a_mem;
	    retval->is_malloced = FALSE;
	}
	else
	{
	    v_retval = retval = (cw_mem_t *) mem_malloc(a_internal,
							sizeof(cw_mem_t));
	    retval->is_malloced = TRUE;
	}
	retval->mem = a_internal;
#ifdef CW_MEM_ERROR
	mema_new(&retval->mema, (cw_opaque_alloc_t *) mem_malloc_e,
		 (cw_opaque_calloc_t *) mem_calloc_e,
		 (cw_opaque_realloc_t *) mem_realloc_e,
		 (cw_opaque_dealloc_t *) mem_free_e, a_internal);
#endif
#ifdef CW_THREADS
	mtx_new(&retval->lock);
#endif
	try_stage = 1;

#ifdef CW_MEM_ERROR
	retval->addr_hash = dch_new(NULL, &retval->mema, CW_MEM_BASE_TABLE,
				    CW_MEM_BASE_GROW, CW_MEM_BASE_SHRINK,
				    ch_direct_hash, ch_direct_key_comp);
	try_stage = 2;
#endif
	retval->handler_data = NULL;
    }
    xep_catch(CW_ONYXX_OOM)
    {
	retval = (cw_mem_t *)v_retval;
	switch (try_stage)
	{
	    case 1:
	    {
#ifdef CW_THREADS
		mtx_delete(&retval->lock);
#endif
#ifdef CW_MEM_ERROR
		mema_delete(&retval->mema);
#endif
		if (retval->is_malloced)
		{
		    mem_free(a_internal, retval);
		}
	    }
	    case 0:
	    {
		break;
	    }
	}
    }
    xep_end();
    return retval;
}

void
mem_delete(cw_mem_t *a_mem)
{
    cw_check_ptr(a_mem);

#ifdef CW_MEM_ERROR
    {
	cw_uint32_t i, num_addrs;
	void *addr;
	struct cw_mem_item_s *allocation;

	num_addrs = dch_count(a_mem->addr_hash);

	if (num_addrs > 0)
	{
	    fprintf(stderr, "%s(%p): %u unfreed allocation%s\n",
		    __func__, a_mem, num_addrs, num_addrs != 1 ? "s" : "");
	}
	for (i = 0; i < num_addrs; i++)
	{
	    dch_remove_iterate(a_mem->addr_hash, &addr, (void **) &allocation,
			       NULL);
	    fprintf(stderr,
		    "%s(%p): %p, size %zu never freed (allocated at %s:%u)\n",
		    __func__, a_mem, addr,
		    allocation->size, allocation->filename,
		    allocation->line_num);
	    mem_free(a_mem->mem, allocation->filename);
	    mem_free(a_mem->mem, allocation);
	}
	dch_delete(a_mem->addr_hash);
#ifdef CW_THREADS
	mtx_delete(&a_mem->lock);
#endif
	mema_delete(&a_mem->mema);
    }
#endif

    if (a_mem->is_malloced)
    {
	mem_free(a_mem->mem, a_mem);
    }
}

void *
mem_malloc_e(cw_mem_t *a_mem, size_t a_size, const char *a_filename,
	     cw_uint32_t a_line_num)
{
    void *retval;

    cw_assert(a_size > 0);

#ifdef CW_MEM_ERROR
#ifdef CW_THREADS
    if (a_mem != NULL)
    {
	mtx_lock(&a_mem->lock);
    }
#endif
#endif

    retval = cw_malloc(a_size);
    if (retval == NULL)
    {
#ifdef CW_MEM_ERROR
	fprintf(stderr, "%s(%p): %p <-- malloc(%zu) at %s:%u\n",
		__func__, a_mem, retval, a_size, a_filename, a_line_num);
#endif
	xep_throw(CW_ONYXX_OOM);
    }

#ifdef CW_MEM_ERROR
    if (a_mem != NULL)
    {
	struct cw_mem_item_s *old_allocation;

	if (a_filename == NULL)
	{
	    a_filename = "<?>";
	}

	if (dch_search(a_mem->addr_hash, retval, (void **) &old_allocation)
	    == FALSE)
	{
	    fprintf(stderr, "%s(%p): %p multiply-allocated "
		    "(was at %s:%u, size %zu; now at %s:%u, size %zu)\n",
		    __func__, a_mem, retval,
		    old_allocation->filename, old_allocation->line_num,
		    old_allocation->size, a_filename, a_line_num,
		    a_size);
	}
	else
	{
	    struct cw_mem_item_s *allocation;

	    allocation = mem_malloc(a_mem->mem, sizeof(struct cw_mem_item_s));
	    memset(retval, 0xa5, a_size);

	    allocation->size = a_size;
	    allocation->filename = mem_malloc(a_mem->mem,
					      strlen(a_filename) + 1);
	    memcpy(allocation->filename, a_filename,
		   strlen(a_filename) + 1);
	    allocation->line_num = a_line_num;

#ifdef CW_MEM_VERBOSE
	    fprintf(stderr, "%s(%p): %p <-- malloc(%zu) at %s:%u\n",
		    __func__, a_mem, retval, a_size, a_filename,
		    a_line_num);
#endif
	    dch_insert(a_mem->addr_hash, retval, allocation, &allocation->chi);
	}
#ifdef CW_THREADS
	mtx_unlock(&a_mem->lock);
#endif
    }
#endif

    return retval;
}

void *
mem_calloc_e(cw_mem_t *a_mem, size_t a_number, size_t a_size,
	     const char *a_filename, cw_uint32_t a_line_num)
{
    void *retval;

    cw_assert(a_size * a_number > 0);

#ifdef CW_MEM_ERROR
#ifdef CW_THREADS
    if (a_mem != NULL)
    {
	mtx_lock(&a_mem->lock);
    }
#endif
#endif

    retval = cw_calloc(a_number, a_size);
    if (retval == NULL)
    {
#ifdef CW_MEM_ERROR
	fprintf(stderr, "%s(%p): %p <-- calloc(%zu, %zu) at %s:%u\n",
		__func__, a_mem, retval, a_number, a_size,
		a_filename, a_line_num);
#endif
	xep_throw(CW_ONYXX_OOM);
    }

#ifdef CW_MEM_ERROR
    if (a_mem != NULL)
    {
	struct cw_mem_item_s *old_allocation;

	if (a_filename == NULL)
	{
	    a_filename = "<?>";
	}

	if (dch_search(a_mem->addr_hash, retval, (void **) &old_allocation)
	    == FALSE)
	{
	    fprintf(stderr, "%s(%p): %p multiply-allocated "
		    "(was at %s:%u, size %zu; now at %s:%u, size %zu\n",
		    __func__, a_mem, retval,
		    old_allocation->filename, old_allocation->line_num,
		    old_allocation->size, a_filename, a_line_num,
		    a_size);
	}
	else
	{
	    struct cw_mem_item_s *allocation;

	    allocation = mem_malloc(a_mem->mem, sizeof(struct cw_mem_item_s));
	    /* Leave the memory alone, since calloc() is supposed to return
	     * zeroed memory. */

	    allocation->size = a_number * a_size;
	    allocation->filename = mem_malloc(a_mem->mem,
					      strlen(a_filename) + 1);
	    memcpy(allocation->filename, a_filename, strlen(a_filename) + 1);
	    allocation->line_num = a_line_num;

#ifdef CW_MEM_VERBOSE
	    fprintf(stderr, "%s(%p): %p <-- calloc(%zu, %zu) at %s:%u\n",
		    __func__, a_mem, retval, a_number, a_size,
		    a_filename, a_line_num);
#endif
	    dch_insert(a_mem->addr_hash, retval, allocation,
		       &allocation->chi);
	}
    }
#ifdef CW_THREADS
    if (a_mem != NULL)
    {
	mtx_unlock(&a_mem->lock);
    }
#endif
#endif

    return retval;
}

void *
mem_realloc_e(cw_mem_t *a_mem, void *a_ptr, size_t a_size, size_t a_old_size,
	      const char *a_filename, cw_uint32_t a_line_num)
{
    void *retval;

    cw_check_ptr(a_ptr);
    cw_assert(a_size > 0);

#ifdef CW_MEM_ERROR
#ifdef CW_THREADS
    if (a_mem != NULL)
    {
	mtx_lock(&a_mem->lock);
    }
#endif
#endif

    retval = cw_realloc(a_ptr, a_size);
    if (retval == NULL)
    {
#ifdef CW_MEM_ERROR
	fprintf(stderr, "%s(%p): %p <-- realloc(%p, %zu) at %s:%u\n",
		__func__, a_mem, retval, a_ptr, a_size,
		a_filename, a_line_num);
#endif
	xep_throw(CW_ONYXX_OOM);
    }

#ifdef CW_MEM_ERROR
    if (a_mem != NULL)
    {
	struct cw_mem_item_s *allocation;

	if (a_filename == NULL)
	{
	    a_filename = "<?>";
	}

	if (dch_remove(a_mem->addr_hash, a_ptr, NULL, (void **) &allocation,
		       NULL))
	{
	    fprintf(stderr, "%s(%p): %p not allocated\n", __func__, a_mem,
		    a_ptr);
	}
	else
	{
	    char *old_filename;
	    size_t old_size;
	    cw_uint32_t old_line_num;

	    old_filename = allocation->filename;
	    old_size = allocation->size;
	    old_line_num = allocation->line_num;
	    allocation->filename = mem_malloc(a_mem->mem,
					      strlen(a_filename) + 1);
	    memcpy(allocation->filename, a_filename, strlen(a_filename) + 1);
	    allocation->size = a_size;
	    allocation->line_num = a_line_num;

	    dch_insert(a_mem->addr_hash, retval, allocation, &allocation->chi);
	    if (a_size > old_size)
	    {
		memset(((cw_uint8_t *) retval) + old_size, 0xa5,
		       a_size - old_size);
	    }
	    if (a_old_size != 0 && a_old_size != old_size)
	    {
		fprintf(stderr, "%s(%p): Wrong size %zu for %p "
			"at %s:%u (size %zu, allocated at %s:%u)\n",
			__func__, a_mem, a_old_size, a_ptr,
			a_filename, a_line_num, old_size,
			old_filename, old_line_num);
	    }
#ifdef CW_MEM_VERBOSE
	    fprintf(stderr, "%s(%p): %p <-- realloc(%p, %zu) at "
		    "%s:%u (was size %zu, allocated at %s:%u)\n",
		    __func__, a_mem, retval, a_ptr, a_size,
		    a_filename, a_line_num, old_size, old_filename,
		    old_line_num);
#endif
	    mem_free(a_mem->mem, old_filename);
	}
    }
#ifdef CW_THREADS
    if (a_mem != NULL)
    {
	mtx_unlock(&a_mem->lock);
    }
#endif
#endif

    return retval;
}

void
mem_free_e(cw_mem_t *a_mem, void *a_ptr, size_t a_size, const char *a_filename,
	   cw_uint32_t a_line_num)
{
#ifdef CW_MEM_ERROR
    if (a_mem != NULL)
    {
	struct cw_mem_item_s *allocation;

	if (a_filename == NULL)
	{
	    a_filename = "<?>";
	}

#ifdef CW_THREADS
	mtx_lock(&a_mem->lock);
#endif

	if (dch_remove(a_mem->addr_hash, a_ptr, NULL, (void **) &allocation,
		       NULL))
	{
	    fprintf(stderr, "%s(%p): %p not allocated, attempted "
		    "to free at %s:%u\n", __func__, a_mem, a_ptr,
		    a_filename, a_line_num);
	}
	else
	{
	    if (a_size != 0 && a_size != allocation->size)
	    {
		fprintf(stderr, "%s(%p): Wrong size %zu for %p "
			"at %s:%u (size %zu, allocated at %s:%u)\n",
			__func__, a_mem, a_size, a_ptr,
			a_filename, a_line_num, allocation->size,
			allocation->filename, allocation->line_num);
	    }
#ifdef CW_MEM_VERBOSE
	    fprintf(stderr, "%s(%p): free(%p) at %s:%u "
		    "(size %zu, allocated at %s:%u)\n", __func__,
		    a_mem, a_ptr, a_filename, a_line_num,
		    allocation->size, allocation->filename,
		    allocation->line_num);
#endif
	    memset(a_ptr, 0x5a, allocation->size);
	    mem_free(a_mem->mem, allocation->filename);
	    mem_free(a_mem->mem, allocation);
	}
    }
#endif

    cw_free(a_ptr);

#ifdef CW_MEM_ERROR
#ifdef CW_THREADS
    if (a_mem != NULL)
    {
	mtx_unlock(&a_mem->lock);
    }
#endif
#endif
}