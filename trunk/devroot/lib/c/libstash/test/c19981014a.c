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
 * buf test.
 *
 ****************************************************************************/

#define _INC_BUF_H_
#define _INC_GLOB_H_
#include <libstash.h>

int
main()
{
  cw_buf_t * buf_a, buf_b;

  glob_new();

  buf_a = buf_new(NULL, FALSE);
  _cw_check_ptr(buf_a);
  buf_delete(buf_a);

  _cw_assert(&buf_b == buf_new(&buf_b, TRUE));

  

  glob_delete();
  return 0;
}
