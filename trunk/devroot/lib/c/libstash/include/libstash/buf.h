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
 * The buf and bufc classes implement a buffer abstraction.  These classes are
 * designed specifically to handle streaming and transparent extensible
 * buffering of data for applications such as socket programs.  The main
 * features include:
 *
 * - Dynamically extensible and contractible buffering.
 * - Internal reference counting, which avoids copying between buf's, and allows
 *   for compact usage of memory buffers.
 * - 8, 32, and 64 bit read functions for arbitrary byte offsets (within the
 *   valid data range) within buf's.
 * - Easy ability to use with readv() and writev().
 *
 ****************************************************************************/

/* Pseudo-opaque typedefs. */
typedef struct cw_buf_s cw_buf_t;
typedef struct cw_bufc_s cw_bufc_t;

/* Opaque typedef. */
typedef struct cw_bufel_s cw_bufel_t;

/* The following data types should be considered opaque. */
struct cw_bufc_s
{
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
#ifdef _CW_REENTRANT
  cw_mtx_t lock;
#endif
  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;
  void (*buffer_dealloc_func)(void *, void *);
  void * buffer_dealloc_arg;
  cw_uint32_t ref_count;
  cw_bool_t is_writeable;
  cw_uint32_t buf_size;
  cw_uint8_t * buf;
};

struct cw_buf_s
{
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_bool_t is_threadsafe;
  cw_mtx_t lock;
#endif
  cw_uint32_t size;

