/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Library-private data structures and methods for libsock.
 *
 ****************************************************************************/

/****************************************************************************
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
 ****************************************************************************/
void		libsock_l_wakeup(void);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Register a_sock with the I/O thread.
 *
 ****************************************************************************/
cw_bool_t	libsock_l_register_sock(cw_sock_t *a_sock);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sockfd : A file descriptor number.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Unregister the sock corresponding to a_sockfd with the I/O thread.
 *
 ****************************************************************************/
cw_bool_t	libsock_l_unregister_sock(cw_uint32_t a_sockfd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sockfd : A file descriptor number.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Tell the I/O thread that the sock corresponding to a_sockfd has queued
 * outgoing data.
 *
 ****************************************************************************/
cw_bool_t	libsock_l_out_notify(cw_uint32_t a_sockfd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sockfd : A file descriptor number.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Tell the I/O thread that the sock corresponding to a_sockfd has space
 * available for incoming data.
 *
 ****************************************************************************/
cw_bool_t	libsock_l_in_space(cw_uint32_t a_sockfd);

/****************************************************************************
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
 ****************************************************************************/
cw_bool_t	libsock_l_get_host_ip(const char *a_host_str, cw_uint32_t
    *r_host_ip);
