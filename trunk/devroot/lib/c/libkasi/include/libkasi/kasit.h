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

typedef struct cw_kasit_s cw_kasit_t;

struct cw_kasit_s
{
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t magic_a;
#endif
  cw_mtx_t lock;
  void (*dealloc_func)(void *, void *);
  void * dealloc_arg;
  
#if (defined(_LIBKASI_DBG) || defined(_LIBKASI_DEBUG))
  cw_uint32_t size_of;
  cw_uint32_t magic_b;
#endif
};

cw_kasit_t *
kasit_new(cw_kasit_t * a_kasit,
	  void (*a_dealloc_func)(void * dealloc_arg, void * kasit),
	  void * a_dealloc_arg,
	  cw_kasi_t * a_kasi);

void
kasit_delete(cw_kasit_t * a_kasit);

cw_bool_t
kasit_interp_str(cw_kasit_t * a_kasit, const char * a_str);

cw_bool_t
kasit_interp_buf(cw_kasit_t * a_kasit, cw_buf_t * a_buf);

void
kasit_detach_str(cw_kasit_t * a_kasit, const char * a_str);

void
kasit_detach_buf(cw_kasit_t * a_kasit, cw_buf_t * a_buf);
