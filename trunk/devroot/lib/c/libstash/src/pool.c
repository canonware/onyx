/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#ifdef _LIBSTASH_DBG
#define _CW_POOL_MAGIC 0x8383764a
#endif

cw_pool_t *
pool_new(cw_pool_t *a_pool, cw_mem_t *a_mem, cw_uint32_t a_buffer_size)
{
	cw_pool_t	*retval;

	_cw_assert(a_buffer_size > 0);

	if (a_pool == NULL) {
		retval = (cw_pool_t *)_cw_mem_malloc(a_mem, sizeof(cw_pool_t));
		if (retval == NULL)
			goto OOM_1;
		memset(retval, 0, sizeof(cw_pool_t));
		retval->is_malloced = TRUE;
	} else {
		retval = a_pool;
		memset(retval, 0, sizeof(cw_pool_t));
		retval->is_malloced = FALSE;
	}

	retval->mem = a_mem;
	mtx_new(&retval->lock);

	retval->buffer_size = a_buffer_size;
	qs_new(&retval->spares);

#ifdef _LIBSTASH_DBG
	if (dch_new(&retval->addr_hash, a_mem, 8, 6, 2, ch_direct_hash,
	    ch_direct_key_comp) == NULL)
		goto OOM_2;

	retval->magic = _CW_POOL_MAGIC;
#endif

	return retval;

#ifdef _LIBSTASH_DBG
	OOM_2:
	if (retval->is_malloced)
		_cw_mem_free(a_mem, retval);
	retval = NULL;
#endif
	OOM_1:
	return retval;
}

void
pool_delete(cw_pool_t *a_pool)
{
	cw_pool_spare_t	*spare;

	_cw_check_ptr(a_pool);
	_cw_assert(a_pool->magic == _CW_POOL_MAGIC);

#ifdef _LIBSTASH_DBG
	{
		cw_uint32_t	i, num_addrs;
		void		*addr;
		cw_pool_item_t	*allocation;

		num_addrs = dch_count(&a_pool->addr_hash);

		if (dbg_is_registered(cw_g_dbg, "pool_verbose") ||
		    (dbg_is_registered(cw_g_dbg, "pool_error") && (0 <
		    num_addrs))) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "[i] leaked buffer[s]\n",
			    num_addrs, num_addrs != 1 ? "s" : "");
		}
		for (i = 0; i < num_addrs; i++) {
			dch_remove_iterate(&a_pool->addr_hash, &addr,
			    (void **)&allocation, NULL);
			if (dbg_is_registered(cw_g_dbg, "pool_error")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "0x[p] never freed (allocated at [s],"
				    " line [i])\n", addr, ((NULL ==
				    allocation->filename) ? "<?>" :
				    allocation->filename),
				    allocation->line_num);
			}
			_cw_mem_free(a_pool->mem, allocation);
		}
		dch_delete(&a_pool->addr_hash);
	}
#endif

	for (spare = qs_top(&a_pool->spares); spare != NULL; spare =
	     qs_top(&a_pool->spares)) {
		qs_pop(&a_pool->spares, link);
		_cw_mem_free(a_pool->mem, spare);
	}

	mtx_delete(&a_pool->lock);

	if (a_pool->is_malloced)
		_cw_mem_free(a_pool->mem, a_pool);
#ifdef _LIBSTASH_DBG
	else
		memset(a_pool, 0x5a, sizeof(cw_pool_t));
#endif
}

cw_uint32_t
pool_buffer_size_get(cw_pool_t *a_pool)
{
	_cw_check_ptr(a_pool);
	_cw_assert(a_pool->magic == _CW_POOL_MAGIC);

	return a_pool->buffer_size;
}

void
pool_drain(cw_pool_t *a_pool)
{
	cw_pool_spare_t	*spare;

	_cw_check_ptr(a_pool);
	_cw_assert(a_pool->magic == _CW_POOL_MAGIC);
	mtx_lock(&a_pool->lock);

	for (spare = qs_top(&a_pool->spares); spare != NULL; spare =
	     qs_top(&a_pool->spares)) {
		qs_pop(&a_pool->spares, link);
		_cw_mem_free(a_pool->mem, spare);
	}

	mtx_unlock(&a_pool->lock);
}

