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
#define _CW_DCH_MAGIC 0x4327589e
#endif

static void dch_p_grow(cw_dch_t *a_dch);
static void dch_p_shrink(cw_dch_t *a_dch);
static void dch_p_insert(cw_ch_t *a_ch, cw_chi_t * a_chi);

cw_dch_t *
dch_new(cw_dch_t *a_dch, cw_mem_t *a_mem, cw_uint32_t a_base_table, cw_uint32_t
    a_base_grow, cw_uint32_t a_base_shrink, cw_ch_hash_t *a_hash,
    cw_ch_key_comp_t *a_key_comp)
{
	cw_dch_t	*retval;

	_cw_assert(a_base_table > 0);
	_cw_assert(a_base_grow > 0);
	_cw_assert(a_base_grow > a_base_shrink);

	if (a_dch != NULL) {
		retval = a_dch;
		memset(retval, 0, sizeof(cw_dch_t));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_dch_t *)mem_malloc(a_mem, sizeof(cw_dch_t));
		memset(retval, 0, sizeof(cw_dch_t));
		retval->is_malloced = TRUE;
	}

	retval->mem = a_mem;
	retval->base_table = a_base_table;
	retval->base_grow = a_base_grow;
	retval->base_shrink = a_base_shrink;
	retval->grow_factor = 1;
	retval->hash = a_hash;
	retval->key_comp = a_key_comp;

	xep_begin();
	volatile cw_dch_t	*v_retval;
	xep_try {
		v_retval = retval;
		retval->ch = ch_new(NULL, a_mem, retval->base_table,
		    retval->hash, retval->key_comp);
	}
	xep_catch(_CW_STASHX_OOM) {
		retval = (cw_dch_t *)v_retval;
		if (a_dch->is_malloced)
			mem_free(a_mem, retval);
	}
	xep_end();

#ifdef _LIBSTASH_DBG
	retval->magic = _CW_DCH_MAGIC;
#endif

	return retval;
}

void
dch_delete(cw_dch_t *a_dch)
{
	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);

	ch_delete(a_dch->ch);

	if (TRUE == a_dch->is_malloced)
		mem_free(a_dch->mem, a_dch);
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

void
dch_insert(cw_dch_t *a_dch, const void *a_key, const void *a_data, cw_chi_t
    *a_chi)
{
	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);

	dch_p_grow(a_dch);
	ch_insert(a_dch->ch, a_key, a_data, a_chi);
}

cw_bool_t
dch_remove(cw_dch_t *a_dch, const void *a_search_key, void **r_key, void
    **r_data, cw_chi_t **r_chi)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_dch);
	_cw_assert(a_dch->magic == _CW_DCH_MAGIC);

	dch_p_shrink(a_dch);
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

	dch_p_shrink(a_dch);
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
	out_put(out_err, "[s]: num_grows: [i], num_shrinks: [i]\n",
	    a_prefix, a_dch->num_grows, a_dch->num_shrinks);
#endif
	out_put(out_err, "[s]: is_malloced: [s]\n",
	    a_prefix, (a_dch->is_malloced) ? "TRUE" : "FALSE");
	out_put(out_err,
	    "[s]: base_table: [i], base_grow: [i], base_shrink: [i]\n",
	    a_prefix, a_dch->base_table, a_dch->base_grow,
	    a_dch->base_shrink);
	out_put(out_err, "[s]: grow_factor: [i]\n",
	    a_prefix, a_dch->grow_factor);

	ch_dump(a_dch->ch, a_prefix);
}

/* Given the ch API, there is no way to both safely and efficiently transfer the
 * contents of one ch to another.  Therefore, this function mucks with ch
 * internals. */
static void
dch_p_grow(cw_dch_t *a_dch)
{
	cw_ch_t		*t_ch;
	cw_chi_t	*chi;
	cw_uint32_t	count, i;

	count = ch_count(a_dch->ch);

	if ((count + 1) > (a_dch->grow_factor * a_dch->base_grow)) {
		/* Too big.  Create a new ch twice as large and populate it. */
		t_ch = ch_new(NULL, a_dch->mem, a_dch->base_table *
		    a_dch->grow_factor * 2, a_dch->hash, a_dch->key_comp);
		for (i = 0; i < count; i++) {
			chi = ql_first(&a_dch->ch->chi_ql);
			ql_remove(&a_dch->ch->chi_ql, chi, ch_link);
			ql_elm_new(chi, slot_link);
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
		ql_first(&a_dch->ch->chi_ql) = NULL;
		ch_delete(a_dch->ch);
		a_dch->ch = t_ch;
	}
}

/* Given the ch API, there is no way to both safely and efficiently transfer the
 * contents of one ch to another.  Therefore, this function mucks with ch
 * internals. */
static void
dch_p_shrink(cw_dch_t *a_dch)
{
	cw_ch_t		*t_ch;
	cw_chi_t	*chi;
	cw_uint32_t	count, i;

	count = ch_count(a_dch->ch);

	if ((count - 1 < a_dch->base_shrink * a_dch->grow_factor) &&
	    (a_dch->grow_factor > 1)) {
		cw_uint32_t	new_factor;

		/*
		 * Too big.  Create a new ch with the smallest grow factor that
		 * does not cause the ch to be overflowed.
		 */
		for (new_factor = 1; new_factor * a_dch->base_grow <= count - 1;
		     new_factor *= 2) {
			_cw_assert(new_factor < a_dch->grow_factor);
		}
		_cw_assert(new_factor > 0);
		_cw_assert(new_factor < a_dch->grow_factor);

		t_ch = ch_new(NULL, a_dch->mem, a_dch->base_table * new_factor,
		    a_dch->hash, a_dch->key_comp);
		for (i = 0; i < count; i++) {
			chi = ql_first(&a_dch->ch->chi_ql);
			ql_remove(&a_dch->ch->chi_ql, chi, ch_link);
			ql_elm_new(chi, slot_link);
			dch_p_insert(t_ch, chi);
		}

		a_dch->grow_factor = new_factor;
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
		ql_first(&a_dch->ch->chi_ql) = NULL;
		ch_delete(a_dch->ch);
		a_dch->ch = t_ch;
	}
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

	/* Hook into ch-wide list. */
	ql_tail_insert(&a_ch->chi_ql, a_chi, ch_link);

	/* Hook into the slot list. */
#ifdef _LIBSTASH_DBG
	if (ql_first(&a_ch->table[slot]) != NULL)
		a_ch->num_collisions++;
#endif
	ql_head_insert(&a_ch->table[slot], a_chi, slot_link);

	a_ch->count++;
#ifdef _LIBSTASH_DBG
	a_ch->num_inserts++;
#endif
}
