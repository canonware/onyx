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
/* XXX Set to something larger than 2 after debugged. */
#define _LIBSTASH_BUF_ARRAY_MIN_SIZE 2

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

static cw_bool_t
bufel_p_merge_bufel(cw_bufel_t * a_a, cw_bufel_t * a_b);

static cw_bufc_t *
bufc_new(void * a_buffer, cw_uint32_t a_size,
	 void (*a_dealloc_func)(void * dealloc_arg, void * buffer_p),
	 void * a_dealloc_arg);

static void
bufc_delete(cw_bufc_t * a_bufc);

static void
bufc_dump(cw_bufc_t * a_bufc, const char * a_prefix);

static cw_uint32_t
bufc_get_size(cw_bufc_t * a_bufc);

static char *
bufc_get_p(cw_bufc_t * a_bufc);

static void
bufc_ref_increment(cw_bufc_t * a_bufc);

static void
bufc_ref_decrement(cw_bufc_t * a_bufc);