void *
pool_get(cw_pool_t *a_pool, const char *a_filename, cw_uint32_t a_line_num)
{
	void		*retval;
	cw_pool_spare_t	*spare;

	_cw_check_ptr(a_pool);
	_cw_assert(a_pool->magic == _CW_POOL_MAGIC);
	mtx_lock(&a_pool->lock);

	spare = qs_top(&a_pool->spares);
	if (spare != NULL) {
		qs_pop(&a_pool->spares, link);
		retval = (void *)spare;
	} else {
		if (a_pool->buffer_size >= sizeof(cw_pool_spare_t))
			retval = (void *)_cw_mem_malloc(a_pool->mem,
			    a_pool->buffer_size);
		else
			retval = (void *)_cw_mem_malloc(a_pool->mem,
			    sizeof(cw_pool_spare_t));
	}

#ifdef _LIBSTASH_DBG
	if (a_filename == NULL)
		a_filename = "<?>";
	if (retval == NULL) {
		if (dbg_is_registered(cw_g_dbg, "pool_error")) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "Memory allocation failed at [s], line [i]\n",
			    a_filename, a_line_num);
		}
	} else {
		cw_pool_item_t	*old_allocation;

		if (dch_search(&a_pool->addr_hash, retval, (void
		    **)&old_allocation) == FALSE) {
			if (dbg_is_registered(cw_g_dbg, "pool_error")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "0x[p] multiply-allocated "
				    "(was at [s], line [i];"
				    " now at [s], line [i])\n",
				    retval, old_allocation->filename,
				    old_allocation->line_num, a_filename,
				    a_line_num);
			}
		} else {
			cw_pool_item_t	*allocation;

			allocation = _cw_mem_malloc(a_pool->mem,
			    sizeof(cw_pool_item_t));
			if (allocation == NULL) {
				if (dbg_is_registered(cw_g_dbg, "pool_error")) {
					_cw_out_put_e("Memory allocation error;"
					    " unable to record pool allocation "
					    "0x[p] at [s], line [i]\n",
					    sizeof(cw_pool_item_t), retval,
					    a_filename, a_line_num);
				}
			} else {
				memset(retval, 0xa5, a_pool->buffer_size);

				allocation->filename = a_filename;
				allocation->line_num = a_line_num;

				if (dbg_is_registered(cw_g_dbg,
				    "pool_verbose")) {
					out_put_e(cw_g_out, NULL, 0,
					    __FUNCTION__,
					    "0x[p] ([i] B) <-- pool_get() at"
					    " [s], line [i]\n", retval,
					    a_pool->buffer_size, a_filename,
					    a_line_num);
				}
				if (dch_insert(&a_pool->addr_hash, retval,
				    allocation, NULL) == TRUE) {
					if (dbg_is_registered(cw_g_dbg,
					    "pool_error")) {
						_cw_out_put_e("Memory "
						    "allocation error; unable "
						    "to record pool allocation "
						    "0x[p] at [s], line [i]\n",
						    sizeof(cw_pool_item_t),
						    retval, a_filename,
						    a_line_num);
					}
				}
			}
		}
	}
#endif
	mtx_unlock(&a_pool->lock);
	return retval;
}

void
pool_put(cw_pool_t *a_pool, void *a_buffer, const char *a_filename, cw_uint32_t
    a_line_num)
{
	cw_pool_spare_t	*spare;

	_cw_check_ptr(a_pool);
	_cw_assert(a_pool->magic == _CW_POOL_MAGIC);
	mtx_lock(&a_pool->lock);

#ifdef _LIBSTASH_DBG
	if (a_filename == NULL)
		a_filename = "<?>";
	{
		cw_pool_item_t	*allocation;

		if (dch_remove(&a_pool->addr_hash, a_buffer, NULL, (void
		    **)&allocation, NULL)) {
			if (dbg_is_registered(cw_g_dbg, "pool_error")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "0x[p] not allocated, "
				    "attempted to free at [s], line [i]\n",
				    a_buffer, a_filename, a_line_num);
			}
			/*
			 * Don't put the non-allocated object on the spares
			 * stack.
			 */
			goto RETURN;
		} else {
			if (dbg_is_registered(cw_g_dbg, "pool_verbose")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Freeing 0x[p] at [s], line [i], "
				    "allocated at [s], line [i]\n",
				    a_buffer, a_filename, a_line_num,
				    allocation->filename, allocation->line_num);
			}
			memset(a_buffer, 0x5a, a_pool->buffer_size);
			_cw_mem_free(a_pool->mem, allocation);
		}
	}
#endif

	spare = (cw_pool_spare_t *)a_buffer;
	qs_elm_new(spare, link);
	qs_push(&a_pool->spares, spare, link);

#ifdef _LIBSTASH_DBG
	RETURN:
#endif
	mtx_unlock(&a_pool->lock);
}

void
pool_dump(cw_pool_t *a_pool, const char *a_prefix)
{
	cw_pool_spare_t	*spare;

	_cw_check_ptr(a_pool);
	_cw_assert(a_pool->magic == _CW_POOL_MAGIC);
	mtx_lock(&a_pool->lock);

	_cw_out_put("[s]start ==========================================\n",
	    a_prefix);
	_cw_out_put("[s]buffer_size : [i]\n", a_prefix, a_pool->buffer_size);

	if (qs_top(&a_pool->spares) != NULL) {
		cw_uint32_t	i = 0;

		qs_foreach(spare, &a_pool->spares, link) {
			i++;
		}
		_cw_out_put("[s]spares ([i]) : \n", a_prefix, i);

		qs_foreach(spare, &a_pool->spares, link) {
			_cw_out_put("[s]             0x[p]\n", a_prefix, spare);
		}
	} else
		_cw_out_put("[s]spares (0) : (null)\n", a_prefix);

	_cw_out_put("[s]end ============================================\n",
	    a_prefix);

	mtx_unlock(&a_pool->lock);
}
