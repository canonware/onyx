/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libkasi/libkasi.h"
#include <libstash/libstash.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

#define _BUF_SIZE 4096

int
main(int argc, char ** argv)
{
  cw_kasi_t kasi;
  cw_kasit_t kasit;
  char input[_BUF_SIZE];
  ssize_t bytes_read;
  cw_out_t out;
  cw_bool_t is_tty;
  
  libstash_init();
/*    dbg_register(cw_g_dbg, "pezz_verbose"); */
  
  /* XXX Set up oom handler. */

  is_tty = (cw_bool_t) isatty(0);

  out_new(&out);
  out_set_default_fd(&out, 1);

  if (TRUE == is_tty)
  {
    out_put(&out, "kasi, version [s].\n", _LIBKASI_VERSION);
    out_put(&out,
	    "See http://www.canonware.com/software/kasi/ for information.\n");
  }
	    
  kasi_new(&kasi);
  kasit_new(&kasit, NULL, NULL, &kasi);

#if (1)
  _cw_out_put("sizeof(cw_kasio_t): [i]\n", sizeof(cw_kasio_t));
  _cw_out_put("sizeof(cw_kasioe_t): [i]\n", sizeof(cw_kasioe_t));
  _cw_out_put("sizeof(cw_kasioe_t): [i]\n", sizeof(cw_kasioe_t));
  _cw_out_put("sizeof(cw_kasioe_array_t): [i]\n", sizeof(cw_kasioe_array_t));
  _cw_out_put("sizeof(cw_kasioe_condition_t): [i]\n",
	      sizeof(cw_kasioe_condition_t));
  _cw_out_put("sizeof(cw_kasioe_dict_t): [i]\n", sizeof(cw_kasioe_dict_t));
  _cw_out_put("sizeof(cw_kasioe_lock_t): [i]\n", sizeof(cw_kasioe_lock_t));
  _cw_out_put("sizeof(cw_kasioe_mstate_t): [i]\n", sizeof(cw_kasioe_mstate_t));
  _cw_out_put("sizeof(cw_kasioe_name_t): [i]\n", sizeof(cw_kasioe_name_t));
  _cw_out_put("sizeof(cw_kasioe_number_t): [i]\n", sizeof(cw_kasioe_number_t));
  _cw_out_put("sizeof(cw_kasioe_operator_t): [i]\n",
	      sizeof(cw_kasioe_operator_t));
  _cw_out_put("sizeof(cw_kasioe_packedarray_t): [i]\n",
	      sizeof(cw_kasioe_packedarray_t));
  _cw_out_put("sizeof(cw_kasioe_string_t): [i]\n", sizeof(cw_kasioe_string_t));
  _cw_out_put("\n");

  _cw_out_put("sizeof(cw_kasis_t): [i]\n", sizeof(cw_kasis_t));
  _cw_out_put("sizeof(cw_kasiso_t): [i]\n", sizeof(cw_kasiso_t));
  _cw_out_put("sizeof(cw_kasisc_t): [i]\n", sizeof(cw_kasisc_t));
  _cw_out_put("\n");
  
  _cw_out_put("sizeof(cw_kasid_t): [i]\n", sizeof(cw_kasid_t));
  _cw_out_put("sizeof(cw_kasid_t)[[[i]]: [i]\n",
	      256, _CW_KASID_ENTS2SIZEOF(256));
  _cw_out_put("sizeof(cw_kasid_t)[[[i]]: [i]\n", 16, _CW_KASID_ENTS2SIZEOF(16));
  _cw_out_put("sizeof(cw_kasido_t): [i]\n", sizeof(cw_kasido_t));
  _cw_out_put("\n");
  
  _cw_out_put("sizeof(cw_buf_t): [i]\n", sizeof(cw_buf_t));
  _cw_out_put("sizeof(cw_ring_t): [i]\n", sizeof(cw_ring_t));
  _cw_out_put("sizeof(cw_oh_t): [i]\n", sizeof(cw_oh_t));
  _cw_out_put("sizeof(cw_ch_t): [i]\n", sizeof(cw_ch_t));
  _cw_out_put("sizeof(cw_chi_t): [i]\n", sizeof(cw_chi_t));
  _cw_out_put("sizeof(cw_ch_t)[[[i]]: [i]\n", 256, _CW_CH_TABLE2SIZEOF(256));
  _cw_out_put("sizeof(cw_ch_t)[[[i]]: [i]\n", 16, _CW_CH_TABLE2SIZEOF(16));
  _cw_out_put("sizeof(cw_dch_t): [i]\n", sizeof(cw_dch_t));
  _cw_out_put("\n");
#endif
  
  while (1)
  {
    if (TRUE == is_tty)
    {
      out_put(&out, "kasi> ");
    }
    
    /* Read input. */
    bytes_read = read(0, input, _BUF_SIZE - 1);
    if (0 >= bytes_read)
    {
      break;
    }
    kasit_interp_str(&kasit, input, (cw_uint32_t) bytes_read);
  }
		     
  kasi_delete(&kasi);
  libstash_shutdown();
  return 0;
}
