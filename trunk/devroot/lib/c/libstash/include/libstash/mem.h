/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

typedef struct cw_mem_s cw_mem_t;

cw_mem_t *
mem_new();

void
mem_delete(cw_mem_t * a_mem);

#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void *
mem_malloc(cw_mem_t * a_mem, size_t a_size, const char * a_filename,
	   cw_uint32_t a_line_num);
#else
void *
mem_malloc(cw_mem_t * a_mem, size_t a_size);
#endif

#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void *
mem_calloc(cw_mem_t * a_mem, size_t a_number, size_t a_size,
	   const char * a_filename, cw_uint32_t a_line_num);
#else
void *
mem_calloc(cw_mem_t * a_mem, size_t a_number, size_t a_size);
#endif

#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void *
mem_realloc(cw_mem_t * a_mem, void * a_ptr, size_t a_size,
	    const char * a_filename, cw_uint32_t a_line_num);
#else
void *
mem_realloc(cw_mem_t * a_mem, void * a_ptr, size_t a_size);
#endif

#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void
mem_free(cw_mem_t * a_mem, void * a_ptr, const char * a_filename,
	 cw_uint32_t a_line_num);
#else
void
mem_free(cw_mem_t * a_mem, void * a_ptr);
#endif

void
mem_dealloc(void * a_mem, void * a_ptr);
