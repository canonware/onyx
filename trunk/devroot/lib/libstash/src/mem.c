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
    void * addr;
    struct cw_mem_item_s * allocation;
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
			     &addr, (void **) &allocation);
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p, size %u never freed (allocated at %s, line %d)\n",
		    addr, allocation->size,
		    ((NULL == allocation->filename)
		     ? "<?>" : allocation->filename),
		    allocation->line_num);
      }
    }
    oh_delete(&a_mem->addr_hash);
  }
#endif
  
  _cw_free(a_mem);
}

#ifdef _LIBSTASH_DBG
void *
mem_malloc(cw_mem_t * a_mem, size_t a_size, const char * a_filename,
	   cw_uint32_t a_line_num)
#else
void *
mem_malloc(cw_mem_t * a_mem, size_t a_size)
#endif
{
  void * retval;
  
  retval = _cw_malloc(a_size);
  
#ifdef _LIBSTASH_DBG
  if (NULL != a_mem)
  {
    struct cw_mem_item_s * old_allocation;
    
    if (FALSE == oh_item_search(&a_mem->addr_hash,
				a_mem,
				(void **) &old_allocation))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	_cw_assert(old_allocation != NULL);
	
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p multiply-allocated (was at %s, line %d, size %u;"
		    " now at %s, line %d, size %u)\n",
		    retval,
		    ((NULL == old_allocation->filename)
		     ? "<?>" : old_allocation->filename),
		    old_allocation->line_num,
		    old_allocation->size,
		    (NULL == a_filename) ? "<?>" : a_filename,
		    a_line_num,
		    a_size);
      }
    }
    else
    {
      struct cw_mem_item_s * allocation;

      allocation = _cw_malloc(sizeof(struct cw_mem_item_s));
      if (allocation == NULL)
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "malloc(%d) returned NULL\n", sizeof(struct cw_mem_item_s));
	abort();
      }
	
      allocation->size = a_size;
      allocation->filename = a_filename;
      allocation->line_num = a_line_num;
      
      if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p <-- malloc(%u) at %s, line %d\n", retval, a_size,
		    (NULL == a_filename) ? "<?>" : a_filename, a_line_num);
      }
      
      _cw_assert(FALSE == oh_item_insert(&a_mem->addr_hash, retval,
					 allocation));
    }
  }
#else
  if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"%p <-- malloc(%u)\n", retval, a_size);
  }
#endif
  
  if (retval == NULL)
  {
    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		"malloc(%d) returned NULL\n", a_size);
    abort();
  }

#ifdef _LIBSTASH_DBG
  bzero(retval, a_size);
#endif
  
  return retval;
}

#ifdef _LIBSTASH_DBG
void *
mem_calloc(cw_mem_t * a_mem, size_t a_number, size_t a_size,
	   const char * a_filename, cw_uint32_t a_line_num)
#else
void *
mem_calloc(cw_mem_t * a_mem, size_t a_number, size_t a_size)
#endif
{
  void * retval;

  retval = _cw_calloc(a_number, a_size);

#ifdef _LIBSTASH_DBG
  if (NULL != a_mem)
  {
    struct cw_mem_item_s * old_allocation;
    
    if (FALSE == oh_item_search(&a_mem->addr_hash,
				a_mem,
				(void **) &old_allocation))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	_cw_assert(old_allocation != NULL);
	
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p multiply-allocated (was at %s, line %d, size %u;"
		    " now at %s, line %d, size %u)\n",
		    retval,
		    ((NULL == old_allocation->filename)
		     ? "<?>" : old_allocation->filename),
		    old_allocation->line_num,
		    old_allocation->size,
		    (NULL == a_filename) ? "<?>" : a_filename,
		    a_line_num,
		    a_size);
      }
    }
    else
    {
      struct cw_mem_item_s * allocation;

      allocation = _cw_malloc(sizeof(struct cw_mem_item_s));
      if (allocation == NULL)
      {
	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		    "malloc(%d) returned NULL\n", sizeof(struct cw_mem_item_s));
	abort();
      }
	
      allocation->size = a_size;
      allocation->filename = a_filename;
      allocation->line_num = a_line_num;
      
      if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p <-- calloc(%u, %u) at %s, line %d\n",
		    retval, a_number, a_size,
		    (NULL == a_filename) ? "<?>" : a_filename, a_line_num);
      }
      
      _cw_assert(FALSE == oh_item_insert(&a_mem->addr_hash, retval,
					 allocation));
    }
  }
