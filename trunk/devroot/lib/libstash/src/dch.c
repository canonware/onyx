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
#define _CW_DCH_MAGIC 0x4327589e
#endif

static cw_bool_t dch_p_grow(cw_dch_t *a_dch);
static cw_bool_t dch_p_shrink(cw_dch_t *a_dch);
static void dch_p_insert(cw_ch_t *a_ch, cw_chi_t * a_chi);

cw_dch_t *
dch_new(cw_dch_t *a_dch, cw_uint32_t a_base_table, cw_uint32_t a_base_grow,
    cw_uint32_t a_base_shrink, cw_ch_hash_t *a_hash, cw_ch_key_comp_t
    *a_key_comp)
{
	cw_dch_t	*retval;

	_cw_assert(a_base_table > 0);
	_cw_assert(a_base_grow > 0);
	_cw_assert(a_base_grow > a_base_shrink);

	if (a_dch != NULL) {
		retval = a_dch;
		bzero(retval, sizeof(cw_dch_t));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_dch_t *)_cw_malloc(sizeof(cw_dch_t));
		if (retval == NULL)
			goto RETURN;
		bzero(retval, sizeof(cw_dch_t));
		retval->is_malloced = TRUE;
	}

	retval->base_table = a_base_table;
	retval->base_grow = a_base_grow;
	retval->base_shrink = a_base_shrink;
	retval->grow_factor = 1;
	retval->hash = a_hash;
	retval->key_comp = a_key_comp;

	retval->ch = ch_new(NULL, retval->base_table, retval->hash,
	    retval->key_comp);
	if (retval == NULL) {
		if (a_dch->is_malloced)
			_cw_free(a_dch);
		goto RETURN;
	}
#ifdef _LIBSTASH_DBG
	retval->magic = _CW_DCH_MAGIC;
#endif

	RETURN:
	return retval;
}

void
dch_delete(cw_dch_t *a_dch)
{
	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);

	ch_delete(a_dch->ch);

	if (TRUE == a_dch->is_malloced)
		_cw_free(a_dch);
#ifdef _LIBSTASH_DBG
	else
		memset(a_dch, 0x5a, sizeof(cw_dch_t));
#endif
}

cw_uint32_t
dch_count(cw_dch_t *a_dch)
{
	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);

	return ch_count(a_dch->ch);
}

cw_bool_t
dch_insert(cw_dch_t *a_dch, const void *a_key, const void *a_data, cw_chi_t
    *a_chi)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);

	if (dch_p_grow(a_dch)) {
		retval = TRUE;
		goto RETURN;
	}
	if (ch_insert(a_dch->ch, a_key, a_data, a_chi)) {
		retval = TRUE;
		goto RETURN;
	}
	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
dch_remove(cw_dch_t *a_dch, const void *a_search_key, void **r_key, void
    **r_data, cw_chi_t **r_chi)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);

	if (dch_p_shrink(a_dch)) {
		retval = TRUE;
		goto RETURN;
	}
	if (ch_remove(a_dch->ch, a_search_key, r_key, r_data, r_chi)) {
		retval = TRUE;
		goto RETURN;
	}
	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
dch_search(cw_dch_t *a_dch, const void *a_key, void **r_data)
{
	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);

	return ch_search(a_dch->ch, a_key, r_data);
}

cw_bool_t
dch_get_iterate(cw_dch_t *a_dch, void **r_key, void **r_data)
{
	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);

	return ch_get_iterate(a_dch->ch, r_key, r_data);
}

cw_bool_t
dch_remove_iterate(cw_dch_t *a_dch, void **r_key, void **r_data, cw_chi_t
    **r_chi)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);

	if (dch_p_shrink(a_dch)) {
		retval = TRUE;
		goto RETURN;
	}
	if (ch_remove_iterate(a_dch->ch, r_key, r_data, r_chi)) {
		retval = TRUE;
		goto RETURN;
	}
	retval = FALSE;
	RETURN:
	return retval;
}

void
dch_dump(cw_dch_t *a_dch, const char *a_prefix)
{
	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);
	_cw_check_ptr(a_prefix);

#ifdef _LIBSTASH_DBG
	_cw_out_put("[s]: num_grows: [i], num_shrinks: [i]\n",
	    a_prefix, a_dch->num_grows, a_dch->num_shrinks);
#endif
	_cw_out_put("[s]: is_malloced: [s]\n",
	    a_prefix, (a_dch->is_malloced) ? "TRUE" : "FALSE");
	_cw_out_put("[s]: base_table: [i], base_grow: [i], base_shrink: [i]\n",
	    a_prefix, a_dch->base_table, a_dch->base_grow,
	    a_dch->base_shrink);
	_cw_out_put("[s]: grow_factor: [i]\n",
	    a_prefix, a_dch->grow_factor);

	ch_dump(a_dch->ch, a_prefix);
}

