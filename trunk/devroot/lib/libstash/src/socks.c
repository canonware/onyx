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
 * $Revision: 173 $
 * $Date: 1998-08-26 12:34:42 -0700 (Wed, 26 Aug 1998) $
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
  
  if (_cw_pmatch(_STASH_DBG_R_SOCKS_FUNC))
  {
    _cw_marker("Enter socks_new()");
  }
  if (_cw_pmatch(_STASH_DBG_R_SOCKS_FUNC))
  {
    _cw_marker("Exit socks_new()");
  }
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
  if (_cw_pmatch(_STASH_DBG_R_SOCKS_FUNC))
  {
    _cw_marker("Enter socks_delete()");
  }
  if (_cw_pmatch(_STASH_DBG_R_SOCKS_FUNC))
  {
    _cw_marker("Exit socks_delete()");
  }
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

  if (_cw_pmatch(_STASH_DBG_R_SOCKS_FUNC))
  {
    _cw_marker("Enter socks_listen()");
  }
  if (_cw_pmatch(_STASH_DBG_R_SOCKS_FUNC))
  {
    _cw_marker("Exit socks_listen()");
  }
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

  if (_cw_pmatch(_STASH_DBG_R_SOCKS_FUNC))
  {
    _cw_marker("Enter socks_accept_block()");
  }
  if (_cw_pmatch(_STASH_DBG_R_SOCKS_FUNC))
  {
    _cw_marker("Exit socks_accept_block()");
  }
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

  if (_cw_pmatch(_STASH_DBG_R_SOCKS_FUNC))
  {
    _cw_marker("Enter socks_accept_noblock()");
  }
  if (_cw_pmatch(_STASH_DBG_R_SOCKS_FUNC))
  {
    _cw_marker("Exit socks_accept_noblock()");
  }
  return retval;
}
