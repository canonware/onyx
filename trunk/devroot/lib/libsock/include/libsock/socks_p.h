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

struct cw_socks_s
{
  cw_bool_t is_listening;
  int sockfd;
};
