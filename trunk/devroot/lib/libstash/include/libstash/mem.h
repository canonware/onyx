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

typedef struct cw_mem_s cw_mem_t;

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_data : Data pointer, as specified by the a_data argument of
 *          mem_set_oom_handler().
 *
 * a_size : Size of failed memory allocation.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE : Don't retry allocation.
 *          TRUE : Retry allocation.
 *
 * <<< Description >>>
 *
 * Prototype for allocation error handler.
 *
 ****************************************************************************/
typedef cw_bool_t cw_mem_oom_handler_t(const void *a_data, cw_uint32_t a_size);

cw_mem_t *mem_new(void);

void    mem_delete(cw_mem_t *a_mem);

void    mem_set_oom_handler(cw_mem_t *a_mem, cw_mem_oom_handler_t
    *a_oom_handler, const void *a_data);

#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void   *mem_malloc(cw_mem_t *a_mem, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num);
#else
void   *mem_malloc(cw_mem_t *a_mem, size_t a_size);
#endif

#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void   *mem_calloc(cw_mem_t *a_mem, size_t a_number, size_t a_size, const char
    *a_filename, cw_uint32_t a_line_num);
#else
void   *mem_calloc(cw_mem_t *a_mem, size_t a_number, size_t a_size);
#endif

#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void   *mem_realloc(cw_mem_t *a_mem, void *a_ptr, size_t a_size, const char
    *a_filename, cw_uint32_t a_line_num);
#else
void   *mem_realloc(cw_mem_t *a_mem, void *a_ptr, size_t a_size);
#endif

#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void    mem_free(cw_mem_t *a_mem, void *a_ptr, const char *a_filename,
    cw_uint32_t a_line_num);
#else
void    mem_free(cw_mem_t *a_mem, void *a_ptr);
#endif

void    mem_dealloc(void *a_mem, void *a_ptr);
