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
 *
 *
 ****************************************************************************/

/* Opaque type. */
typedef struct cw_socks_s cw_socks_t;

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 *
 *
 * <<< Output(s) >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
#define socks_new _CW_NS_LIBSOCK(socks_new)
cw_socks_t *
socks_new();

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 *
 *
 * <<< Output(s) >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
#define socks_delete _CW_NS_LIBSOCK(socks_delete)
void
socks_delete(cw_socks_t * a_socks);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 *
 *
 * <<< Output(s) >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
#define socks_listen _CW_NS_LIBSOCK(socks_listen)
cw_bool_t
socks_listen(cw_socks_t * a_socks, int * a_port);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 *
 *
 * <<< Output(s) >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
#define socks_accept_block _CW_NS_LIBSOCK(socks_accept_block)
cw_sock_t *
socks_accept_block(cw_socks_t * a_socks, cw_sock_t * a_sock);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 *
 *
 * <<< Output(s) >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
#define socks_accept_noblock _CW_NS_LIBSOCK(socks_accept_noblock)
cw_sock_t *
socks_accept_noblock(cw_socks_t * a_socks, cw_sock_t * a_sock);
