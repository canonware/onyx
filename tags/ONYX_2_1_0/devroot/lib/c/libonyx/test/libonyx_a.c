/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"

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
	fprintf(stderr, "Test begin\n");
	do_mem();

	fprintf(stderr, "libonyx_init()\n");
	libonyx_init();
	do_mem();

	fprintf(stderr, "libonyx_shutdown()\n");
	libonyx_shutdown();
	do_mem();

	fprintf(stderr, "Test end\n");
	return 0;
}
