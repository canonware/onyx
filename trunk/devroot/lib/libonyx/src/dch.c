/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
******************************************************************************
*
* <Copyright = jasone>
* <License>
*
******************************************************************************
*
* Version: Onyx <Version = onyx>
*
******************************************************************************/

#include "../include/libonyx/libonyx.h"

#ifdef CW_DBG
#define CW_DCH_MAGIC 0x4327589e
#endif

static void
dch_p_grow(cw_dch_t *a_dch);
static void
dch_p_shrink(cw_dch_t *a_dch);
static void
dch_p_insert(cw_ch_t *a_ch, cw_chi_t * a_chi);

cw_dch_t *
dch_new(cw_dch_t *a_dch, cw_mema_t *a_mema, cw_uint32_t a_base_table,
	cw_uint32_t a_base_grow, cw_uint32_t a_base_shrink,
	cw_ch_hash_t *a_hash, cw_ch_key_comp_t *a_key_comp)
{
    cw_dch_t *retval;

    cw_check_ptr(a_mema);
    cw_check_ptr(mema_alloc_get(a_mema));
    cw_check_ptr(mema_calloc_get(a_mema));
    cw_check_ptr(mema_dealloc_get(a_mema));
    cw_assert(a_base_table > 0);
    cw_assert(a_base_grow > 0);
    cw_assert(a_base_grow > a_base_shrink);
    cw_check_ptr(a_hash);
    cw_check_ptr(a_key_comp);

    if (a_dch != NULL)
    {
	retval = a_dch;
	memset(retval, 0, sizeof(cw_dch_t));
	retval->is_malloced = FALSE;
    }
    else
    {
	retval = (cw_dch_t *) cw_opaque_calloc(mema_calloc_get(a_mema), 1,
					       mema_arg_get(a_mema),
					       sizeof(cw_dch_t));
	retval->is_malloced = TRUE;
    }

    retval->mema = a_mema;
    retval->base_table = a_base_table;
    retval->base_grow = a_base_grow;
    retval->base_shrink = a_base_shrink;
    retval->shrinkable = TRUE;
    retval->grow_factor = 1;
    retval->hash = a_hash;
    retval->key_comp = a_key_comp;

    xep_begin();
    volatile cw_dch_t *v_retval;
    xep_try
    {
	v_retval = retval;
	retval->ch = ch_new(NULL, a_mema, retval->base_table, retval->hash,
			    retval->key_comp);
    }
    xep_catch(CW_ONYXX_OOM)
    {
	retval = (cw_dch_t *) v_retval;
	if (a_dch->is_malloced)
	{
	    cw_opaque_dealloc(mema_dealloc_get(a_mema), mema_arg_get(a_mema),
			      retval, sizeof(cw_dch_t));
	}
    }
    xep_end();

#ifdef CW_DBG
    retval->magic = CW_DCH_MAGIC;
#endif

    return retval;
}

void
dch_delete(cw_dch_t *a_dch)
{
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

#ifdef CW_DCH_VERBOSE
    fprintf(stderr,
	    "%s(%p): num_collisions: %llu, num_inserts: %llu,"
	    " num_removes: %llu, num_searches: %llu, num_grows: %llu,"
	    " num_shrinks: %llu\n",
	    __FUNCTION__, a_dch, a_dch->ch->num_collisions,
	    a_dch->ch->num_inserts, a_dch->ch->num_removes,
	    a_dch->ch->num_searches, a_dch->num_grows, a_dch->num_shrinks);
#endif

    ch_delete(a_dch->ch);

    if (a_dch->is_malloced)
    {
	cw_opaque_dealloc(mema_dealloc_get(a_dch->mema),
			  mema_arg_get(a_dch->mema), a_dch, sizeof(cw_dch_t));
    }
#ifdef CW_DBG
    else
    {
	memset(a_dch, 0x5a, sizeof(cw_dch_t));
    }
#endif
}

cw_uint32_t
dch_count(cw_dch_t *a_dch)
{
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    return ch_count(a_dch->ch);
}

cw_bool_t
dch_shrinkable_get(cw_dch_t *a_dch)
{
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    return a_dch->shrinkable;
}

void
dch_shrinkable_set(cw_dch_t *a_dch, cw_bool_t a_shrinkable)
{
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    a_dch->shrinkable = a_shrinkable;
}

void
dch_insert(cw_dch_t *a_dch, const void *a_key, const void *a_data,
	   cw_chi_t *a_chi)
{
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    dch_p_grow(a_dch);
    ch_insert(a_dch->ch, a_key, a_data, a_chi);
}

