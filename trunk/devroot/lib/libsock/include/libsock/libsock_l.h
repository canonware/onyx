/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Library-private data structures and methods for libsock.
 *
 ******************************************************************************/

/* Message type. */
typedef enum {
	LIBSOCK_MSG_NONE,
	LIBSOCK_MSG_REGISTER,
	LIBSOCK_MSG_UNREGISTER,
	LIBSOCK_MSG_OUT_NOTIFY,
	LIBSOCK_MSG_IN_SPACE,
	LIBSOCK_MSG_IN_NOTIFY
} cw_libsock_msg_type_t;

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * If no data has been written to the pipe that causes poll() to return, write
 * a byte.  This function makes it possible to use a non-polling poll loop.
 *
 ******************************************************************************/
void		libsock_l_wakeup(void);

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
 *
 * a_msg : Type of message:
 *         LIBSOCK_MSG_REGISTER   : Register a_sock with the I/O thread.
 *         LIBSOCK_MSG_UNREGISTER : Unregister the sock corresponding to
 *                                  a_sockfd with the I/O thread.
 *         LIBSOCK_MSG_OUT_NOTIFY : Tell the I/O thread that the sock
 *                                  corresponding to a_sockfd has queued
 *                                  outgoing data.
 *         LIBSOCK_MSG_IN_SPACE   : Tell the I/O thread that the sock
 *                                  corresponding to a_sockfd has space
 *                                  available for incoming data.
 *         LIBSOCK_MSG_IN_NOTIFY  : Not used with this function.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Exception(s) >>>
 *
 * _CW_STASHX_OOM.
 *
 * <<< Description >>>
 *
 * Send a message to the I/O thread.
 *
 ******************************************************************************/
void		libsock_l_message(cw_sock_t *a_sock, cw_libsock_msg_type_t
    a_msg);

/******************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_host_str : Pointer to a string that represents an IP address or hostname.
 *
 * r_host_ip : Pointer to space for an IP address.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : No match for *a_host_str.
 *
 * *r_host_ip : IP address corresponding to *a_host_str, or invalid if retval ==
 *              TRUE.
 *
 * <<< Description >>>
 *
 * Convert a_host_str to an IP address and put it in *a_host_ip.  Return TRUE if
 * there is an error.
 *
 * This function is necessary because gethostbyname() is not reentrant.  Of
 * course, there may be gethostbyname_r() on the system, but even if there is,
 * the resolver functions and BIND probably are not reentrant, according to
 * Stevens's UNP, 2nd Ed., Volume 1, page 305.  That being the case, this code
 * doesn't even bother with gethostbyname_r().
 *
 ******************************************************************************/
cw_bool_t	libsock_l_host_ip_get(const char *a_host_str, cw_uint32_t
    *r_host_ip);
