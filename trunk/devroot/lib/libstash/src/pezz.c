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
pezz_new(cw_pezz_t *a_pezz, cw_uint32_t a_buffer_size,
    cw_uint32_t a_num_buffers)
{
	cw_pezz_t *retval;

	_cw_assert(0 != (a_buffer_size * a_num_buffers));

	if (NULL == a_pezz) {
		retval = (cw_pezz_t *)_cw_malloc(sizeof(cw_pezz_t));
		if (NULL == retval)
			goto RETURN;
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
	if (NULL == retval->mem_blocks) {
		if (retval->is_malloced)
			_cw_free(retval);
		retval = NULL;
		goto RETURN;
	}
	retval->ring_blocks = (cw_ring_t **)_cw_calloc(1, sizeof(cw_ring_t *));
	if (NULL == retval->ring_blocks) {
		_cw_free(retval->mem_blocks);
		if (retval->is_malloced)
			_cw_free(retval);
		retval = NULL;
		goto RETURN;
	}
	retval->mem_blocks[0] = (void *)_cw_calloc(retval->block_num_buffers,
	    retval->buffer_size);
	if (NULL == retval->mem_blocks[0]) {
		_cw_free(retval->ring_blocks);
		_cw_free(retval->mem_blocks);
		if (retval->is_malloced)
			_cw_free(retval);
		retval = NULL;
		goto RETURN;
	}
	retval->ring_blocks[0] =
	    (cw_ring_t *)_cw_calloc(retval->block_num_buffers,
	    sizeof(cw_ring_t));
	if (NULL == retval->ring_blocks[0]) {
		_cw_free(retval->mem_blocks[0]);
		_cw_free(retval->ring_blocks);
		_cw_free(retval->mem_blocks);
		if (retval->is_malloced)
			_cw_free(retval);
		retval = NULL;
		goto RETURN;
	} {
		cw_uint32_t i;
		cw_ring_t *t_ring;

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
	}
	retval->num_blocks = 1;

#ifdef _LIBSTASH_DBG
	oh_new(&retval->addr_hash);
	oh_set_h1(&retval->addr_hash, oh_h1_direct);
	oh_set_key_compare(&retval->addr_hash, oh_key_compare_direct);

	retval->magic = _CW_PEZZ_MAGIC;
#endif

RETURN:
	return retval;
}

void
pezz_delete(cw_pezz_t *a_pezz)
{
	cw_uint32_t i;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);

#ifdef _LIBSTASH_DBG
	{
		cw_uint64_t i, num_addrs;
		void   *addr;
		cw_pezz_item_t *allocation;

		num_addrs = oh_get_num_items(&a_pezz->addr_hash);

		if (dbg_is_registered(cw_g_dbg, "pezz_verbose") ||
		    (dbg_is_registered(cw_g_dbg, "pezz_error") && (0 <
		    num_addrs))) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "[q] leaked buffer[s]\n",
			    num_addrs,
			    num_addrs != 1 ? "s" : "");
		}
		for (i = 0; i < num_addrs; i++) {
			oh_item_delete_iterate(&a_pezz->addr_hash, &addr,
			    (void **)&allocation);
			if (dbg_is_registered(cw_g_dbg, "pezz_error")) {
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "0x[p] never freed (allocated at [s], line [i])\n",
				    addr, ((NULL == allocation->filename) ?
				    "<?>" : allocation->filename),
				    allocation->line_num);
			}
			_cw_free(allocation);
		}
		oh_delete(&a_pezz->addr_hash);
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

	if (TRUE == a_pezz->is_malloced)
		_cw_free(a_pezz);
}

cw_uint32_t
pezz_get_buffer_size(cw_pezz_t *a_pezz)
{
	cw_uint32_t retval;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);

	retval = a_pezz->buffer_size;

	return retval;
}

