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

/* Minimum size for buf's internal bufel array. */
#ifdef _LIBSTASH_DBG
/* Make it small to increase the likelihood of hitting bugs in the debug
 * library. */
#  define _LIBSTASH_BUF_ARRAY_MIN_SIZE 2
#else
#  define _LIBSTASH_BUF_ARRAY_MIN_SIZE 64
#endif

static void
buf_p_rebuild_cumulative_index(cw_buf_t * a_buf);

static void
buf_p_get_data_position(cw_buf_t * a_buf,
			cw_uint32_t a_offset,
			cw_uint32_t * a_array_element,
			cw_uint32_t * a_bufel_offset);

static void
buf_p_fit_array(cw_buf_t * a_buf, cw_uint32_t a_min_array_size);

static void
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
bufel_p_merge_bufel(cw_bufel_t * a_a, cw_bufel_t * a_b);

static void
bufc_p_dump(cw_bufc_t * a_bufc, const char * a_prefix);

static cw_uint32_t
bufc_p_get_size(cw_bufc_t * a_bufc);

static char *
bufc_p_get_p(cw_bufc_t * a_bufc);

static cw_uint32_t
bufc_p_get_ref_count(cw_bufc_t * a_bufc);

static void
bufc_p_ref_increment(cw_bufc_t * a_bufc);

static void
bufc_p_ref_decrement(cw_bufc_t * a_bufc);
