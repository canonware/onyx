/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include <libstil/libstil.h>
#include <libstash/libstash.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

#define _BUF_SIZE 4096

int
main(int argc, char **argv)
{
	cw_stil_t	stil;
	cw_stilt_t	stilt;
	char		input[_BUF_SIZE];
	ssize_t		bytes_read;
	cw_out_t	out;
	cw_bool_t	is_tty;

	libstash_init();
/*  	dbg_register(cw_g_dbg, "mem_verbose"); */
/*  	dbg_register(cw_g_dbg, "pezz_verbose"); */

	/* XXX Set up oom handler. */

	is_tty = (cw_bool_t)isatty(0);

	out_new(&out, cw_g_mem);
	out_set_default_fd(&out, 1);

	if (is_tty) {
		out_put(&out, "stil, version [s].\n", _LIBSTIL_VERSION);
		out_put(&out,
		    "See http://www.canonware.com/stil/ for information.\n");
	}
	if (stil_new(&stil) == NULL) {
		_cw_out_put_e("Error in stil_new()\n");
		exit(1);
	}
	if (stilt_new(&stilt, &stil) == NULL) {
		_cw_out_put_e("Error in stilt_new()\n");
		exit(1);
	}

#if (0)
	_cw_out_put("sizeof(cw_stil_t): [i]\n", sizeof(cw_stil_t));
	_cw_out_put("\n");

	_cw_out_put("sizeof(cw_stilt_t): [i]\n", sizeof(cw_stilt_t));
	_cw_out_put("\n");

	_cw_out_put("sizeof(cw_stiln_t): [i]\n", sizeof(cw_stiln_t));
	_cw_out_put("sizeof(cw_stilng_t): [i]\n", sizeof(cw_stilng_t));
	_cw_out_put("sizeof(cw_stilnt_t): [i]\n", sizeof(cw_stilnt_t));
	_cw_out_put("\n");
	
	_cw_out_put("sizeof(cw_stila_t): [i]\n", sizeof(cw_stila_t));
	_cw_out_put("sizeof(cw_stilag_t): [i]\n", sizeof(cw_stilag_t));
	_cw_out_put("sizeof(cw_stilat_t): [i]\n", sizeof(cw_stilat_t));
	_cw_out_put("\n");

	_cw_out_put("sizeof(cw_stilo_t): [i]\n", sizeof(cw_stilo_t));
	_cw_out_put("sizeof(cw_stiloe_t): [i]\n", sizeof(cw_stiloe_t));
	_cw_out_put("sizeof(cw_stiloe_name_t): [i]\n",
	    sizeof(cw_stiloe_name_t));
	_cw_out_put("sizeof(cw_stiloe_dicto_t): [i]\n",
	    sizeof(cw_stiloe_dicto_t));
	_cw_out_put("\n");

	_cw_out_put("sizeof(cw_stils_t): [i]\n", sizeof(cw_stils_t));
	_cw_out_put("sizeof(cw_stilso_t): [i]\n", sizeof(cw_stilso_t));
	_cw_out_put("sizeof(cw_stilsc_t): [i]\n", sizeof(cw_stilsc_t));
	_cw_out_put("\n");

	_cw_out_put("sizeof(cw_buf_t): [i]\n", sizeof(cw_buf_t));
	_cw_out_put("sizeof(cw_ch_t): [i]\n", sizeof(cw_ch_t));
	_cw_out_put("sizeof(cw_chi_t): [i]\n", sizeof(cw_chi_t));
	_cw_out_put("sizeof(cw_ch_t)[[[i]]: [i]\n", 256,
	    _CW_CH_TABLE2SIZEOF(256));
	_cw_out_put("sizeof(cw_ch_t)[[[i]]: [i]\n", 16,
	    _CW_CH_TABLE2SIZEOF(16));
	_cw_out_put("sizeof(cw_dch_t): [i]\n", sizeof(cw_dch_t));
	_cw_out_put("\n");
#endif

	for (;;) {
		if (is_tty)
			out_put(&out, "stil> ");
		/* Read input. */
		bytes_read = read(0, input, _BUF_SIZE - 1);
		if (bytes_read <= 0)
			break;
		stilt_interp_str(&stilt, input, (cw_uint32_t)bytes_read);
/*  		_cw_out_put("pstack:\n"); */
		op_pstack(&stilt);
/*  		_cw_out_put("stack:\n"); */
/*  		op_stack(&stilt); */
	}

	stilt_delete(&stilt);
	stil_delete(&stil);
	libstash_shutdown();
	return 0;
}
