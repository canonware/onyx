/* -*-mode:c-*-
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
 *
 *
 ****************************************************************************/

#ifndef _SOCK_H_
#define _SOCK_H_

typedef struct cw_sock_s cw_sock_t;

struct cw_sock_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_buf_t in_buf;
  cw_buf_t out_buf;
  cw_bool_t is_connected;
  int sockfd;
};

#define sock_new _CW_NS_ANY(sock_new)
#define sock_delete _CW_NS_ANY(sock_delete)
#define sock_get_fd _CW_NS_ANY(sock_get_fd)
#define sock_is_connected _CW_NS_ANY(sock_is_connected)
#define sock_connect _CW_NS_ANY(sock_connect)
#define sock_wrap _CW_NS_ANY(sock_wrap)
#define sock_disconnect _CW_NS_ANY(sock_disconnect)
#define sock_wait_for_in_data _CW_NS_ANY(sock_wait_for_in_data)
#define sock_get_max_read _CW_NS_ANY(sock_get_max_read)
#define sock_read_noblock _CW_NS_ANY(sock_read_noblock)
#define sock_read_block _CW_NS_ANY(sock_read_block)
#define sock_write _CW_NS_ANY(sock_write)
#define sock_flush_out _CW_NS_ANY(sock_flush_out)

cw_sock_t * sock_new(cw_sock_t * a_sock_o);
void sock_delete(cw_sock_t * a_sock_o);

int sock_get_fd(cw_sock_t * a_sock_o);
cw_bool_t sock_is_connected(cw_sock_t * a_sock_o);

cw_bool_t sock_connect(cw_sock_t * a_sock_o, char * a_server_host,
		       int a_port);
cw_bool_t sock_wrap(cw_sock_t * a_sock_o, int a_sockfd);
cw_bool_t sock_disconnect(cw_sock_t * a_sock_o);

cw_bool_t sock_wait_for_in_data(cw_sock_t * a_sock_o);
cw_uint32_t sock_get_max_read(cw_sock_t * a_sock_o);
cw_buf_t * sock_read_noblock(cw_sock_t * a_sock_o, cw_buf_t * a_spare);
cw_buf_t * sock_read_block(cw_sock_t * a_sock_o, cw_buf_t * a_spare);
cw_bool_t sock_write(cw_sock_t * a_sock_o, cw_buf_t * a_buf_o);
cw_bool_t sock_flush_out(cw_sock_t * a_sock_o);

#endif /* _SOCK_H_ */
