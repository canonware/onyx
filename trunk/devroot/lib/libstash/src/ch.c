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
#define _CW_CH_MAGIC 0x574936af
#define _CW_CHI_MAGIC 0xabdcee0e
#endif

cw_ch_t *
ch_new(cw_ch_t *a_ch, cw_mem_t *a_mem, cw_uint32_t a_table_size, cw_ch_hash_t
    *a_hash, cw_ch_key_comp_t *a_key_comp)
{
	cw_ch_t	*retval;

	_cw_assert(a_table_size > 0);

	if (NULL != a_ch) {
		retval = a_ch;
		memset(retval, 0, _CW_CH_TABLE2SIZEOF(a_table_size));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_ch_t
		    *)_cw_mem_malloc(a_mem, _CW_CH_TABLE2SIZEOF(a_table_size));
		if (NULL == retval)
			goto RETURN;
		memset(retval, 0, _CW_CH_TABLE2SIZEOF(a_table_size));
		retval->is_malloced = TRUE;
	}

	retval->mem = a_mem;
	retval->table_size = a_table_size;
	retval->hash = a_hash;
	retval->key_comp = a_key_comp;

#ifdef _LIBSTASH_DBG
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
	_cw_assert(a_ch->magic == _CW_CH_MAGIC);

	while (ql_first(&a_ch->chi_ql) != NULL) {
		chi = ql_first(&a_ch->chi_ql);
		_cw_check_ptr(chi);
		_cw_assert(chi->magic == _CW_CHI_MAGIC);
		ql_head_remove(&a_ch->chi_ql, cw_chi_t, ch_link);
		if (chi->is_malloced)
			_cw_mem_free(a_ch->mem, chi);
#ifdef _LIBSTASH_DBG
		else
			memset(chi, 0x5a, sizeof(cw_chi_t));
#endif
	}

	if (a_ch->is_malloced)
		_cw_mem_free(a_ch->mem, a_ch);
#ifdef _LIBSTASH_DBG
	else
		memset(a_ch, 0x5a, _CW_CH_TABLE2SIZEOF(a_ch->table_size));
#endif
}

cw_uint32_t
ch_count(cw_ch_t *a_ch)
{
	_cw_check_ptr(a_ch);
	_cw_assert(a_ch->magic == _CW_CH_MAGIC);

	return a_ch->count;
}

