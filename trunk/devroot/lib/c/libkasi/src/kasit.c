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

cw_kasit_t *
kasit_new(cw_kasit_t * a_kasit,
	  void (*a_dealloc_func)(void * dealloc_arg, void * kasit),
	  void * a_dealloc_arg,
	  cw_kasi_t * a_kasi)
{
  return NULL; /* XXX */
}

void
kasit_delete(cw_kasit_t * a_kasit)
{
}

cw_bool_t
kasit_interp_str(cw_kasit_t * a_kasit, const char * a_str)
{
  out_put(cw_g_out, "Got to [s]()\n", __FUNCTION__);
  return FALSE; /* XXX */
}

cw_bool_t
kasit_interp_buf(cw_kasit_t * a_kasit, cw_buf_t * a_buf)
{
	
  return FALSE; /* XXX */
}

void
kasit_detach_str(cw_kasit_t * a_kasit, const char * a_str)
{
}

void
kasit_detach_buf(cw_kasit_t * a_kasit, cw_buf_t * a_buf)
{
}

cw_bool_t
kasit_p_feed(cw_kasit_t * a_kasit, const char * a_str)
{
  return TRUE; /* XXX */
}

void *
kasit_p_entry(cw_kasit_t * a_kasit, void * a_arg)
{
  return NULL;
}
