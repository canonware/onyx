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
 * Simple extensible (mostly FIFO) buffer class.  Although this may be
 * useful for other purposes, buf and bufel are designed to support
 * asynchronous buffering of I/O for the sock (socket) class.
 *
 ****************************************************************************/

#ifdef _LIBSTASH_DBG
#  define _CW_BUF_MAGIC 0xb00f0001
#  define _CW_BUFEL_MAGIC 0xb00f0002
#  define _CW_BUFC_MAGIC 0xb00f0003
#  define _CW_BUFPOOL_MAGIC 0xb00f0004
#endif

/* Pseudo-opaque typedefs. */
typedef struct cw_bufpool_s cw_bufpool_t;
typedef struct cw_buf_s cw_buf_t;
typedef struct cw_bufel_s cw_bufel_t;

/* The following data types should be considered opaque. */
struct cw_bufpool_s
{
  cw_bool_t is_malloced;
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
#ifdef _CW_REENTRANT
  cw_mtx_t lock;
#endif
  cw_uint32_t buffer_size;
  cw_uint32_t max_spare_buffers;
  cw_list_t spare_buffers;
};

typedef struct
{
  cw_bool_t is_malloced;
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
  cw_uint32_t buf_size;
  char * buf;
} cw_bufc_t;

struct cw_bufel_s
{
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
  cw_bool_t is_malloced;

  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;

  cw_uint32_t beg_offset;
  cw_uint32_t end_offset;
  cw_bufc_t * bufc;
};

typedef struct
{
  cw_bufel_t bufel;
  cw_uint32_t cumulative_size;
} cw_bufel_array_el_t;

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
  cw_bufel_array_el_t * array;
  struct iovec * iov;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufpool : Pointer to space for a bufpool, or NULL.
 *
 * a_buffer_size : Size of buffers to use.
 *
 * a_max_spare_buffers : Maximum number of buffers to cache.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a bufpool.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
#define bufpool_new _CW_NS_LIBSTASH(bufpool_new)
cw_bufpool_t *
bufpool_new(cw_bufpool_t * a_bufpool, cw_uint32_t a_buffer_size,
	    cw_uint32_t a_max_spare_buffers);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufpool : Pointer to a bufpool.
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
#define bufpool_delete _CW_NS_LIBSTASH(bufpool_delete)
void
bufpool_delete(cw_bufpool_t * a_bufpool);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufpool : Pointer to a bufpool.
 *
 * <<< Output(s) >>>
 *
 * retval : Size of buffers that a_bufpool is using.
 *
 * <<< Description >>>
 *
 * Return the size of the buffers that a_bufpool is using.
 *
 ****************************************************************************/
#define bufpool_get_buffer_size _CW_NS_LIBSTASH(bufpool_get_buffer_size)
cw_uint32_t
bufpool_get_buffer_size(cw_bufpool_t * a_bufpool);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufpool : Pointer to a bufpool.
 *
 * <<< Output(s) >>>
 *
 * retval : Maximum number of spare buffers that will be cached.
 *
 * <<< Description >>>
 *
 * Return the maximum number of spare buffers that will be cached.
 *
 ****************************************************************************/
#define bufpool_get_max_spare_buffers \
        _CW_NS_LIBSTASH(bufpool_get_max_spare_buffers)
cw_uint32_t
bufpool_get_max_spare_buffers(cw_bufpool_t * a_bufpool);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufpool : Pointer to a bufpool.
 *
 * a_max_spare_buffers : Maximum number of spare buffers to cache.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set the maximum number of spare buffers to cache to a_max_spare_buffers.
 *
 ****************************************************************************/
#define bufpool_set_max_spare_buffers \
        _CW_NS_LIBSTASH(bufpool_set_max_spare_buffers)
void
bufpool_set_max_spare_buffers(cw_bufpool_t * a_bufpool,
			      cw_uint32_t a_max_spare_buffers);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufpool : Pointer to a bufpool.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a buffer.
 *
 * <<< Description >>>
 *
 * Return a buffer of size bufpool_get_buffer_size(a_bufpool).
 *
 ****************************************************************************/
#define bufpool_get_buffer _CW_NS_LIBSTASH(bufpool_get_buffer)
void *
bufpool_get_buffer(cw_bufpool_t * a_bufpool);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufpool : Pointer to a bufpool.
 *
 * a_buffer : Pointer to a buffer (of size bufpool_get_buffer_size(a_bufpool)).
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Release a_buffer and let a_bufpool deal with it.
 *
 ****************************************************************************/