cw_bool_t
ch_insert(cw_ch_t *a_ch, const void *a_key, const void *a_data, cw_chi_t
    *a_chi)
{
	cw_bool_t	retval;
	cw_uint32_t	slot;
	cw_chi_t	*chi;

	_cw_check_ptr(a_ch);
	_cw_assert(a_ch->magic == _CW_CH_MAGIC);

	/* Initialize chi. */
	if (a_chi != NULL) {
		chi = a_chi;
		chi->is_malloced = FALSE;
	} else {
		chi = (cw_chi_t *)_cw_mem_malloc(a_ch->mem, sizeof(cw_chi_t));
		if (chi == NULL) {
			retval = TRUE;
			goto RETURN;
		}
		chi->is_malloced = TRUE;
	}
	chi->key = a_key;
	chi->data = a_data;
	ql_elm_new(chi, ch_link);
	ql_elm_new(chi, slot_link);
	slot = a_ch->hash(a_key) % a_ch->table_size;
	chi->slot = slot;
#ifdef _LIBSTASH_DBG
	chi->magic = _CW_CHI_MAGIC;
#endif

	/* Hook into ch-wide list. */
	ql_tail_insert(&a_ch->chi_ql, chi, ch_link);

	/* Hook into the slot list. */
#ifdef _LIBSTASH_DBG
	if (ql_first(&a_ch->table[slot]) != NULL)
		a_ch->num_collisions++;
#endif
	ql_head_insert(&a_ch->table[slot], chi, slot_link);

	a_ch->count++;
#ifdef _LIBSTASH_DBG
	a_ch->num_inserts++;
#endif

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
ch_remove(cw_ch_t *a_ch, const void *a_search_key, void **r_key, void **r_data,
    cw_chi_t **r_chi)
{
	cw_bool_t	retval;
	cw_uint32_t	slot;
	cw_chi_t	*chi;

	_cw_check_ptr(a_ch);
	_cw_assert(a_ch->magic == _CW_CH_MAGIC);

	slot = a_ch->hash(a_search_key) % a_ch->table_size;

	for (chi = ql_first(&a_ch->table[slot]); chi != NULL; chi =
	    ql_next(&a_ch->table[slot], chi, slot_link)) {
		_cw_check_ptr(chi);
		_cw_assert(chi->magic == _CW_CHI_MAGIC);

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
			if (chi->is_malloced)
				_cw_mem_free(a_ch->mem, chi);
			else if (r_chi != NULL) {
#ifdef _LIBSTASH_DBG
				chi->magic = 0;
#endif
				*r_chi = chi;
			}

			a_ch->count--;
#ifdef _LIBSTASH_DBG
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
	_cw_assert(a_ch->magic == _CW_CH_MAGIC);

	slot = a_ch->hash(a_key) % a_ch->table_size;

	for (chi = ql_first(&a_ch->table[slot]); chi != NULL; chi =
	     ql_next(&a_ch->table[slot], chi, slot_link)) {
		_cw_check_ptr(chi);
		_cw_assert(chi->magic == _CW_CHI_MAGIC);

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
	_cw_assert(a_ch->magic == _CW_CH_MAGIC);

	chi = ql_first(&a_ch->chi_ql);
	if (chi == NULL) {
		retval = TRUE;
		goto RETURN;
	}
	_cw_check_ptr(chi);
	_cw_assert(chi->magic == _CW_CHI_MAGIC);
	if (r_key != NULL)
		*r_key = (void *)chi->key;
	if (r_data != NULL)
		*r_data = (void *)chi->data;

	/* Rotate the list. */
	ql_first(&a_ch->chi_ql) = ql_next(&a_ch->chi_ql,
	    ql_first(&a_ch->chi_ql), ch_link);

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
	_cw_assert(a_ch->magic == _CW_CH_MAGIC);

	chi = ql_first(&a_ch->chi_ql);
	if (chi == NULL) {
		retval = TRUE;
		goto RETURN;
	}
	_cw_check_ptr(chi);
	_cw_assert(chi->magic == _CW_CHI_MAGIC);

	/* Detach from the ch-wide list. */
	ql_remove(&a_ch->chi_ql, chi, ch_link);

	/* Detach from the slot list. */
	ql_remove(&a_ch->table[chi->slot], chi, slot_link);

	if (r_key != NULL)
		*r_key = (void *)chi->key;
	if (r_data != NULL)
		*r_data = (void *)chi->data;
	if (chi->is_malloced)
		_cw_mem_free(a_ch->mem, chi);
	else if (r_chi != NULL) {
#ifdef _LIBSTASH_DBG
		chi->magic = 0;
#endif
		*r_chi = chi;
	}

	a_ch->count--;
#ifdef _LIBSTASH_DBG
	a_ch->num_removes++;
#endif

	retval = FALSE;
	RETURN:
	return retval;
}

void
ch_dump(cw_ch_t *a_ch, const char *a_prefix)
{
	cw_uint32_t	i;
	cw_chi_t	*chi;

	_cw_check_ptr(a_ch);
	_cw_assert(a_ch->magic == _CW_CH_MAGIC);
	_cw_check_ptr(a_prefix);

#ifdef _LIBSTASH_DBG
	_cw_out_put("[s]: num_collisions: [i], num_inserts: [i],"
	    " num_removes: [i]\n",
	    a_prefix, a_ch->num_collisions, a_ch->num_inserts,
	    a_ch->num_removes);
#endif

	_cw_out_put("[s]: is_malloced: [s]\n",
	    a_prefix, (a_ch->is_malloced) ? "TRUE" : "FALSE");
	_cw_out_put("[s]: ql_first(chi_ql): 0x[p]\n",
	    a_prefix, ql_first(&a_ch->chi_ql));
	_cw_out_put("[s]: count: [i], table_size: [i]\n",
	    a_prefix, a_ch->count, a_ch->table_size);

	/* Table. */
	_cw_out_put("[s]: table --------------------------------------"
	    "----------------------\n", a_prefix);

	for (i = 0; i < a_ch->table_size; i++) {
		if (ql_first(&a_ch->table[i]) != NULL) {
			ql_foreach(chi, &a_ch->chi_ql, ch_link) {
				_cw_out_put("[s]: key: 0x[p], data: 0x[p],"
				    " slot: [i]\n", a_prefix, chi->key,
				    chi->data, chi->slot);
			}
		} else
			_cw_out_put("[s]: [i]: NULL\n", a_prefix, i);
	}

	/* chi list. */
	_cw_out_put("[s]: chi_list -----------------------------------"
	    "----------------------\n", a_prefix);
	if (ql_first(&a_ch->chi_ql) != NULL) {
		ql_foreach(chi, &a_ch->chi_ql, ch_link) {
			_cw_out_put("[s]: key: 0x[p], data: 0x[p], slot: [i]\n",
			    a_prefix, chi->key, chi->data, chi->slot);
		}
	} else
		_cw_out_put("[s]: Empty\n", a_prefix);
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

	retval = (cw_uint32_t)a_key;

	/* Shift right until we've shifted one 1 bit off. */
	for (i = 0; i < 8 * sizeof(void *); i++) {
		if ((retval & 0x1) == 1) {
			retval >>= 1;
			break;
		} else
			retval >>= 1;
	}

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

