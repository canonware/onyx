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
  cw_bufel_t * bufel_a, bufel_b;

  glob_new();
  dbg_turn_on(g_dbg_o, _STASH_DBG_C_FUNC);

  buf_a = buf_new(NULL, FALSE);
  _cw_check_ptr(buf_a);
  buf_delete(buf_a);

  _cw_assert(&buf_b == buf_new(&buf_b, TRUE));

  _cw_assert(buf_get_size(&buf_b) == 0);

  bufel_a = bufel_new(NULL);
  _cw_check_ptr(bufel_a);

  _cw_assert(bufel_get_size(bufel_a) == 0);
  _cw_assert(bufel_get_beg_offset(bufel_a) == 0);
  _cw_assert(bufel_get_end_offset(bufel_a) == 0);
  
  dbg_turn_on(g_dbg_o, _STASH_DBG_C_FUNC);
  
  _cw_assert(FALSE == bufel_set_size(bufel_a, 4096));
  
  bufel_set_end_offset(bufel_a, 20);

  bufel_set_uint32(bufel_a, 4, 0x20212223);

  _cw_assert(0x23 == bufel_get_uint8(bufel_a, 4));
  _cw_assert(0x20 == bufel_get_uint8(bufel_a, 4));
  _cw_assert(0x21 == bufel_get_uint8(bufel_a, 5));
  _cw_assert(0x22 == bufel_get_uint8(bufel_a, 6));
  _cw_assert(0x23 == bufel_get_uint8(bufel_a, 7));
  
  _cw_assert(0x20212223 == bufel_get_uint32(bufel_a, 4));

  glob_delete();
  return 0;
}