#else
  if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"%p <-- calloc(%u, %u)\n", retval, a_number, a_size);
  }
#endif

  if (retval == NULL)
  {
    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		"calloc(%d, %d) returned NULL\n", a_number, a_size);
    abort();
  }

#ifdef _LIBSTASH_DBG
  bzero(retval, a_number * a_size);
#endif
  
  return retval;
}

#ifdef _LIBSTASH_DBG
void *
mem_realloc(cw_mem_t * a_mem, void * a_ptr, size_t a_size,
	    const char * a_filename, cw_uint32_t a_line_num)
#else
void *
mem_realloc(cw_mem_t * a_mem, void * a_ptr, size_t a_size)
#endif
{
  void * retval;

  _cw_check_ptr(a_ptr);

  retval = _cw_realloc(a_ptr, a_size);
  
#ifdef _LIBSTASH_DBG
  if (NULL != a_mem)
  {
    void * junk;
    struct cw_mem_item_s * allocation;
    
    if (TRUE == oh_item_delete(&a_mem->addr_hash, a_ptr, &junk,
			       (void **) &allocation))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p not allocated\n", a_ptr);
      }
    }
    else
    {
      const char * old_filename;
      cw_uint32_t old_size, old_line_num;

      /* Do this before inserting into the hash table, since only the hash table
       * is thread-safe. */
      old_filename = allocation->filename;
      old_size = allocation->size;
      old_line_num = allocation->line_num;
      allocation->filename = a_filename;
      allocation->size = a_size;
      allocation->line_num = a_line_num;
      
      if (TRUE == oh_item_insert(&a_mem->addr_hash, retval, allocation))
      {
	if (dbg_is_registered(cw_g_dbg, "mem_error"))
	{
	  log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		      "%p multiply-allocated\n", retval);
	}
      }

      if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "reallocing %p"
		    " (was size %u, allocated at %s, line %d)"
		    " to %p, size %u at %s, line %d\n",
		    a_ptr,
		    old_size,
		    ((NULL == old_filename)
		     ? "<?>" : old_filename),
		    old_line_num,
		    retval,
		    a_size,
		    (NULL == a_filename) ? "<?>" : a_filename,
		    a_line_num);

	allocation->size = a_size;
	allocation->filename = a_filename;
	allocation->line_num = a_line_num;
      }
    }
  }
#else
  if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"%p <-- realloc(%p, %u)\n", retval, a_ptr, a_size);
  }
#endif
  
  if (retval == NULL)
  {
    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		"realloc(%p, %d) returned NULL\n", a_ptr, a_size);
    abort();
  }
  
  return retval;
}

#ifdef _LIBSTASH_DBG
void
mem_free(cw_mem_t * a_mem, void * a_ptr,
	 const char * a_filename, cw_uint32_t a_line_num)
#else
void
mem_free(cw_mem_t * a_mem, void * a_ptr)
#endif
{
#ifdef _LIBSTASH_DBG
  if (NULL != a_mem)
  {
    void * junk;
    struct cw_mem_item_s * allocation;
    
    if (TRUE == oh_item_delete(&a_mem->addr_hash, a_ptr, &junk,
			       (void **) &allocation))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "%p not allocated, attempted to free at %s, line %d\n",
		    a_ptr,
		    (NULL == a_filename) ? "<?>" : a_filename,
		    a_line_num);
      }
    }
    else
    {
      if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
      {
	log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		    "Freeing %p, size %u, at %s, line %d "
		    "(allocated at %s, line %d)\n",
		    a_ptr,
		    allocation->size,
		    (NULL == a_filename) ? "<?>" : a_filename,
		    a_line_num,
		    ((NULL == allocation->filename)
		     ? "<?>" : allocation->filename),
		    allocation->line_num);
      }
      _cw_free(allocation);
    }
  }
#else
  if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
  {
    log_eprintf(cw_g_log, NULL, 0, __FUNCTION__,
		"free(%p)\n", a_ptr);
  }
#endif

  _cw_free(a_ptr);
}

void
mem_dealloc(void * a_mem, void * a_ptr)
{
#ifdef _LIBSTASH_DBG
  mem_free((cw_mem_t *) a_mem, a_ptr, NULL, 0);
#else
  mem_free((cw_mem_t *) a_mem, a_ptr);
#endif
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
