/****************************************************************************
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

#include "../include/libstash/libstash.h"
#include "../include/libstash/mem_l.h"


struct cw_mem_s {
	cw_mtx_t	lock;

#ifdef _LIBSTASH_DBG
#define _CW_MEM_TABLE_SIZE 1024		/* Number of slots in hash table. */
	cw_ch_t		*addr_hash;
#endif

	cw_mem_oom_handler_t *oom_handler;
        const void	*handler_data;
};

#ifdef _LIBSTASH_DBG
struct cw_mem_item_s {
	cw_uint32_t	size;
        const char	*filename;
	cw_uint32_t	line_num;
	cw_chi_t	chi;		/* For internal ch linkage. */
};
#endif

cw_mem_t *
mem_new(void)
{
	cw_mem_t	*retval;

	retval = (cw_mem_t *)_cw_malloc(sizeof(cw_mem_t));
	if (retval == NULL)
		goto OOM_1;
	mtx_new(&retval->lock);

#ifdef _LIBSTASH_DBG
	retval->addr_hash = (cw_ch_t
	    *)_cw_malloc(_CW_CH_TABLE2SIZEOF(_CW_MEM_TABLE_SIZE));
	if (retval->addr_hash == NULL)
		goto OOM_2;
	ch_new(retval->addr_hash, _CW_MEM_TABLE_SIZE, ch_hash_direct,
	    ch_key_comp_direct);
#endif

	retval->oom_handler = NULL;
	retval->handler_data = NULL;

	return retval;

#ifdef _LIBSTASH_DBG
	OOM_2:
	mtx_delete(&retval->lock);
	_cw_free(retval);
	retval = NULL;
#endif
	OOM_1:
	return retval;
}

void
mem_delete(cw_mem_t *a_mem)
{
	_cw_check_ptr(a_mem);

#ifdef _LIBSTASH_DBG
	{
		cw_uint64_t	i, num_addrs;
		void		*addr;
		struct cw_mem_item_s *allocation;

		num_addrs = ch_count(a_mem->addr_hash);

		if (dbg_is_registered(cw_g_dbg, "mem_verbose") ||
		    (dbg_is_registered(cw_g_dbg, "mem_error") && (0 <
		    num_addrs))) {
			char	buf[1025];

			bzero(buf, sizeof(buf));
			_cw_out_put_sn(buf, 1024,
			    "[s](): [q] unfreed allocation[s]\n", __FUNCTION__,
			    num_addrs, num_addrs != 1 ? "s" : "");
			_cw_out_put(buf);
		}
		for (i = 0; i < num_addrs; i++) {
			ch_remove_iterate(a_mem->addr_hash, &addr, (void
			    **)&allocation, NULL);
			if (dbg_is_registered(cw_g_dbg, "mem_error")) {
				char	buf[1025];

				bzero(buf, sizeof(buf));
				_cw_out_put_sn(buf, 1024,
				    "[s](): 0x[p], size [i] never freed "
				    "(allocated at [s], line [i])\n",
				    __FUNCTION__, addr, allocation->size,
				    allocation->filename, allocation->line_num);
				_cw_out_put(buf);
			}
			_cw_free(allocation);
		}
		ch_delete(a_mem->addr_hash);
		_cw_free(a_mem->addr_hash);
		mtx_delete(&a_mem->lock);
	}
#endif

	_cw_free(a_mem);
}

void
mem_set_oom_handler(cw_mem_t *a_mem, cw_mem_oom_handler_t * a_oom_handler, const
    void *a_data)
{
	_cw_check_ptr(a_mem);

	mtx_lock(&a_mem->lock);

	a_mem->oom_handler = a_oom_handler;
	a_mem->handler_data = a_data;

	mtx_unlock(&a_mem->lock);
}

