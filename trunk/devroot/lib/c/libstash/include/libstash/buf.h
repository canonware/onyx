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
 ****************************************************************************/

/* Minimum size for buf's internal bufel array.  This must be a power of two,
 * since bit masks are used to wrap around the end of the circular
 * bufel_array.  Note also that static arrays are included in the buf structure
 * for this number of elements, so the higher this number, the larger cw_buf_t.
 */
#define _LIBSTASH_BUF_ARRAY_MIN_SIZE 4

/* Pseudo-opaque typedefs. */
typedef struct cw_buf_s cw_buf_t;
typedef struct cw_bufc_s cw_bufc_t;
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

struct cw_bufel_s
{
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif
  cw_uint32_t beg_offset;
  cw_uint32_t end_offset;
  cw_bufc_t * bufc;
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
  cw_bufel_t static_bufel_array[_LIBSTASH_BUF_ARRAY_MIN_SIZE];
  cw_uint32_t static_cumulative_index[_LIBSTASH_BUF_ARRAY_MIN_SIZE];
  struct iovec static_iov[_LIBSTASH_BUF_ARRAY_MIN_SIZE];
};

cw_buf_t *
buf_new(cw_buf_t * a_buf);

#ifdef _CW_REENTRANT
cw_buf_t *
buf_new_r(cw_buf_t * a_buf);
#endif

void
buf_delete(cw_buf_t * a_buf);

void
buf_dump(cw_buf_t * a_buf, const char * a_prefix);

cw_sint32_t
buf_out_metric(const char * a_format, cw_uint32_t a_len, const void * a_arg);

char *
buf_out_render(const char * a_format, cw_uint32_t a_len, const void * a_arg,
	       char * r_buf);

cw_uint32_t
buf_get_size(cw_buf_t * a_buf);

cw_uint32_t
buf_get_num_bufels(cw_buf_t * a_buf);

const struct iovec *
buf_get_iovec(cw_buf_t * a_buf, cw_uint32_t a_max_data,
	      cw_bool_t a_is_sys_iovec, int * r_iovec_count);

cw_bool_t
buf_catenate_buf(cw_buf_t * a_a, cw_buf_t * a_b, cw_bool_t a_preserve);

cw_bool_t
buf_split(cw_buf_t * a_a, cw_buf_t * a_b, cw_uint32_t a_offset);

cw_bool_t
buf_prepend_bufc(cw_buf_t * a_buf, cw_bufc_t * a_bufc,
		 cw_uint32_t a_beg_offset, cw_uint32_t a_end_offset);

cw_bool_t
buf_append_bufc(cw_buf_t * a_buf, cw_bufc_t * a_bufc,
		cw_uint32_t a_beg_offset, cw_uint32_t a_end_offset);

cw_bool_t
buf_release_head_data(cw_buf_t * a_buf, cw_uint32_t a_amount);

cw_bool_t
buf_release_tail_data(cw_buf_t * a_buf, cw_uint32_t a_amount);

cw_uint8_t
buf_get_uint8(cw_buf_t * a_buf, cw_uint32_t a_offset);

cw_uint32_t
buf_get_uint32(cw_buf_t * a_buf, cw_uint32_t a_offset);

cw_uint64_t
buf_get_uint64(cw_buf_t * a_buf, cw_uint32_t a_offset);

cw_bool_t
buf_set_uint8(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint8_t a_val);

cw_bool_t
buf_set_uint32(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint32_t a_val);

cw_bool_t
buf_set_uint64(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint64_t a_val);

cw_bool_t
buf_set_range(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint32_t a_length,
	      cw_uint8_t * a_val, cw_bool_t a_is_writeable);

cw_bufc_t *
bufc_new(cw_bufc_t * a_bufc,
	 void (*a_dealloc_func)(void * dealloc_arg, void * bufc),
	 void * a_dealloc_arg);

void
bufc_delete(cw_bufc_t * a_bufc);

void
bufc_set_buffer(cw_bufc_t * a_bufc, void * a_buffer, cw_uint32_t a_size,
		cw_bool_t a_is_writeable,
		void (*a_dealloc_func)(void * dealloc_arg, void * buffer),
		void * a_dealloc_arg);

cw_uint32_t
bufc_get_size(cw_bufc_t * a_bufc);
