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
/*    dbg_turn_on(g_dbg_o, _STASH_DBG_C_FUNC); */

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
  
  _cw_assert(FALSE == bufel_set_size(bufel_a, 4096));
  
  bufel_set_end_offset(bufel_a, 512);
  
  {
    cw_uint32_t i;

    for (i = 0; i < 256; i++)
    {
      bufel_set_uint8(bufel_a, i, i);
    }

    log_printf(g_log_o, "lower char dump:\n");
    for (i = 0; i < 256; i += 8)
    {
      log_printf(g_log_o,
		 "%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x\n",
		 i, bufel_get_uint8(bufel_a, i),
		 i + 1, bufel_get_uint8(bufel_a, i + 1),
		 i + 2, bufel_get_uint8(bufel_a, i + 2),
		 i + 3, bufel_get_uint8(bufel_a, i + 3),
		 i + 4, bufel_get_uint8(bufel_a, i + 4),
		 i + 5, bufel_get_uint8(bufel_a, i + 5),
		 i + 6, bufel_get_uint8(bufel_a, i + 6),
		 i + 7, bufel_get_uint8(bufel_a, i + 7));
    }
    
    log_printf(g_log_o, "lower long dump:\n");
    for (i = 0; i < 256; i += 8)
    {
      log_printf(g_log_o, "%03u->0x%08x:%03u->0x%08x\n",
		 i, bufel_get_uint32(bufel_a, i),
		 i + 4, bufel_get_uint32(bufel_a, i + 4));
    }

    /* Copy the bytes from the lower 256 bytes into the upper 256 bytes,
     * but reverse them. */
    for (i = 0; i < 256; i += 4)
    {
      bufel_set_uint32(bufel_a, (512 - 4) - i,
		       (((cw_uint32_t) (bufel_get_uint8(bufel_a, i)) << 24)
		       | (((cw_uint32_t) bufel_get_uint8(bufel_a, i + 1)) << 16)
		       | (((cw_uint32_t) bufel_get_uint8(bufel_a, i + 2)) << 8)
		       | ((cw_uint32_t) bufel_get_uint8(bufel_a, i + 3))));
    }

    log_printf(g_log_o, "upper char dump:\n");
    for (i = 256; i < 512; i += 8)
    {
      log_printf(g_log_o,
		 "%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x:%03u->0x%02x\n",
		 i, bufel_get_uint8(bufel_a, i),
		 i + 1, bufel_get_uint8(bufel_a, i + 1),
		 i + 2, bufel_get_uint8(bufel_a, i + 2),
		 i + 3, bufel_get_uint8(bufel_a, i + 3),
		 i + 4, bufel_get_uint8(bufel_a, i + 4),
		 i + 5, bufel_get_uint8(bufel_a, i + 5),
		 i + 6, bufel_get_uint8(bufel_a, i + 6),
		 i + 7, bufel_get_uint8(bufel_a, i + 7));
    }
    
    log_printf(g_log_o, "upper long dump:\n");
    for (i = 256; i < 512; i += 8)
    {
      log_printf(g_log_o, "%03u->0x%08x:%03u->0x%08x\n",
		 i, bufel_get_uint32(bufel_a, i),
		 i + 4, bufel_get_uint32(bufel_a, i + 4));
    }
  }

  glob_delete();
  return 0;
}
