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
typedef struct cw_mem_item_s cw_mem_item_t;

struct cw_mem_item_s
{
    void *addr;
    size_t size;
    char *filename;
    cw_uint32_t line_num;
    cw_chi_t chi; /* For internal dch linkage. */
    ql_elm(cw_mem_item_t) link; /* For iteration. */
};
#endif

/* Globals. */
cw_mema_t *cw_g_mema = NULL;

/* File-global variables. */
static cw_mema_t s_mema;

#ifdef CW_DBG
static cw_bool_t s_mem_initialized = FALSE;
#endif

#ifdef CW_THREADS
static cw_mtx_t s_mem_lock;
#endif

#ifdef CW_MEM_ERROR
cw_mema_t s_mem_mema;

/* Slots in base hash table. */
#define CW_MEM_BASE_TABLE 1024
/* Maximum fullness of base table. */
#define CW_MEM_BASE_GROW 256
/* Proportional minimal fullness. */
#define CW_MEM_BASE_SHRINK 32
static cw_dch_t *s_mem_addr_hash;
static ql_head(cw_mem_item_t) s_mem_addr_list;
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

/* The following private functions are used for the internal dch that tracks
 * memory allocations. */
#ifdef CW_MEM_ERROR
static void *
mem_p_malloc_e(void *a_arg, size_t a_size, const char *a_filename,
	       cw_uint32_t a_line_num)
{
    return cw_malloc(a_size);
}

static void *
mem_p_calloc_e(void *a_arg, size_t a_number, size_t a_size, const
	       char *a_filename, cw_uint32_t a_line_num)
{
    return cw_calloc(a_number, a_size);
}

static void *
mem_p_realloc_e(void *a_arg, void *a_ptr, size_t a_size,
		size_t a_old_size, const char *a_filename,
		cw_uint32_t a_line_num)
{
    return cw_realloc(a_ptr, a_size);
}

static void
mem_p_free_e(void *a_arg, void *a_ptr, size_t a_size,
	     const char *a_filename, cw_uint32_t a_line_num)
{
    cw_free(a_ptr);
}
#endif

void
mem_l_init(void)
{
    cw_assert(s_mem_initialized == FALSE);

    cw_g_mema = mema_new(&s_mema,
			 (cw_opaque_alloc_t *) mem_malloc_e,
			 (cw_opaque_calloc_t *) mem_calloc_e,
			 (cw_opaque_realloc_t *) mem_realloc_e,
			 (cw_opaque_dealloc_t *) mem_free_e,
			 NULL);

#ifdef CW_MEM_ERROR
    mema_new(&s_mem_mema,
	     (cw_opaque_alloc_t *) mem_p_malloc_e,
	     (cw_opaque_calloc_t *) mem_p_calloc_e,
	     (cw_opaque_realloc_t *) mem_p_realloc_e,
	     (cw_opaque_dealloc_t *) mem_p_free_e,
	     NULL);
    s_mem_addr_hash = dch_new(NULL, &s_mem_mema, CW_MEM_BASE_TABLE,
			      CW_MEM_BASE_GROW, CW_MEM_BASE_SHRINK,
			      ch_direct_hash, ch_direct_key_comp);
    ql_new(&s_mem_addr_list);
#endif
#ifdef CW_THREADS
    mtx_new(&s_mem_lock);
#endif

#ifdef CW_DBG
    s_mem_initialized = TRUE;
#endif
}

void
mem_l_shutdown(void)
{
    cw_assert(s_mem_initialized);

#ifdef CW_DBG
    s_mem_initialized = FALSE;
#endif

    mema_delete(cw_g_mema);
    cw_g_mema = NULL;

#ifdef CW_MEM_ERROR
    {
	cw_mem_item_t *allocation;

	if (dch_count(s_mem_addr_hash) > 0)
	{
	    fprintf(stderr, "%s(): %u unfreed allocation%s\n",
		    __func__, dch_count(s_mem_addr_hash),
		    dch_count(s_mem_addr_hash) != 1 ? "s" : "");
	}

	for (allocation = ql_first(&s_mem_addr_list);
	     allocation != NULL;
	     allocation = ql_first(&s_mem_addr_list))
	{
	    dch_chi_remove(s_mem_addr_hash, &allocation->chi);
	    ql_remove(&s_mem_addr_list, allocation, link);
	    fprintf(stderr,
		    "%s(): %p, size %zu never freed (allocated at %s:%u)\n",
		    __func__, allocation->addr,
		    allocation->size, allocation->filename,
		    allocation->line_num);
	    cw_free(allocation->filename);
	    cw_free(allocation);
	}

	dch_delete(s_mem_addr_hash);
	mema_delete(&s_mem_mema);
#ifdef CW_THREADS
	mtx_delete(&s_mem_lock);
#endif
    }
#endif
}

