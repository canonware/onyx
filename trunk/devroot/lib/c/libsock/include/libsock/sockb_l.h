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

void
sockb_l_register_sock(cw_sock_t * a_sock);

void
sockb_l_unregister_sock(cw_uint32_t * a_sockfd);

void
sockb_l_out_notify(cw_uint32_t * a_sockfd);

cw_bool_t
sockb_l_get_host_ip(char * a_host_str, cw_uint32_t * a_host_ip);

int
sockb_l_get_spare_fd(void);
