/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Public interface for a memory allocation (malloc) wrapper class.  For the
 * debug versions of libstash, extra information is hashed for each memory
 * allocation that allows tracking of the following:
 *
 * - File/line number of allocation.
 * - Double allocation/deallocation of the same address.
 * - Memory leaks (memory left allocated at mem destruction time.
 *
 * Also, the debug versions of the library set all newly allocated bytes to
 * 0xa5, and all deallocated bytes to 0x5a.  This tends to cause things to break
 * sooner when uninitialized or deallocated memory is referenced.
 *
 * In general, this class doesn't need to be used directly.  Instead, there are
 * several preprocessor macros that can be used: _cw_malloc(), _cw_calloc(),
 * _cw_realloc(), _cw_free(), and _cw_dealloc().
 *
 ****************************************************************************/

typedef struct cw_mem_s cw_mem_t;

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a mem.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_mem_t *
mem_new(void);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mem : Pointer to a mem.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
mem_delete(cw_mem_t * a_mem);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mem : Pointer to a mem.
 *
 * a_size : Size of memory range desired.
 *
 * a_filename : Should be __FILE__.
 *
 * a_line_num : Should be __LINE__.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a memory range, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * malloc() wrapper.
 *
 ****************************************************************************/
#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void *
mem_malloc(cw_mem_t * a_mem, size_t a_size, const char * a_filename,
	   cw_uint32_t a_line_num);
#else
void *
mem_malloc(cw_mem_t * a_mem, size_t a_size);
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mem : Pointer to a mem.
 *
 * a_number : Number of elements.
 *
 * a_size : Size of each element desired.
 *
 * a_filename : Should be __FILE__.
 *
 * a_line_num : Should be __LINE__.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a memory range, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * calloc() wrapper.
 *
 ****************************************************************************/
#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void *
mem_calloc(cw_mem_t * a_mem, size_t a_number, size_t a_size,
	   const char * a_filename, cw_uint32_t a_line_num);
#else
void *
mem_calloc(cw_mem_t * a_mem, size_t a_number, size_t a_size);
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mem : Pointer to a mem.
 *
 * a_ptr : Pointer to memory range to be reallocated.
 *
 * a_size : Size of memory range desired.
 *
 * a_filename : Should be __FILE__.
 *
 * a_line_num : Should be __LINE__.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a memory range, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * realloc() wrapper.
 *
 ****************************************************************************/
#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void *
mem_realloc(cw_mem_t * a_mem, void * a_ptr, size_t a_size,
	    const char * a_filename, cw_uint32_t a_line_num);
#else
void *
mem_realloc(cw_mem_t * a_mem, void * a_ptr, size_t a_size);
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mem : Pointer to a mem.
 *
 * a_ptr : Pointer to memory range to be freed.
 *
 * a_size : Size of memory range desired.
 *
 * a_filename : Should be __FILE__.
 *
 * a_line_num : Should be __LINE__.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * free() wrapper.
 *
 ****************************************************************************/
#if (defined(_LIBSTASH_DEBUG) || defined(_LIBSTASH_DBG))
void
mem_free(cw_mem_t * a_mem, void * a_ptr, const char * a_filename,
	 cw_uint32_t a_line_num);
#else
void
mem_free(cw_mem_t * a_mem, void * a_ptr);
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mem : Pointer to a mem.
 *
 * a_ptr : Pointer to be freed.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Basically the same as mem_free(), but with a different function prototype.
 * This function can be used for passing a function pointer to do automatic
 * deallocation, as in the buf and ring classes.
 *
 ****************************************************************************/
void
mem_dealloc(void * a_mem, void * a_ptr);