#define bufpool_put_buffer _CW_NS_LIBSTASH(bufpool_put_buffer)
void
bufpool_put_buffer(void * a_bufpool, void * a_buffer);

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
 * retval : Pointer to a buf.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
#define buf_new _CW_NS_LIBSTASH(buf_new)
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
#define buf_delete _CW_NS_LIBSTASH(buf_delete)
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
#define buf_dump _CW_NS_LIBSTASH(buf_dump)
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
#define buf_get_size _CW_NS_LIBSTASH(buf_get_size)
cw_uint32_t
buf_get_size(cw_buf_t * a_buf);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_max_data : Maximum number of bytes of space to include in the iovec.
 *
 * a_iovec_count : Pointer to an int.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to an iovec array that represents the internal data buffers
 *          in a_buf.
 *
 * *a_iovec_count : Number of valid iovec structures in retval.
 *
 * <<< Description >>>
 *
 * Build an iovec array that represents the valid data in a_buf's internal
 * buffers (up to a_max_data bytes) and return a pointer to it.
 *
 ****************************************************************************/
#define buf_get_iovec _CW_NS_LIBSTASH(buf_get_iovec)
const struct iovec *
buf_get_iovec(cw_buf_t * a_buf, cw_uint32_t a_max_data, int * a_iovec_count);

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
 * None.
 *
 * <<< Description >>>
 *
 * Catenate two bufs.  a_b is left unmodified if a_preserve is TRUE.
 *
 ****************************************************************************/
#define buf_catenate_buf _CW_NS_LIBSTASH(buf_catenate_buf)
void
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
 * None.
 *
 * <<< Description >>>
 *
 * Split a_b at offset a_offset.  Append the data before a_offset to a_a, and
 * leave the remainder in a_b.
 *
 ****************************************************************************/
#define buf_split _CW_NS_LIBSTASH(buf_split)
void
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
 * None.
 *
 * <<< Description >>>
 *
 * Prepend the data from a_bufel to a_buf.  a_bufel is not modified.
 *
 ****************************************************************************/
#define buf_prepend_bufel _CW_NS_LIBSTASH(buf_prepend_bufel)
void
buf_prepend_bufel(cw_buf_t * a_buf, cw_bufel_t * a_bufel);

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
 * None.
 *
 * <<< Description >>>
 *
 * Append the data from a_bufel to a_buf.  a_bufel is not modified.
 *
 ****************************************************************************/
