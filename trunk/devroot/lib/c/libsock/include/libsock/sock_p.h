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

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a sock.
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
sock_p_config_socket(cw_sock_t * a_sock);

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
