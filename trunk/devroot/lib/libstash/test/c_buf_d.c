/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Test the endianness behavior of buf.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

int
main()
{
	cw_buf_t	*buf;
	cw_bufc_t	*bufc;
	cw_uint32_t	longs[8];
	cw_uint64_t	quads[4];

	libstash_init();
	_cw_out_put("Test begin\n");

	longs[0] = 0x01020304;
	longs[1] = 0x05060708;
	longs[2] = 0x090a0b0c;
	longs[3] = 0x0d0e0f10;
	longs[4] = 0x11121314;
	longs[5] = 0x15161718;
	longs[6] = 0x191a1b1c;
	longs[7] = 0x1d1e1f20;

	quads[0] = ((cw_uint64_t)0x21222324 << 32) + 0x25262728;
	quads[1] = ((cw_uint64_t)0x292a2b2c << 32) + 0x2d2e2f30;
	quads[2] = ((cw_uint64_t)0x31323334 << 32) + 0x35363738;
	quads[3] = ((cw_uint64_t)0x393a3b3c << 32) + 0x3d3e3f40;

	buf = buf_new(NULL, cw_g_mem);
	_cw_check_ptr(buf);

	bufc = bufc_new(NULL, cw_g_mem, NULL, NULL);
	_cw_check_ptr(bufc);
	bufc_buffer_set(bufc, longs, sizeof(longs), TRUE, NULL, NULL);
	buf_bufc_append(buf, bufc, 0, sizeof(longs));
	bufc_delete(bufc);

	bufc = bufc_new(NULL, cw_g_mem, NULL, NULL);
	_cw_check_ptr(bufc);
	bufc_buffer_set(bufc, quads, sizeof(quads), TRUE, NULL, NULL);
	buf_bufc_append(buf, bufc, 0, sizeof(quads));
	bufc_delete(bufc);

	_cw_out_put("longs[[0-3]: 0x[i|b:16|w:8|p:0] 0x[i|b:16|w:8|p:0] "
	    "0x[i|b:16|w:8|p:0] 0x[i|b:16|w:8|p:0]\n", longs[0], longs[1],
	    longs[2], longs[3]);
	_cw_out_put("buf[[0-3]32: 0x[i|b:16|w:8|p:0] 0x[i|b:16|w:8|p:0] "
	    "0x[i|b:16|w:8|p:0] 0x[i|b:16|w:8|p:0]\n", buf_uint32_get(buf, 0),
	    buf_uint32_get(buf, 4), buf_uint32_get(buf, 8), buf_uint32_get(buf,
	    12));

	_cw_out_put("longs[[4-7]: 0x[i|b:16|w:8|p:0] 0x[i|b:16|w:8|p:0] "
	    "0x[i|b:16|w:8|p:0] 0x[i|b:16|w:8|p:0]\n", longs[4], longs[5],
	    longs[6], longs[7]);
	_cw_out_put("buf[[4-7]32: 0x[i|b:16|w:8|p:0] 0x[i|b:16|w:8|p:0] "
	    "0x[i|b:16|w:8|p:0] 0x[i|b:16|w:8|p:0]\n", buf_uint32_get(buf, 16),
	    buf_uint32_get(buf, 20), buf_uint32_get(buf, 24),
	    buf_uint32_get(buf, 28));

	_cw_out_put("quads[[0-1]: 0x[q|b:16|w:16|p:0] 0x[q|b:16|w:16|p:0]\n",
	    quads[0], quads[1]);
	_cw_out_put("buf[[0-1]64: 0x[q|b:16|w:16|p:0] 0x[q|b:16|w:16|p:0]\n",
	    buf_uint64_get(buf, 32), buf_uint64_get(buf, 40));

	_cw_out_put("quads[[2-3]: 0x[q|b:16|w:16|p:0] 0x[q|b:16|w:16|p:0]\n",
	    quads[2], quads[3]);
	_cw_out_put("buf[[2-3]64: 0x[q|b:16|w:16|p:0] 0x[q|b:16|w:16|p:0]\n",
	    buf_uint64_get(buf, 48), buf_uint64_get(buf, 56));

	buf_delete(buf);

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