/* Given the ch API, there is no way to both safely and efficiently transfer the
 * contents of one ch to another.  Therefore, this function mucks with ch
 * internals. */
static cw_bool_t
dch_p_grow(cw_dch_t *a_dch)
{
	cw_bool_t	retval;
	cw_ch_t		*t_ch;
	cw_chi_t	*chi;
	cw_uint32_t	count, i;

	count = ch_count(a_dch->ch);

	if ((count + 1) > (a_dch->grow_factor * a_dch->base_grow)) {
		/* Too big.  Create a new ch twice as large and populate it. */
		t_ch = ch_new(NULL, a_dch->base_table * a_dch->grow_factor * 2,
		    a_dch->hash, a_dch->key_comp);
		if (t_ch == NULL) {
			retval = TRUE;
			goto RETURN;
		}
		for (i = 0; i < count; i++) {
			chi = a_dch->ch->chi_qr;
			a_dch->ch->chi_qr = qr_next(a_dch->ch->chi_qr, ch_link);
			qr_remove(chi, ch_link);
			qr_remove(chi, slot_link);
			dch_p_insert(t_ch, chi);
		}

		a_dch->grow_factor *= 2;
#ifdef _LIBSTASH_DBG
		a_dch->num_grows++;
		t_ch->num_collisions += a_dch->ch->num_collisions;
		t_ch->num_inserts += a_dch->ch->num_inserts;
		t_ch->num_removes += a_dch->ch->num_removes;
#endif
		/*
		 * Set to NULL to keep ch_delete() from deleting all the
		 * items.
		 */
		a_dch->ch->chi_qr = NULL;
		ch_delete(a_dch->ch);
		a_dch->ch = t_ch;
	}

	retval = FALSE;
	RETURN:
	return retval;
}

/* Given the ch API, there is no way to both safely and efficiently transfer the
 * contents of one ch to another.  Therefore, this function mucks with ch
 * internals. */
static cw_bool_t
dch_p_shrink(cw_dch_t *a_dch)
{
	cw_bool_t	retval;
	cw_ch_t		*t_ch;
	cw_chi_t	*chi;
	cw_uint32_t	count, i;

	count = ch_count(a_dch->ch);

	if ((count - 1) < (a_dch->grow_factor > 1) && ((a_dch->grow_factor *
	    a_dch->base_shrink))) {
		/* Too big.  Create a new ch half as large and populate it. */
		t_ch = ch_new(NULL, a_dch->base_table * a_dch->grow_factor / 2,
		    a_dch->hash, a_dch->key_comp);
		if (t_ch == NULL) {
			retval = TRUE;
			goto RETURN;
		}
		for (i = 0; i < count; i++) {
			chi = a_dch->ch->chi_qr;
			a_dch->ch->chi_qr = qr_next(a_dch->ch->chi_qr, ch_link);
			qr_remove(chi, ch_link);
			qr_remove(chi, slot_link);
			dch_p_insert(t_ch, chi);
		}

		a_dch->grow_factor /= 2;
#ifdef _LIBSTASH_DBG
		a_dch->num_shrinks++;
		t_ch->num_collisions += a_dch->ch->num_collisions;
		t_ch->num_inserts += a_dch->ch->num_inserts;
		t_ch->num_removes += a_dch->ch->num_removes;
#endif
		/*
		 * Set to NULL to keep ch_delete() from deleting all the
		 * items.
		 */
		a_dch->ch->chi_qr = NULL;
		ch_delete(a_dch->ch);
		a_dch->ch = t_ch;
	}

	retval = FALSE;
	RETURN:
	return retval;
}

/* Given the ch API, there is no way to both safely and efficiently transfer the
 * contents of one ch to another.  Therefore, this function mucks with ch
 * internals. */
static void
dch_p_insert(cw_ch_t *a_ch, cw_chi_t * a_chi)
{
	cw_uint32_t	slot;

	/* Initialize a_chi. */
	slot = a_ch->hash(a_chi->key) % a_ch->table_size;
	a_chi->slot = slot;

	/* Hook into ch-wide ring. */
	if (a_ch->chi_qr != NULL)
		qr_meld(a_ch->chi_qr, a_chi, ch_link);
	else
		a_ch->chi_qr = a_chi;

	if (a_ch->table[slot] != NULL) {
		/*
		 * Other chi's in this slot already.  Put this one at the
		 * head, in order to implement LIFO ordering for multiple
		 * chi's with the same key.
		 */
		qr_meld(a_chi, a_ch->table[slot], slot_link);

#ifdef _LIBSTASH_DBG
		a_ch->num_collisions++;
#endif
	}
	a_ch->table[slot] = a_chi;

	a_ch->count++;
#ifdef _LIBSTASH_DBG
	a_ch->num_inserts++;
#endif
}
