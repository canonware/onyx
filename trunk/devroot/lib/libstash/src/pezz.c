/****************************************************************************
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

#include "../include/libstash/libstash.h"

#ifdef _LIBSTASH_DBG
#define _CW_PEZZ_MAGIC 0x4e224e22
#endif

cw_pezz_t *
pezz_new(cw_pezz_t *a_pezz, cw_uint32_t a_buffer_size, cw_uint32_t
    a_num_buffers)
{
	cw_pezz_t	*retval;
	cw_uint32_t	i;
	cw_ring_t	*t_ring;

	_cw_assert(0 != (a_buffer_size * a_num_buffers));

	if (a_pezz == NULL) {
		retval = (cw_pezz_t *)_cw_malloc(sizeof(cw_pezz_t));
		if (retval == NULL)
			goto OOM_1;
		bzero(retval, sizeof(cw_pezz_t));
		retval->is_malloced = TRUE;
	} else {
		retval = a_pezz;
		bzero(retval, sizeof(cw_pezz_t));
		retval->is_malloced = FALSE;
	}

	mtx_new(&retval->lock);

	retval->buffer_size = a_buffer_size;
	retval->block_num_buffers = a_num_buffers;

	/* Allocate and initialize first block. */
	retval->mem_blocks = (void **)_cw_calloc(1, sizeof(void *));
	if (retval->mem_blocks == NULL)
		goto OOM_2;
	retval->ring_blocks = (cw_ring_t **)_cw_calloc(1, sizeof(cw_ring_t *));
	if (retval->ring_blocks == NULL)
		goto OOM_3;
	retval->mem_blocks[0] = (void *)_cw_calloc(retval->block_num_buffers,
	    retval->buffer_size);
	if (retval->mem_blocks[0] == NULL)
		goto OOM_4;
	retval->ring_blocks[0] =
	    (cw_ring_t *)_cw_calloc(retval->block_num_buffers,
	    sizeof(cw_ring_t));
	if (retval->ring_blocks[0] == NULL)
		goto OOM_5;

	/* Initialize spare_buffers to have something in it. */
	retval->spare_buffers = retval->ring_blocks[0];
	ring_new(retval->spare_buffers);
	ring_set_data(retval->spare_buffers, retval->mem_blocks[0]);

	for (i = 1; i < retval->block_num_buffers; i++) {
		t_ring = &retval->ring_blocks[0][i];
		ring_new(t_ring);
		ring_set_data(t_ring, (((cw_uint8_t
		    *)retval->mem_blocks[0]) + (i * 
		    retval->buffer_size)));
		ring_meld(retval->spare_buffers, t_ring);
	}
	retval->num_blocks = 1;

#ifdef _LIBSTASH_DBG
	if (dch_new(&retval->addr_hash, a_num_buffers * 3, a_num_buffers * 2, 0,
	    ch_hash_direct, ch_key_comp_direct) == NULL)
		goto OOM_6;

	retval->magic = _CW_PEZZ_MAGIC;
#endif

	return retval;

#ifdef _LIBSTASH_DBG
	OOM_6:
	_cw_free(retval->ring_blocks[0]);
#endif
	OOM_5:
	_cw_free(retval->mem_blocks[0]);
	OOM_4:
	_cw_free(retval->ring_blocks);
	OOM_3:
	_cw_free(retval->mem_blocks);
	OOM_2:
	mtx_delete(&retval->lock);
	if (retval->is_malloced)
		_cw_free(retval);
	retval = NULL;
	OOM_1:
	return retval;
}

void
pezz_delete(cw_pezz_t *a_pezz)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);

