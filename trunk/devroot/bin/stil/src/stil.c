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
	cw_stilts_t	stilts;
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

	if (stil_new(&stil) == NULL) {
		_cw_out_put_e("Error in stil_new()\n");
		exit(1);
	}
	if (stilt_new(&stilt, &stil) == NULL) {
		_cw_out_put_e("Error in stilt_new()\n");
		exit(1);
	}
	if (stilts_new(&stilts, &stilt) == NULL) {
		_cw_out_put_e("Error in stilts_new()\n");
		exit(1);
	}

#if (0)
	_cw_out_put("sizeof(cw_stil_t): [i]\n", sizeof(cw_stil_t));
	_cw_out_put("\n");

	_cw_out_put("sizeof(cw_stilt_t): [i]\n", sizeof(cw_stilt_t));
	_cw_out_put("sizeof(cw_stilts_t): [i]\n", sizeof(cw_stilts_t));
	_cw_out_put("\n");

	_cw_out_put("sizeof(cw_stilng_t): [i]\n", sizeof(cw_stilng_t));
	_cw_out_put("sizeof(cw_stilnt_t): [i]\n", sizeof(cw_stilnt_t));
	_cw_out_put("\n");
	
	_cw_out_put("sizeof(cw_stila_t): [i]\n", sizeof(cw_stila_t));
	_cw_out_put("sizeof(cw_stilag_t): [i]\n", sizeof(cw_stilag_t));
	_cw_out_put("sizeof(cw_stilat_t): [i]\n", sizeof(cw_stilat_t));
	_cw_out_put("\n");

	_cw_out_put("sizeof(cw_stilo_t): [i]\n", sizeof(cw_stilo_t));
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

	if (is_tty) {
		cw_uint8_t	code[] =
		    "product print `, version ' print version print \".\n\""
		    " print"
		    " `See http://www.canonware.com/stil/ for information.\n'"
		    " print\n";

		stilt_interp_str(&stilt, &stilts, code, sizeof(code) - 1);
	}
		
	for (;;) {
		if (is_tty) {
			cw_uint8_t	code[] = "prompt\n";

			stilt_interp_str(&stilt, &stilts, code, sizeof(code) -
			    1);
		}

		/* Read input. */
		bytes_read = read(0, input, _BUF_SIZE - 1);
		if (bytes_read <= 0)
			break;
		stilt_interp_str(&stilt, &stilts, input,
		    (cw_uint32_t)bytes_read);
	}

	stilts_delete(&stilts, &stilt);
	stilt_delete(&stilt);
	stil_delete(&stil);
	libstash_shutdown();
	return 0;
}
