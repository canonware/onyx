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
 * Public interface for the socks (socket server) class.  socks is used to
 * accept connections to a port and create a sock instance for each connection.
 *
 ****************************************************************************/

/* Opaque type. */
typedef struct cw_socks_s cw_socks_t;

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a socks, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_socks_t *
socks_new(void);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_socks : Pointer to a socks.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
socks_delete(cw_socks_t * a_socks);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_socks : Pointer to a socks.
 *
 * a_mask : Mask of client addresses to listen to (INADDR_ANY, INADDR_LOOPBACK,
 *          etc.).
 *
 * r_port : Port number to listen on, or 0.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : socket() error.
 *               : bind() error.
 *               : getsockname() error.
 *
 * *r_port : Port number that a_socks is listening on, or invalid if retval ==
 *           TRUE.
 *
 * <<< Description >>>
 *
 * Do setup and start accepting connections on *r_port.  If *r_port is 0, let
 * the OS choose what port number to use, and assign the number to *r_port
 * before returning.
 *
 ****************************************************************************/
cw_bool_t
socks_listen(cw_socks_t * a_socks, cw_uint32_t a_mask, int * r_port);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_socks : Pointer to a socks.
 *
 * a_timeout : Pointer to a timeout value, specified as an absolute time
 *             interval, or NULL.  A NULL value will cause this function to
 *             block indefinitely.
 *
 * r_sock : Pointer to a sock that is not connected.
 *
 * <<< Output(s) >>>
 *
 * retval : r_sock, or NULL.
 *          NULL : Not listening.
 *               : Cannot allocate a file descriptor.
 *               : accept() error.
 *               : sock_wrap() error.
 *
 * <<< Description >>>
 *
 * Accept a connection.  Don't return until someone connects, or the timeout
 * expires.
 *
 ****************************************************************************/
cw_sock_t *
socks_accept(cw_socks_t * a_socks, struct timespec * a_timeout,
	     cw_sock_t * r_sock);
