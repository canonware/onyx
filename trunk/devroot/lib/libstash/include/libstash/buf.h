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

#ifdef _STASH_DBG
#define _CW_BUF_MAGIC 0xb00f0001
#define _CW_BUFEL_MAGIC 0xb00f0002
#define _CW_BUFC_MAGIC 0xb00f0003
#define _CW_BUFPOOL_MAGIC 0xb00f0004
#endif

typedef struct cw_bufpool_s cw_bufpool_t;
typedef struct cw_buf_s cw_buf_t;
typedef struct cw_bufel_s cw_bufel_t;

struct cw_bufpool_s
{
#ifdef _STASH_DBG
  cw_uint32_t magic;
#endif
  cw_uint32_t buffer_size;
  cw_uint32_t max_spare_buffers;
  cw_list_t spare_buffers;
};

typedef struct
{
#ifdef _STASH_DBG
  cw_uint32_t magic;
#endif
#ifdef _CW_REENTRANT
  cw_mtx_t lock;
#endif
  void (*free_func)(void *, void *);
  void * free_arg;
  cw_uint32_t ref_count;
  cw_uint32_t buf_size;
  char * buf;
} cw_bufc_t;

struct cw_bufel_s
{
#ifdef _STASH_DBG
  cw_uint32_t magic;
#endif
  cw_bool_t is_malloced;
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
#ifdef _STASH_DBG
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
};

#define bufpool_new _CW_NS_STASH(bufpool_new)
cw_bufpool_t *
bufpool_new(cw_bufpool_t * a_bufpool, cw_uint32_t a_buffer_size,
	    cw_uint32_t a_max_spare_buffers);

#define bufpool_delete _CW_NS_STASH(bufpool_delete)
void
bufpool_delete(cw_bufpool_t * a_bufpool);

#define bufpool_get_buffer_size _CW_NS_STASH(bufpool_get_buffer_size)
cw_uint32_t
bufpool_get_buffer_size(cw_bufpool_t * a_bufpool);

#define bufpool_get_max_spare_buffers \
        _CW_NS_STASH(bufpool_get_max_spare_buffers)
cw_uint32_t
bufpool_get_max_spare_buffers(cw_bufpool_t * a_bufpool);

#define bufpool_set_max_spare_buffers \
        _CW_NS_STASH(bufpool_set_max_spare_buffers)
void
bufpool_set_max_spare_buffers(cw_bufpool_t * a_bufpool,
			      cw_uint32_t a_max_spare_buffers);

#define bufpool_get_buffer _CW_NS_STASH(bufpool_get_buffer)
void *
bufpool_get_buffer(cw_bufpool_t * a_bufpool);

#define bufpool_put_buffer _CW_NS_STASH(bufpool_put_buffer)
void
bufpool_put_buffer(cw_bufpool_t * a_bufpool, void * a_buffer);

/****************************************************************************
 *
 * buf constructor.
 *
 ****************************************************************************/
#define buf_new _CW_NS_STASH(buf_new)
#ifdef _CW_REENTRANT
cw_buf_t *
buf_new(cw_buf_t * a_buf, cw_bool_t a_is_threadsafe);
#else
cw_buf_t *
buf_new(cw_buf_t * a_buf);
#endif

/****************************************************************************
 *
 * buf destructor.
 *
 ****************************************************************************/
#define buf_delete _CW_NS_STASH(buf_delete)
void
buf_delete(cw_buf_t * a_buf);

/****************************************************************************
 *
 * Returns the amount of valid data in bytes.
 *
 ****************************************************************************/
#define buf_get_size _CW_NS_STASH(buf_get_size)
cw_uint32_t
buf_get_size(cw_buf_t * a_buf);

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
 * a_try_bufel_merge : If TRUE, try to merge a_a's last bufel and a_b's first
 *                     bufel, if the two bufel's reference the same internal
 *                     buffer and are consectutive and contiguous.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Catenate two bufs.  a_b is left unmodified.
 *
 ****************************************************************************/