#ifdef _LIBSTASH_DBG
	{
		cw_uint32_t	i, num_addrs;
		void		*addr;
		cw_pezz_item_t	*allocation;

		num_addrs = dch_count(&a_pezz->addr_hash);

		if (dbg_is_registered(cw_g_dbg, "pezz_verbose") ||
		    (dbg_is_registered(cw_g_dbg, "pezz_error") && (0 <
		    num_addrs))) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "[i] leaked buffer[s]\n",
			    num_addrs, num_addrs != 1 ? "s" : "");
		}
		for (i = 0; i < num_addrs; i++) {
			dch_remove_iterate(&a_pezz->addr_hash, &addr,
			    (void **)&allocation, NULL);
			if (dbg_is_registered(cw_g_dbg, "pezz_error")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "0x[p] never freed (allocated at [s],"
				    " line [i])\n", addr, ((NULL ==
				    allocation->filename) ? "<?>" :
				    allocation->filename),
				    allocation->line_num);
			}
			_cw_free(allocation);
		}
		dch_delete(&a_pezz->addr_hash);
	}
#endif

	for (i = 0; i < a_pezz->num_blocks; i++) {
		_cw_free(a_pezz->mem_blocks[i]);
		_cw_free(a_pezz->ring_blocks[i]);
	}
	_cw_free(a_pezz->mem_blocks);
	_cw_free(a_pezz->ring_blocks);

	mtx_delete(&a_pezz->lock);

#ifdef _LIBSTASH_DBG
	a_pezz->magic = 0;
#endif

	if (a_pezz->is_malloced)
		_cw_free(a_pezz);
}

cw_uint32_t
pezz_get_buffer_size(cw_pezz_t *a_pezz)
{
	cw_uint32_t	retval;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);

	retval = a_pezz->buffer_size;

	return retval;
}

