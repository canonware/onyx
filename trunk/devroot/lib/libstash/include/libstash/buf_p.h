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
static void
buf_p_rebuild_cumulative_index(cw_buf_t * a_buf);

static void
buf_p_get_data_position(cw_buf_t * a_buf,
			cw_uint32_t a_offset,
			cw_uint32_t * a_array_element,
			cw_uint32_t * a_bufel_offset);

static void
buf_p_fit_array(cw_buf_t * a_buf, cw_uint32_t a_min_array_size);

/* */
static cw_bufc_t *
bufc_new(void * a_buffer, cw_uint32_t a_size,
	 void (*a_free_func)(void * free_arg, void * buffer_p),
	 void * a_free_arg);

static void
bufc_delete(cw_bufc_t * a_bufc);

static cw_uint32_t
bufc_get_size(cw_bufc_t * a_bufc);

static char *
bufc_get_p(cw_bufc_t * a_bufc);

static void
bufc_ref_increment(cw_bufc_t * a_bufc);

static void
bufc_ref_decrement(cw_bufc_t * a_bufc);
