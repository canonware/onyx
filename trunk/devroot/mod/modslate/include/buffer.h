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

/* Predicates. */

cw_nxn_t
buffer_type(cw_nxo_t *a_nxo);

cw_nxn_t
marker_type(cw_nxo_t *a_nxo);

/* Hooks. */

/* buffer. */
void
modslate_buffer(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_aux(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_setaux(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_seq(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_length(void *a_data, cw_nxo_t *a_thread);

void
modslate_buffer_lines(void *a_data, cw_nxo_t *a_thread);

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

/* marker. */
void
modslate_marker(void *a_data, cw_nxo_t *a_thread);

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
