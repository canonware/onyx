/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
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

cw_pezz_t *
pezz_new(cw_pezz_t * a_pezz, cw_uint32_t a_buffer_size,
	 cw_uint32_t a_num_buffers)
{
  cw_pezz_t * retval;

  _cw_assert(0 != (a_buffer_size * a_num_buffers));

  if (NULL == a_pezz)
  {
    retval = (cw_pezz_t *) _cw_malloc(sizeof(cw_pezz_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    bzero(retval, sizeof(cw_pezz_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_pezz;
    bzero(retval, sizeof(cw_pezz_t));
    retval->is_malloced = FALSE;
  }

#ifdef _CW_REENTRANT
  mtx_new(&retval->lock);
#endif

  retval->buffer_size = a_buffer_size;
  retval->num_buffers = a_num_buffers;
  
  /* Calculate how big to make the memory block and allocate it. */
  retval->rings
    = (cw_ring_t *) _cw_calloc(retval->num_buffers, sizeof(cw_ring_t));
  if (NULL == retval->rings)
  {
    if (retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }
  
  retval->mem_base
    = (void *) _cw_calloc(retval->num_buffers, retval->buffer_size);
  if (NULL == retval->mem_base)
  {
    _cw_free(retval->rings);
    if (retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }
  
  retval->mem_end = (retval->mem_base
		     + (retval->buffer_size * retval->num_buffers));

  /* Iterate through the buffers, initialize their ring elements, and insert
   * them into the spare buffers ring. */
  {
    cw_uint32_t i;
    cw_ring_t * ring;

    /* Initialize spare buffers ring to have something in it. */
    retval->spare_buffers = &retval->rings[0];
    ring_new(retval->spare_buffers, NULL, NULL);
    ring_set_data(retval->spare_buffers, retval->mem_base);
    
    for (i = 1; i < retval->num_buffers; i++)
    {
      ring = &retval->rings[i];
      ring_new(ring, NULL, NULL);
      ring_set_data(ring, retval->mem_base + (i * retval->buffer_size));
      ring_meld(retval->spare_buffers, ring);
    }
  }

#ifdef _LIBSTASH_DBG
  retval->magic = _CW_PEZZ_MAGIC;
#endif

  RETURN:
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
  _cw_free(a_pezz->rings);

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

cw_uint32_t
pezz_get_buffer_size(cw_pezz_t * a_pezz)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_pezz);
  _cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);

  retval = a_pezz->buffer_size;

  return retval;
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
  }
  else
  {
    /* No buffers available.  malloc() one. */
    retval = _cw_malloc(a_pezz->buffer_size);
#ifdef _LIBSTASH_DBG
    if (NULL != retval)
    {
      a_pezz->num_overflow++;
      if (a_pezz->num_overflow > a_pezz->max_overflow)
      {
	a_pezz->max_overflow = a_pezz->num_overflow;
      }
    }
#endif
  }
  
#ifdef _CW_REENTRANT
  mtx_unlock(&a_pezz->lock);
#endif
  return retval;
}

void
pezz_put(void * a_pezz, void * a_buffer)
{
  cw_pezz_t * pezz = (cw_pezz_t *) a_pezz;
  
  _cw_check_ptr(pezz);
  _cw_assert(pezz->magic == _CW_PEZZ_MAGIC);
#ifdef _CW_REENTRANT
  mtx_lock(&pezz->lock);
#endif

  if ((a_buffer >= pezz->mem_base) && (a_buffer < pezz->mem_end))
  {
    cw_ring_t * t_ring;
    
    /* a_buffer was allocated from the memory block. */
    t_ring = &pezz->rings[(a_buffer - pezz->mem_base)
			   / pezz->buffer_size];
    _cw_assert(ring_get_data(t_ring) == a_buffer);

    if (NULL != pezz->spare_buffers)
    {
      ring_meld(pezz->spare_buffers, t_ring);
    }
    else
    {
      pezz->spare_buffers = t_ring;
    }
  }
  else
  {
    /* a_buffer was malloc()ed. */
#ifdef _LIBSTASH_DBG
    pezz->num_overflow--;
#endif
    _cw_free(a_buffer);
  }
#ifdef _CW_REENTRANT
  mtx_unlock(&pezz->lock);
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
  log_printf(cw_g_log, "%srings : %p\n",
	     a_prefix, a_pezz->rings);
  log_printf(cw_g_log, "%sbuffer_size : %lu\n",
	     a_prefix, a_pezz->buffer_size);
  log_printf(cw_g_log, "%snum_buffers %lu\n",
	     a_prefix, a_pezz->num_buffers);
  if (NULL != a_pezz->spare_buffers)
  {
    char * prefix = (char *) _cw_malloc(strlen(a_prefix) + 17);

    log_printf(cw_g_log, "%sspare_buffers : \n",
	       a_prefix);

    if (NULL == prefix)
    {
      sprintf(prefix,    "%s              : ", a_prefix);
    }
    else
    {
      prefix = (char *) a_prefix;
    }
    
    ring_dump(a_pezz->spare_buffers, prefix);

    if (NULL != prefix)
    {
      _cw_free(prefix);
    }
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