void *
mem_malloc(cw_mem_t *a_mem, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	void	*retval;

	_cw_assert(a_size > 0);

#ifdef _LIBSTASH_DBG
	if (a_mem != NULL)
		mtx_lock(&a_mem->lock);
#endif

	retval = _cw_malloc(a_size);

	if (retval == NULL) {
		if (a_mem != NULL) {
#ifndef _LIBSTASH_DBG
			mtx_lock(&a_mem->lock);
#endif
			if (a_mem->oom_handler != NULL) {
				if (a_mem->oom_handler(a_mem->handler_data,
				    a_size))
					retval = _cw_malloc(a_size);
			}
#ifndef _LIBSTASH_DBG
			mtx_unlock(&a_mem->lock);
#endif
		}
	}
#ifdef _LIBSTASH_DBG
	if (a_filename == NULL)
		a_filename = "<?>";
	if (retval == NULL) {
		if (dbg_is_registered(cw_g_dbg, "mem_error")) {
			char	buf[1025];

			bzero(buf, sizeof(buf));
			_cw_out_put_sn(buf, 1024,
			    "[s](): malloc([i]) returned NULL at [s], "
			    "line [i]\n",
			    __FUNCTION__, a_size, a_filename, a_line_num);
			_cw_out_put(buf);
		}
	} else if (a_mem != NULL) {
		struct cw_mem_item_s	*old_allocation;

		if (ch_search(a_mem->addr_hash, retval, (void
		    **)&old_allocation) == FALSE) {
			if (dbg_is_registered(cw_g_dbg, "mem_error")) {
				char	buf[1025];

				_cw_check_ptr(old_allocation);

				bzero(buf, sizeof(buf));
				_cw_out_put_sn(buf, 1024,
				    "[s](): 0x[p] multiply-allocated "
				    "(was at [s], line [i], size [i];"
				    " now at [s], line [i], size [i])\n",
				    __FUNCTION__, retval,
				    old_allocation->filename,
				    old_allocation->line_num,
				    old_allocation->size, a_filename,
				    a_line_num, a_size);
				_cw_out_put(buf);
			}
		} else {
			struct cw_mem_item_s	*allocation;

			allocation = _cw_malloc(sizeof(struct cw_mem_item_s));
			if (allocation == NULL) {
				char	buf[1025];

				bzero(buf, sizeof(buf));
				_cw_out_put_sn(buf, 1024,
				    "[s](): malloc([i]) returned NULL\n",
				    __FUNCTION__, sizeof(struct cw_mem_item_s));
				_cw_out_put(buf);
			} else {
				memset(retval, 0xa5, a_size);

				allocation->size = a_size;
				allocation->filename = a_filename;
				allocation->line_num = a_line_num;

				if (dbg_is_registered(cw_g_dbg,
				    "mem_verbose")) {
					char	buf[1025];

					bzero(buf, sizeof(buf));
					_cw_out_put_sn(buf, 1024,
					    "[s](): 0x[p] <-- malloc([i]) "
					    "at [s], line [i]\n", __FUNCTION__,
					    retval, a_size, a_filename,
					    a_line_num);
					_cw_out_put(buf);
				}
				if (ch_insert(a_mem->addr_hash, retval,
				    allocation, &allocation->chi)) {
					if (dbg_is_registered(cw_g_dbg,
					    "mem_error")) {
						char	buf[1025];

						bzero(buf, sizeof(buf));

						_cw_out_put_sn(buf, 1024,
						    "[s](): Memory allocation"
						    " error; unable to record"
						    " allocation 0x[p] at [s]"
						    ", line [i]\n",
						    __FUNCTION__, retval,
						    a_filename, a_line_num);
						_cw_out_put(buf);
					}
				}
			}
		}
		mtx_unlock(&a_mem->lock);
	}
#endif

	return retval;
}

