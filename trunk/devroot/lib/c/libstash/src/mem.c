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
 *
 *
 ****************************************************************************/

#include "libstash/libstash.h"

#include "libstash/mem_p.h"
#include "libstash/mem_l.h"

cw_mem_t *
mem_new()
{
  cw_mem_t * retval;

  retval = (cw_mem_t *) _cw_malloc(sizeof(cw_mem_t));
  _cw_check_ptr(retval);

#ifdef _LIBSTASH_DBG
#  ifdef _CW_REENTRANT
  oh_new(&retval->addr_hash, TRUE);
#  else
  oh_new(&retval->addr_hash);
#  endif
  oh_set_h1(&retval->addr_hash, mem_p_oh_h1);
  oh_set_key_compare(&retval->addr_hash, mem_p_oh_key_compare);
#endif

  return retval;
}

void
mem_delete(cw_mem_t * a_mem)
{
  _cw_check_ptr(a_mem);

#ifdef _LIBSTASH_DBG
  {
    cw_uint64_t i, num_addrs;
    void * addr, * junk;
    char buf[21];

    if ((dbg_is_registered(cw_g_dbg, "mem_verbose"))
	|| ((dbg_is_registered(cw_g_dbg, "mem_error"))
	    && (0 < oh_get_num_items(&a_mem->addr_hash))))
    {
      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		  "%s unfreed allocations\n",
		  log_print_uint64(oh_get_num_items(&a_mem->addr_hash),
				   10,
				   buf));
    }
    for (i = 0,
	   num_addrs = oh_get_num_items(&a_mem->addr_hash);
	 i < num_addrs;
	 i++)
    {
      oh_item_delete_iterate(&a_mem->addr_hash,
			     &addr, &junk);
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "memory at %p never freed\n", addr);
      }
    }
    oh_delete(&a_mem->addr_hash);
  }
#endif
  
  _cw_free(a_mem);
}

void *
mem_malloc(cw_mem_t * a_mem, size_t a_size)
{
  void * retval;
  
/*    _cw_check_ptr(a_mem); */

  retval = _cw_malloc(a_size);
  if (retval == NULL)
  {
    log_eprintf(cw_g_log, __FILE__, __LINE__, "mem_malloc",
		"malloc(%d) returned NULL\n", a_size);
    abort();
  }

#ifdef _LIBSTASH_DBG
  bzero(retval, a_size);
#endif

  if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"%p <-- malloc(%u)\n", retval, a_size);
  }

#ifdef _LIBSTASH_DBG
  if (NULL != a_mem)
  {
    if (TRUE == oh_item_insert(&a_mem->addr_hash, retval, NULL))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p multiply-allocated\n", retval);
      }
    }
  }
#endif
  
  return retval;
}

void *
mem_calloc(cw_mem_t * a_mem, size_t a_number,
	   size_t a_size)
{
  void * retval;

/*    _cw_check_ptr(a_mem); */

  retval = _cw_calloc(a_number, a_size);
  if (retval == NULL)
  {
    log_eprintf(cw_g_log, __FILE__, __LINE__, "mem_calloc",
		"calloc(%d, %d) returned NULL\n", a_number, a_size);
    abort();
  }

#ifdef _LIBSTASH_DBG
  bzero(retval, a_number * a_size);
#endif
  
  if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"%p <-- calloc(%u, %u)\n", retval, a_number, a_size);
  }

#ifdef _LIBSTASH_DBG
  if (NULL != a_mem)
  {
    if (TRUE == oh_item_insert(&a_mem->addr_hash, retval, NULL))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p multiply-allocated\n", retval);
      }
    }
  }
#endif
  
  return retval;
}

void *
mem_realloc(cw_mem_t * a_mem, void * a_ptr, size_t a_size)
{
  void * retval;

/*    _cw_check_ptr(a_mem); */
  _cw_check_ptr(a_ptr);

  retval = _cw_realloc(a_ptr, a_size);
  if (retval == NULL)
  {
    log_eprintf(cw_g_log, __FILE__, __LINE__, "mem_realloc",
		"realloc(%p, %d) returned NULL\n", a_ptr, a_size);
    abort();
  }
  
  if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"%p <-- realloc(%p, %u)\n", retval, a_ptr, a_size);
  }

#ifdef _LIBSTASH_DBG
  if (NULL != a_mem)
  {
    void * junk_a, * junk_b;
    
    if (TRUE == oh_item_delete(&a_mem->addr_hash, a_ptr, &junk_a, &junk_b))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p not allocated\n", a_ptr);
      }
    }
    if (TRUE == oh_item_insert(&a_mem->addr_hash, retval, NULL))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p multiply-allocated\n", retval);
      }
    }
  }
#endif

  return retval;
}

void
mem_free(cw_mem_t * a_mem, void * a_ptr)
{
/*    _cw_check_ptr(a_mem); */
  _cw_check_ptr(a_ptr);

  _cw_free(a_ptr);
  
  if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"free(%p)\n", a_ptr);
  }

#ifdef _LIBSTASH_DBG
  if (NULL != a_mem)
  {
    void * junk_a, * junk_b;
    
    if (TRUE == oh_item_delete(&a_mem->addr_hash, a_ptr, &junk_a, &junk_b))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p not allocated\n", a_ptr);
      }
    }
  }
#endif
}

void
mem_dealloc(void * a_mem, void * a_ptr)
{
  mem_free((cw_mem_t *) a_mem, a_ptr);
}

#ifdef _LIBSTASH_DBG
static cw_uint64_t
mem_p_oh_h1(cw_oh_t * a_oh, const void * a_key)
{
  cw_uint64_t retval, key = (cw_uint32_t) a_key;

  retval = key >> 4;

  return retval;
}

static cw_bool_t
mem_p_oh_key_compare(const void * a_k1, const void * a_k2)
{
  return (a_k1 == a_k2) ?  TRUE : FALSE;
}
#endif
