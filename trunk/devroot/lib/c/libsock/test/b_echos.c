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
 * Multi-threaded concurrent line echo server that uses the sock and socks
 * classes.
 *
 ****************************************************************************/

#define _LIBSOCK_USE_SOCKS
#include <libsock/libsock.h>

/*  #include <libstash/libstash_r.h> */

void *
handle_client(void * a_arg)
{
  cw_buf_t buf;
  cw_sock_t * sock = (cw_sock_t *) a_arg;
  const struct iovec * iovec;
  int iovec_count;
  cw_uint32_t i;

  log_printf(cw_g_log, "New connection\n");

  buf_new(&buf, FALSE);
  
  while (1)
  {
/*      buf_dump(&buf, __FUNCTION__ "(0) "); */
    if (-1 == sock_read_block(sock, &buf))
    {
      break;
    }

/*      buf_dump(&buf, __FUNCTION__ "(1) "); */
/*      log_printf(cw_g_log, "read %u bytes\n", */
/*  	       buf_get_size(&buf)); */
    
    iovec = buf_get_iovec(&buf, buf_get_size(&buf), &iovec_count);
/*      log_printf(cw_g_log, "iovec_count == %lu\n", iovec_count); */

    log_printf(cw_g_log, "Got :");
    for (i = 0; i < iovec_count; i++)
    {
      log_nprintf(cw_g_log, iovec[i].iov_len,
		  "%s",
		  iovec[i].iov_base);
    }
    log_printf(cw_g_log, ":\n");
    
    if (-1 == sock_write(sock, &buf))
    {
      break;
    }
    
    _cw_assert(0 == buf_get_size(&buf));
  }
  
  buf_delete(&buf);
  sock_delete(sock);

  log_printf(cw_g_log, "Connection closed\n");
  
  return NULL;
}

int
main(int argc, char ** argv)
{
  cw_socks_t * socks;
  cw_sock_t * sock, * sock_ptr;
  int port;

  log_printf(cw_g_log, "%s: pid %d\n", argv[0], getpid());

  if (argc != 2)
  {
    port = 0;
  }
  else
  {
    port = strtol(argv[1], (char **) NULL, 10);
  }
  
  libstash_init();
  sockb_init(512, 0);
/*    sockb_init(512, 1000000); */
  
/*    dbg_register(cw_g_dbg, "sockb_verbose"); */
  dbg_register(cw_g_dbg, "sockb_error");
/*    dbg_register(cw_g_dbg, "socks_verbose"); */
  dbg_register(cw_g_dbg, "socks_error");
/*    dbg_register(cw_g_dbg, "sock_verbose"); */
  dbg_register(cw_g_dbg, "sock_error");
  dbg_register(cw_g_dbg, "mem_verbose");
  
  socks = socks_new();
  _cw_assert(FALSE == socks_listen(socks, &port));
  log_printf(cw_g_log, "%s: Listening on port %d\n", argv[0], port);

  for (sock_ptr = NULL; sock_ptr == NULL; sock_ptr = NULL)
  {
    sock = sock_new(NULL, 1024);
    _cw_assert(NULL != (sock_ptr = socks_accept_block(socks, sock)));
    thd_new(NULL, handle_client, (void *) sock);
  }

  sock_delete(sock);
  socks_delete(socks);
  
  sockb_shutdown();
  libstash_shutdown();
  return 0;
}
