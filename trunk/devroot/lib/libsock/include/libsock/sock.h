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
 * Implementation of a TCP/IP socket wrapper class.  The sock class communicates
 * with the sockb class in order to implement asynchronous reads and writes
 * efficiently.
 *
 ****************************************************************************/

typedef struct cw_sock_s cw_sock_t;

struct cw_sock_s
{
  cw_bool_t is_malloced;

  cw_mtx_t lock;
  cw_cnd_t callback_cnd;
  cw_bool_t is_registered;

  cw_uint32_t os_outbuf_size;
  cw_uint16_t port;

  cw_mtx_t state_lock;
  int sockfd;
  cw_bool_t is_connected;
  cw_bool_t error;

  cw_uint32_t in_max_buf_size;
  cw_mtx_t in_lock;
  cw_buf_t in_buf;
  cw_uint32_t in_need_signal_count;
  cw_cnd_t in_cnd;

  cw_mtx_t out_lock;
  cw_buf_t out_buf;
  cw_uint32_t out_need_broadcast_count;
  cw_bool_t out_is_flushed;
  cw_cnd_t out_cnd;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to space for a sock, or NULL.
 *
 * a_in_max_buf_size : Maximum number of bytes of incoming data to buffer.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a cw_sock_t instance, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * sock constructor.
 *
 ****************************************************************************/
cw_sock_t *
sock_new(cw_sock_t * a_sock, cw_uint32_t a_in_max_buf_size);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a cw_sock_t instance.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * sock destructor.
 *
 ****************************************************************************/
void
sock_delete(cw_sock_t * a_sock);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a cw_sock_t instance.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == not connected, TRUE == connected.
 *
 * <<< Description >>>
 *
 * Returns TRUE if a_sock is connected.
 *
 ****************************************************************************/
cw_bool_t
sock_is_connected(cw_sock_t * a_sock);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a cw_sock_t instance.
 *
 * <<< Output(s) >>>
 *
 * retval : Port number on this end of the connection.
 *
 * <<< Description >>>
 *
 * Return the local port number for the socket.
 *
 ****************************************************************************/
cw_uint32_t
sock_get_port(cw_sock_t * a_sock);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a cw_sock_t instance.
 *
 * a_server_host : String that represents remote hostname or IP address.
 *
 * a_port : Remote port number to connect to.
 *
 * a_timeout : Connect timeout, or NULL for no timeout.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Already connected.
 *               : socket() error.
 *               : close() error.
 *               : sock_p_config_socket() error.
 *               : sockb_l_get_host_ip() error.
 *               : connect() error.
 *
 * <<< Description >>>
 *
 * Connect to a remote socket.
 *
 ****************************************************************************/
cw_bool_t
sock_connect(cw_sock_t * a_sock, char * a_server_host, int a_port,
	     struct timeval * a_timeout);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a cw_sock_t instance.
 *
 * a_sock_fd : File descriptor number.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : sock_p_config_socket() error.
 *
 * <<< Description >>>
 *
 * Wrap an open socket descriptor inside a sock.
 *
 ****************************************************************************/
cw_bool_t
sock_wrap(cw_sock_t * a_sock, int a_sockfd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a cw_sock_t instance.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Not connected.
 *               : fcntl() error.
 *               : close() error.
 *
 * <<< Description >>>
 *
 * Disconnect a_sock.
 *
 ****************************************************************************/
cw_bool_t
sock_disconnect(cw_sock_t * a_sock);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a cw_sock_t instance.
 *
 * a_spare : Pointer to a spare cw_buf_t, to which data may be appended.
 *
 * a_max_read : Maximum number of bytes to read into a_spare, or 0 for no limit.
 *
 * <<< Output(s) >>>
 *
 * retval : -1 : Socket error.
 *        : 0 : No data read.
 *        : > 0 : Data read.
 *
 * <<< Description >>>
 *
 * Read data from the socket if any is there.  Do not block, regardless of
 * whether data is available.
 *
 ****************************************************************************/
cw_sint32_t
sock_read_noblock(cw_sock_t * a_sock, cw_buf_t * a_spare,
		  cw_sint32_t a_max_read);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a cw_sock_t instance.
 *
 * a_spare : Pointer to a spare cw_buf_t, to which data may be appended.
 *
 * a_max_read : Maximum number of bytes to read into a_spare, or 0 for no limit.
 *
 * a_timeout : Maximum time to wait for data before returning, or NULL to wait
 *             indefinitely.
 *
 * <<< Output(s) >>>
 *
 * retval : -1 : Error.  This should only happen if the socket has been
 *               closed.
 *        : 0 : Timeout.
 *        : > 0 : Data read.
 *
 * <<< Description >>>
 *
 * Read data from the socket, and don't return until there is data (not
 * necessarily a_max_read), or the timeout expires.
 *
 ****************************************************************************/
cw_sint32_t
sock_read_block(cw_sock_t * a_sock, cw_buf_t * a_spare,	cw_sint32_t a_max_read,
		struct timespec * a_timeout);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a cw_sock_t instance.
 *
 * a_buf : Pointer to a cw_buf_t instance.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *
 * <<< Description >>>
 *
 * Queue the data in a_buf for writing.  Once the data has been added to the
 * write queue, notify g_sockb.
 *
 ****************************************************************************/
cw_bool_t
sock_write(cw_sock_t * a_sock, cw_buf_t * a_buf);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_sock : Pointer to a cw_sock_t instance.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *
 * <<< Description >>>
 *
 * Flush the outgoing data queue (actually, just wait until it has all been
 * sent), and don't return until done.
 *
 ****************************************************************************/
cw_bool_t
sock_flush_out(cw_sock_t * a_sock);
