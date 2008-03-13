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

#include "../include/libonyx/libonyx.h"

#ifdef _CW_DBG
#define _CW_CH_MAGIC 0x574936af
#define _CW_CHI_MAGIC 0xabdcee0e
#endif

cw_ch_t *
ch_new(cw_ch_t *a_ch, cw_opaque_alloc_t *a_alloc, cw_opaque_dealloc_t
    *a_dealloc, void *a_arg, cw_uint32_t a_table_size, cw_ch_hash_t *a_hash,
    cw_ch_key_comp_t *a_key_comp)
{
	cw_ch_t	*retval;

	_cw_check_ptr(a_alloc);
	_cw_check_ptr(a_dealloc);
	_cw_assert(a_table_size > 0);
	_cw_check_ptr(a_hash);
	_cw_check_ptr(a_key_comp);

	if (NULL != a_ch) {
		retval = a_ch;
		memset(retval, 0, _CW_CH_TABLE2SIZEOF(a_table_size));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_ch_t
		    *)a_alloc(a_arg, _CW_CH_TABLE2SIZEOF(a_table_size),
		    __FILE__, __LINE__);
		if (NULL == retval)
			goto RETURN;
		memset(retval, 0, _CW_CH_TABLE2SIZEOF(a_table_size));
		retval->is_malloced = TRUE;
	}

	retval->alloc = a_alloc;
	retval->dealloc = a_dealloc;
	retval->arg = a_arg;
	retval->table_size = a_table_size;
	retval->hash = a_hash;
	retval->key_comp = a_key_comp;

#ifdef _CW_DBG
	retval->magic = _CW_CH_MAGIC;
#endif

	RETURN:
	return retval;
}

void
ch_delete(cw_ch_t *a_ch)
{
	cw_chi_t	*chi;

	_cw_check_ptr(a_ch);
	_cw_dassert(a_ch->magic == _CW_CH_MAGIC);

	while (ql_first(&a_ch->chi_ql) != NULL) {
		chi = ql_first(&a_ch->chi_ql);
		_cw_check_ptr(chi);
		_cw_dassert(chi->magic == _CW_CHI_MAGIC);
		ql_head_remove(&a_ch->chi_ql, cw_chi_t, ch_link);
		if (chi->is_malloced)
			_cw_opaque_dealloc(a_ch->dealloc, a_ch->arg, chi,
			    sizeof(cw_chi_t));
#ifdef _CW_DBG
		else
			memset(chi, 0x5a, sizeof(cw_chi_t));
#endif
	}

	if (a_ch->is_malloced)
		_cw_opaque_dealloc(a_ch->dealloc, a_ch->arg, a_ch,
		    _CW_CH_TABLE2SIZEOF(a_ch->table_size));
#ifdef _CW_DBG
	else
		memset(a_ch, 0x5a, _CW_CH_TABLE2SIZEOF(a_ch->table_size));
#endif
}

cw_uint32_t
ch_count(cw_ch_t *a_ch)
{
	_cw_check_ptr(a_ch);
	_cw_dassert(a_ch->magic == _CW_CH_MAGIC);

	return a_ch->count;
}

void
ch_insert(cw_ch_t *a_ch, const void *a_key, const void *a_data, cw_chi_t
    *a_chi)
{
	cw_uint32_t	slot;
	cw_chi_t	*chi;

	_cw_check_ptr(a_ch);
	_cw_dassert(a_ch->magic == _CW_CH_MAGIC);

	/* Initialize chi. */
	if (a_chi != NULL) {
		chi = a_chi;
		chi->is_malloced = FALSE;
	} else {
		chi = (cw_chi_t *)_cw_opaque_alloc(a_ch->alloc, a_ch->arg,
		    sizeof(cw_chi_t));
		chi->is_malloced = TRUE;
	}
	chi->key = a_key;
	chi->data = a_data;
	ql_elm_new(chi, ch_link);
	ql_elm_new(chi, slot_link);
	slot = a_ch->hash(a_key) % a_ch->table_size;
	chi->slot = slot;
#ifdef _CW_DBG
	chi->magic = _CW_CHI_MAGIC;
#endif

	/* Hook into ch-wide list. */
	ql_tail_insert(&a_ch->chi_ql, chi, ch_link);

	/* Hook into the slot list. */
#ifdef _CW_DBG
	if (ql_first(&a_ch->table[slot]) != NULL)
		a_ch->num_collisions++;
#endif
	ql_head_insert(&a_ch->table[slot], chi, slot_link);

	a_ch->count++;
#ifdef _CW_DBG
	a_ch->num_inserts++;
#endif
}

cw_bool_t
ch_remove(cw_ch_t *a_ch, const void *a_search_key, void **r_key, void **r_data,
    cw_chi_t **r_chi)
{
	cw_bool_t	retval;
	cw_uint32_t	slot;
	cw_chi_t	*chi;

	_cw_check_ptr(a_ch);
	_cw_dassert(a_ch->magic == _CW_CH_MAGIC);

	slot = a_ch->hash(a_search_key) % a_ch->table_size;

	for (chi = ql_first(&a_ch->table[slot]); chi != NULL; chi =
	    ql_next(&a_ch->table[slot], chi, slot_link)) {
		_cw_check_ptr(chi);
		_cw_dassert(chi->magic == _CW_CHI_MAGIC);

		/* Is this the chi we want? */
		if (a_ch->key_comp(a_search_key, chi->key)) {
			/* Detach from ch-wide list. */
			ql_remove(&a_ch->chi_ql, chi, ch_link);
			
			/* Detach from the slot list. */
			ql_remove(&a_ch->table[slot], chi, slot_link);

			if (r_key != NULL)
				*r_key = (void *)chi->key;
			if (r_data != NULL)
				*r_data = (void *)chi->data;
			if (chi->is_malloced) {
				_cw_opaque_dealloc(a_ch->dealloc, a_ch->arg,
				    chi, sizeof(cw_chi_t));
			}
			else if (r_chi != NULL) {
#ifdef _CW_DBG
				chi->magic = 0;
#endif
				*r_chi = chi;
			}

			a_ch->count--;
#ifdef _CW_DBG
			a_ch->num_removes++;
#endif
			retval = FALSE;
			goto RETURN;
		}
	}

	retval = TRUE;
	RETURN:
	return retval;
}

