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
 * $Revision: 154 $
 * $Date: 1998-07-29 16:56:41 -0700 (Wed, 29 Jul 1998) $
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

#define _INC_SOCK_H_
#include <libstash.h>

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_sock_t *
sock_new(cw_sock_t * a_sock_o)
{
  cw_sock_t * retval;
  
  if (a_sock_o == NULL)
  {
    retval = (cw_sock_t *) _cw_malloc(sizeof(cw_sock_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_sock_o;
    retval->is_malloced = FALSE;
  }

  mtx_new(&retval->lock);
  retval->is_connected = FALSE;
  
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
sock_delete(cw_sock_t * a_sock_o)
{
  _cw_check_ptr(a_sock_o);

  /* Close the fd if open. */
  if (a_sock_o->is_connected == TRUE)
  {
    close(a_sock_o->sockfd);
  }

  mtx_delete(&a_sock_o->lock);
  
  if (a_sock_o->is_malloced == TRUE)
  {
    _cw_free(a_sock_o);
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
int
sock_get_fd(cw_sock_t * a_sock_o)
{
  int retval;
  
  _cw_check_ptr(a_sock_o);
  mtx_lock(&a_sock_o->lock);

  if (a_sock_o->is_connected)
  {
    retval = a_sock_o->sockfd;
  }
  else
  {
    retval = -1;
  }
  
  mtx_unlock(&a_sock_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
sock_is_connected(cw_sock_t * a_sock_o)
{
  _cw_check_ptr(a_sock_o);

  return a_sock_o->is_connected;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
sock_connect(cw_sock_t * a_sock_o, char * a_server_host,
	     int a_port)
{
  cw_bool_t retval;
  struct sockaddr_in server_addr;
  
  _cw_check_ptr(a_sock_o);
  _cw_check_ptr(a_server_host);
  mtx_lock(&a_sock_o->lock);

  a_sock_o->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (a_sock_o->sockfd < 0)
  {
    /* Error. */
    retval = TRUE;
    goto RETURN;
  }

  bzero(&server_addr, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  (server_addr.sin_addr).s_addr = inet_addr(a_server_host);
  server_addr.sin_port = htons(a_port);

  if (connect(a_sock_o->sockfd,
	      (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in))
      < 0)
  {
    retval = TRUE;
    goto RETURN;
  }
  else
  {
    retval = FALSE;
  }
  
 RETURN:
  mtx_unlock(&a_sock_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
sock_wrap(cw_sock_t * a_sock_o, int a_sockfd)
{
  cw_bool_t retval;
  int opt_val, opt_len;
  
  _cw_check_ptr(a_sock_o);
  mtx_lock(&a_sock_o->lock);

  opt_len = sizeof(int);
  if (getsockopt(a_sockfd, SOL_SOCKET, SO_RCVBUF, (void *) &opt_val, &opt_len)
      != 0)
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    a_sock_o->sockfd = a_sockfd;
    a_sock_o->is_connected = TRUE;
  }
  
  mtx_unlock(&a_sock_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
sock_disconnect(cw_sock_t * a_sock_o)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sock_o);
  mtx_lock(&a_sock_o->lock);

  if (a_sock_o->is_connected == TRUE)
  {
    retval = FALSE;
    close(a_sock_o->sockfd);
    a_sock_o->is_connected = FALSE;
  }
  else
  {
    retval = TRUE;
  }

  mtx_unlock(&a_sock_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
sock_wait_for_in_data(cw_sock_t * a_sock_o)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sock_o);
  mtx_lock(&a_sock_o->lock);

  mtx_unlock(&a_sock_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_uint32_t
sock_get_max_read(cw_sock_t * a_sock_o)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_sock_o);
  mtx_lock(&a_sock_o->lock);

  mtx_unlock(&a_sock_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_buf_t *
sock_read_noblock(cw_sock_t * a_sock_o, cw_buf_t * a_spare)
{
  cw_buf_t * retval;
  
  _cw_check_ptr(a_sock_o);
  mtx_lock(&a_sock_o->lock);

  mtx_unlock(&a_sock_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_buf_t *
sock_read_block(cw_sock_t * a_sock_o, cw_buf_t * a_spare)
{
  cw_buf_t * retval;
  
  _cw_check_ptr(a_sock_o);
  mtx_lock(&a_sock_o->lock);

  mtx_unlock(&a_sock_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
sock_write(cw_sock_t * a_sock_o, cw_buf_t * a_buf_o)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sock_o);
  mtx_lock(&a_sock_o->lock);

  mtx_unlock(&a_sock_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
sock_flush_out(cw_sock_t * a_sock_o)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_sock_o);
  mtx_lock(&a_sock_o->lock);

  mtx_unlock(&a_sock_o->lock);
  return retval;
}

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
