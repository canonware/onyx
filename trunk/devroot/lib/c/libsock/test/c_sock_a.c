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
 * Verify that a sock object can be re-used.
 *
 ****************************************************************************/

#include <libsock/libsock.h>

/* Number of times to connect to the server thread. */
#define _CW_NCONNS 10

void *
handle_client(void * a_arg)
{
  cw_socks_t * socks = (cw_socks_t *) a_arg;
  cw_sock_t sock;
  cw_uint32_t i, message;
  cw_buf_t buf;

  buf_new(&buf);
  sock_new(&sock, 16384);

  for (i = 0; i < _CW_NCONNS; i++)
  {
    _cw_assert(NULL != socks_accept(socks, NULL, &sock));
    buf_release_head_data(&buf, buf_get_size(&buf));
    while (0 < sock_read(&sock, &buf, 0, NULL))
    {
    }

    _cw_assert(4 <= buf_get_size(&buf));
    message = buf_get_uint32(&buf, 0);
    out_put(cw_g_out, "Connection [i], message [i]\n", i, message);

    _cw_assert(FALSE == sock_disconnect(&sock));
  }

  sock_delete(&sock);
  buf_delete(&buf);
  return NULL;
}

int
main()
{
  cw_socks_t * socks = NULL;
  int port = 0;
  cw_sock_t sock;
  cw_thd_t * thd;
  cw_buf_t buf;
  cw_uint32_t i;
  
  libstash_init();
/*    dbg_register(cw_g_dbg, "sock_error"); */
/*    dbg_register(cw_g_dbg, "sock_sockopt"); */
  out_put(cw_g_out, "Test begin\n");
  sockb_init(1024, /* a_max_fds */
	     4096, /* a_bufc_size */
	     16    /* a_max_spare_bufcs */
	     );

  buf_new(&buf);

  socks = socks_new();
  _cw_check_ptr(socks);
/*    if (TRUE == socks_listen(socks, INADDR_LOOPBACK, &port)) */
  if (TRUE == socks_listen(socks, INADDR_ANY, &port))
  {
    out_put(cw_g_out, "Error listening on port [i]\n", port);
    goto RETURN;
  }

  thd = thd_new(NULL, handle_client, (void *) socks);
  _cw_check_ptr(thd);

  _cw_assert(NULL != sock_new(&sock, 16384));
  for (i = 0; i < _CW_NCONNS; i++)
  {
    _cw_assert(0 == sock_connect(&sock, "localhost", port, NULL));

    /* Write some data. */
    buf_set_uint32(&buf, 0, i);
    _cw_assert(FALSE == sock_write(&sock, &buf));

    _cw_assert(FALSE == sock_flush_out(&sock));
    _cw_assert(FALSE == sock_disconnect(&sock));
  }
  sock_delete(&sock);
  thd_join(thd);
  
  RETURN:
  if (NULL != socks)
  {
    socks_delete(socks);
  }
  sockb_shutdown();
  buf_delete(&buf);
  out_put(cw_g_out, "Test end\n");
  libstash_shutdown();
  return 0;
}
