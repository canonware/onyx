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
cw_nxn_t buffer_type(cw_nxo_t *a_nxo);
cw_nxn_t marker_type(cw_nxo_t *a_nxo);

/* Operators. */
void	slate_buffer(cw_nxo_t *a_thread);
void	slate_buffer_aux(cw_nxo_t *a_thread);
void	slate_buffer_setaux(cw_nxo_t *a_thread);
void	slate_buffer_seq(cw_nxo_t *a_thread);
void	slate_buffer_length(cw_nxo_t *a_thread);
void	slate_buffer_lines(cw_nxo_t *a_thread);
void	slate_buffer_undoable(cw_nxo_t *a_thread);
void	slate_buffer_redoable(cw_nxo_t *a_thread);
void	slate_buffer_undo(cw_nxo_t *a_thread);
void	slate_buffer_redo(cw_nxo_t *a_thread);
void	slate_buffer_history_active(cw_nxo_t *a_thread);
void	slate_buffer_history_setactive(cw_nxo_t *a_thread);
void	slate_buffer_history_startgroup(cw_nxo_t *a_thread);
void	slate_buffer_history_endgroup(cw_nxo_t *a_thread);
void	slate_buffer_history_flush(cw_nxo_t *a_thread);

void	slate_marker(cw_nxo_t *a_thread);
void	slate_marker_seq(cw_nxo_t *a_thread);
void	slate_marker_copy(cw_nxo_t *a_thread);
void	slate_marker_buffer(cw_nxo_t *a_thread);
void	slate_marker_line(cw_nxo_t *a_thread);
void	slate_marker_seekline(cw_nxo_t *a_thread);
void	slate_marker_position(cw_nxo_t *a_thread);
void	slate_marker_seek(cw_nxo_t *a_thread);
void	slate_marker_before_get(cw_nxo_t *a_thread);
void	slate_marker_after_get(cw_nxo_t *a_thread);
void	slate_marker_before_set(cw_nxo_t *a_thread);
void	slate_marker_after_set(cw_nxo_t *a_thread);
void	slate_marker_before_insert(cw_nxo_t *a_thread);
void	slate_marker_after_insert(cw_nxo_t *a_thread);
void	slate_marker_range_get(cw_nxo_t *a_thread);
void	slate_marker_range_cut(cw_nxo_t *a_thread);
