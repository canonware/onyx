/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Verify that libstash works, even if libstash_init() is not called.
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

void
do_mem(void)
{
	void	*p;

	p = _cw_malloc(1024);
	_cw_check_ptr(p);

	p = _cw_realloc(p, 2048);
	_cw_check_ptr(p);

	_cw_free(p);

	p = _cw_calloc(8, 128);
	_cw_check_ptr(p);

	_cw_free(p);
}

int
main()
{
	_cw_out_put("Test begin\n");
	do_mem();

	_cw_out_put("libstash_init()\n");
	libstash_init();
	do_mem();

	_cw_out_put("libstash_shutdown()\n");
	libstash_shutdown();
	do_mem();

	_cw_out_put("Test end\n");
	return 0;
}
