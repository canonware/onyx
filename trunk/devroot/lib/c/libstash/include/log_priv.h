/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 111 $
 * $Date: 1998-06-30 15:55:50 -0700 (Tue, 30 Jun 1998) $
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

#define log_p_uint64_base10_add _CW_NS_CMN(log_p_uint64_base10_add)

void log_p_uint64_base10_add(char * a_result, char * a_a, char * a_b);

#endif /* _LOG_PRIV_H_ */