#define buf_append_buf _CW_NS_STASH(buf_append_buf)
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
#define buf_split _CW_NS_STASH(buf_split)
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
#define buf_prepend_bufel _CW_NS_STASH(buf_prepend_bufel)
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
#define buf_append_bufel _CW_NS_STASH(buf_append_bufel)
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
#define buf_release_head_data _CW_NS_STASH(buf_release_head_data)
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
#define buf_release_tail_data _CW_NS_STASH(buf_release_tail_data)
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
#define buf_get_uint8 _CW_NS_STASH(buf_get_uint8)
cw_uint8_t
buf_get_uint8(cw_buf_t * a_buf, cw_uint32_t a_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_offset : Offset in bytes of uint8 to set.
 *
 * a_val : Value to set the uint8 at offset a_offset in a_buf.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set the uint8 at offset a_offset to a_val.
 *
 ****************************************************************************/
#define buf_set_uint8 _CW_NS_STASH(buf_set_uint8)
void
buf_set_uint8(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint8_t a_val);

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
#define buf_get_uint32 _CW_NS_STASH(buf_get_uint32)
cw_uint32_t
buf_get_uint32(cw_buf_t * a_buf, cw_uint32_t a_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_offset : Offset in bytes of uint32 to set.
 *
 * a_val : Value to set the uint32 at offset a_offset in a_buf.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set the uint32 at offset a_offset to a_val.
 *
 ****************************************************************************/
#define buf_set_uint32 _CW_NS_STASH(buf_set_uint32)
void
buf_set_uint32(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint32_t a_val);

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
#define buf_get_uint64 _CW_NS_STASH(buf_get_uint64)
cw_uint64_t
buf_get_uint64(cw_buf_t * a_buf, cw_uint32_t a_offset);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_buf : Pointer to a buf.
 *
 * a_offset : Offset in bytes of uint64 to set.
 *
 * a_val : Value to set the uint64 at offset a_offset in a_buf.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set the uint64 at offset a_offset to a_val.
 *
 ****************************************************************************/
#define buf_set_uint64 _CW_NS_STASH(buf_set_uint64)
void
buf_set_uint64(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint64_t a_val);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_bufel : Pointer to space for a bufel, or NULL.
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
#define bufel_new _CW_NS_STASH(bufel_new)
cw_bufel_t *
bufel_new(cw_bufel_t * a_bufel);

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
#define bufel_delete _CW_NS_STASH(bufel_delete)
void
bufel_delete(cw_bufel_t * a_bufel);

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
#define bufel_get_size _CW_NS_STASH(bufel_get_size)
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
#define bufel_get_beg_offset _CW_NS_STASH(bufel_get_beg_offset)
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
#define bufel_set_beg_offset _CW_NS_STASH(bufel_set_beg_offset)
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
#define bufel_get_end_offset _CW_NS_STASH(bufel_get_end_offset)
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
#define bufel_set_end_offset _CW_NS_STASH(bufel_set_end_offset)
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
#define bufel_get_valid_data_size _CW_NS_STASH(bufel_get_valid_data_size)
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
#define bufel_get_data_ptr _CW_NS_STASH(bufel_get_data_ptr)
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
 * a_free_func :
 *
 * a_free_arg :
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set the internal buffer to a_buf.  Assume the a_buf is a_size bytes long, and
 * set the beginning offset to 0 and the end offset to a_size.  If there is
 * already an internal buffer, unreference the old one.  Note that a_buf _must_
 * be allocated on the heap, since the bufel class will eventually free it.
 *
 ****************************************************************************/
#define bufel_set_data_ptr _CW_NS_STASH(bufel_set_data_ptr)
void
bufel_set_data_ptr(cw_bufel_t * a_bufel, void * a_buf, cw_uint32_t a_size,
		   void (*a_free_func)(void * free_arg, void * buffer_p),
		   void * a_free_arg);