void *
mem_calloc(cw_mem_t *a_mem, size_t a_number, size_t a_size, const char
    *a_filename, cw_uint32_t a_line_num)
{
	void	*retval;

	_cw_assert(a_size * a_number > 0);

#ifdef _LIBSTASH_DBG
	if (a_mem != NULL)
		mtx_lock(&a_mem->lock);
#endif

	retval = _cw_calloc(a_number, a_size);

	if (retval == NULL) {
		if (a_mem != NULL) {
#ifndef _LIBSTASH_DBG
			mtx_lock(&a_mem->lock);
#endif
			if (a_mem->oom_handler != NULL) {
				if (a_mem->oom_handler(a_mem->handler_data,
				    a_number * a_size))
					retval = _cw_calloc(a_number, a_size);
			}
#ifndef _LIBSTASH_DBG
			mtx_unlock(&a_mem->lock);
#endif
		}
	}
#ifdef _LIBSTASH_DBG
	if (a_filename == NULL)
		a_filename = "<?>";
	if (retval == NULL) {
		if (dbg_is_registered(cw_g_dbg, "mem_error")) {
			char	buf[1025];

			bzero(buf, sizeof(buf));
			_cw_out_put_sn(buf, 1024,
			    "[s](): calloc([i], [i]) returned NULL "
			    "at [s], line [i]\n", __FUNCTION__, a_number,
			    a_size, a_filename, a_line_num);
			_cw_out_put(buf);
		}
	} else if (a_mem != NULL) {
		struct cw_mem_item_s	*old_allocation;

		if (ch_search(a_mem->addr_hash, retval, (void
		    **)&old_allocation) == FALSE) {
			if (dbg_is_registered(cw_g_dbg, "mem_error")) {
				char	buf[1025];

				_cw_check_ptr(old_allocation);

				bzero(buf, sizeof(buf));
				_cw_out_put_sn(buf, 1024,
				    "[s](): 0x[p] multiply-allocated "
				    "(was at [s], line [i], size [i];"
				    " now at [s], line [i], size [i])\n",
				    __FUNCTION__, retval,
				    old_allocation->filename,
				    old_allocation->line_num,
				    old_allocation->size, a_filename,
				    a_line_num, a_size);
				_cw_out_put(buf);
			}
		} else {
			struct cw_mem_item_s	*allocation;

			allocation = _cw_malloc(sizeof(struct cw_mem_item_s));
			if (allocation == NULL) {
				char	buf[1025];

				bzero(buf, sizeof(buf));
				_cw_out_put_sn(buf, 1024,
				    "[s](): malloc([i]) returned NULL\n",
				    __FUNCTION__, sizeof(struct cw_mem_item_s));
				_cw_out_put(buf);
			} else {
				/*
				 * Leave the memory alone, since calloc() is
				 * supposed to return zeroed memory.
				 */

				allocation->size = a_number * a_size;
				allocation->filename = a_filename;
				allocation->line_num = a_line_num;

				if (dbg_is_registered(cw_g_dbg,
				    "mem_verbose")) {
					char	buf[1025];

					bzero(buf, sizeof(buf));
					_cw_out_put_sn(buf, 1024,
					    "[s](): 0x[p] <-- calloc([i], [i]) "
					    "at [s], line [i]\n",
					    __FUNCTION__, retval, a_number,
					    a_size, a_filename, a_line_num);
					_cw_out_put(buf);
				}
				if (ch_insert(a_mem->addr_hash, retval,
				    allocation, &allocation->chi)) {
					if (dbg_is_registered(cw_g_dbg,
					    "mem_error")) {
						char	buf[1025];

						bzero(buf, sizeof(buf));

						_cw_out_put_sn(buf, 1024,
						    "[s](): Memory allocation "
						    "error; unable to record "
						    "allocation 0x[p] at [s], "
						    "line [i]\n", __FUNCTION__,
						    retval, a_filename,
						    a_line_num);
						_cw_out_put(buf);
					}
				}
			}
		}
		mtx_unlock(&a_mem->lock);
	}
#endif

	return retval;
}

