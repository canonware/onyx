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
  retval->block_num_buffers = a_num_buffers;

  /* Allocate and initialize first block. */
  retval->mem_blocks = (void **) _cw_calloc(1, sizeof(void *));
  if (NULL == retval->mem_blocks)
  {
    if (retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }
  retval->ring_blocks = (cw_ring_t **) _cw_calloc(1, sizeof(cw_ring_t *));
  if (NULL == retval->ring_blocks)
  {
    _cw_free(retval->mem_blocks);
    if (retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }
  retval->mem_blocks[0] = (void *) _cw_calloc(retval->block_num_buffers,
					      retval->buffer_size);
  if (NULL == retval->mem_blocks[0])
  {
    _cw_free(retval->ring_blocks);
    _cw_free(retval->mem_blocks);
    if (retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }
  retval->ring_blocks[0] = (cw_ring_t *) _cw_calloc(retval->block_num_buffers,
						    sizeof(cw_ring_t));
  if (NULL == retval->ring_blocks[0])
  {
    _cw_free(retval->mem_blocks[0]);
    _cw_free(retval->ring_blocks);
    _cw_free(retval->mem_blocks);
    if (retval->is_malloced)
    {
      _cw_free(retval);
    }
    retval = NULL;
    goto RETURN;
  }
  {
    cw_uint32_t i;
    cw_ring_t * t_ring;

    /* Initialize spare_buffers to have something in it. */
    retval->spare_buffers = retval->ring_blocks[0];
    ring_new(retval->spare_buffers, NULL, NULL);
    ring_set_data(retval->spare_buffers, retval->mem_blocks[0]);
    
    for (i = 1; i < retval->block_num_buffers; i++)
    {
      t_ring = &retval->ring_blocks[0][i];
      ring_new(t_ring, NULL, NULL);
      ring_set_data(t_ring, retval->mem_blocks[0] + (i * retval->buffer_size));
      ring_meld(retval->spare_buffers, t_ring);
    }
  }
  retval->num_blocks = 1;
  
#ifdef _LIBSTASH_DBG
  retval->magic = _CW_PEZZ_MAGIC;
#endif

  RETURN:
  return retval;
}

void
pezz_delete(cw_pezz_t * a_pezz)
{
  cw_uint32_t i;
  
  _cw_check_ptr(a_pezz);
  _cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);

#ifdef _LIBSTASH_DBG
  if (dbg_is_registered(cw_g_dbg, "pezz_error"))
  {
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

      if (i < (a_pezz->num_blocks * a_pezz->block_num_buffers))
      {
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		  "[i32] leaked buffer[s]\n",
		  (a_pezz->num_blocks * a_pezz->block_num_buffers) - i,
		  (((a_pezz->num_blocks
		     * a_pezz->block_num_buffers) - i) != 1) ? "s" : "");
      }
    }
    else
    {
      /* All the memory buffers are leaked! */
      out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
		"[i32] leaked buffer[s] (all of them)\n",
		(a_pezz->num_blocks * a_pezz->block_num_buffers),
		((a_pezz->num_blocks * a_pezz->block_num_buffers) != 1)
		? "s" : "");
    }
  }
#endif

  for (i = 0; i < a_pezz->num_blocks; i++)
  {
    _cw_free(a_pezz->mem_blocks[i]);
    _cw_free(a_pezz->ring_blocks[i]);
  }
  _cw_free(a_pezz->mem_blocks);
  _cw_free(a_pezz->ring_blocks);

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

  if (a_pezz->spare_buffers == NULL)
  {
    void ** t_mem_blocks;
    cw_ring_t ** t_ring_blocks;
    
    /* No buffers available.  Add a block. */
    t_mem_blocks = (void **) _cw_realloc(a_pezz->mem_blocks,
					 ((a_pezz->num_blocks + 1)
					  * sizeof(void *)));
    if (NULL == t_mem_blocks)
    {
      retval = NULL;
      goto RETURN;
    }
    a_pezz->mem_blocks = t_mem_blocks;
    
    t_ring_blocks = (cw_ring_t **) _cw_realloc(a_pezz->ring_blocks,
					       ((a_pezz->num_blocks + 1)
						* sizeof(cw_ring_t *)));
    if (NULL == t_ring_blocks)
    {
      retval = NULL;
      goto RETURN;
    }
    a_pezz->ring_blocks = t_ring_blocks;
    
    a_pezz->mem_blocks[a_pezz->num_blocks]
      = (void *) _cw_calloc(a_pezz->block_num_buffers,
			    a_pezz->buffer_size);
    if (NULL == a_pezz->mem_blocks[a_pezz->num_blocks])
    {
      retval = NULL;
      goto RETURN;
    }
    
    a_pezz->ring_blocks[a_pezz->num_blocks]
      = (cw_ring_t *) _cw_calloc(a_pezz->block_num_buffers,
				 sizeof(cw_ring_t));
    if (NULL == a_pezz->ring_blocks[a_pezz->num_blocks])
    {
      _cw_free(a_pezz->mem_blocks[a_pezz->num_blocks]);
      retval = NULL;
      goto RETURN;
    }

    /* All of the allocation succeeded. */
    {
      cw_uint32_t i;
      cw_ring_t * t_ring;

      /* Initialize spare_buffers to have something in it. */
      a_pezz->spare_buffers = a_pezz->ring_blocks[a_pezz->num_blocks];
      ring_new(a_pezz->spare_buffers, NULL, NULL);
      ring_set_data(a_pezz->spare_buffers,
		    a_pezz->mem_blocks[a_pezz->num_blocks]);

      for (i = 1; i < a_pezz->block_num_buffers; i++)
      {
	t_ring = &a_pezz->ring_blocks[a_pezz->num_blocks][i];
	ring_new(t_ring, NULL, NULL);
	ring_set_data(t_ring,
		      (a_pezz->mem_blocks[a_pezz->num_blocks]
		       + (i * a_pezz->buffer_size)));
	ring_meld(a_pezz->spare_buffers, t_ring);
      }
    }

    /* Do this last so that num_blocks can be used as an index above. */
    a_pezz->num_blocks++;
  }

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
    if (NULL != a_pezz->spare_rings)
    {
      ring_meld(a_pezz->spare_rings, t_ring);
    }

    /* Do this even if we just did a ring_meld() in order to make the ring act
     * like a stack, hopefully improving cache locality. */
    a_pezz->spare_rings = t_ring;
  }

  RETURN:
