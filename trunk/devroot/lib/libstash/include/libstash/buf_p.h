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
 * Private buf, bufel, and bufc interfaces.
 *
 ****************************************************************************/

/* Minimum size for buf's internal bufel array.  This must be a power of two,
 * since bit masks are used to wrap around the end of the circular
 * bufel_array. */
#define _LIBSTASH_BUF_ARRAY_MIN_SIZE 1

#ifdef _LIBSTASH_DBG
#  define _CW_BUF_MAGIC 0xb00f0001
#  define _CW_BUFEL_MAGIC 0xb00f0002
#  define _CW_BUFC_MAGIC 0xb00f0003
#endif

static void
buf_p_rebuild_cumulative_index(cw_buf_t * a_buf);

static void
buf_p_get_data_position(cw_buf_t * a_buf,
			cw_uint32_t a_offset,
			cw_uint32_t * a_array_element,
			cw_uint32_t * a_bufel_offset);

static cw_bool_t
buf_p_fit_array(cw_buf_t * a_buf, cw_uint32_t a_min_array_size);

static cw_bool_t
buf_p_catenate_buf(cw_buf_t * a_a, cw_buf_t * a_b, cw_bool_t a_preserve);

#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
static void
buf_p_copy_array(cw_buf_t * a_a, cw_buf_t * a_b,
		 cw_uint32_t a_num_elements,
		 cw_uint32_t a_a_start, cw_uint32_t a_b_start,
		 cw_bool_t a_is_destructive);
#else
static void
buf_p_copy_array(cw_buf_t * a_a, cw_buf_t * a_b,
		 cw_uint32_t a_num_elements,
		 cw_uint32_t a_a_start, cw_uint32_t a_b_start);
#endif

static cw_bool_t
buf_p_make_range_writeable(cw_buf_t * a_buf, cw_uint32_t a_offset,
			   cw_uint32_t a_length);
  
static cw_bool_t
bufel_p_merge_bufel(cw_bufel_t * a_a, cw_bufel_t * a_b);

static cw_bool_t
bufel_p_make_writeable(cw_bufel_t * a_bufel);

static void
bufc_p_dump(cw_bufc_t * a_bufc, const char * a_prefix);

static cw_uint32_t
bufc_p_get_size(cw_bufc_t * a_bufc);

static cw_uint8_t *
bufc_p_get_p(cw_bufc_t * a_bufc);

static cw_bool_t
bufc_p_get_is_writeable(cw_bufc_t * a_bufc);

static cw_uint32_t
bufc_p_get_ref_count(cw_bufc_t * a_bufc);

static void
bufc_p_ref_increment(cw_bufc_t * a_bufc);

static void
bufc_p_ref_decrement(cw_bufc_t * a_bufc);
