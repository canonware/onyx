/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Slate <Version = slate>
 *
 ******************************************************************************/

/* modslate initialization. */
void
modslate_buffer_init(cw_nxo_t *a_thread);

/* Handles. */

/* buffer. */
void
modslate_buffer(void *a_data, cw_nxo_t *a_thread);

//void
//modslate_buffer_p(void *a_data, cw_nxo_t *a_thread);

//void
//modslate_buffer_aux_get(void *a_data, cw_nxo_t *a_thread);

//void
//modslate_buffer_aux_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_seq(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_length(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_lines(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_extents(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_undoable(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_redoable(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_undo(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_redo(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_history_active(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_history_setactive(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_history_startgroup(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_history_endgroup(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_history_flush(void *a_data, cw_nxo_t *a_thread);

#ifdef CW_BUF_DUMP
void
modslate_buffer_dump(void *a_data, cw_nxo_t *a_thread);
#endif

#ifdef CW_BUF_VALIDATE
void
modslate_buffer_validate(void *a_data, cw_nxo_t *a_thread);
#endif

/* marker. */
void
modslate_marker(void *a_data, cw_nxo_t *a_thread);

//void
//modslate_marker_p(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_seq(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_copy(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_buffer(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_line(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_seekline(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_position(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_seek(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_before_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_after_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_before_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_after_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_before_insert(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_after_insert(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_range_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_marker_range_cut(void *a_data, cw_nxo_t *a_thread);

#ifdef CW_BUF_DUMP
void
modslate_marker_dump(void *a_data, cw_nxo_t *a_thread);
#endif

#ifdef CW_BUF_VALIDATE
void
modslate_marker_validate(void *a_data, cw_nxo_t *a_thread);
#endif

/* extent. */
void
modslate_extent(void *a_data, cw_nxo_t *a_thread);

//void
//modslate_extent_p(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_copy(void *a_data, cw_nxo_t *a_thread);

//void
//modslate_extent_aux_get(void *a_data, cw_nxo_t *a_thread);

//void
//modslate_extent_aux_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_buffer(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_beg_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_beg_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_end_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_end_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_beg_open_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_beg_open_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_end_open_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_end_open_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_before_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_at_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_after_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_prev_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_next_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_detachable_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_detachable_set(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_detached_get(void *a_data, cw_nxo_t *a_thread);

void
modslate_extent_detach(void *a_data, cw_nxo_t *a_thread);

#ifdef CW_BUF_DUMP
void
modslate_extent_dump(void *a_data, cw_nxo_t *a_thread);
#endif

#ifdef CW_BUF_VALIDATE
void
modslate_extent_validate(void *a_data, cw_nxo_t *a_thread);
#endif
