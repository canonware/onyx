/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#ifdef _LIBSTASH_DBG
#define _CW_PEZZ_MAGIC 0x4e224e22
#endif

cw_pezz_t *
pezz_new(cw_pezz_t *a_pezz, cw_mem_t *a_mem, cw_uint32_t a_buffer_size,
    cw_uint32_t a_num_buffers)
{
	cw_pezz_t		*retval;
	cw_pezzi_t		*pezzi;
	cw_uint32_t		i;
	volatile cw_uint32_t	try_stage = 0;

	_cw_assert(0 != (a_buffer_size * a_num_buffers));

	xep_begin();
	volatile cw_pezz_t	*v_retval;
	xep_try {
		if (a_pezz != NULL) {
			v_retval = retval = a_pezz;
			memset(retval, 0, sizeof(cw_pezz_t));
			retval->is_malloced = FALSE;
		} else {
			v_retval = retval = (cw_pezz_t *)mem_malloc(a_mem,
			    sizeof(cw_pezz_t));
			memset(retval, 0, sizeof(cw_pezz_t));
			retval->is_malloced = TRUE;
		}
		try_stage = 1;

		retval->mem = a_mem;
		mtx_new(&retval->lock);

		/* Make sure the elements are big enough to contain a pezzi. */
		if (a_buffer_size >= sizeof(cw_pezzi_t))
			retval->buffer_size = a_buffer_size;
		else
			retval->buffer_size = sizeof(cw_pezzi_t);

		/*
		 * Round element size up to the nearest multiple of alignment
		 * required on this architecture.
		 *
		 * __alignof__ is a GCCism.
		 *
		 * This code assumes that alignment is a power of 2.
		 */
		retval->aligned_size = retval->buffer_size;
		if (retval->buffer_size & (__alignof__(void *) - 1)) {
			retval->aligned_size += __alignof__(void *);
			retval->aligned_size &= ~(__alignof__(void *) - 1);
		}

		retval->block_num_buffers = a_num_buffers;

		/* Allocate and initialize first block. */
		retval->mem_blocks = (void **)mem_calloc(a_mem, 1,
		    sizeof(void *));
		try_stage = 2;

		retval->mem_blocks[0] = (void *)mem_calloc(a_mem,
		    retval->block_num_buffers, retval->aligned_size);
		try_stage = 3;

		/* Initialize spares to have something in it. */
		qs_new(&retval->spares);

		for (i = 0; i < retval->block_num_buffers; i++) {
			pezzi = (cw_pezzi_t *)(((cw_uint8_t
			    *)retval->mem_blocks[0]) + (i *
			    retval->aligned_size));
			qs_elm_new(pezzi, link);
			qs_push(&retval->spares, pezzi, link);
		}
		retval->num_blocks = 1;

#ifdef _LIBSTASH_PEZZ_DBG
		dch_new(&retval->addr_hash, a_mem, a_num_buffers * 3,
		    a_num_buffers * 2, 0, ch_direct_hash, ch_direct_key_comp);
		try_stage = 4;
#endif

#ifdef _LIBSTASH_DBG
		retval->magic = _CW_PEZZ_MAGIC;
#endif
	}
	xep_catch(_CW_STASHX_OOM) {
		retval = (cw_pezz_t *)v_retval;
		switch (try_stage) {
		case 3:
			mem_free(a_mem, retval->mem_blocks[0]);
		case 2:
			mem_free(a_mem, retval->mem_blocks);
		case 1:
			mtx_delete(&retval->lock);
			if (retval->is_malloced)
				mem_free(a_mem, retval);
		case 0:
			break;
		default:
			_cw_not_reached();
		}
	}
	xep_end();

	return retval;
}

void
pezz_delete(cw_pezz_t *a_pezz)
{
	cw_uint32_t	i;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);

