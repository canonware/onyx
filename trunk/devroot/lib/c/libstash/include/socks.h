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

#ifndef _SOCKS_H_
#define _SOCKS_H_

typedef struct cw_socks_s cw_socks_t;

struct cw_socks_s
{
  cw_bool_t is_malloced;
  cw_bool_t is_listenting;
  cw_uint32_t sockaddrs_struct_size; /* XXX Do we really need this? */
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
};

#define socks_new _CW_NS_ANY(socks_new)
#define socks_delete _CW_NS_ANY(socks_delete)
#define socks_listen _CW_NS_ANY(socks_listen)
#define socks_accept_block _CW_NS_ANY(socks_accept_block)
#define socks_accept_noblock _CW_NS_ANY(socks_accept_noblock)

cw_socks_t * socks_new(cw_socks_t * a_socks_o);
void socks_delete(cw_socks_t * a_socks_o);

cw_bool_t socks_listen(cw_socks_t * a_socks_o, int a_port);

cw_sock_t * socks_accept_block(cw_socks_t * a_socks_o, cw_sock_t * a_sock_o);
cw_sock_t * socks_accept_noblock(cw_socks_t * a_socks_o, cw_sock_t * a_sock_o);

#endif /* _SOCKS_H_ */