  cw_uint32_t array_size;
  cw_uint32_t array_num_valid;
  cw_uint32_t array_start;
  cw_uint32_t array_end;
  cw_bool_t is_cumulative_valid;
  cw_bool_t is_cached_bufel_valid;
  cw_uint32_t cached_bufel;
  cw_bufel_t * bufel_array;
  cw_uint32_t * cumulative_index;
  struct iovec * iov;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to space for a buf, or NULL.
 *
 * a_is_thread_safe : FALSE == not thread safe, TRUE == thread safe.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a buf, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
#ifdef _CW_REENTRANT
cw_buf_t *
buf_new(cw_buf_t * a_buf, cw_bool_t a_is_threadsafe);
#else
cw_buf_t *
buf_new(cw_buf_t * a_buf);
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
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
buf_delete(cw_buf_t * a_buf);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 *
 *
 * <<< Output(s) >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
buf_dump(cw_buf_t * a_buf, const char * a_prefix);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of bytes of valid data.
 *
 * <<< Description >>>
 *
 * Return the amount of valid data in bytes.
 *
 ****************************************************************************/
cw_uint32_t
buf_get_size(cw_buf_t * a_buf);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of bufel's in a_buf (same as iovec count in buf_get_iovec()).
 *
 * <<< Description >>>
 *
 * Return the number of bufel's in a_buf.
 *
 ****************************************************************************/
cw_uint32_t
buf_get_num_bufels(cw_buf_t * a_buf);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_max_data : Maximum number of bytes of space to include in the iovec.
 *
 * a_is_sys_iovec : If TRUE, limit *r_iovec_count to the maximum iovec count
 *                  supported by this system for readv()/writev().
 *
 * a_iovec_count : Pointer to an int.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to an iovec array that represents the internal data buffers
 *          in a_buf.
 *
 * *r_iovec_count : Number of valid iovec structures in retval.
 *
 * <<< Description >>>
 *
 * Build an iovec array that represents the valid data in a_buf's internal
 * buffers (up to a_max_data bytes) and return a pointer to it.
 *
 ****************************************************************************/
const struct iovec *
buf_get_iovec(cw_buf_t * a_buf, cw_uint32_t a_max_data,
	      cw_bool_t a_is_sys_iovec, int * r_iovec_count);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to a buf.
 *
 * a_b : Pointer to a buf.
 *
 * a_preserve : If TRUE, preserve a_b (don't modify it).  If FALSE, release the
 *              data in a_b after catenating a_b to a_a.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Catenate two bufs.  a_b is left unmodified if a_preserve is TRUE.
 *
 ****************************************************************************/
cw_bool_t
buf_catenate_buf(cw_buf_t * a_a, cw_buf_t * a_b, cw_bool_t a_preserve);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to a buf.
 *
 * a_b : Pointer to a buf.
 *
 * a_offset : Offset at which to split a_b.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.  a_a and a_b are returned to their
 *                 original states.
 *
 * <<< Description >>>
 *
 * Split a_b at offset a_offset.  Append the data before a_offset to a_a, and
 * leave the remainder in a_b.
 *
 ****************************************************************************/
cw_bool_t
buf_split(cw_buf_t * a_a, cw_buf_t * a_b, cw_uint32_t a_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_bufel : Pointer to a bufel.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.  a_buf is still valid.
 *
 * <<< Description >>>
 *
 * Prepend the data from a_bufel to a_buf.  a_bufel is not modified.
 *
 ****************************************************************************/
cw_bool_t
buf_prepend_bufel(cw_buf_t * a_buf, cw_bufel_t * a_bufel);


/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_bufc : Pointer to a bufc.
 *
 * a_beg_offset : Offset of first valid byte in a_bufc's memory buffer.
 *
 * a_end_offset : Offset of first byte past the valid range of bytes in a_bufc's
 *                memory buffer.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.  a_buf is still valid.
 *
 * <<< Description >>>
 *
 * Prepend a_bufc, bytes a_beg_offset .. (a_end_offset - 1) to a_buf.
 *
 ****************************************************************************/
cw_bool_t
buf_prepend_bufc(cw_buf_t * a_buf, cw_bufc_t * a_bufc,
		 cw_uint32_t a_beg_offset, cw_uint32_t a_end_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_bufc : Pointer to a bufc.
 *
 * a_beg_offset : Offset of first valid byte in a_bufc's memory buffer.
 *
 * a_end_offset : Offset of first byte past the valid range of bytes in a_bufc's
 *                memory buffer.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.  a_buf is still valid.
 *
 * <<< Description >>>
 *
 * Append a_bufc, bytes a_beg_offset .. (a_end_offset - 1) to a_buf.
 *
 ****************************************************************************/
cw_bool_t
buf_append_bufc(cw_buf_t * a_buf, cw_bufc_t * a_bufc,
		cw_uint32_t a_beg_offset, cw_uint32_t a_end_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_bufel : Pointer to a bufel.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.  a_buf is still valid.
 *
 * <<< Description >>>
 *
 * Append the data from a_bufel to a_buf.  a_bufel is not modified.
 *
 ****************************************************************************/
cw_bool_t
buf_append_bufel(cw_buf_t * a_buf, cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_amount : Number of bytes of data to release from the head of a_buf.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *
 * <<< Description >>>
 *
 * Release a_amount bytes from the head of a_buf.
 *
 ****************************************************************************/
cw_bool_t
buf_release_head_data(cw_buf_t * a_buf, cw_uint32_t a_amount);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_amount : Number of bytes of data to release from the tail of a_buf.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *
 * <<< Description >>>
 *
 * Release a_amount bytes from the tail of a_buf.
 *
 ****************************************************************************/
cw_bool_t
buf_release_tail_data(cw_buf_t * a_buf, cw_uint32_t a_amount);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_offset : Offset in bytes of uint8 to return.
 *
 * <<< Output(s) >>>
 *
 * retval : Value of the uint8 at offset a_offset in a_buf.
 *
 * <<< Description >>>
 *
 * Return the uint8 at offset a_offset.
 *
 ****************************************************************************/
cw_uint8_t
buf_get_uint8(cw_buf_t * a_buf, cw_uint32_t a_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_offset : Offset in bytes of uint32 to return.
 *
 * <<< Output(s) >>>
 *
 * retval : Value of the uint32 at offset a_offset in a_buf.
 *
 * <<< Description >>>
 *
 * Return the uint32 at offset a_offset.
 *
 ****************************************************************************/
cw_uint32_t
buf_get_uint32(cw_buf_t * a_buf, cw_uint32_t a_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_offset : Offset in bytes of uint64 to return.
 *
 * <<< Output(s) >>>
 *
 * retval : Value of the uint64 at offset a_offset in a_buf.
 *
 * <<< Description >>>
 *
 * Return the uint64 at offset a_offset.
 *
 ****************************************************************************/
cw_uint64_t
buf_get_uint64(cw_buf_t * a_buf, cw_uint32_t a_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_offset : Offset in bytes of data to set. (a_offset <= buf_get_size(a_buf))
 *
 * a_val : Value to set data at a_offset to.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : memory allocation error.
 *
 * <<< Description >>>
 *
 * Set the uint8 at a_offset to a_val.
 *
 ****************************************************************************/
cw_bool_t
buf_set_uint8(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint8_t a_val);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_offset : Offset in bytes of data to set. (a_offset <= buf_get_size(a_buf))
 *
 * a_val : Value to set data at a_offset to.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : memory allocation error.
 *
 * <<< Description >>>
 *
 * Set the uint32 at a_offset to a_val.
 *
 ****************************************************************************/
cw_bool_t
buf_set_uint32(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint32_t a_val);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_offset : Offset in bytes of data to set. (a_offset <= buf_get_size(a_buf))
 *
 * a_val : Value to set data at a_offset to.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : memory allocation error.
 *
 * <<< Description >>>
 *
 * Set the uint64 at a_offset to a_val.
 *
 ****************************************************************************/
cw_bool_t
buf_set_uint64(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint64_t a_val);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_offset : Offset in bytes of data to set. (a_offset <= buf_get_size(a_buf))
 *
 * a_length : Number of bytes to copy from a_val.
 *
 * a_val : Value to set data at a_offset to.
 *
 * a_is_writeable : FALSE == non-writeable buffer, TRUE == writeable buffer.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : memory allocation error.
 *
 * <<< Description >>>
 *
 * Copy a_offset bytes from a_val to a_buf at offset a_offset.
 *
 ****************************************************************************/
cw_bool_t
buf_set_range(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint32_t a_length,
	      cw_uint8_t * a_val, cw_bool_t a_is_writeable);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufc : Pointer to space for a bufc, or NULL.
 *
 * a_dealloc_func : Pointer to a deallocation function for a_bufc, or NULL.
 *                  Ignored if a_bufc == NULL.
 *
 * a_dealloc_arg : First argument to a_dealloc_func.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to an initialized bufc, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.  The return value should be used in a call to bufc_set_buffer(),
 * then bufel_set_bufc().  bufc_delete() can be called at any time up until the
 * call to bufel_set_bufc() to deallocate, but should not be called thereafter.
 *
 ****************************************************************************/
cw_bufc_t *
bufc_new(cw_bufc_t * a_bufc,
	 void (*a_dealloc_func)(void * dealloc_arg, void * bufel),
	 void * a_dealloc_arg);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufc : Pointer to a bufc.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.  Only call this if a_bufc was never used in a call to
 * bufel_set_bufc().
 *
 ****************************************************************************/
void
bufc_delete(cw_bufc_t * a_bufc);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufc : Pointer to a bufc.
 *
 * a_buffer : Pointer to a buffer.
 *
 * a_size : Size of *a_buffer.
 *
 * a_dealloc_func : Pointer to a deallocation function for a_buffer, or NULL.
 *
 * a_dealloc_arg : First argument to a_dealloc_func.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set a_bufc's internal data buffer to a_buffer, with size a_size, and
 * deallocation function a_dealloc_func(a_dealloc_arg, a_buffer).
 *
 ****************************************************************************/
void
bufc_set_buffer(cw_bufc_t * a_bufc, void * a_buffer, cw_uint32_t a_size,
		cw_bool_t a_is_writeable,
		void (*a_dealloc_func)(void * dealloc_arg, void * buffer),
		void * a_dealloc_arg);