void *
mem_malloc_e(void *a_arg, size_t a_size, const char *a_filename,
	     cw_uint32_t a_line_num)
{
    void *retval;
#ifdef CW_MEM_ERROR
    cw_mem_item_t *old_allocation;
#endif

    cw_assert(s_mem_initialized);
    cw_assert(a_size > 0);

#ifdef CW_MEM_ERROR
#ifdef CW_THREADS
    mtx_lock(&s_mem_lock);
#endif
#endif

    retval = cw_malloc(a_size);
    if (retval == NULL)
    {
#ifdef CW_MEM_ERROR
	fprintf(stderr, "%s(): %p <-- malloc(%zu) at %s:%u\n",
		__func__, retval, a_size, a_filename, a_line_num);
#endif
	xep_throw(CW_ONYXX_OOM);
    }

#ifdef CW_MEM_ERROR
    if (a_filename == NULL)
    {
	a_filename = "<?>";
    }

    if (dch_search(s_mem_addr_hash, retval, (void **) &old_allocation)
	== FALSE)
    {
	fprintf(stderr, "%s(): %p multiply-allocated "
		"(was at %s:%u, size %zu; now at %s:%u, size %zu)\n",
		__func__, retval,
		old_allocation->filename, old_allocation->line_num,
		old_allocation->size, a_filename, a_line_num,
		a_size);
    }
    else
    {
	cw_mem_item_t *allocation;

	allocation = (cw_mem_item_t *) cw_malloc(sizeof(cw_mem_item_t));
	memset(retval, 0xa5, a_size);

	allocation->addr = retval;
	allocation->size = a_size;
	allocation->filename = cw_malloc(strlen(a_filename) + 1);
	memcpy(allocation->filename, a_filename,
	       strlen(a_filename) + 1);
	allocation->line_num = a_line_num;
	ql_elm_new(allocation, link);

#ifdef CW_MEM_VERBOSE
	fprintf(stderr, "%s(): %p <-- malloc(%zu) at %s:%u\n",
		__func__, retval, a_size, a_filename,
		a_line_num);
#endif
	dch_insert(s_mem_addr_hash, retval, allocation, &allocation->chi);
	ql_tail_insert(&s_mem_addr_list, allocation, link);
    }
#ifdef CW_THREADS
    mtx_unlock(&s_mem_lock);
#endif
#endif

    return retval;
}

void *
mem_calloc_e(void *a_arg, size_t a_number, size_t a_size,
	     const char *a_filename, cw_uint32_t a_line_num)
{
    void *retval;
#ifdef CW_MEM_ERROR
    cw_mem_item_t *old_allocation;
#endif

    cw_assert(s_mem_initialized);
    cw_assert(a_size * a_number > 0);

#ifdef CW_MEM_ERROR
#ifdef CW_THREADS
    mtx_lock(&s_mem_lock);
#endif
#endif

    retval = cw_calloc(a_number, a_size);
    if (retval == NULL)
    {
#ifdef CW_MEM_ERROR
	fprintf(stderr, "%s(): %p <-- calloc(%zu, %zu) at %s:%u\n",
		__func__, retval, a_number, a_size,
		a_filename, a_line_num);
#endif
	xep_throw(CW_ONYXX_OOM);
    }

#ifdef CW_MEM_ERROR
    if (a_filename == NULL)
    {
	a_filename = "<?>";
    }

    if (dch_search(s_mem_addr_hash, retval, (void **) &old_allocation)
	== FALSE)
    {
	fprintf(stderr, "%s(): %p multiply-allocated "
		"(was at %s:%u, size %zu; now at %s:%u, size %zu\n",
		__func__, retval,
		old_allocation->filename, old_allocation->line_num,
		old_allocation->size, a_filename, a_line_num,
		a_size);
    }
    else
    {
	cw_mem_item_t *allocation;

	allocation = (cw_mem_item_t *) cw_malloc(sizeof(cw_mem_item_t));
	/* Leave the memory alone, since calloc() is supposed to return
	 * zeroed memory. */

	allocation->addr = retval;
	allocation->size = a_number * a_size;
	allocation->filename = cw_malloc(strlen(a_filename) + 1);
	memcpy(allocation->filename, a_filename, strlen(a_filename) + 1);
	allocation->line_num = a_line_num;
	ql_elm_new(allocation, link);

#ifdef CW_MEM_VERBOSE
	fprintf(stderr, "%s(): %p <-- calloc(%zu, %zu) at %s:%u\n",
		__func__, retval, a_number, a_size,
		a_filename, a_line_num);
#endif
	dch_insert(s_mem_addr_hash, retval, allocation,
		   &allocation->chi);
	ql_tail_insert(&s_mem_addr_list, allocation, link);
    }
#ifdef CW_THREADS
    mtx_unlock(&s_mem_lock);
#endif
#endif

    return retval;
}

