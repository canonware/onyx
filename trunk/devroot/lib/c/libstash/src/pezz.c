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
 * <<< Description >>>
 *
 * Implementation of the pezz class.
 *
 ****************************************************************************/

#define _LIBSTASH_USE_PEZZ
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#ifdef _LIBSTASH_DBG
#  define _CW_PEZZ_MAGIC 0x4e224e22
#endif

/* Round up to a multiple of 16 to avoid alignment problems. */
#define _CW_PEZZ_RING_SPACE (sizeof(cw_ring_t) + (16 - sizeof(cw_ring_t) % 16))

cw_pezz_t *
pezz_new(cw_pezz_t * a_pezz, cw_uint32_t a_buffer_size,
	 cw_uint32_t a_num_buffers)
{
  cw_pezz_t * retval;

  _cw_assert(0 != (a_buffer_size * a_num_buffers));

  if (NULL == a_pezz)
  {
    retval = (cw_pezz_t *) _cw_malloc(sizeof(cw_pezz_t));
    bzero(retval, sizeof(cw_pezz_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_pezz;
    bzero(retval, sizeof(cw_pezz_t));
    retval->is_malloced = FALSE;
  }

#ifdef _LIBSTASH_DBG
  retval->magic = _CW_PEZZ_MAGIC;
#endif

#ifdef _CW_REENTRANT
  mtx_new(&retval->lock);
#endif

  /* Calculate how big to make the memory block and allocate it. */
  {
    cw_uint32_t block_size;

    retval->internal_buffer_size = (_CW_PEZZ_RING_SPACE
				    + a_buffer_size
				    + (16 - a_buffer_size % 16));
    block_size = retval->internal_buffer_size * a_num_buffers;
    retval->mem_base = (void *) _cw_malloc(block_size);
    retval->mem_end = retval->mem_base + block_size;
  }

  retval->buffer_size = a_buffer_size;
  retval->num_buffers = a_num_buffers;
  
  /* Iterate through the buffers, initialize their ring elements, and insert
   * them into the spare buffers ring. */
  {
    cw_uint32_t i;
    cw_ring_t * ring;

    /* Initialize spare buffers ring to have something in it. */
    retval->spare_buffers = (cw_ring_t *) retval->mem_base;
    ring_new(retval->spare_buffers, NULL, NULL);
    ring_set_data(retval->spare_buffers,
		  _CW_PEZZ_RING_SPACE + ((void *) retval->spare_buffers));
    
    for (i = 1; i < retval->num_buffers; i++)
    {
      ring = (cw_ring_t *) (retval->mem_base
			    + (i * retval->internal_buffer_size));
      ring_new(ring, NULL, NULL);
      ring_set_data(ring,
		    _CW_PEZZ_RING_SPACE + ((void *) ring));
      ring_meld(retval->spare_buffers, ring);
    }
  }
    
  return retval;
}

void
pezz_delete(cw_pezz_t * a_pezz)
{
  _cw_check_ptr(a_pezz);
  _cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);

#ifdef _LIBSTASH_DBG
  if (dbg_is_registered(cw_g_dbg, "pezz_error"))
  {
    /* See if there are any leaked overflow buffers. */
    if (0 < a_pezz->num_overflow)
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "%lu leaked overflow buffer%s\n",
		  a_pezz->num_overflow,
		  a_pezz->num_overflow != 1 ? "s" : "");
    }
    /* See if there are any leaked memory block buffers. */
    if (NULL != a_pezz->spare_buffers)
    {
      cw_uint32_t i;
      cw_ring_t * t_ring;

      t_ring = a_pezz->spare_buffers;
      i = 0;
      do
      {
	t_ring = ring_next(t_ring);
	i++;
      } while (t_ring != a_pezz->spare_buffers);

      if (i < a_pezz->num_buffers)
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%lu leaked buffer%s\n",
		    a_pezz->num_buffers - i,
		    ((a_pezz->num_buffers - i) != 1) ? "s" : "");
      }
    }
    else
    {
      /* All the memory buffers are leaked! */
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "%lu leaked buffer%s (all of them)\n",
		  a_pezz->num_buffers,
		  (a_pezz->num_buffers != 1) ? "s" : "");
    }
  }
  if (dbg_is_registered(cw_g_dbg, "pezz_verbose"))
  {
    if (0 < a_pezz->max_overflow)
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "Maximum overflow was %lu\n", a_pezz->max_overflow);
    }
  }
#endif

  _cw_free(a_pezz->mem_base);

#ifdef _CW_REENTRANT
  mtx_delete(&a_pezz->lock);
#endif

