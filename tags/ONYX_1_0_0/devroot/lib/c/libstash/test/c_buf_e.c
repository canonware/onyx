/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Check for array bounds overrun.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#define _ARR_SIZE 64

void
print_arr(const char *a_arr, cw_uint32_t a_size, const char *a_prefix)
{
	cw_uint32_t	i;

	for (i = 0; i < a_size; i++) {
		if ((i % 16) == 0) {
			out_put(out_err, "[s] 0x[i|w:4|p:0|b:16]:", a_prefix,
			    i);
		}
		out_put(out_err, " [i|w:2|p:0|b:16]", a_arr[i]);

		if ((i % 16) == 15)
			out_put(out_err, "\n");
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
	out_put(out_err, "Test begin\n");

	foo();

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}