/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 158 $
 * $Date: 1998-07-29 17:57:57 -0700 (Wed, 29 Jul 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define _INC_SOCKS_H_
#include <libstash.h>

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_socks_t *
socks_new(cw_socks_t * a_socks_o)
{
  cw_socks_t * retval;
  
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
socks_delete(cw_socks_t * a_socks_o)
{
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
socks_listen(cw_socks_t * a_socks_o, int a_port)
{
  cw_bool_t retval;

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_sock_t *
socks_accept_block(cw_socks_t * a_socks_o, cw_sock_t * a_sock_o)
{
  cw_sock_t * retval;

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_sock_t *
socks_accept_noblock(cw_socks_t * a_socks_o, cw_sock_t * a_sock_o)
{
  cw_sock_t * retval;

  return retval;
}