void *
mem_realloc_e(void *a_arg, void *a_ptr, size_t a_size, size_t a_old_size,
	      const char *a_filename, cw_uint32_t a_line_num)
{
    void *retval;
#ifdef CW_MEM_ERROR
    cw_mem_item_t *allocation;
#endif

    cw_assert(s_mem_initialized);
    cw_check_ptr(a_ptr);
    cw_assert(a_size > 0);

#ifdef CW_MEM_ERROR
#ifdef CW_THREADS
    mtx_lock(&s_mem_lock);
#endif
#endif

    retval = cw_realloc(a_ptr, a_size);
    if (retval == NULL)
    {
#ifdef CW_MEM_ERROR
	fprintf(stderr, "%s(): %p <-- realloc(%p, %zu) at %s:%u\n",
		__func__, retval, a_ptr, a_size,
		a_filename, a_line_num);
#endif
	xep_throw(CW_ONYXX_OOM);
    }

#ifdef CW_MEM_ERROR
    if (a_filename == NULL)
    {
	a_filename = "<?>";
    }

    if (dch_remove(s_mem_addr_hash, a_ptr, NULL, (void **) &allocation,
		   NULL))
    {
	fprintf(stderr, "%s(): %p not allocated\n", __func__, a_ptr);
    }
    else
    {
	char *old_filename;
	size_t old_size;
	cw_uint32_t old_line_num;

	ql_remove(&s_mem_addr_list, allocation, link);

	old_filename = allocation->filename;
	old_size = allocation->size;
	old_line_num = allocation->line_num;
	allocation->filename = cw_malloc(strlen(a_filename) + 1);
	memcpy(allocation->filename, a_filename, strlen(a_filename) + 1);
	allocation->addr = retval;
	allocation->size = a_size;
	allocation->line_num = a_line_num;

	dch_insert(s_mem_addr_hash, retval, allocation, &allocation->chi);
	ql_tail_insert(&s_mem_addr_list, allocation, link);
	if (a_size > old_size)
	{
	    memset(((cw_uint8_t *) retval) + old_size, 0xa5,
		   a_size - old_size);
	}
	if (a_old_size != 0 && a_old_size != old_size)
	{
	    fprintf(stderr, "%s(): Wrong size %zu for %p "
		    "at %s:%u (size %zu, allocated at %s:%u)\n",
		    __func__, a_old_size, a_ptr,
		    a_filename, a_line_num, old_size,
		    old_filename, old_line_num);
	}
#ifdef CW_MEM_VERBOSE
	fprintf(stderr, "%s(): %p <-- realloc(%p, %zu) at "
		"%s:%u (was size %zu, allocated at %s:%u)\n",
		__func__, retval, a_ptr, a_size,
		a_filename, a_line_num, old_size, old_filename,
		old_line_num);
#endif
	cw_free(old_filename);
    }
#ifdef CW_THREADS
    mtx_unlock(&s_mem_lock);
#endif
#endif

    return retval;
}

void
mem_free_e(void *a_arg, void *a_ptr, size_t a_size, const char *a_filename,
	   cw_uint32_t a_line_num)
{
#ifdef CW_MEM_ERROR
    cw_mem_item_t *allocation;
#endif

    cw_assert(s_mem_initialized);

#ifdef CW_MEM_ERROR
    if (a_filename == NULL)
    {
	a_filename = "<?>";
    }

#ifdef CW_THREADS
    mtx_lock(&s_mem_lock);
#endif

    if (dch_remove(s_mem_addr_hash, a_ptr, NULL, (void **) &allocation,
		   NULL))
    {
	fprintf(stderr, "%s(): %p not allocated, attempted "
		"to free at %s:%u\n", __func__, a_ptr,
		a_filename, a_line_num);
    }
    else
    {
	ql_remove(&s_mem_addr_list, allocation, link);

	if (a_size != 0 && a_size != allocation->size)
	{
	    fprintf(stderr, "%s(): Wrong size %zu for %p "
		    "at %s:%u (size %zu, allocated at %s:%u)\n",
		    __func__, a_size, a_ptr,
		    a_filename, a_line_num, allocation->size,
		    allocation->filename, allocation->line_num);
	}
#ifdef CW_MEM_VERBOSE
	fprintf(stderr, "%s(): free(%p) at %s:%u "
		"(size %zu, allocated at %s:%u)\n", __func__,
		a_ptr, a_filename, a_line_num,
		allocation->size, allocation->filename,
		allocation->line_num);
#endif
	memset(a_ptr, 0x5a, allocation->size);
	cw_free(allocation->filename);
	cw_free(allocation);
    }
#endif

    cw_free(a_ptr);

#ifdef CW_MEM_ERROR
#ifdef CW_THREADS
    mtx_unlock(&s_mem_lock);
#endif
#endif
}