cw_bool_t
ch_search(cw_ch_t *a_ch, const void *a_key, void **r_data)
{
	cw_bool_t	retval;
	cw_uint32_t	slot;
	cw_chi_t	*chi;

	_cw_check_ptr(a_ch);
	_cw_dassert(a_ch->magic == _CW_CH_MAGIC);

	slot = a_ch->hash(a_key) % a_ch->table_size;

	for (chi = ql_first(&a_ch->table[slot]); chi != NULL; chi =
	     ql_next(&a_ch->table[slot], chi, slot_link)) {
		_cw_check_ptr(chi);
		_cw_dassert(chi->magic == _CW_CHI_MAGIC);

		/* Is this the chi we want? */
		if (a_ch->key_comp(a_key, chi->key) == TRUE) {
			if (r_data != NULL)
				*r_data = (void *)chi->data;
			retval = FALSE;
			goto RETURN;
		}
	}

	retval = TRUE;
	RETURN:
	return retval;
}

cw_bool_t
ch_get_iterate(cw_ch_t *a_ch, void **r_key, void **r_data)
{
	cw_bool_t	retval;
	cw_chi_t	*chi;

	_cw_check_ptr(a_ch);
	_cw_dassert(a_ch->magic == _CW_CH_MAGIC);

	chi = ql_first(&a_ch->chi_ql);
	if (chi == NULL) {
		retval = TRUE;
		goto RETURN;
	}
	_cw_check_ptr(chi);
	_cw_dassert(chi->magic == _CW_CHI_MAGIC);
	if (r_key != NULL)
		*r_key = (void *)chi->key;
	if (r_data != NULL)
		*r_data = (void *)chi->data;

	/* Rotate the list. */
	ql_first(&a_ch->chi_ql) = qr_next(ql_first(&a_ch->chi_ql), ch_link);

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
ch_remove_iterate(cw_ch_t *a_ch, void **r_key, void **r_data, cw_chi_t **r_chi)
{
	cw_bool_t	retval;
	cw_chi_t	*chi;

	_cw_check_ptr(a_ch);
	_cw_dassert(a_ch->magic == _CW_CH_MAGIC);

	chi = ql_first(&a_ch->chi_ql);
	if (chi == NULL) {
		retval = TRUE;
		goto RETURN;
	}
	_cw_check_ptr(chi);
	_cw_dassert(chi->magic == _CW_CHI_MAGIC);

	/* Detach from the ch-wide list. */
	ql_remove(&a_ch->chi_ql, chi, ch_link);

	/* Detach from the slot list. */
	ql_remove(&a_ch->table[chi->slot], chi, slot_link);

	if (r_key != NULL)
		*r_key = (void *)chi->key;
	if (r_data != NULL)
		*r_data = (void *)chi->data;
	if (chi->is_malloced)
		_cw_opaque_dealloc(a_ch->dealloc, a_ch->arg, chi,
		    sizeof(cw_chi_t));
	else if (r_chi != NULL) {
#ifdef _CW_DBG
		chi->magic = 0;
#endif
		*r_chi = chi;
	}

	a_ch->count--;
#ifdef _CW_DBG
	a_ch->num_removes++;
#endif

	retval = FALSE;
	RETURN:
	return retval;
}

cw_uint32_t
ch_string_hash(const void *a_key)
{
	cw_uint32_t	retval;
	char		*str;

	_cw_check_ptr(a_key);

	for (str = (char *)a_key, retval = 0; *str != 0; str++)
		retval = retval * 33 + *str;

	return retval;
}

cw_uint32_t
ch_direct_hash(const void *a_key)
{
	cw_uint32_t	retval, i;
#if (SIZEOF_INT_P == 4)
	cw_uint32_t	t = (cw_uint32_t)a_key;
#elif (SIZEOF_INT_P == 8)
	cw_uint64_t	t = (cw_uint64_t)a_key;
#else
#error Unsupported pointer size
#endif

	/* Shift right until we've shifted one 1 bit off. */
	for (i = 0; i < 8 * sizeof(void *); i++) {
		if ((t & 0x1) == 1) {
			t >>= 1;
			break;
		} else
			t >>= 1;
	}

#if (SIZEOF_INT_P == 4)
	retval = t;
#elif (SIZEOF_INT_P == 8)
	retval = (cw_uint32_t)t;
#else
#error Unsupported pointer size
#endif
	return retval;
}

cw_bool_t
ch_string_key_comp(const void *a_k1, const void *a_k2)
{
	_cw_check_ptr(a_k1);
	_cw_check_ptr(a_k2);

	return strcmp((char *)a_k1, (char *)a_k2) ? FALSE : TRUE;
}

cw_bool_t
ch_direct_key_comp(const void *a_k1, const void *a_k2)
{
	return (a_k1 == a_k2) ? TRUE : FALSE;
}

