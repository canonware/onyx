/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"

#ifdef _LIBSTASH_DBG
#define _CW_CH_MAGIC 0x574936af
#endif

cw_ch_t *
ch_new(cw_ch_t *a_ch, cw_mem_t *a_mem, cw_uint32_t a_table_size, cw_ch_hash_t
    *a_hash, cw_ch_key_comp_t *a_key_comp)
{
	cw_ch_t	*retval;

	_cw_assert(a_table_size > 0);

	if (NULL != a_ch) {
		retval = a_ch;
		bzero(retval, _CW_CH_TABLE2SIZEOF(a_table_size));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_ch_t
		    *)_cw_malloc(_CW_CH_TABLE2SIZEOF(a_table_size));
		if (NULL == retval)
			goto RETURN;
		bzero(retval, _CW_CH_TABLE2SIZEOF(a_table_size));
		retval->is_malloced = TRUE;
	}

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

	if (a_ch->chi_qr != NULL) {
		do {
			chi = a_ch->chi_qr;
			a_ch->chi_qr = qr_next(a_ch->chi_qr, ch_link);
			qr_remove(chi, ch_link);
			if (chi->is_malloced)
				_cw_free(chi);
#ifdef _LIBSTASH_DBG
			else
				memset(chi, 0x5a, sizeof(cw_chi_t));
#endif
		} while (chi != a_ch->chi_qr);
	}

	if (a_ch->is_malloced)
		_cw_free(a_ch);
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
		chi = (cw_chi_t *)_cw_malloc(sizeof(cw_chi_t));
		if (chi == NULL) {
			retval = TRUE;
			goto RETURN;
		}
		chi->is_malloced = TRUE;
	}
	chi->key = a_key;
	chi->data = a_data;
	qr_new(chi, ch_link);
	qr_new(chi, slot_link);
	slot = a_ch->hash(a_key) % a_ch->table_size;
	chi->slot = slot;

	/* Hook into ch-wide ring. */
	if (a_ch->chi_qr != NULL)
		qr_meld(a_ch->chi_qr, chi, ch_link);
	else
		a_ch->chi_qr = chi;

	if (a_ch->table[slot] != NULL) {
		/*
		 * Other chi's in this slot already.  Put this one at the
		 * head, in order to implement LIFO ordering for multiple
		 * chi's with the same key.
		 */
		qr_meld(chi, a_ch->table[slot], slot_link);

#ifdef _LIBSTASH_DBG
		a_ch->num_collisions++;
#endif
	}
	a_ch->table[slot] = chi;

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

	if (a_ch->table[slot] == NULL) {
		retval = TRUE;
		goto RETURN;
	}
	chi = a_ch->table[slot];
	do {
		/* Is this the chi we want? */
		if (a_ch->key_comp(a_search_key, chi->key) == TRUE) {
			/* Detach from ch-wide ring. */
			if (a_ch->chi_qr == chi) {
				a_ch->chi_qr = qr_next(a_ch->chi_qr, ch_link);
				if (a_ch->chi_qr == chi) {
					/* Last chi in the ch. */
					a_ch->chi_qr = NULL;
				}
			}
			qr_remove(chi, ch_link);
			
			/* Detach from the slot ring. */
			if (a_ch->table[slot] == chi) {
				a_ch->table[slot] = qr_next(a_ch->table[slot],
				    slot_link);
				if (a_ch->table[slot] == chi) {
					/* Last chi in this slot. */
					a_ch->table[slot] = NULL;
				}
			}
			qr_remove(chi, slot_link);

			if (r_key != NULL)
				*r_key = (void *)chi->key;
			if (r_data != NULL)
				*r_data = (void *)chi->data;
			if (chi->is_malloced)
				_cw_free(chi);
			else if (r_chi != NULL)
				*r_chi = chi;

			a_ch->count--;
#ifdef _LIBSTASH_DBG
			a_ch->num_removes++;
#endif
			retval = FALSE;
			goto RETURN;
		}
		chi = qr_next(chi, slot_link);
	} while (chi != a_ch->table[slot]);

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

	if (a_ch->table[slot] == NULL) {
		retval = TRUE;
		goto RETURN;
	}
	chi = a_ch->table[slot];
	do {
		/* Is this the chi we want? */
		if (a_ch->key_comp(a_key, chi->key) == TRUE) {
			if (r_data != NULL)
				*r_data = (void *)chi->data;
			retval = FALSE;
			goto RETURN;
		}
		chi = qr_next(chi, slot_link);
	} while (chi != a_ch->table[slot]);

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

	if (a_ch->chi_qr == NULL) {
		retval = TRUE;
		goto RETURN;
	}
	chi = a_ch->chi_qr;
	if (r_key != NULL)
		*r_key = (void *)chi->key;
	if (r_data != NULL)
		*r_data = (void *)chi->data;
	a_ch->chi_qr = qr_next(a_ch->chi_qr, ch_link);

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

	if (a_ch->chi_qr == NULL) {
		retval = TRUE;
		goto RETURN;
	}
	chi = a_ch->chi_qr;

	/* Detach from the ch-wide ring. */
	a_ch->chi_qr = qr_next(a_ch->chi_qr, ch_link);
	if (a_ch->chi_qr == chi) {
		/* Last chi in the ch. */
		a_ch->chi_qr = NULL;
	}
	qr_remove(chi, ch_link);
	
	/* Detach from the slot ring. */
	if (a_ch->table[chi->slot] == chi) {
		a_ch->table[chi->slot] = qr_next(a_ch->table[chi->slot],
		    slot_link);
		if (a_ch->table[chi->slot] == chi) {
			/* Last chi in this slot. */
			a_ch->table[chi->slot] = NULL;
		}
	}
	qr_remove(chi, slot_link);
	
	if (r_key != NULL)
		*r_key = (void *)chi->key;
	if (r_data != NULL)
		*r_data = (void *)chi->data;
	if (chi->is_malloced)
		_cw_free(chi);
	else if (r_chi != NULL)
		*r_chi = chi;

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
	_cw_out_put("[s]: chi_qr: 0x[p]\n",
	    a_prefix, a_ch->chi_qr);
	_cw_out_put("[s]: count: [i], table_size: [i]\n",
	    a_prefix, a_ch->count, a_ch->table_size);

	/* Table. */
	_cw_out_put("[s]: table --------------------------------------"
	    "----------------------\n", a_prefix);

	for (i = 0; i < a_ch->table_size; i++) {
		if (a_ch->table[i] != NULL) {
			chi = a_ch->table[i];
			do {
				_cw_out_put("[s]: [i]: key: 0x[p],"
				    " data: 0x[p], slot: [i]\n", a_prefix, i,
				    chi->key, chi->data, chi->slot);
				chi = qr_next(chi, slot_link);
			} while (chi != a_ch->table[i]);
		} else
			_cw_out_put("[s]: [i]: NULL\n", a_prefix, i);
	}

	/* chi ring. */
	_cw_out_put("[s]: chi_ring -----------------------------------"
	    "----------------------\n", a_prefix);
	if (a_ch->chi_qr != NULL) {
		chi = a_ch->chi_qr;
		do {
			_cw_out_put("[s]: key: 0x[p], data: 0x[p], slot: [i]\n",
			    a_prefix, chi->key, chi->data, chi->slot);
			chi = qr_next(chi, ch_link);
		} while (chi != a_ch->chi_qr);
	} else
		_cw_out_put("[s]: Empty\n", a_prefix);
}

cw_uint32_t
ch_hash_string(const void *a_key)
{
	cw_uint32_t	retval;
	char		*str;

	_cw_check_ptr(a_key);

	for (str = (char *)a_key, retval = 0; *str != 0; str++)
		retval = retval * 33 + *str;

	return retval;
}

cw_uint32_t
ch_hash_direct(const void *a_key)
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
ch_key_comp_string(const void *a_k1, const void *a_k2)
{
	_cw_check_ptr(a_k1);
	_cw_check_ptr(a_k2);

	return strcmp((char *)a_k1, (char *)a_k2) ? FALSE : TRUE;
}

cw_bool_t
ch_key_comp_direct(const void *a_k1, const void *a_k2)
{
	return (a_k1 == a_k2) ? TRUE : FALSE;
}