#ifdef _LIBSTASH_DBG
  a_pezz->magic = 0;
#endif
  
  if (TRUE == a_pezz->is_malloced)
  {
    _cw_free(a_pezz);
  }
}

void *
pezz_get(cw_pezz_t * a_pezz)
{
  void * retval;

  _cw_check_ptr(a_pezz);
  _cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
#ifdef _CW_REENTRANT
  mtx_lock(&a_pezz->lock);
#endif

  if (a_pezz->spare_buffers != NULL)
  {
    cw_ring_t * t_ring;
    
    t_ring = a_pezz->spare_buffers;
    a_pezz->spare_buffers = ring_cut(t_ring);
    if (a_pezz->spare_buffers == t_ring)
    {
      /* This was the last element in the ring. */
      a_pezz->spare_buffers = NULL;
    }
    retval = ring_get_data(t_ring);
    _cw_assert(retval == (_CW_PEZZ_RING_SPACE + (void *) t_ring));
  }
  else
  {
    /* No buffers available.  malloc() one. */
#ifdef _LIBSTASH_DBG
    a_pezz->num_overflow++;
    if (a_pezz->num_overflow > a_pezz->max_overflow)
    {
      a_pezz->max_overflow = a_pezz->num_overflow;
    }
#endif

    retval = _cw_malloc(a_pezz->buffer_size);
  }
  
#ifdef _CW_REENTRANT
  mtx_unlock(&a_pezz->lock);
#endif
  return retval;
}

void
pezz_put(cw_pezz_t * a_pezz, void * a_buffer)
{
  _cw_check_ptr(a_pezz);
  _cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
#ifdef _CW_REENTRANT
  mtx_lock(&a_pezz->lock);
#endif

  if ((a_buffer > a_pezz->mem_base) && (a_buffer < a_pezz->mem_end))
  {
    cw_ring_t * t_ring;
    
    /* a_buffer was allocated from the memory block. */
    t_ring = (cw_ring_t *) (a_buffer - _CW_PEZZ_RING_SPACE);
    _cw_assert(ring_get_data(t_ring) == a_buffer);

    if (NULL != a_pezz->spare_buffers)
    {
      ring_meld(a_pezz->spare_buffers, t_ring);
    }
    else
    {
      a_pezz->spare_buffers = t_ring;
    }
  }
  else
  {
    /* a_buffer was malloc()ed. */
#ifdef _LIBSTASH_DBG
    a_pezz->num_overflow--;
#endif
    _cw_free(a_buffer);
  }
#ifdef _CW_REENTRANT
  mtx_unlock(&a_pezz->lock);
#endif
}

void
pezz_dump(cw_pezz_t * a_pezz, const char * a_prefix)
{
  _cw_check_ptr(a_pezz);
  _cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
#ifdef _CW_REENTRANT
  mtx_lock(&a_pezz->lock);
#endif

  log_printf(cw_g_log, "%sstart ==========================================\n",
	     a_prefix);
  log_printf(cw_g_log, "%smem_base : %p\n",
	     a_prefix, a_pezz->mem_base);
  log_printf(cw_g_log, "%smem_end : %p\n",
	     a_prefix, a_pezz->mem_end);
  log_printf(cw_g_log, "%sinternal_buffer_size : %lu\n",
	     a_prefix, a_pezz->internal_buffer_size);
  log_printf(cw_g_log, "%sbuffer_size : %lu\n",
	     a_prefix, a_pezz->buffer_size);
  log_printf(cw_g_log, "%snum_buffers %lu\n",
	     a_prefix, a_pezz->num_buffers);
  if (NULL != a_pezz->spare_buffers)
  {
    char * prefix = (char *) _cw_malloc(strlen(a_prefix) + 17);

    log_printf(cw_g_log, "%sspare_buffers : \n",
	       a_prefix);
    sprintf(prefix,    "%s              : ", a_prefix);
    
    ring_dump(a_pezz->spare_buffers, prefix);

    _cw_free(prefix);
  }
  else
  {
    log_printf(cw_g_log, "%sspare_buffers : (null)\n",
	       a_prefix);
  }

#ifdef _LIBSTASH_DBG
  log_printf(cw_g_log, "%snum_overflow : %lu\n",
	     a_prefix, a_pezz->num_overflow);
  log_printf(cw_g_log, "%smax_overflow : %lu\n",
	     a_prefix, a_pezz->max_overflow);
#endif
  
  log_printf(cw_g_log, "%send ============================================\n",
	     a_prefix);

#ifdef _CW_REENTRANT
  mtx_unlock(&a_pezz->lock);
#endif
}

