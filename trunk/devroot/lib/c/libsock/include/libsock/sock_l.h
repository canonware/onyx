/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Library-private interface of the sock class.
 *
 ******************************************************************************/

/*******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
 *
 * <<< Output(s) >>>
 *
 * retval : Minimum number of bytes available buffer space.
 *
 * <<< Description >>>
 *
 * Return the minimum incoming buffer space available.
 *
 ******************************************************************************/
cw_uint32_t	sock_l_in_space_get(cw_sock_t *a_sock);

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock instance.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of bytes of incoming data buffered.
 *
 * <<< Description >>>
 *
 * Return the number of bytes of incoming data buffered for a_sock.
 *
 ******************************************************************************/
cw_uint32_t	sock_l_in_size_get(cw_sock_t *a_sock);

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock instance.
 *
 * <<< Output(s) >>>
 *
 * retval : Maximum number of bytes of data to buffer for this sock.
 *
 * <<< Description >>>
 *
 * Return the maximum number of bytes of data to buffer for a_sock.
 *
 ******************************************************************************/
cw_uint32_t	sock_l_in_max_buf_size_get(cw_sock_t *a_sock);

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
 *
 * r_buf : Pointer to a buf.
 *
 * <<< Output(s) >>>
 *
 * *r_buf : Outgoing data are appended to a_buf.
 *
 * <<< Description >>>
 *
 * Get the data that is buffered in out_buf.  If there is no data buffered, then
 * signal flush_cond.
 *
 * Note that this assumes the I/O thread will write all data that it has before
 * asking for more.  If for some reason this needs to change, then a more
 * sophisticated message passing scheme will be necessary for flushing the
 * output buffer.
 *
 ******************************************************************************/
void		sock_l_out_data_get(cw_sock_t *a_sock, cw_buf_t *r_buf);

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
 *
 * a_buf : Pointer to a buf.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of buffered outgoing data bytes.
 *
 * <<< Description >>>
 *
 * Push data back into out_buf.
 *
 ******************************************************************************/
cw_uint32_t	sock_l_out_data_put_back(cw_sock_t *a_sock, cw_buf_t *a_buf);

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
 *
 * a_buf : Pointer to a buf.
 *
 * <<< Output(s) >>>
 *
 * retval : Remaining available buffer space.
 *
 * <<< Description >>>
 *
 * Append data to the incoming data buffer.
 *
 ******************************************************************************/
cw_uint32_t	sock_l_in_data_put(cw_sock_t *a_sock, cw_buf_t *a_buf);

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
 *
 * a_error : FALSE == success, TRUE == error.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * The I/O thread calls this function to notify the sock of the result of a
 * message.
 *
 ******************************************************************************/
void		sock_l_message_callback(cw_sock_t *a_sock, cw_bool_t a_error);

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * The I/O thread calls this to notify a_sock that there was an error.
 *
 ******************************************************************************/
void		sock_l_error_callback(cw_sock_t *a_sock);
