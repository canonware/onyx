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
 * $Revision: 136 $
 * $Date: 1998-07-10 13:11:48 -0700 (Fri, 10 Jul 1998) $
 *
 * <<< Description >>>
 *
 * Test for 64 bit value printing in the log class.
 *
 ****************************************************************************/

#ifndef _CW_DEBUG
#  define _CW_DEBUG
#endif

#define _INC_GLOB_H_
#include <libstash.h>

int
main()
{
  cw_uint64_t i, j, k, l;
  char a[65], b[65];
    
  glob_new();

  /* The following grossness shuts the compiler up.  IMO the compiler is
   * out of line in generating warnings on these. */
/*   i = 0xffff0000ffffabcd; */
  i = ((cw_uint64_t) 0xffff0000 << 32) + 0xffffabcd;
  
/*   j = 0x0123456789abcdef; */
  j = ((cw_uint64_t) 0x01234567 << 32) + 0x89abcdef;

/*   k = 123456789012345; */
  k = ((cw_uint64_t) 12345678 * 10000000) + 9012345;
  l = 42;
  
  log_printf(g_log_o, "0xffff0000ffffabcd --> 0x%s (16), %s (2)\n",
	     log_print_uint64(i, 16, a), log_print_uint64(i, 2, b));
  log_printf(g_log_o, "0x0123456789abcdef --> 0x%s (16), %s (2)\n",
	     log_print_uint64(j, 16, a), log_print_uint64(j, 2, b));
  log_printf(g_log_o, "123456789012345 --> %s (10)\n",
	     log_print_uint64(k, 10, a));
  log_printf(g_log_o, "42 --> %s (10)\n",
	     log_print_uint64(l, 10, a));

  glob_delete();

  return 0;
}
