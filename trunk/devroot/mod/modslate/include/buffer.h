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

/* slate initialization. */
void	slate_buffer_init(cw_nxo_t *a_thread);

/* Operators. */
void	slate_buffer(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_aux(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_setaux(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_seq(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_length(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_lines(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_undoable(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_redoable(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_undo(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_redo(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_history_active(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_history_setactive(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_history_startgroup(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_history_endgroup(void *a_data, cw_nxo_t *a_thread);
void	slate_buffer_history_flush(void *a_data, cw_nxo_t *a_thread);

void	slate_marker(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_seq(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_copy(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_buffer(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_line(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_seekline(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_position(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_seek(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_before_get(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_after_get(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_before_set(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_after_set(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_before_insert(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_after_insert(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_range_get(void *a_data, cw_nxo_t *a_thread);
void	slate_marker_range_cut(void *a_data, cw_nxo_t *a_thread);
