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
 * Library-private interface of the sock class.
 *
 ****************************************************************************/

#define sock_l_get_fd _CW_NS_LIBSOCK(sock_l_get_fd)
int
sock_l_get_fd(cw_sock_t * a_sock);

/****************************************************************************
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
 ****************************************************************************/
#define sock_l_get_in_size _CW_NS_LIBSOCK(sock_l_in_size)
cw_uint32_t
sock_l_get_in_size(cw_sock_t * a_sock);

/****************************************************************************
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
 * Return the number of maximum number of bytes of data to buffer for a_sock.
 *
 ****************************************************************************/
#define sock_l_get_in_max_buf_size _CW_NS_LIBSOCK(sock_l_get_in_max_buf_size)
cw_uint32_t
sock_l_get_in_max_buf_size(cw_sock_t * a_sock);

#define sock_l_get_out_data _CW_NS_LIBSOCK(sock_l_get_out_data)
void
sock_l_get_out_data(cw_sock_t * a_sock, cw_buf_t * a_buf);

#define sock_l_put_back_out_data _CW_NS_LIBSOCK(sock_l_put_back_out_data)
cw_uint32_t
sock_l_put_back_out_data(cw_sock_t * a_sock, cw_buf_t * a_buf);

#define sock_l_put_in_data _CW_NS_LIBSOCK(sock_l_put_in_data)
void
sock_l_put_in_data(cw_sock_t * a_sock, cw_buf_t * a_buf);

#define sock_l_message_callback _CW_NS_LIBSOCK(sock_l_message_callback)
void
sock_l_message_callback(cw_sock_t * a_sock);

#define sock_l_error_callback _CW_NS_LIBSOCK(sock_l_error_callback)
void
sock_l_error_callback(cw_sock_t * a_sock);
