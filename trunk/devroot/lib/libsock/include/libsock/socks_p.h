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
 * Private data structure for the socks class.
 *
 ****************************************************************************/

#define _LIBSOCK_SOCKS_MAGIC 0x19730803

struct cw_socks_s
{
#ifdef _LIBSOCK_DBG
  cw_uint32_t magic;
#endif
  cw_bool_t is_listening;
  int sockfd;
};