#define buf_append_bufel _CW_NS_LIBSTASH(buf_append_bufel)
void
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
#define buf_release_head_data _CW_NS_LIBSTASH(buf_release_head_data)
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
#define buf_release_tail_data _CW_NS_LIBSTASH(buf_release_tail_data)
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
#define buf_get_uint8 _CW_NS_LIBSTASH(buf_get_uint8)
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
#define buf_get_uint32 _CW_NS_LIBSTASH(buf_get_uint32)
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
#define buf_get_uint64 _CW_NS_LIBSTASH(buf_get_uint64)
cw_uint64_t
buf_get_uint64(cw_buf_t * a_buf, cw_uint32_t a_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to space for a bufel, or NULL.
 *
 * a_dealloc_func : Pointer to a deallocation function for a_bufel, or NULL.
 *                  Ignored if a_bufel == NULL.
 *
 * a_dealloc_arg : First argument to a_dealloc_func.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a bufel.
 *
 * <<< Description >>>
 *
 * bufel constructor.
 *
 ****************************************************************************/
#define bufel_new _CW_NS_LIBSTASH(bufel_new)
cw_bufel_t *
bufel_new(cw_bufel_t * a_bufel,
	  void (*a_dealloc_func)(void * dealloc_arg, void * bufel),
	  void * a_dealloc_arg);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to a bufel.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * bufel destructor.
 *
 ****************************************************************************/
#define bufel_delete _CW_NS_LIBSTASH(bufel_delete)
void
bufel_delete(cw_bufel_t * a_bufel);

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
#define bufel_dump _CW_NS_LIBSTASH(bufel_dump)
void
bufel_dump(cw_bufel_t * a_bufel, const char * a_prefix);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to a bufel.
 *
 * <<< Output(s) >>>
 *
 * retval : Size of a_bufel's buffer, in bytes.
 *
 * <<< Description >>>
 *
 * Return the total size of a_bufel's buffer.
 * 
 ****************************************************************************/
#define bufel_get_size _CW_NS_LIBSTASH(bufel_get_size)
cw_uint32_t
bufel_get_size(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to a bufel.
 *
 * <<< Output(s) >>>
 *
 * retval : Offset from the beginning of the internal buffer to the begin
 *          pointer.
 *
 * <<< Description >>>
 *
 * Return the offset to the begin pointer.
 *
 ****************************************************************************/
#define bufel_get_beg_offset _CW_NS_LIBSTASH(bufel_get_beg_offset)
cw_uint32_t
bufel_get_beg_offset(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to a bufel.
 *
 * a_offset : Offset in bytes from the beginning of the internal buffer to set
 *            the begin pointer to.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Attempted to set the begin offset to something greater than
 *                 the end offset.
 *
 * <<< Description >>>
 *
 * Set the begin pointer offset.
 *
 ****************************************************************************/
#define bufel_set_beg_offset _CW_NS_LIBSTASH(bufel_set_beg_offset)
cw_bool_t
bufel_set_beg_offset(cw_bufel_t * a_bufel, cw_uint32_t a_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to a bufel.
 *
 * <<< Output(s) >>>
 *
 * retval : Offset from the beginning of the internal buffer to the end pointer.
 *
 * <<< Description >>>
 *
 * Return the offset to the end pointer.
 *
 ****************************************************************************/
#define bufel_get_end_offset _CW_NS_LIBSTASH(bufel_get_end_offset)
cw_uint32_t
bufel_get_end_offset(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to a bufel.
 *
 * a_offset : Offset in bytes from the beginning of the internal buffer to set
 *            the end pointer to.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Attempted to set the end offset to something less than
 *                 the begin offset.
 *
 * <<< Description >>>
 *
 * Set the end pointer offset.
 *
 ****************************************************************************/
#define bufel_set_end_offset _CW_NS_LIBSTASH(bufel_set_end_offset)
cw_bool_t
bufel_set_end_offset(cw_bufel_t * a_bufel, cw_uint32_t a_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to a bufel.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of valid data bytes (data between the begin offset and end
 *          offset).
 *
 * <<< Description >>>
 *
 * Return the number of bytes between the beginning offset marker and the end
 * offset marker.
 *
 ****************************************************************************/
#define bufel_get_valid_data_size _CW_NS_LIBSTASH(bufel_get_valid_data_size)
cw_uint32_t
bufel_get_valid_data_size(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to a bufel.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a_bufel's internal buffer
 *
 * <<< Description >>>
 *
 * Return a pointer to the internal buffer.
 *
 ****************************************************************************/
#define bufel_get_data_ptr _CW_NS_LIBSTASH(bufel_get_data_ptr)
const void *
bufel_get_data_ptr(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to a bufel.
 *
 * a_buf : Pointer to a memory buffer.
 *
 * a_size : Size in bytes of a_buf.
 *
 * a_free_func : Pointer to a function that handles reclamation of a_buf, or
 *               NULL.
 *
 * a_free_arg : First argument to a_free_func().
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set the internal buffer to a_buf.  Assume the a_buf is a_size bytes long, and
 * set the beginning offset to 0 and the end offset to a_size.  If there is
 * already an internal buffer, unreference the old one.
 *
 ****************************************************************************/
#define bufel_set_bufc _CW_NS_LIBSTASH(bufel_set_bufc)
void
bufel_set_bufc(cw_bufel_t * a_bufel, cw_bufc_t * a_bufc);

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
#define bufc_new _CW_NS_LIBSTASH(bufc_new)
cw_bufc_t *
bufc_new(cw_bufc_t * a_bufc,
	 void (*a_dealloc_func)(void * dealloc_arg, void * bufel),
	 void * a_dealloc_arg);

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
#define bufc_delete _CW_NS_LIBSTASH(bufc_delete)
void
bufc_delete(cw_bufc_t * a_bufc);

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
#define bufc_set_buffer _CW_NS_LIBSTASH(bufc_set_buffer)
void
bufc_set_buffer(cw_bufc_t * a_bufc, void * a_buffer, cw_uint32_t a_size,
		void (*a_dealloc_func)(void * dealloc_arg, void * buffer),
		void * a_dealloc_arg);
