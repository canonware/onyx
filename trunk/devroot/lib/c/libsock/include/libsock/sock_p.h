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
 * Private interface of the sock class.
 *
 ****************************************************************************/

#ifdef _LIBSOCK_DBG
#  define _LIBSOCK_SOCK_MAGIC 0x12348765
#endif

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
 *
 * a_init : FALSE == use a_sockfd as is, TRUE == initialize a_sockfd.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : [gs]et_sockopt() error.
 *               : fcntl() error.
 *
 * <<< Description >>>
 *
 * Set socket options for a_sock.
 *
 ****************************************************************************/
static cw_bool_t
sock_p_config_socket(cw_sock_t * a_sock, cw_bool_t a_init);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : fcntl() error.
 *               : close() error.
 *               : a_sock is not open.
 *
 * <<< Description >>>
 *
 * Disconnect a_sock.
 *
 ****************************************************************************/
static cw_bool_t
sock_p_disconnect(cw_sock_t * a_sock);
