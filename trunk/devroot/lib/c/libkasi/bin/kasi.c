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

  while (1)
  {
    out_put(&out, "kasi> ");
    /* XXX Read input. */
    bytes_read = read(0, input, _BUF_SIZE - 1);
    if (-1 == bytes_read)
    {
      out_put(cw_g_out, "read() error: [s]\n", strerror(errno));
      break;
    }
    /* Terminate the input string. */
    input[bytes_read] = '\0';
    
    kasit_interp_str(&kasit, input);
  }
    
  out_put(cw_g_out, "Woohoo\n");
		     
  kasi_delete(&kasi);
  libstash_shutdown();
  return 0;
}
