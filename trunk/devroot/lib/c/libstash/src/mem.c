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
 * Note that the esoteric usage of out_put_*() is necessary in order to avoid
 * infinite allocation loops.
 *
 ****************************************************************************/

#include "libstash/libstash.h"

#include "libstash/mem_p.h"
#include "libstash/mem_l.h"

cw_mem_t *
mem_new(void)
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
  oh_set_h1(&retval->addr_hash, oh_h1_direct);
  oh_set_key_compare(&retval->addr_hash, oh_key_compare_direct);
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

    if ((dbg_is_registered(cw_g_dbg, "mem_verbose"))
	|| ((dbg_is_registered(cw_g_dbg, "mem_error"))
	    && (0 < oh_get_num_items(&a_mem->addr_hash))))
    {
      char buf[1025];

      bzero(buf, sizeof(buf));
      out_put_sn(cw_g_out, buf, 1024,
		 "[s](): [i64] unfreed allocation[s]\n",
		 __FUNCTION__, oh_get_num_items(&a_mem->addr_hash),
		 oh_get_num_items(&a_mem->addr_hash) != 1 ? "s" : "");
      out_put(cw_g_out, buf);
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
	char buf[1025];

	bzero(buf, sizeof(buf));
	out_put_sn(cw_g_out, buf, 1024,
		   "[s](): 0x[p], size [i32] never freed "
		   "(allocated at [s], line [i32])\n",
		   __FUNCTION__, addr, allocation->size,
		   ((NULL == allocation->filename)
		    ? "<?>" : allocation->filename),
		   allocation->line_num);
	out_put(cw_g_out, buf);
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

  _cw_assert(a_size > 0);
  
  retval = _cw_malloc(a_size);
  
#ifdef _LIBSTASH_DBG
  if (NULL == retval)
  {
    if (dbg_is_registered(cw_g_dbg, "mem_error"))
    {
      char buf[1025];

      bzero(buf, sizeof(buf));
      out_put_sn(cw_g_out, buf, 1024,
		 "[s](): malloc([i32]) returned NULL at [s], line [i32]\n",
		 __FUNCTION__, a_size, a_filename, a_line_num);
      out_put(cw_g_out, buf);
    }
  }
  else if (NULL != a_mem)
  {
    struct cw_mem_item_s * old_allocation;
    
    if (FALSE == oh_item_search(&a_mem->addr_hash,
				a_mem,
				(void **) &old_allocation))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	char buf[1025];

	_cw_check_ptr(old_allocation);
	
	bzero(buf, sizeof(buf));
	out_put_sn(cw_g_out, buf, 1024,
		   "[s](): 0x[p] multiply-allocated "
		   "(was at [s], line [i32], size [i32];"
		   " now at [s], line [i32], size [i32])\n",
		   __FUNCTION__, retval,
		   ((NULL == old_allocation->filename)
		    ? "<?>" : old_allocation->filename),
		   old_allocation->line_num,
		   old_allocation->size,
		   (NULL == a_filename) ? "<?>" : a_filename,
		   a_line_num,
		   a_size);
	out_put(cw_g_out, buf);
      }
    }
    else
    {
      struct cw_mem_item_s * allocation;

      allocation = _cw_malloc(sizeof(struct cw_mem_item_s));
      if (allocation == NULL)
      {
	char buf[1025];

	bzero(buf, sizeof(buf));
	out_put_sn(cw_g_out, buf, 1024,
		   "[s](): malloc([i32]) returned NULL\n",
		   __FUNCTION__, sizeof(struct cw_mem_item_s));
	out_put(cw_g_out, buf);
      }
      else
      {
	memset(retval, 0xa5, a_size);
	
	allocation->size = a_size;
	allocation->filename = a_filename;
	allocation->line_num = a_line_num;
      
	if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
	{
	  char buf[1025];

	  bzero(buf, sizeof(buf));
	  out_put_sn(cw_g_out, buf, 1024,
		     "[s](): 0x[p] <-- malloc([i32]) at [s], line [i32]\n",
		     __FUNCTION__, retval, a_size,
		     (NULL == a_filename) ? "<?>" : a_filename, a_line_num);
	  out_put(cw_g_out, buf);
	}

	_cw_assert(0 == oh_item_insert(&a_mem->addr_hash, retval,
				       allocation));
      }
    }
  }
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

  _cw_assert(a_size * a_number > 0);
  
  retval = _cw_calloc(a_number, a_size);

