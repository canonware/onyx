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
  
  libstash_init();
  /* XXX Set up oom handler. */

  out_new(&out);
  out_set_default_fd(&out, 1);

  kasi_new(&kasi);
  kasit_new(&kasit, NULL, NULL, &kasi);

#if (1)
  out_put(cw_g_out, "sizeof(cw_kasio_t): [i]\n", sizeof(cw_kasio_t));
  out_put(cw_g_out, "\n");

  out_put(cw_g_out, "sizeof(cw_kasis_t): [i]\n", sizeof(cw_kasis_t));
  out_put(cw_g_out, "sizeof(cw_kasiso_t): [i]\n", sizeof(cw_kasiso_t));
  out_put(cw_g_out, "sizeof(cw_kasisc_t): [i]\n",
	  sizeof(cw_kasisc_t));
  out_put(cw_g_out, "\n");
  
  out_put(cw_g_out, "sizeof(cw_kasid_t): [i]\n", sizeof(cw_kasid_t));
  out_put(cw_g_out, "sizeof(cw_kasid_t)[[[i]]: [i]\n",
	  256, _CW_KASID_ENTS2SIZEOF(256));
  out_put(cw_g_out, "sizeof(cw_kasid_t)[[[i]]: [i]\n",
	  16, _CW_KASID_ENTS2SIZEOF(16));
  out_put(cw_g_out, "sizeof(cw_kasido_t): [i]\n", sizeof(cw_kasido_t));
  out_put(cw_g_out, "\n");
  
  out_put(cw_g_out, "sizeof(cw_buf_t): [i]\n", sizeof(cw_buf_t));
  out_put(cw_g_out, "sizeof(cw_ring_t): [i]\n", sizeof(cw_ring_t));
  out_put(cw_g_out, "sizeof(cw_oh_t): [i]\n", sizeof(cw_oh_t));
  out_put(cw_g_out, "sizeof(cw_ch_t): [i]\n", sizeof(cw_ch_t));
  out_put(cw_g_out, "sizeof(cw_chi_t): [i]\n", sizeof(cw_chi_t));
  out_put(cw_g_out, "sizeof(cw_ch_t)[[[i]]: [i]\n",
	  256, _CW_CH_TABLE2SIZEOF(256));
  out_put(cw_g_out, "sizeof(cw_ch_t)[[[i]]: [i]\n",
	  16, _CW_CH_TABLE2SIZEOF(16));
  out_put(cw_g_out, "\n");
#endif
  
  while (1)
  {
    out_put(&out, "kasi> ");
    
    /* Read input. */
    bytes_read = read(0, input, _BUF_SIZE - 1);
    if (0 >= bytes_read)
    {
      break;
    }
    kasit_interp_str(&kasit, input, (cw_uint32_t) bytes_read);
  }
    
  out_put(cw_g_out, "Woohoo\n");
		     
  kasi_delete(&kasi);
  libstash_shutdown();
  return 0;
}