cw_bool_t
dch_remove(cw_dch_t *a_dch, const void *a_search_key, void **r_key,
	   void **r_data, cw_chi_t **r_chi)
{
    cw_bool_t retval;

    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    if (a_dch->shrinkable)
    {
	dch_p_shrink(a_dch);
    }
    if (ch_remove(a_dch->ch, a_search_key, r_key, r_data, r_chi))
    {
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
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    return ch_search(a_dch->ch, a_key, r_data);
}

cw_bool_t
dch_get_iterate(cw_dch_t *a_dch, void **r_key, void **r_data)
{
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    return ch_get_iterate(a_dch->ch, r_key, r_data);
}

cw_bool_t
dch_remove_iterate(cw_dch_t *a_dch, void **r_key, void **r_data,
		   cw_chi_t **r_chi)
{
    cw_bool_t retval;

    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    if (a_dch->shrinkable)
    {
	dch_p_shrink(a_dch);
    }
    if (ch_remove_iterate(a_dch->ch, r_key, r_data, r_chi))
    {
	retval = TRUE;
	goto RETURN;
    }
    retval = FALSE;
    RETURN:
    return retval;
}

/* Given the ch API, there is no way to both safely and efficiently transfer the
 * contents of one ch to another.  Therefore, this function mucks with ch
 * internals. */
static void
dch_p_grow(cw_dch_t *a_dch)
{
    cw_ch_t *t_ch;
    cw_chi_t *chi;
    cw_uint32_t count, i;

    count = ch_count(a_dch->ch);

    if ((count + 1) > (a_dch->grow_factor * a_dch->base_grow))
    {
	/* Too big.  Create a new ch twice as large and populate it. */
	t_ch = ch_new(NULL, a_dch->mema,
		      a_dch->base_table * a_dch->grow_factor * 2,
		      a_dch->hash, a_dch->key_comp);
	for (i = 0; i < count; i++)
	{
	    chi = ql_first(&a_dch->ch->chi_ql);
	    ql_remove(&a_dch->ch->chi_ql, chi, ch_link);
	    ql_elm_new(chi, slot_link);
	    dch_p_insert(t_ch, chi);
	}

	a_dch->grow_factor *= 2;
#ifdef CW_DCH_COUNT
	a_dch->num_grows++;
	t_ch->num_collisions += a_dch->ch->num_collisions;
	t_ch->num_inserts += a_dch->ch->num_inserts;
	t_ch->num_removes += a_dch->ch->num_removes;
	t_ch->num_searches += a_dch->ch->num_searches;
#endif
	/* Set to NULL to keep ch_delete() from deleting all the items. */
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
    cw_ch_t *t_ch;
    cw_chi_t *chi;
    cw_uint32_t count, i;

    count = ch_count(a_dch->ch);

    if ((count - 1 < a_dch->base_shrink * a_dch->grow_factor)
	&& (a_dch->grow_factor > 1))
    {
	cw_uint32_t new_factor;

	/*
	 * Too big.  Create a new ch with the smallest grow factor that
	 * does not cause the ch to be overflowed.
	 */
	for (new_factor = 1; new_factor * a_dch->base_grow <= count - 1;
	     new_factor *= 2)
	{
	    cw_assert(new_factor < a_dch->grow_factor);
	}
	cw_assert(new_factor > 0);
	cw_assert(new_factor < a_dch->grow_factor);

	t_ch = ch_new(NULL, a_dch->mema,
		      a_dch->base_table * new_factor, a_dch->hash,
		      a_dch->key_comp);
	for (i = 0; i < count; i++)
	{
	    chi = ql_first(&a_dch->ch->chi_ql);
	    ql_remove(&a_dch->ch->chi_ql, chi, ch_link);
	    ql_elm_new(chi, slot_link);
	    dch_p_insert(t_ch, chi);
	}

	a_dch->grow_factor = new_factor;
#ifdef CW_DCH_COUNT
	a_dch->num_shrinks++;
	t_ch->num_collisions += a_dch->ch->num_collisions;
	t_ch->num_inserts += a_dch->ch->num_inserts;
	t_ch->num_removes += a_dch->ch->num_removes;
	t_ch->num_searches += a_dch->ch->num_searches;
#endif
	/* Set to NULL to keep ch_delete() from deleting all the items. */
	ql_first(&a_dch->ch->chi_ql) = NULL;
	ch_delete(a_dch->ch);
	a_dch->ch = t_ch;
    }
}

/* Given the ch API, there is no way to both safely and efficiently transfer the
 * contents of one ch to another.  Therefore, this function mucks with ch
 * internals. */
static void
dch_p_insert(cw_ch_t *a_ch, cw_chi_t *a_chi)
{
    cw_uint32_t slot;

    /* Initialize a_chi. */
    slot = a_ch->hash(a_chi->key) % a_ch->table_size;
    a_chi->slot = slot;

    /* Hook into ch-wide list. */
    ql_tail_insert(&a_ch->chi_ql, a_chi, ch_link);

    /* Hook into the slot list. */
#ifdef CW_DCH_COUNT
    if (ql_first(&a_ch->table[slot]) != NULL)
    {
	a_ch->num_collisions++;
    }
#endif
    ql_head_insert(&a_ch->table[slot], a_chi, slot_link);

    a_ch->count++;
#ifdef CW_DCH_COUNT
    a_ch->num_inserts++;
#endif
}