void *
mem_realloc(cw_mem_t *a_mem, void *a_ptr, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	void	*retval;

	_cw_check_ptr(a_ptr);
	_cw_assert(a_size > 0);

#ifdef _LIBSTASH_DBG
	if (a_mem != NULL)
		mtx_lock(&a_mem->lock);
#endif

	retval = _cw_realloc(a_ptr, a_size);

	if (retval == NULL) {
		if (a_mem != NULL) {
#ifndef _LIBSTASH_DBG
			mtx_lock(&a_mem->lock);
#endif
			if (a_mem->oom_handler != NULL) {
				if (a_mem->oom_handler(a_mem->handler_data,
				    a_size))
					retval = _cw_realloc(a_ptr, a_size);
			}
#ifndef _LIBSTASH_DBG
			mtx_unlock(&a_mem->lock);
#endif
		}
	}
#ifdef _LIBSTASH_DBG
	if (a_filename == NULL)
		a_filename = "<?>";
	if (retval == NULL) {
		if (dbg_is_registered(cw_g_dbg, "mem_error")) {
			char	buf[1025];

			bzero(buf, sizeof(buf));
			_cw_out_put_sn(buf, 1024, "[s](): realloc(0x[p], [i]) "
			    "returned NULL at [s], line [i]\n", __FUNCTION__,
			    a_ptr, a_size, a_filename, a_line_num);
			_cw_out_put(buf);
		}
	} else if (a_mem != NULL) {
		struct cw_mem_item_s	*allocation;

		if (ch_remove(a_mem->addr_hash, a_ptr, NULL, (void
		    **)&allocation, NULL)) {
			if (dbg_is_registered(cw_g_dbg, "mem_error")) {
				char	buf[1025];

				bzero(buf, sizeof(buf));
				_cw_out_put_sn(buf, 1024,
				    "[s](): 0x[p] not allocated\n",
				    __FUNCTION__, a_ptr);
				_cw_out_put(buf);
			}
		} else {
			const char	*old_filename;
			cw_uint32_t	old_size, old_line_num;

			old_filename = allocation->filename;
			old_size = allocation->size;
			old_line_num = allocation->line_num;
			allocation->filename = a_filename;
			allocation->size = a_size;
			allocation->line_num = a_line_num;

			if (ch_insert(a_mem->addr_hash, retval, allocation,
			    &allocation->chi)) {
				if (dbg_is_registered(cw_g_dbg, "mem_error")) {
					char	buf[1025];

					bzero(buf, sizeof(buf));

					_cw_out_put_sn(buf, 1024,
					    "[s](): Memory allocation error; "
					    "unable to record allocation 0x[p]"
					    " at [s], line [i]\n", __FUNCTION__,
					    retval, a_filename, a_line_num);
					_cw_out_put(buf);
				}
			}
			if (a_size > old_size) {
				memset(((cw_uint8_t *)retval) + old_size,
				    0xa5, a_size - old_size);
			}
			if (dbg_is_registered(cw_g_dbg, "mem_verbose")) {
				char	buf[1025];

				bzero(buf, sizeof(buf));
				_cw_out_put_sn(buf, 1024,
				    "[s](): reallocing 0x[p]"
				    " (was size [i], allocated at [s], line"
				    " [i]) to 0x[p], size [i] at [s], line"
				    " [i]\n", __FUNCTION__, a_ptr, old_size,
				    old_filename, old_line_num, retval, a_size,
				    a_filename, a_line_num);
				_cw_out_put(buf);
			}
		}
		mtx_unlock(&a_mem->lock);
	}
#endif

	return retval;
}

void
mem_free(cw_mem_t *a_mem, void *a_ptr, const char *a_filename, cw_uint32_t
    a_line_num)
{
#ifdef _LIBSTASH_DBG
	if (a_filename == NULL)
		a_filename = "<?>";
	if (a_mem != NULL) {
		struct cw_mem_item_s	*allocation;

		mtx_lock(&a_mem->lock);

		if (ch_remove(a_mem->addr_hash, a_ptr, NULL,
			(void **)&allocation, NULL)) {
			if (dbg_is_registered(cw_g_dbg, "mem_error")) {
				char	buf[1025];

				bzero(buf, sizeof(buf));
				_cw_out_put_sn(buf, 1024,
				    "[s](): 0x[p] not allocated, "
				    "attempted to free at [s], line [i]\n",
				    __FUNCTION__, a_ptr, a_filename,
				    a_line_num);
				_cw_out_put(buf);
			}
		} else {
			if (dbg_is_registered(cw_g_dbg, "mem_verbose")) {
				char	buf[1025];

				bzero(buf, sizeof(buf));
				_cw_out_put_sn(buf, 1024,
				    "[s](): Freeing 0x[p], size [i], at [s], "
				    "line [i] (allocated at [s], line [i])\n",
				    __FUNCTION__, a_ptr, allocation->size,
				    a_filename, a_line_num,
				    allocation->filename, allocation->line_num);
				_cw_out_put(buf);
			}
			memset(a_ptr, 0x5a, allocation->size);
			_cw_free(allocation);
		}
	}
#endif

	_cw_free(a_ptr);

#ifdef _LIBSTASH_DBG
	if (a_mem != NULL)
		mtx_unlock(&a_mem->lock);
#endif
}
