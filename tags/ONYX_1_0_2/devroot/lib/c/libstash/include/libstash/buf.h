/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

/*
 * Minimum size for buf's internal bufel array.  This must be a power of two,
 * since bit masks are used to wrap around the end of the circular bufel_array.
 * Note also that static arrays are included in the buf structure for this
 * number of elements, so the higher this number, the larger cw_buf_t.
 */
#define _CW_BUF_ARRAY_MIN_SIZE 4

/* Pseudo-opaque typedefs. */
typedef struct cw_buf_s cw_buf_t;
typedef struct cw_bufc_s cw_bufc_t;
typedef struct cw_bufel_s cw_bufel_t;

/* The following data types should be considered opaque. */
struct cw_bufc_s {
#ifdef _CW_DBG
	cw_uint32_t	magic_a;
#endif
	cw_mtx_t	lock;
	cw_opaque_dealloc_t *dealloc_func;
	void		*dealloc_arg;
	cw_opaque_dealloc_t *buffer_dealloc_func;
	void		*buffer_dealloc_arg;
	cw_uint32_t	ref_count;
	cw_bool_t	is_writeable;
	cw_uint32_t	buf_size;
	cw_uint8_t	*buf;
#ifdef _CW_DBG
	cw_uint32_t	size_of;
	cw_uint32_t	magic_b;
#endif
};

struct cw_bufel_s {
#ifdef _CW_DBG
	cw_uint32_t	magic_a;
#endif
	cw_uint32_t	beg_offset;
	cw_uint32_t	end_offset;
	cw_bufc_t	*bufc;
#ifdef _CW_DBG
	cw_uint32_t	size_of;
	cw_uint32_t	magic_b;
#endif
};

struct cw_buf_s {
#ifdef _CW_DBG
	cw_uint32_t	magic_a;
#endif
	cw_mem_t	*mem;
	cw_bool_t	is_malloced:1;
	cw_bool_t	is_threadsafe:1;
	cw_mtx_t	lock;
	cw_uint32_t	size;

	cw_uint32_t	array_size;
	cw_uint32_t	array_num_valid;
	cw_uint32_t	array_start;
	cw_uint32_t	array_end;
	cw_bool_t	is_cumulative_valid:1;
	cw_bool_t	is_cached_bufel_valid:1;
	cw_uint32_t	cached_bufel;
	cw_bufel_t	*bufel_array;
	cw_uint32_t	*cumulative_index;
	struct iovec	*iov;
	cw_bufel_t	static_bufel_array[_CW_BUF_ARRAY_MIN_SIZE];
	cw_uint32_t	static_cumulative_index[_CW_BUF_ARRAY_MIN_SIZE];
	struct iovec	static_iov[_CW_BUF_ARRAY_MIN_SIZE];
#ifdef _CW_DBG
	cw_uint32_t	size_of;
	cw_uint32_t	magic_b;
#endif
};

cw_buf_t	*buf_new(cw_buf_t *a_buf, cw_mem_t *a_mem);
cw_buf_t	*buf_new_r(cw_buf_t *a_buf, cw_mem_t *a_mem);
void		buf_delete(cw_buf_t *a_buf);
void		buf_dump(cw_buf_t *a_buf, const char *a_prefix);
cw_uint32_t	buf_size_get(cw_buf_t *a_buf);
cw_uint32_t	buf_num_bufels_get(cw_buf_t *a_buf);
const struct iovec *buf_iovec_get(cw_buf_t *a_buf, cw_uint32_t a_max_data,
    cw_bool_t a_is_sys_iovec, int *r_iovec_count);
void		buf_buf_catenate(cw_buf_t *a_a, cw_buf_t *a_b, cw_bool_t
    a_preserve);
void		buf_split(cw_buf_t *a_a, cw_buf_t *a_b, cw_uint32_t a_offset);
void		buf_bufc_prepend(cw_buf_t *a_buf, cw_bufc_t *a_bufc, cw_uint32_t
    a_beg_offset, cw_uint32_t a_end_offset);
void		buf_bufc_append(cw_buf_t *a_buf, cw_bufc_t *a_bufc, cw_uint32_t
    a_beg_offset, cw_uint32_t a_end_offset);
cw_bool_t	buf_head_data_release(cw_buf_t *a_buf, cw_uint32_t a_amount);
cw_bool_t	buf_tail_data_release(cw_buf_t *a_buf, cw_uint32_t a_amount);
cw_uint8_t	buf_uint8_get(cw_buf_t *a_buf, cw_uint32_t a_offset);
cw_uint32_t	buf_uint32_get(cw_buf_t *a_buf, cw_uint32_t a_offset);
cw_uint64_t	buf_uint64_get(cw_buf_t *a_buf, cw_uint32_t a_offset);
void		buf_uint8_set(cw_buf_t *a_buf, cw_uint32_t a_offset, cw_uint8_t
    a_val);
void		buf_uint32_set(cw_buf_t *a_buf, cw_uint32_t a_offset,
    cw_uint32_t a_val);
void		buf_uint64_set(cw_buf_t *a_buf, cw_uint32_t a_offset,
    cw_uint64_t a_val);
void		buf_range_set(cw_buf_t *a_buf, cw_uint32_t a_offset, cw_uint32_t
    a_length, cw_uint8_t *a_val, cw_bool_t a_is_writeable);

cw_bufc_t	*bufc_new(cw_bufc_t *a_bufc, cw_mem_t *a_mem,
    cw_opaque_dealloc_t *a_dealloc_func, void *a_dealloc_arg);
void		bufc_delete(cw_bufc_t *a_bufc);
void		bufc_buffer_set(cw_bufc_t *a_bufc, void *a_buffer, cw_uint32_t
    a_size, cw_bool_t a_is_writeable, cw_opaque_dealloc_t *a_dealloc_func, void
    *a_dealloc_arg);
cw_uint32_t	bufc_size_get(cw_bufc_t *a_bufc);