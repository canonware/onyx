/****************************************************************************
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
 * Check for array bounds overrun.
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#define _ARR_SIZE 64

void
print_arr(const char *a_arr, cw_uint32_t a_size, const char *a_prefix)
{
	cw_uint32_t	i;

	for (i = 0; i < a_size; i++) {
		if ((i % 16) == 0)
			_cw_out_put("[s] 0x[i|w:4|p:0|b:16]:", a_prefix, i);
		_cw_out_put(" [i|w:2|p:0|b:16]", a_arr[i]);

		if ((i % 16) == 15)
			_cw_out_put("\n");
	}
}

void
foo(void)
{
	char		a[_ARR_SIZE];
	cw_buf_t	buf;
	char		b[_ARR_SIZE];

	memset(a, 'a', _ARR_SIZE);
	memset(b, 'b', _ARR_SIZE);

	print_arr(a, _ARR_SIZE, "a(0)");
	print_arr(b, _ARR_SIZE, "b(0)");

	buf_new(&buf, cw_g_mem);

	print_arr(a, _ARR_SIZE, "a(1)");
	print_arr(b, _ARR_SIZE, "b(1)");

	buf_delete(&buf);

	print_arr(a, _ARR_SIZE, "a(2)");
	print_arr(b, _ARR_SIZE, "b(2)");
}

int
main()
{
	libstash_init();
	_cw_out_put("Test begin\n");

	foo();

	_cw_out_put("Test end\n");
	libstash_shutdown();
	return 0;
}
