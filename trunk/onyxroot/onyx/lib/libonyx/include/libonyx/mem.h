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

#ifdef CW_DBG
/* Track allocations and report leaks. */
#define CW_MEM_ERROR
#ifdef CW_MEM_ERROR
/* Report all allocations. */
/* #define CW_MEM_VERBOSE */
#endif
#endif

struct cw_mema_s
{
    cw_bool_t is_malloced;

    cw_opaque_alloc_t *alloc;
    cw_opaque_calloc_t *calloc;
    cw_opaque_realloc_t *realloc;
    cw_opaque_dealloc_t *dealloc;

    void *arg;
};

/* Global variables. */
extern cw_mema_t *cw_g_mema;

/* mema. */
cw_mema_t *
mema_new(cw_mema_t *a_mema, cw_opaque_alloc_t *a_alloc,
	 cw_opaque_calloc_t *a_calloc, cw_opaque_realloc_t *a_realloc,
	 cw_opaque_dealloc_t *a_dealloc, void *a_arg);

void
mema_delete(cw_mema_t *a_mema);

#ifndef CW_USE_INLINES
cw_opaque_alloc_t *
mema_alloc_get(cw_mema_t *a_mema);

cw_opaque_calloc_t *
mema_calloc_get(cw_mema_t *a_mema);

cw_opaque_realloc_t *
mema_realloc_get(cw_mema_t *a_mema);

cw_opaque_dealloc_t *
mema_dealloc_get(cw_mema_t *a_mema);

void *
mema_arg_get(cw_mema_t *a_mema);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_MEM_C_))
CW_INLINE cw_opaque_alloc_t *
mema_alloc_get(cw_mema_t *a_mema)
{
    return a_mema->alloc;
}

CW_INLINE cw_opaque_calloc_t *
mema_calloc_get(cw_mema_t *a_mema)
{
    return a_mema->calloc;
}

CW_INLINE cw_opaque_realloc_t *
mema_realloc_get(cw_mema_t *a_mema)
{
    return a_mema->realloc;
}

CW_INLINE cw_opaque_dealloc_t *
mema_dealloc_get(cw_mema_t *a_mema)
{
    return a_mema->dealloc;
}

CW_INLINE void *
mema_arg_get(cw_mema_t *a_mema)
{
    return a_mema->arg;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_MEM_C_)) */

/* mem. */
void *
mem_malloc_e(void *a_arg, size_t a_size, const char *a_filename,
	     cw_uint32_t a_line_num);

void *
mem_calloc_e(void *a_arg, size_t a_number, size_t a_size, const
	     char *a_filename, cw_uint32_t a_line_num);

void *
mem_realloc_e(void *a_arg, void *a_ptr, size_t a_size,
	      size_t a_old_size, const char *a_filename,
	      cw_uint32_t a_line_num);

void
mem_free_e(void *a_arg, void *a_ptr, size_t a_size,
	   const char *a_filename, cw_uint32_t a_line_num);

/* These macros are declared differently, depending on whether this is a debug
 * library, because consistently using arguments of NULL and 0 reduces the size
 * of the generated binary.  Since these arguments aren't used in the optimized
 * library anyway, this is a free (though perhaps small) memory savings. */
#ifdef CW_MEM_ERROR
#define mem_malloc(a_mem, a_size)					\
    mem_malloc_e((a_mem), (a_size), __FILE__, __LINE__)
#define mem_calloc(a_mem, a_number, a_size)				\
    mem_calloc_e((a_mem), (a_number), (a_size), __FILE__, __LINE__)
#define mem_realloc(a_mem, a_ptr, a_size)				\
    mem_realloc_e((a_mem), (a_ptr), (a_size), 0, __FILE__, __LINE__)
#define mem_free(a_mem, a_ptr)						\
    mem_free_e((a_mem), (a_ptr), 0, __FILE__, __LINE__)

#define cw_malloc(a_size)						\
    mem_malloc_e(NULL, (a_size), __FILE__, __LINE__)
#define cw_calloc(a_number, a_size)					\
    mem_calloc_e(NULL, (a_number), (a_size), __FILE__, __LINE__)
#define cw_realloc(a_ptr, a_size)					\
    mem_realloc_e(NULL, (a_ptr), (a_size), 0, __FILE__, __LINE__)
#define cw_free(a_ptr)							\
    mem_free_e(NULL, (a_ptr), 0, __FILE__, __LINE__)

#else
#define mem_malloc(a_mem, a_size)					\
    mem_malloc_e((a_mem), (a_size), NULL, 0)
#define mem_calloc(a_mem, a_number, a_size)				\
    mem_calloc_e((a_mem), (a_number), (a_size), NULL, 0)
#define mem_realloc(a_mem, a_ptr, a_size)				\
    mem_realloc_e((a_mem), (a_ptr), (a_size), 0, NULL, 0)
#define mem_free(a_mem, a_ptr)						\
    mem_free_e((a_mem), (a_ptr), 0, NULL, 0)

#define cw_malloc(a_size)						\
    mem_malloc_e(NULL, (a_size), NULL, 0)
#define cw_calloc(a_number, a_size)					\
    mem_calloc_e(NULL, (a_number), (a_size), NULL, 0)
#define cw_realloc(a_ptr, a_size)					\
    mem_realloc_e(NULL, (a_ptr), (a_size), 0, NULL, 0)
#define cw_free(a_ptr)							\
    mem_free_e(NULL, (a_ptr), 0, NULL, 0)

#endif