#ifdef _CW_REENTRANT
  mtx_unlock(&a_pezz->lock);
#endif
  return retval;
}

void
pezz_put(void * a_pezz, void * a_buffer)
{
  cw_pezz_t * pezz = (cw_pezz_t *) a_pezz;
  cw_ring_t * t_ring;
  
  _cw_check_ptr(pezz);
  _cw_assert(pezz->magic == _CW_PEZZ_MAGIC);
#ifdef _CW_REENTRANT
  mtx_lock(&pezz->lock);
#endif

  t_ring = pezz->spare_rings;
  pezz->spare_rings = ring_cut(t_ring);
  if (pezz->spare_rings == t_ring)
  {
    /* spare_rings is empty. */
    pezz->spare_rings = NULL;
  }

  ring_set_data(t_ring, a_buffer);

  if (NULL != pezz->spare_buffers)
  {
    ring_meld(t_ring, pezz->spare_buffers);
  }
  pezz->spare_buffers = t_ring;
  
#ifdef _CW_REENTRANT
  mtx_unlock(&pezz->lock);
#endif
}

void
pezz_dump(cw_pezz_t * a_pezz, const char * a_prefix)
{
  cw_uint32_t i;
  
  _cw_check_ptr(a_pezz);
  _cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
#ifdef _CW_REENTRANT
  mtx_lock(&a_pezz->lock);
#endif

  out_put(cw_g_out, "[s]start ==========================================\n",
	  a_prefix);
  out_put(cw_g_out, "[s]buffer_size : [i32]\n",
	  a_prefix, a_pezz->buffer_size);
  out_put(cw_g_out, "[s]block_num_buffers : [i32]\n",
	  a_prefix, a_pezz->block_num_buffers);
  out_put(cw_g_out, "[s]num_blocks : [i32]\n",
	  a_prefix, a_pezz->num_blocks);

  out_put(cw_g_out, "[s]mem_blocks : 0x[p]\n",
	  a_prefix, a_pezz->mem_blocks);
  out_put(cw_g_out, "[s]ring_blocks : 0x[p]\n",
	  a_prefix, a_pezz->ring_blocks);
  
  for (i = 0; i < a_pezz->num_blocks; i++);
  {
    out_put(cw_g_out,
	    "[s]mem_blocks[[[i32]] : 0x[p], ring_blocks[[[i32]] : 0x[p]\n",
	    a_prefix, i, a_pezz->mem_blocks[i], i, a_pezz->ring_blocks[i]);
  }
  
  if (NULL != a_pezz->spare_buffers)
  {
    char * prefix = (char *) _cw_malloc(strlen(a_prefix) + 17);

    out_put(cw_g_out, "[s]spare_buffers : \n",
	    a_prefix);

    if (NULL != prefix)
    {
      out_put_s(cw_g_out, prefix,    "[s]              : ", a_prefix);
      ring_dump(a_pezz->spare_buffers, prefix);
      _cw_free(prefix);
    }
    else
    {
      prefix = (char *) a_prefix;
      ring_dump(a_pezz->spare_buffers, prefix);
    }
  }
  else
  {
    out_put(cw_g_out, "[s]spare_buffers : (null)\n",
	    a_prefix);
  }

  out_put(cw_g_out, "[s]end ============================================\n",
	  a_prefix);

#ifdef _CW_REENTRANT
  mtx_unlock(&a_pezz->lock);
#endif
}