#ifdef _LIBSTASH_PEZZ_DBG
	{
		cw_uint32_t	i, num_addrs;
		void		*addr;
		cw_pezz_item_t	*allocation;

		num_addrs = dch_count(&a_pezz->addr_hash);

		if (0 < num_addrs) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "[i] leaked buffer[s]\n",
			    num_addrs, num_addrs != 1 ? "s" : "");
		}
		for (i = 0; i < num_addrs; i++) {
			dch_remove_iterate(&a_pezz->addr_hash, &addr,
			    (void **)&allocation, NULL);
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "0x[p] never freed (allocated at [s], line [i])\n",
			    addr, ((NULL == allocation->filename) ? "<?>" :
			    allocation->filename), allocation->line_num);
			mem_free(a_pezz->mem, allocation);
		}
		dch_delete(&a_pezz->addr_hash);
	}
#endif

	for (i = 0; i < a_pezz->num_blocks; i++) {
		mem_free(a_pezz->mem, a_pezz->mem_blocks[i]);
	}
	mem_free(a_pezz->mem, a_pezz->mem_blocks);

	mtx_delete(&a_pezz->lock);

	if (a_pezz->is_malloced)
		mem_free(a_pezz->mem, a_pezz);
#ifdef _LIBSTASH_PEZZ_DBG
	else
		memset(a_pezz, 0x5a, sizeof(cw_pezz_t));
#endif
}

cw_uint32_t
pezz_buffer_size_get(cw_pezz_t *a_pezz)
{
	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);

	return a_pezz->buffer_size;
}

void *
pezz_get_e(cw_pezz_t *a_pezz, const char *a_filename, cw_uint32_t a_line_num)
{
	void		*retval;
	cw_pezzi_t	*pezzi;

	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
	mtx_lock(&a_pezz->lock);

	if (qs_top(&a_pezz->spares) == NULL) {
		void		**t_mem_blocks;
		cw_uint32_t	i;

		/* No buffers available.  Add a block. */
		t_mem_blocks = (void **)mem_realloc(a_pezz->mem,
		    a_pezz->mem_blocks, ((a_pezz->num_blocks + 1) * sizeof(void
		    *)));
		a_pezz->mem_blocks = t_mem_blocks;

		a_pezz->mem_blocks[a_pezz->num_blocks] = (void
		    *)mem_calloc(a_pezz->mem, a_pezz->block_num_buffers,
		    a_pezz->aligned_size);

		/* Initialize spares to have something in it. */
		for (i = 0; i < a_pezz->block_num_buffers; i++) {
			pezzi = (cw_pezzi_t *)(((cw_uint8_t
			    *)a_pezz->mem_blocks[a_pezz->num_blocks]) + (i *
			    a_pezz->aligned_size));
			qs_elm_new(pezzi, link);
			qs_push(&a_pezz->spares, pezzi, link);
		}

		/*
		 * Do this last so that num_blocks can be used as an index
		 * above.
		 */
		a_pezz->num_blocks++;
	}

	retval = (void *)qs_top(&a_pezz->spares);
	qs_pop(&a_pezz->spares, link);

#ifdef _LIBSTASH_PEZZ_DBG
	{
		cw_pezz_item_t	*old_allocation;

		if (a_filename == NULL)
			a_filename = "<?>";

		if (dch_search(&a_pezz->addr_hash, retval, (void
		    **)&old_allocation) == FALSE) {
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "0x[p] multiply-allocated (was at [s], line [i];"
			    " now at [s], line [i])\n", retval,
			    old_allocation->filename, old_allocation->line_num,
			    a_filename, a_line_num);
		} else {
			cw_pezz_item_t		*allocation;
			volatile cw_uint32_t	try_stage = 0;

			xep_begin();
			volatile cw_pezz_item_t	*v_allocation;
			xep_try {
				v_allocation = allocation =
				    mem_malloc(a_pezz->mem,
				    sizeof(cw_pezz_item_t));
				try_stage = 1;

				memset(retval, 0xa5, a_pezz->aligned_size);

				allocation->filename = a_filename;
				allocation->line_num = a_line_num;

#ifdef _LIBSTASH_PEZZ_VERBOSE
				out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
				    "0x[p] ([i] B) <-- pezz_get() at [s], line "
				    "[i]\n", retval, a_pezz->buffer_size,
				    a_filename, a_line_num);
#endif

				dch_insert(&a_pezz->addr_hash, retval,
				    allocation, NULL);
				try_stage = 2;
			}
			xep_catch (_CW_STASHX_OOM) {
				allocation = (cw_pezz_item_t *)v_allocation;
				switch (try_stage) {
				case 1:
					mem_free(a_pezz->mem, allocation);
				case 0:
					break;
				default:
					_cw_not_reached();
				}
				xep_handled();
			}
			xep_end();
		}
	}
