/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

typedef struct cw_sock_s cw_sock_t;

struct cw_sock_s {
#if (defined(_LIBSOCK_DBG) || defined(_LIBSOCK_DEBUG))
	cw_uint32_t	magic;
#endif
	cw_bool_t	is_malloced;

	cw_uint32_t	os_inbuf_size;
	cw_uint32_t	os_outbuf_size;
	cw_uint16_t	port;

	cw_mtx_t	state_lock;
	int		sockfd;
	cw_bool_t	is_connected;
	cw_bool_t	in_progress;
	cw_bool_t	error;

	cw_mtx_t	lock;
	cw_cnd_t	callback_cnd;
	cw_bool_t	is_registered;

	cw_uint32_t	in_max_buf_size;
	cw_mtx_t	in_lock;
	cw_buf_t	in_buf;
	cw_uint32_t	in_need_signal_count;
	cw_cnd_t	in_cnd;

	cw_mtx_t	out_lock;
	cw_bool_t	io_in_progress;
	cw_buf_t	out_buf;
	cw_uint32_t	out_need_broadcast_count;
	cw_bool_t	out_is_flushed;
	cw_cnd_t	out_cnd;
};

cw_sock_t	*sock_new(cw_sock_t *a_sock, cw_uint32_t a_in_max_buf_size);
void		sock_delete(cw_sock_t *a_sock);
cw_bool_t	sock_is_connected(cw_sock_t *a_sock);
cw_uint32_t	sock_get_port(cw_sock_t *a_sock);
cw_sint32_t	sock_connect(cw_sock_t *a_sock, const char *a_server_host, int
    a_port, struct timespec *a_timeout);
cw_bool_t	sock_wrap(cw_sock_t *a_sock, int a_sockfd, cw_bool_t a_init);
cw_bool_t	sock_disconnect(cw_sock_t *a_sock);
cw_uint32_t	sock_buffered_in(cw_sock_t *a_sock);
cw_sint32_t	sock_read(cw_sock_t *a_sock, cw_buf_t *a_spare, cw_sint32_t
    a_max_read, struct timespec *a_timeout);
cw_bool_t	sock_write(cw_sock_t *a_sock, cw_buf_t *a_buf);
cw_bool_t	sock_flush_out(cw_sock_t *a_sock);
int		sock_get_fd(cw_sock_t *a_sock);
