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

/* nxe initialization. */
void	nxe_buffer_init(cw_nxo_t *a_thread);

/* Operators. */
void	nxe_buffer(cw_nxo_t *a_thread);
void	nxe_buffer_length(cw_nxo_t *a_thread);
void	nxe_buffer_lines(cw_nxo_t *a_thread);
void	nxe_buffer_undo(cw_nxo_t *a_thread);
void	nxe_buffer_redo(cw_nxo_t *a_thread);
void	nxe_buffer_history_active(cw_nxo_t *a_thread);
void	nxe_buffer_history_setactive(cw_nxo_t *a_thread);
void	nxe_buffer_history_startgroup(cw_nxo_t *a_thread);
void	nxe_buffer_history_endgroup(cw_nxo_t *a_thread);
void	nxe_buffer_history_flush(cw_nxo_t *a_thread);

void	nxe_marker(cw_nxo_t *a_thread);
void	nxe_marker_copy(cw_nxo_t *a_thread);
void	nxe_marker_buffer(cw_nxo_t *a_thread);
void	nxe_marker_line(cw_nxo_t *a_thread);
void	nxe_marker_seekline(cw_nxo_t *a_thread);
void	nxe_marker_position(cw_nxo_t *a_thread);
void	nxe_marker_seek(cw_nxo_t *a_thread);
void	nxe_marker_before_get(cw_nxo_t *a_thread);
void	nxe_marker_after_get(cw_nxo_t *a_thread);
void	nxe_marker_before_set(cw_nxo_t *a_thread);
void	nxe_marker_after_set(cw_nxo_t *a_thread);
void	nxe_marker_before_insert(cw_nxo_t *a_thread);
void	nxe_marker_after_insert(cw_nxo_t *a_thread);
void	nxe_marker_range_get(cw_nxo_t *a_thread);
void	nxe_marker_range_cut(cw_nxo_t *a_thread);
