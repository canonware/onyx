/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
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
 * Library-private data structures and methods for sockb.
 *
 ****************************************************************************/

#define sockb_l_register_sock _CW_NS_LIBSOCK(sockb_l_register_sock)
void
sockb_l_register_sock(cw_sock_t * a_sock);

#define sockb_l_unregister_sock _CW_NS_LIBSOCK(sockb_l_unregister_sock)
void
sockb_l_unregister_sock(cw_uint32_t * a_sockfd);

#define sockb_l_out_notify _CW_NS_LIBSOCK(sockb_l_out_notify)
void
sockb_l_out_notify(cw_uint32_t * a_sockfd);

#define sockb_l_get_host_ip _CW_NS_LIBSOCK(sockb_l_get_host_ip)
cw_bool_t
sockb_l_get_host_ip(char * a_host_str, cw_uint32_t * a_host_ip);

#define sockb_p_get_spare_fd _CW_NS_LIBSOCK(sockb_p_get_spare_fd)
int
sockb_l_get_spare_fd(void);
