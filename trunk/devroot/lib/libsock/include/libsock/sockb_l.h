/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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
 * Library-private data structures and methods for sockb.
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
 * If no data has been written to the pipe that causes select() to return, write
 * a byte.  This function makes it possible to use a non-polling select loop.
 *
 ****************************************************************************/
void
sockb_l_wakeup(void);

/****************************************************************************
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
 * Register a_sock with the sockb thread.
 *
 ****************************************************************************/
void
sockb_l_register_sock(cw_sock_t * a_sock);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sockfd : Pointer to a file descriptor number.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Unregister the sock corresponding to *a_sockfd with the sockb thread.
 * *a_sockfd must not be modified until after the sockb thread has processed the
 * meessage.
 *
 ****************************************************************************/
void
sockb_l_unregister_sock(cw_uint32_t * a_sockfd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sockfd : Pointer to a file descriptor number.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Tell the sockb thread that the sock corresponding to *a_sockfd has queued
 * outgoing data.  *a_sockfd must not be modified until after the sockb thread
 * has processed the meessage.
 *
 ****************************************************************************/
void
sockb_l_out_notify(cw_uint32_t * a_sockfd);

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
cw_bool_t
sockb_l_get_host_ip(char * a_host_str, cw_uint32_t * r_host_ip);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * retval : File descriptor, or -1.
 *          -1 : No file descriptors available within the range that select()
 *               can use.
 *
 * <<< Description >>>
 *
 * Return a useless, but reserved file descriptor within the range that select()
 * can handle.  The caller is responsible for disposing of the file descriptor.
 *
 ****************************************************************************/
int
sockb_l_get_spare_fd(void);
