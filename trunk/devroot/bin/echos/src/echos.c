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

#include <libsock/libsock.h>

void *
handle_client(void * a_arg)
{
  cw_buf_t buf;
  cw_sock_t * sock = (cw_sock_t *) a_arg;
  const struct iovec * iovec;
  int iovec_count;
  cw_uint32_t i;

  out_put(cw_g_out, "New connection\n");

  buf_new(&buf);
  
  while (1)
  {
    if (0 > sock_read(sock, &buf, 0, NULL))
    {
      break;
    }

    iovec = buf_get_iovec(&buf, 0xffffffff, buf_get_size(&buf), &iovec_count);

    out_put(cw_g_out, "Got :");
    for (i = 0; i < iovec_count; i++)
    {
      out_put_n(cw_g_out, iovec[i].iov_len,
		"[s]",
		iovec[i].iov_base);
    }
    out_put(cw_g_out, ":\n");
    
    if (TRUE == sock_write(sock, &buf))
    {
      break;
    }
    
    _cw_assert(0 == buf_get_size(&buf));
  }
  
  buf_delete(&buf);
  sock_delete(sock);

  out_put(cw_g_out, "Connection closed\n");
  
  return NULL;
}

int
main(int argc, char ** argv)
{
  cw_socks_t * socks;
  cw_sock_t * sock, * sock_ptr;
  int port;
  cw_thd_t * thd;

  out_put(cw_g_out, "[s]: pid [i]\n", argv[0], getpid());

  if (argc != 2)
  {
    port = 0;
  }
  else
  {
    port = strtol(argv[1], (char **) NULL, 10);
  }
  
  libstash_init();
  sockb_init(64, 512, 0);
  
  dbg_register(cw_g_dbg, "sockb_verbose");
  dbg_register(cw_g_dbg, "sockb_error");
  dbg_register(cw_g_dbg, "socks_verbose");
  dbg_register(cw_g_dbg, "socks_error");
  dbg_register(cw_g_dbg, "sock_verbose");
  dbg_register(cw_g_dbg, "sock_error");
/*    dbg_register(cw_g_dbg, "mem_verbose"); */
  
  socks = socks_new();
  if (TRUE == socks_listen(socks, INADDR_ANY, &port))
  {
    out_put(cw_g_out, "[s]: Error listening on port [i]\n", argv[0], port);
    goto RETURN;
  }
  
  out_put(cw_g_out, "[s]: Listening on port [i]\n", argv[0], port);

  for (sock_ptr = NULL; sock_ptr == NULL; sock_ptr = NULL)
  {
    sock = sock_new(NULL, 1024);

    sock_ptr = socks_accept(socks, NULL, sock);
    _cw_check_ptr(sock_ptr);
    
    thd = thd_new(NULL, handle_client, (void *) sock);

    /* Detach the thread. */
    thd_delete(thd);
  }

  sock_delete(sock);
  socks_delete(socks);

  RETURN:
  sockb_shutdown();
  libstash_shutdown();
  return 0;
}