void *
pezz_get(cw_pezz_t *a_pezz, const char *a_filename, cw_uint32_t a_line_num)
{
	void		*retval;
	cw_ring_t	*t_ring;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
	mtx_lock(&a_pezz->lock);

	if (a_pezz->spare_buffers == NULL) {
		void		**t_mem_blocks;
		cw_ring_t	**t_ring_blocks;
		cw_uint32_t	i;

		/* No buffers available.  Add a block. */
		t_mem_blocks = (void **)_cw_realloc(a_pezz->mem_blocks,
		    ((a_pezz->num_blocks + 1) * sizeof(void *)));
		if (t_mem_blocks == NULL) {
			retval = NULL;
			goto RETURN;
		}
		a_pezz->mem_blocks = t_mem_blocks;

		t_ring_blocks = (cw_ring_t **)_cw_realloc(a_pezz->ring_blocks,
		    ((a_pezz->num_blocks + 1) * sizeof(cw_ring_t *)));
		if (t_ring_blocks == NULL) {
			retval = NULL;
			goto RETURN;
		}
		a_pezz->ring_blocks = t_ring_blocks;

		a_pezz->mem_blocks[a_pezz->num_blocks] = (void
		    *)_cw_calloc(a_pezz->block_num_buffers,
		    a_pezz->buffer_size);
		if (a_pezz->mem_blocks[a_pezz->num_blocks] == NULL) {
			retval = NULL;
			goto RETURN;
		}
		a_pezz->ring_blocks[a_pezz->num_blocks] = (cw_ring_t
		    *)_cw_calloc(a_pezz->block_num_buffers, sizeof(cw_ring_t));
		if (a_pezz->ring_blocks[a_pezz->num_blocks] == NULL) {
			_cw_free(a_pezz->mem_blocks[a_pezz->num_blocks]);
			retval = NULL;
			goto RETURN;
		}
		/* All of the allocation succeeded. */

		/* Initialize spare_buffers to have something in it. */
		a_pezz->spare_buffers = a_pezz->ring_blocks[a_pezz->num_blocks];
		ring_new(a_pezz->spare_buffers);
		ring_set_data(a_pezz->spare_buffers,
		    a_pezz->mem_blocks[a_pezz->num_blocks]);

		for (i = 1; i < a_pezz->block_num_buffers; i++) {
			t_ring = &a_pezz->ring_blocks[a_pezz->num_blocks][i];
			ring_new(t_ring);
			ring_set_data(t_ring, (((cw_uint8_t
			    *)a_pezz->mem_blocks[a_pezz->num_blocks]) + (i *
			    a_pezz->buffer_size)));
			ring_meld(a_pezz->spare_buffers, t_ring);
		}

		/*
		 * Do this last so that num_blocks can be used as an index
		 * above.
		 */
		a_pezz->num_blocks++;
	}

	t_ring = a_pezz->spare_buffers;
	a_pezz->spare_buffers = ring_cut(t_ring);
	if (t_ring == a_pezz->spare_buffers) {
		/* This was the last element in the ring. */
		a_pezz->spare_buffers = NULL;
	}
	retval = ring_get_data(t_ring);
	if (a_pezz->spare_rings != NULL)
		ring_meld(a_pezz->spare_rings, t_ring);

	/*
	 * Do this even if we just did a ring_meld() in order to make the ring
	 * act like a stack, hopefully improving cache locality.
	 */
	a_pezz->spare_rings = t_ring;

	RETURN:
#ifdef _LIBSTASH_DBG
	if (a_filename == NULL)
		a_filename = "<?>";
	if (retval == NULL) {
		if (dbg_is_registered(cw_g_dbg, "pezz_error")) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "Memory allocation failed at [s], line [i]\n",
			    a_filename, a_line_num);
		}
	} else {
		cw_pezz_item_t	*old_allocation;

		if (dch_search(&a_pezz->addr_hash, retval, (void
		    **)&old_allocation) == FALSE) {
			if (dbg_is_registered(cw_g_dbg, "pezz_error")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "0x[p] multiply-allocated "
				    "(was at [s], line [i];"
				    " now at [s], line [i])\n",
				    retval, old_allocation->filename,
				    old_allocation->line_num, a_filename,
				    a_line_num);
			}
		} else {
			cw_pezz_item_t	*allocation;

			allocation = _cw_malloc(sizeof(cw_pezz_item_t));
			if (allocation == NULL) {
				if (dbg_is_registered(cw_g_dbg, "pezz_error")) {
					_cw_out_put_e("Memory allocation error;"
					    " unable to record pezz allocation "
					    "0x[p] at [s], line [i]\n",
					    sizeof(cw_pezz_item_t), retval,
					    a_filename, a_line_num);
				}
			} else {
				memset(retval, 0xa5, a_pezz->buffer_size);

				allocation->filename = a_filename;
				allocation->line_num = a_line_num;

				if (dbg_is_registered(cw_g_dbg,
				    "pezz_verbose")) {
					out_put_e(cw_g_out, NULL, 0,
					    __FUNCTION__,
					    "0x[p] ([i] B) <-- pezz_get() at"
					    " [s], line [i]\n", retval,
					    a_pezz->buffer_size, a_filename,
					    a_line_num);
				}
				if (dch_insert(&a_pezz->addr_hash, retval,
				    allocation, NULL) == TRUE) {
					if (dbg_is_registered(cw_g_dbg,
					    "pezz_error")) {
						_cw_out_put_e("Memory "
						    "allocation error; unable "
						    "to record pezz allocation "
						    "0x[p] at [s], line [i]\n",
						    sizeof(cw_pezz_item_t),
						    retval, a_filename,
						    a_line_num);
					}
				}
			}
		}
	}
#endif

	mtx_unlock(&a_pezz->lock);
	return retval;
}

