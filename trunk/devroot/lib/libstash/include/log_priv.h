/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
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
#ifdef _CW_REENTRANT
  cw_mtx_t lock;
#endif
  cw_bool_t is_logfile_open;
  char * logfile_name;
  FILE * log_fp;
};

#define log_p_uint64_base10_add _CW_NS_ANY(log_p_uint64_base10_add)

void log_p_uint64_base10_add(char * a_result, char * a_a, char * a_b);

#endif /* _LOG_PRIV_H_ */
