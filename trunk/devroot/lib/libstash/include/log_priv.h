/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 86 $
 * $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _LOG_PRIV_H_
#define _LOG_PRIV_H_

struct cw_log_s
{
  cw_mtx_t lock;
  cw_bool_t is_logfile_open;
  char * logfile_name;
  FILE * log_fp;
};

#endif /* _LOG_PRIV_H_ */