#ifdef _LIBSTASH_DBG
  if (NULL == retval)
  {
    if (dbg_is_registered(cw_g_dbg, "mem_error"))
    {
      char buf[1025];

      bzero(buf, sizeof(buf));
      out_put_sn(cw_g_out, buf, 1024,
		 "[s](): calloc([i32], [i32]) returned NULL "
		 "at [s], line [i32]\n",
		 __FUNCTION__, a_number, a_size, a_filename, a_line_num);
      out_put(cw_g_out, buf);
    }
  }
  else if (NULL != a_mem)
  {
    struct cw_mem_item_s * old_allocation;
    
    if (FALSE == oh_item_search(&a_mem->addr_hash,
				a_mem,
				(void **) &old_allocation))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	char buf[1025];

	_cw_check_ptr(old_allocation);

	bzero(buf, sizeof(buf));
	out_put_sn(cw_g_out, buf, 1024,
		   "[s](): 0x[p] multiply-allocated "
		   "(was at [s], line [i32], size [i32];"
		   " now at [s], line [i32], size [i32])\n",
		   __FUNCTION__, retval,
		   ((NULL == old_allocation->filename)
		    ? "<?>" : old_allocation->filename),
		   old_allocation->line_num,
		   old_allocation->size,
		   (NULL == a_filename) ? "<?>" : a_filename,
		   a_line_num,
		   a_size);
	out_put(cw_g_out, buf);
      }
    }
    else
    {
      struct cw_mem_item_s * allocation;

      allocation = _cw_malloc(sizeof(struct cw_mem_item_s));
      if (allocation == NULL)
      {
	char buf[1025];

	bzero(buf, sizeof(buf));
	out_put_sn(cw_g_out, buf, 1024,
		   "[s](): malloc([i32]) returned NULL\n",
		   __FUNCTION__, sizeof(struct cw_mem_item_s));
	out_put(cw_g_out, buf);
      }
      else
      {
	memset(retval, 0xa5, a_number * a_size);
	
	allocation->size = a_number * a_size;
	allocation->filename = a_filename;
	allocation->line_num = a_line_num;
      
	if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
	{
	  char buf[1025];

	  bzero(buf, sizeof(buf));
	  out_put_sn(cw_g_out, buf, 1024,
		     "[s](): 0x[p] <-- calloc([i32], [i32]) "
		     "at [s], line [i32]\n",
		     __FUNCTION__, retval, a_number, a_size,
		     (NULL == a_filename) ? "<?>" : a_filename, a_line_num);
	  out_put(cw_g_out, buf);
	}
      
	_cw_assert(0 == oh_item_insert(&a_mem->addr_hash, retval,
				       allocation));
      }
    }
  }
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
  _cw_assert(a_size > 0);

  retval = _cw_realloc(a_ptr, a_size);
  
#ifdef _LIBSTASH_DBG
  if (NULL == a_mem)
  {
    char buf[1025];

    bzero(buf, sizeof(buf));
    out_put_sn(cw_g_out, buf, 1024,
	       "[s](): realloc(0x[p], [i32]) "
	       "returned NULL at [s], line [i32]\n",
	       __FUNCTION__, a_ptr, a_size,
	       a_filename, a_line_num);
    out_put(cw_g_out, buf);
  }
  else
  {
    void * junk;
    struct cw_mem_item_s * allocation;
    
    if (TRUE == oh_item_delete(&a_mem->addr_hash, a_ptr, &junk,
			       (void **) &allocation))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	char buf[1025];

	bzero(buf, sizeof(buf));
	out_put_sn(cw_g_out, buf, 1024,
		   "[s](): 0x[p] not allocated\n",
		   __FUNCTION__, a_ptr);
	out_put(cw_g_out, buf);
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
      
      if (1 == oh_item_insert(&a_mem->addr_hash, retval, allocation))
      {
	if (dbg_is_registered(cw_g_dbg, "mem_error"))
	{
	  char buf[1025];

	  bzero(buf, sizeof(buf));
	  out_put_sn(cw_g_out, buf, 1024,
		     "[s](): 0x[p] multiply-allocated\n",
		     __FUNCTION__, retval);
	  out_put(cw_g_out, buf);
	}
      }

      if (old_size < a_size)
      {
	memset(retval + old_size, 0xa5, a_size - old_size);
      }
      
      if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
      {
	char buf[1025];

	bzero(buf, sizeof(buf));
	out_put_sn(cw_g_out, buf, 1024,
		   "[s](): reallocing 0x[p]"
		   " (was size [i32], allocated at [s], line [i32])"
		   " to 0x[p], size [i32] at [s], line [i32]\n",
		   __FUNCTION__, a_ptr,
		   old_size,
		   ((NULL == old_filename)
		    ? "<?>" : old_filename),
		   old_line_num,
		   retval,
		   a_size,
		   (NULL == a_filename) ? "<?>" : a_filename,
		   a_line_num);
	out_put(cw_g_out, buf);
      }
    }
  }
#endif

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
    struct cw_mem_item_s * allocation;
    
    if (TRUE == oh_item_delete(&a_mem->addr_hash, a_ptr, NULL,
			       (void **) &allocation))
    {
      if (dbg_is_registered(cw_g_dbg, "mem_error"))
      {
	char buf[1025];

	bzero(buf, sizeof(buf));
	out_put_sn(cw_g_out, buf, 1024,
		   "[s](): 0x[p] not allocated, "
		   "attempted to free at [s], line [i32]\n",
		   __FUNCTION__, a_ptr,
		   (NULL == a_filename) ? "<?>" : a_filename,
		   a_line_num);
	out_put(cw_g_out, buf);
      }
    }
    else
    {
      if (dbg_is_registered(cw_g_dbg, "mem_verbose"))
      {
	char buf[1025];

	bzero(buf, sizeof(buf));
	out_put_sn(cw_g_out, buf, 1024,
		   "[s](): Freeing 0x[p], size [i32], at [s], line [i32] "
		   "(allocated at [s], line [i32])\n",
		   __FUNCTION__, a_ptr,
		   allocation->size,
		   (NULL == a_filename) ? "<?>" : a_filename,
		   a_line_num,
		   ((NULL == allocation->filename)
		    ? "<?>" : allocation->filename),
		   allocation->line_num);
	out_put(cw_g_out, buf);
      }
      memset(a_ptr, 0x5a, allocation->size);
      _cw_free(allocation);
    }
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
