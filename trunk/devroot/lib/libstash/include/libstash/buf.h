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

typedef struct cw_buf_s cw_buf_t;
typedef struct cw_bufel_s cw_bufel_t;

struct cw_buf_s
{
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_bool_t is_threadsafe;
  cw_mtx_t lock;
#endif
  cw_uint32_t size;
  cw_list_t bufels;
};

struct cw_bufel_s
{
  cw_bool_t is_malloced;
  cw_uint32_t buf_size;
  cw_uint32_t beg_offset;
  cw_uint32_t end_offset;
  cw_uint32_t * buf;
};

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
 * Concatenates two bufs.  After this function call, a_other is empty, but
 * it still exists.
 *
 ****************************************************************************/
#define buf_prepend_buf _CW_NS_STASH(buf_prepend_buf)
void
buf_prepend_buf(cw_buf_t * a_a, cw_buf_t * a_b);

/****************************************************************************
 *
 * Concatenates two bufs.  After this function call, a_other is empty, but
 * it still exists.
 *
 ****************************************************************************/
#define buf_append_buf _CW_NS_STASH(buf_append_buf)
void
buf_append_buf(cw_buf_t * a_a, cw_buf_t * a_b);

/****************************************************************************
 *
 * Removes the first bufel from a_buf and returns a pointer to it.
 *
 ****************************************************************************/
#define buf_rm_head_bufel _CW_NS_STASH(buf_rm_head_bufel)
cw_bufel_t *
buf_rm_head_bufel(cw_buf_t * a_buf);

/****************************************************************************
 *
 * Prepends a bufel to a_buf.
 *
 ****************************************************************************/
#define buf_prepend_bufel _CW_NS_STASH(buf_prepend_bufel)
void
buf_prepend_bufel(cw_buf_t * a_buf, cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * Appends a bufel to a_buf.
 *
 ****************************************************************************/
#define buf_append_bufel _CW_NS_STASH(buf_append_bufel)
void
buf_append_bufel(cw_buf_t * a_buf, cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * bufel constructor.
 *
 ****************************************************************************/
#define bufel_new _CW_NS_STASH(bufel_new)
cw_bufel_t *
bufel_new(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * bufel destructor.
 *
 ****************************************************************************/
#define bufel_delete _CW_NS_STASH(bufel_delete)
void
bufel_delete(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * Returns total size of a_bufel's internal buffer.  This is _not_
 * necessarily the same as the amount of valid data.
 *
 ****************************************************************************/
#define bufel_get_size _CW_NS_STASH(bufel_get_size)
cw_uint32_t
bufel_get_size(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * Either malloc()s or realloc()s the internal buffer to be of size a_size.  If
 * shrinking the bufel would chop off valid data as indicated by end_offset,
 * return TRUE.
 *
 ****************************************************************************/
#define bufel_set_size _CW_NS_STASH(bufel_set_size)
cw_bool_t
bufel_set_size(cw_bufel_t * a_bufel, cw_uint32_t a_size);

/****************************************************************************
 *
 * Returns the offset to the begin pointer.
 *
 ****************************************************************************/
#define bufel_get_beg_offset _CW_NS_STASH(bufel_get_beg_offset)
cw_uint32_t
bufel_get_beg_offset(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * Sets the begin pointer offset.  If an attempt is made to set the beg_offset
 * to something greater than end_offset, return TRUE.
 *
 ****************************************************************************/
#define bufel_set_beg_offset _CW_NS_STASH(bufel_set_beg_offset)
cw_bool_t
bufel_set_beg_offset(cw_bufel_t * a_bufel, cw_uint32_t a_offset);

/****************************************************************************
 *
 * Returns the offset to the end pointer.
 *
 ****************************************************************************/
#define bufel_get_end_offset _CW_NS_STASH(bufel_get_end_offset)
cw_uint32_t
bufel_get_end_offset(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * Sets the end pointer offset.  If an attempt is made to set the end_offset
 * to something less than beg_offset, return TRUE.  If an attempt is made to set
 * end_offset to something greater than one past the end of the buffer
 * (buf_size), return TRUE.
 *
 ****************************************************************************/
#define bufel_set_end_offset _CW_NS_STASH(bufel_set_end_offset)
cw_bool_t
bufel_set_end_offset(cw_bufel_t * a_bufel, cw_uint32_t a_offset);

/****************************************************************************
 *
 * Returns the number of bytes between the beginning offset marker and the end
 * offset marker.
 *
 ****************************************************************************/
#define bufel_get_valid_data_size _CW_NS_STASH(bufel_get_valid_data_size)
cw_uint32_t
bufel_get_valid_data_size(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * Returns a pointer to the internal buffer.
 *
 ****************************************************************************/
#define bufel_get_data_ptr _CW_NS_STASH(bufel_get_data_ptr)
void *
bufel_get_data_ptr(cw_bufel_t * a_bufel);

/****************************************************************************
 *
 * Sets the internal buffer to a_buf, assumes the a_buf is a_size bytes long,
 * and sets the beginning offset to 0 and the end offset to a_size.  If there is
 * already an internal buffer, the old one is freed.  Note that a_buf _must_ be
 * allocated on the heap, since the bufel class will eventually free it.
 *
 ****************************************************************************/
#define bufel_set_data_ptr _CW_NS_STASH(bufel_set_data_ptr)
void
bufel_set_data_ptr(cw_bufel_t * a_bufel, void * a_buf, cw_uint32_t a_size);

/****************************************************************************
 *
 * Returns the uint8 at offset a_offset.
 *
 ****************************************************************************/
#define bufel_get_uint8 _CW_NS_STASH(bufel_get_uint8)
cw_uint8_t
bufel_get_uint8(cw_bufel_t * a_bufel, cw_uint32_t a_offset);

/****************************************************************************
 *
 * Sets the uint8 at a_offset to a_val.
 *
 ****************************************************************************/
#define bufel_set_uint8 _CW_NS_STASH(bufel_set_uint8)
void
bufel_set_uint8(cw_bufel_t * a_bufel, cw_uint32_t a_offset, cw_uint8_t a_val);

/****************************************************************************
 *
 * Returns the uint32 at offset a_offset.  If a_offset is not a multiple of 4,
 * behavior is undefined.  If the buffer size is not a multiple
 * of 4, reading the last bytes of the buffer has undefined behavior.  That is,
 * segmentation faults could ensue.
 *
 ****************************************************************************/
#define bufel_get_uint32 _CW_NS_STASH(bufel_get_uint32)
cw_uint32_t
bufel_get_uint32(cw_bufel_t * a_bufel, cw_uint32_t a_offset);

/****************************************************************************
 *
 * Sets the uint32 at a_offset to a_val.
 *
 ****************************************************************************/
#define bufel_set_uint32 _CW_NS_STASH(bufel_set_uint32)
void
bufel_set_uint32(cw_bufel_t * a_bufel, cw_uint32_t a_offset, cw_uint32_t a_val);