void   *
pezz_get_e(cw_pezz_t *a_pezz, const char *a_filename, cw_uint32_t a_line_num)
{
	void   *retval;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
	mtx_lock(&a_pezz->lock);

	if (a_pezz->spare_buffers == NULL) {
		void  **t_mem_blocks;
		cw_ring_t **t_ring_blocks;

		/* No buffers available.  Add a block. */
		t_mem_blocks = (void **)_cw_realloc(a_pezz->mem_blocks,
		    ((a_pezz->num_blocks + 1) * sizeof(void *)));
		if (NULL == t_mem_blocks) {
			retval = NULL;
			goto RETURN;
		}
		a_pezz->mem_blocks = t_mem_blocks;

		t_ring_blocks = (cw_ring_t **)_cw_realloc(a_pezz->ring_blocks,
		    ((a_pezz->num_blocks + 1) * sizeof(cw_ring_t *)));
		if (NULL == t_ring_blocks) {
			retval = NULL;
			goto RETURN;
		}
		a_pezz->ring_blocks = t_ring_blocks;

		a_pezz->mem_blocks[a_pezz->num_blocks] = (void
		    *)_cw_calloc(a_pezz->block_num_buffers,
		    a_pezz->buffer_size);
		if (NULL == a_pezz->mem_blocks[a_pezz->num_blocks]) {
			retval = NULL;
			goto RETURN;
		}
		a_pezz->ring_blocks[a_pezz->num_blocks]
		    = (cw_ring_t *)_cw_calloc(a_pezz->block_num_buffers,
		    sizeof(cw_ring_t));
		if (NULL == a_pezz->ring_blocks[a_pezz->num_blocks]) {
			_cw_free(a_pezz->mem_blocks[a_pezz->num_blocks]);
			retval = NULL;
			goto RETURN;
		}
		/* All of the allocation succeeded. */
		{
			cw_uint32_t i;
			cw_ring_t *t_ring;

			/* Initialize spare_buffers to have something in it. */
			a_pezz->spare_buffers =
			    a_pezz->ring_blocks[a_pezz->num_blocks];
			ring_new(a_pezz->spare_buffers);
			ring_set_data(a_pezz->spare_buffers,
			    a_pezz->mem_blocks[a_pezz->num_blocks]);

			for (i = 1; i < a_pezz->block_num_buffers; i++) {
				t_ring =
				    &a_pezz->ring_blocks[a_pezz->num_blocks][i];
				ring_new(t_ring);
				ring_set_data(t_ring, (((cw_uint8_t
				    *)a_pezz->mem_blocks[a_pezz->num_blocks]) +
				    (i * a_pezz->buffer_size)));
				ring_meld(a_pezz->spare_buffers, t_ring);
			}
		}

		/*
		 * Do this last so that num_blocks can be used as an index
		 * above.
		 */
		a_pezz->num_blocks++;
	} {
		cw_ring_t *t_ring;

		t_ring = a_pezz->spare_buffers;
		a_pezz->spare_buffers = ring_cut(t_ring);
		if (a_pezz->spare_buffers == t_ring) {
			/* This was the last element in the ring. */
			a_pezz->spare_buffers = NULL;
		}
		retval = ring_get_data(t_ring);
		if (NULL != a_pezz->spare_rings)
			ring_meld(a_pezz->spare_rings, t_ring);
		/*
		 * Do this even if we just did a ring_meld() in order to
		 * make the ring act like a stack, hopefully improving cache
		 * locality.
		 */
		a_pezz->spare_rings = t_ring;
	}

RETURN:
#ifdef _LIBSTASH_DBG
	if (NULL == a_filename) {
		a_filename = "<?>";
	}
	if (NULL == retval) {
		if (dbg_is_registered(cw_g_dbg, "pezz_error")) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "Memory allocation failed at [s], line [i]\n",
			    a_filename, a_line_num);
		}
	} else {
		cw_pezz_item_t *old_allocation;

		if (FALSE == oh_item_search(&a_pezz->addr_hash,
			retval,
			(void **)&old_allocation)) {
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
			cw_pezz_item_t *allocation;

			allocation = _cw_malloc(sizeof(cw_pezz_item_t));
			if (allocation == NULL) {
				if (dbg_is_registered(cw_g_dbg, "pezz_error")) {
					out_put_e(cw_g_out, __FILE__, __LINE__, __FUNCTION__,
					    "Memory allocation error; "
					    "unable to record pezz allocation "
					    "0x[p] at [s], line [i]\n",
					    sizeof(cw_pezz_item_t),
					    retval, a_filename, a_line_num);
				}
			} else {
				memset(retval, 0xa5, a_pezz->buffer_size);

				allocation->filename = a_filename;
				allocation->line_num = a_line_num;

				if (dbg_is_registered(cw_g_dbg,
				    "pezz_verbose")) {
					out_put_e(cw_g_out, NULL, 0,
					    __FUNCTION__,
					    "0x[p] ([i] B) <-- pezz_get() at [s], line [i]\n",
					    retval, a_pezz->buffer_size,
					    a_filename, a_line_num);
				}
				if (-1 == oh_item_insert(&a_pezz->addr_hash,
				    retval, allocation)) {
					if (dbg_is_registered(cw_g_dbg,
					    "pezz_error")) {
						out_put_e(cw_g_out, __FILE__,
						    __LINE__, __FUNCTION__,
						    "Memory allocation error; "
						    "unable to record pezz allocation "
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

void   *
pezz_get(cw_pezz_t *a_pezz)
{
	return pezz_get_e(a_pezz, NULL, 0);
}

void
pezz_put_e(cw_pezz_t *a_pezz, void *a_buffer, const char *a_filename,
    cw_uint32_t a_line_num)
{
	cw_ring_t *t_ring;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
	mtx_lock(&a_pezz->lock);

#ifdef _LIBSTASH_DBG
	if (NULL == a_filename)
		a_filename = "<?>";
	{
		cw_pezz_item_t *allocation;

		if (TRUE == oh_item_delete(&a_pezz->addr_hash, a_buffer, NULL,
			(void **)&allocation)) {
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
			 * due to a memory allocation error.  If so, then
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
	if (a_pezz->spare_rings == t_ring) {
		/* spare_rings is empty. */
		a_pezz->spare_rings = NULL;
	}
	ring_set_data(t_ring, a_buffer);

	if (NULL != a_pezz->spare_buffers)
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
pezz_put(void *a_pezz, void *a_buffer)
{
	pezz_put_e(a_pezz, a_buffer, NULL, 0);
}

void
pezz_dump(cw_pezz_t *a_pezz, const char *a_prefix)
{
	cw_uint32_t i;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
	mtx_lock(&a_pezz->lock);

	out_put(cw_g_out,
	    "[s]start ==========================================\n", a_prefix);
	out_put(cw_g_out, "[s]buffer_size : [i]\n",
	    a_prefix, a_pezz->buffer_size);
	out_put(cw_g_out, "[s]block_num_buffers : [i]\n",
	    a_prefix, a_pezz->block_num_buffers);
	out_put(cw_g_out, "[s]num_blocks : [i]\n",
	    a_prefix, a_pezz->num_blocks);

	out_put(cw_g_out, "[s]mem_blocks : 0x[p]\n",
	    a_prefix, a_pezz->mem_blocks);
	out_put(cw_g_out, "[s]ring_blocks : 0x[p]\n",
	    a_prefix, a_pezz->ring_blocks);

	for (i = 0; i < a_pezz->num_blocks; i++);
	{
		out_put(cw_g_out,
		    "[s]mem_blocks[[[i]] : 0x[p], ring_blocks[[[i]] : 0x[p]\n",
		    a_prefix, i, a_pezz->mem_blocks[i], i,
		    a_pezz->ring_blocks[i]);
	}

	if (NULL != a_pezz->spare_buffers) {
		char   *prefix = (char *)_cw_malloc(strlen(a_prefix) + 17);

		out_put(cw_g_out, "[s]spare_buffers : \n",
		    a_prefix);

		if (NULL != prefix) {
			out_put_s(cw_g_out, prefix,
			    "[s]              : ", a_prefix);
			ring_dump(a_pezz->spare_buffers, prefix);
			_cw_free(prefix);
		} else {
			prefix = (char *)a_prefix;
			ring_dump(a_pezz->spare_buffers, prefix);
		}
	} else {
		out_put(cw_g_out, "[s]spare_buffers : (null)\n", a_prefix);
	}

	out_put(cw_g_out,
	    "[s]end ============================================\n", a_prefix);

	mtx_unlock(&a_pezz->lock);
}