void
pezz_put(cw_pezz_t *a_pezz, void *a_buffer, const char *a_filename, cw_uint32_t
    a_line_num)
{
	cw_ring_t	*t_ring;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
	mtx_lock(&a_pezz->lock);

#ifdef _LIBSTASH_DBG
	if (a_filename == NULL)
		a_filename = "<?>";
	{
		cw_pezz_item_t	*allocation;

		if (dch_remove(&a_pezz->addr_hash, a_buffer, NULL, (void
		    **)&allocation, NULL)) {
			/*
			 * Bail out in order to prevent corruption of the
			 * internal data structures.  If we were to go ahead
			 * and "free" this allocation, it would take up a
			 * ring structure, making it possible to over-empty
			 * the ring, as well as the likely problem of
			 * actually returning this allocation in a later
			 * call to pezz_get().  Of course, the non-debug
			 * versions of libstash will just blow chunks since
			 * there isn't the extra book keeping that allows
			 * detection of this problem.
			 *
			 * Of course, there is the possibility that the reason
			 * this allocation isn't recorded in the hash table
			 * is due to a memory allocation error.  If so, then
			 * we're causing a memory leak here.  Oh well.  At
			 * least the user got a message about the memory
			 * allocation error already.
			 */
			if (dbg_is_registered(cw_g_dbg, "pezz_error")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "0x[p] not allocated, "
				    "attempted to free at [s], line [i]\n",
				    a_buffer, a_filename, a_line_num);
			}
			goto RETURN;
		} else {
			if (dbg_is_registered(cw_g_dbg, "pezz_verbose")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "Freeing 0x[p] at [s], line [i], "
				    "allocated at [s], line [i]\n",
				    a_buffer, a_filename, a_line_num,
				    allocation->filename, allocation->line_num);
			}
			memset(a_buffer, 0x5a, a_pezz->buffer_size);
			_cw_free(allocation);
		}
	}
#endif

	t_ring = a_pezz->spare_rings;
	a_pezz->spare_rings = ring_cut(t_ring);
	if (t_ring == a_pezz->spare_rings) {
		/* spare_rings is empty. */
		a_pezz->spare_rings = NULL;
	}
	ring_set_data(t_ring, a_buffer);

	if (a_pezz->spare_buffers != NULL)
		ring_meld(t_ring, a_pezz->spare_buffers);
	a_pezz->spare_buffers = t_ring;

#ifdef _LIBSTASH_DBG
	/*
	 * The RETURN label is only used in the debugging versions of
	 * libstash. Prevent a compiler warning.
	 */
RETURN:
#endif
	mtx_unlock(&a_pezz->lock);
}

void
pezz_dump(cw_pezz_t *a_pezz, const char *a_prefix)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
	mtx_lock(&a_pezz->lock);

	_cw_out_put("[s]start ==========================================\n",
	    a_prefix);
	_cw_out_put("[s]buffer_size : [i]\n",
	    a_prefix, a_pezz->buffer_size);
	_cw_out_put("[s]block_num_buffers : [i]\n",
	    a_prefix, a_pezz->block_num_buffers);
	_cw_out_put("[s]num_blocks : [i]\n",
	    a_prefix, a_pezz->num_blocks);

	_cw_out_put("[s]mem_blocks : 0x[p]\n",
	    a_prefix, a_pezz->mem_blocks);
	_cw_out_put("[s]ring_blocks : 0x[p]\n",
	    a_prefix, a_pezz->ring_blocks);

	for (i = 0; i < a_pezz->num_blocks; i++);
	{
		_cw_out_put("[s]mem_blocks[[[i]] : 0x[p], ring_blocks[[[i]] : "
		    "0x[p]\n", a_prefix, i, a_pezz->mem_blocks[i], i,
		    a_pezz->ring_blocks[i]);
	}

	if (a_pezz->spare_buffers != NULL) {
		char	*prefix = (char *)_cw_malloc(strlen(a_prefix) + 17);

		_cw_out_put("[s]spare_buffers : \n",
		    a_prefix);

		if (prefix != NULL) {
			_cw_out_put_s(prefix, "[s]              : ", a_prefix);
			ring_dump(a_pezz->spare_buffers, prefix);
			_cw_free(prefix);
		} else {
			prefix = (char *)a_prefix;
			ring_dump(a_pezz->spare_buffers, prefix);
		}
	} else
		_cw_out_put("[s]spare_buffers : (null)\n", a_prefix);

	_cw_out_put("[s]end ============================================\n",
	    a_prefix);

	mtx_unlock(&a_pezz->lock);
}
