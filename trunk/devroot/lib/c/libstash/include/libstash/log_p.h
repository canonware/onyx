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
 *
 *
 ****************************************************************************/

struct cw_log_s
{
#ifdef _CW_REENTRANT
  cw_mtx_t lock;
#endif
  cw_bool_t is_logfile_open;
  char * logfile_name;
  FILE * log_fp;
};

/****************************************************************************
 *
 * Does a base 10 addition of the strings a_a and a_b, and puts the result
 * into string a_result.
 *
 ****************************************************************************/
static void
log_p_uint64_base10_add(char * a_result, const char * a_a, const char * a_b);