#endif

	mtx_unlock(&a_pezz->lock);
	return retval;
}

void
pezz_put_e(cw_pezz_t *a_pezz, void *a_buffer, const char *a_filename,
    cw_uint32_t a_line_num)
{
	_cw_check_ptr(a_pezz);
	_cw_assert(a_pezz->magic == _CW_PEZZ_MAGIC);
	mtx_lock(&a_pezz->lock);

#ifdef _LIBSTASH_PEZZ_DBG
	{
		cw_pezz_item_t	*allocation;

		if (a_filename == NULL)
			a_filename = "<?>";

		if (dch_remove(&a_pezz->addr_hash, a_buffer, NULL, (void
		    **)&allocation, NULL)) {
			/*
			 * This error condition is either due to programmer
			 * error or an earlier memory allocation error while
			 * recording this buffer in the hash table.  Don't
			 * cache this buffer, under the assumption that we're
			 * here due to programmer error, rather than a memory
			 * allocation error.  If the problem was actually memory
			 * allocation, a message was already printed to that
			 * effect, and all we're doing here is leaking one
			 * buffer.
			 */
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "0x[p] not allocated, attempted to free at [s], "
			    "line [i]\n", a_buffer, a_filename, a_line_num);
			goto RETURN;
		} else {
#ifdef _LIBSTASH_PEZZ_VERBOSE
			out_put_e(cw_g_out, NULL, 0, __FUNCTION__,
			    "Freeing 0x[p] at [s], line [i], allocated at [s], "
			    "line [i]\n", a_buffer, a_filename, a_line_num,
			    allocation->filename, allocation->line_num);
#endif
			memset(a_buffer, 0x5a, a_pezz->aligned_size);
			mem_free(a_pezz->mem, allocation);
		}
	}
#endif

	qs_push(&a_pezz->spares, (cw_pezzi_t *)a_buffer, link);

#ifdef _LIBSTASH_PEZZ_DBG
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
	_cw_out_put("[s]aligned_size : [i]\n",
	    a_prefix, a_pezz->aligned_size);
	_cw_out_put("[s]block_num_buffers : [i]\n",
	    a_prefix, a_pezz->block_num_buffers);
	_cw_out_put("[s]num_blocks : [i]\n",
	    a_prefix, a_pezz->num_blocks);

	_cw_out_put("[s]mem_blocks : 0x[p]\n",
	    a_prefix, a_pezz->mem_blocks);

	for (i = 0; i < a_pezz->num_blocks; i++);
	{
		_cw_out_put("[s]mem_blocks[[[i]] : 0x[p] : "
		    "0x[p]\n", a_prefix, i, a_pezz->mem_blocks[i], i);
	}

	if (qs_top(&a_pezz->spares) != NULL) {
		cw_pezzi_t	*pezzi;

		_cw_out_put("[s]spares : \n",
		    a_prefix);

		qs_foreach(pezzi, &a_pezz->spares, link) {
			_cw_out_put("[s]       : 0x[p]\n", a_prefix, pezzi);
		}
	} else
		_cw_out_put("[s]spares : (none)\n", a_prefix);

	_cw_out_put("[s]end ============================================\n",
	    a_prefix);

	mtx_unlock(&a_pezz->lock);
}
