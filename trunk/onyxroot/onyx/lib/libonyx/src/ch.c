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
#define CW_CH_MAGIC 0x574936af
#define CW_CHI_MAGIC 0xabdcee0e
#endif

cw_ch_t *
ch_new(cw_ch_t *a_ch, cw_mema_t *a_mema, cw_uint32_t a_table_size,
       cw_ch_hash_t *a_hash, cw_ch_key_comp_t *a_key_comp)
{
    cw_ch_t *retval;

    cw_check_ptr(a_mema);
    cw_check_ptr(mema_alloc_get(a_mema));
    cw_check_ptr(mema_calloc_get(a_mema));
    cw_check_ptr(mema_dealloc_get(a_mema));
    cw_assert(a_table_size > 0);
    cw_check_ptr(a_hash);
    cw_check_ptr(a_key_comp);

    if (a_ch != NULL)
    {
	retval = a_ch;
	memset(retval, 0, CW_CH_TABLE2SIZEOF(a_table_size));
	retval->is_malloced = FALSE;
    }
    else
    {
	retval = (cw_ch_t *) cw_opaque_calloc(mema_calloc_get(a_mema),
					      mema_arg_get(a_mema), 1,
					      CW_CH_TABLE2SIZEOF(a_table_size));
	retval->is_malloced = TRUE;
    }

    retval->mema = a_mema;
    retval->table_size = a_table_size;
    retval->hash = a_hash;
    retval->key_comp = a_key_comp;

#ifdef CW_DBG
    retval->magic = CW_CH_MAGIC;
#endif

    return retval;
}

void
ch_delete(cw_ch_t *a_ch)
{
    cw_chi_t *chi;
    cw_uint32_t i;

    cw_check_ptr(a_ch);
    cw_dassert(a_ch->magic == CW_CH_MAGIC);

#ifdef CW_CH_VERBOSE
    fprintf(stderr,
	    "%s(%p): num_collisions: %llu, num_inserts: %llu,"
	    " num_removes: %llu, num_searches: %llu\n",
	    __func__, a_ch, a_ch->num_collisions, a_ch->num_inserts,
	    a_ch->num_removes, a_ch->num_searches);
#endif

    for (i = 0; i < a_ch->table_size; i++)
    {
	while ((chi = ql_first(&a_ch->table[i])) != NULL)
	{
	    cw_dassert(chi->magic == CW_CHI_MAGIC);
	    ql_head_remove(&a_ch->table[i], cw_chi_t, slot_link);
	    if (chi->is_malloced)
	    {
		cw_opaque_dealloc(mema_dealloc_get(a_ch->mema),
				  mema_arg_get(a_ch->mema), chi,
				  sizeof(cw_chi_t));
	    }
#ifdef CW_DBG
	    else
	    {
		memset(chi, 0x5a, sizeof(cw_chi_t));
	    }
#endif
	}
    }

    if (a_ch->is_malloced)
    {
	cw_opaque_dealloc(mema_dealloc_get(a_ch->mema),
			  mema_arg_get(a_ch->mema), a_ch,
			  CW_CH_TABLE2SIZEOF(a_ch->table_size));
    }
#ifdef CW_DBG
    else
    {
	memset(a_ch, 0x5a, CW_CH_TABLE2SIZEOF(a_ch->table_size));
    }
#endif
}

cw_uint32_t
ch_count(cw_ch_t *a_ch)
{
    cw_check_ptr(a_ch);
    cw_dassert(a_ch->magic == CW_CH_MAGIC);

    return a_ch->count;
}

void
ch_insert(cw_ch_t *a_ch, const void *a_key, const void *a_data,
	  cw_chi_t *a_chi)
{
    cw_uint32_t slot;
    cw_chi_t *chi;

    cw_check_ptr(a_ch);
    cw_dassert(a_ch->magic == CW_CH_MAGIC);

    /* Initialize chi. */
    if (a_chi != NULL)
    {
	chi = a_chi;
	chi->is_malloced = FALSE;
    }
    else
    {
	chi = (cw_chi_t *) cw_opaque_alloc(mema_alloc_get(a_ch->mema),
					   mema_arg_get(a_ch->mema),
					   sizeof(cw_chi_t));
	chi->is_malloced = TRUE;
    }
    chi->key = a_key;
    chi->data = a_data;
    ql_elm_new(chi, slot_link);
    slot = a_ch->hash(a_key) % a_ch->table_size;
    chi->slot = slot;
#ifdef CW_DBG
    chi->magic = CW_CHI_MAGIC;
#endif

    /* Hook into the slot list. */
#ifdef CW_CH_COUNT
    if (ql_first(&a_ch->table[slot]) != NULL)
    {
	a_ch->num_collisions++;
    }
#endif
    ql_head_insert(&a_ch->table[slot], chi, slot_link);

    a_ch->count++;
#ifdef CW_CH_COUNT
    a_ch->num_inserts++;
#endif
}

cw_bool_t
ch_remove(cw_ch_t *a_ch, const void *a_search_key, void **r_key, void **r_data,
	  cw_chi_t **r_chi)
{
    cw_bool_t retval;
    cw_uint32_t slot;
    cw_chi_t *chi;

    cw_check_ptr(a_ch);
    cw_dassert(a_ch->magic == CW_CH_MAGIC);

    slot = a_ch->hash(a_search_key) % a_ch->table_size;

    for (chi = ql_first(&a_ch->table[slot]);
	 chi != NULL;
	 chi = ql_next(&a_ch->table[slot], chi, slot_link))
    {
	cw_check_ptr(chi);
	cw_dassert(chi->magic == CW_CHI_MAGIC);

	/* Is this the chi we want? */
	if (a_ch->key_comp(a_search_key, chi->key))
	{
	    /* Detach from the slot list. */
	    ql_remove(&a_ch->table[slot], chi, slot_link);

	    if (r_key != NULL)
	    {
		*r_key = (void *) chi->key;
	    }
	    if (r_data != NULL)
	    {
		*r_data = (void *) chi->data;
	    }
	    if (chi->is_malloced)
	    {
		cw_opaque_dealloc(mema_dealloc_get(a_ch->mema),
				  mema_arg_get(a_ch->mema), chi,
				  sizeof(cw_chi_t));
	    }
	    else if (r_chi != NULL)
	    {
#ifdef CW_DBG
		memset(chi, 0x5a, sizeof(cw_chi_t));
#endif
		*r_chi = chi;
	    }

	    a_ch->count--;
#ifdef CW_CH_COUNT
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

void
ch_chi_remove(cw_ch_t *a_ch, cw_chi_t *a_chi)
{
    cw_check_ptr(a_ch);
    cw_dassert(a_ch->magic == CW_CH_MAGIC);
    cw_check_ptr(a_chi);
    cw_dassert(a_chi->magic == CW_CHI_MAGIC);

    /* Detach from the slot list. */
    ql_remove(&a_ch->table[a_chi->slot], a_chi, slot_link);

    if (a_chi->is_malloced)
    {
	cw_opaque_dealloc(mema_dealloc_get(a_ch->mema),
			  mema_arg_get(a_ch->mema), a_chi,
			  sizeof(cw_chi_t));
    }
#ifdef CW_DBG
    else
    {
	memset(a_chi, 0x5a, sizeof(cw_chi_t));
    }
#endif

    a_ch->count--;
#ifdef CW_CH_COUNT
    a_ch->num_removes++;
#endif
}

cw_bool_t
ch_search(cw_ch_t *a_ch, const void *a_key, void **r_data)
{
    cw_bool_t retval;
    cw_uint32_t slot;
    cw_chi_t *chi;

    cw_check_ptr(a_ch);
    cw_dassert(a_ch->magic == CW_CH_MAGIC);

    slot = a_ch->hash(a_key) % a_ch->table_size;

    for (chi = ql_first(&a_ch->table[slot]);
	 chi != NULL;
	 chi = ql_next(&a_ch->table[slot], chi, slot_link))
    {
	cw_check_ptr(chi);
	cw_dassert(chi->magic == CW_CHI_MAGIC);

	/* Is this the chi we want? */
	if (a_ch->key_comp(a_key, chi->key))
	{
	    if (r_data != NULL)
	    {
		*r_data = (void *) chi->data;
	    }
	    retval = FALSE;
	    goto RETURN;
	}
    }

    retval = TRUE;
    RETURN:
#ifdef CW_CH_COUNT
    a_ch->num_searches++;
#endif
    return retval;
}

#if (0) /* XXX */
cw_bool_t
ch_get_iterate(cw_ch_t *a_ch, void **r_key, void **r_data)
{
    cw_bool_t retval;
    cw_chi_t *chi;

    cw_check_ptr(a_ch);
    cw_dassert(a_ch->magic == CW_CH_MAGIC);

    chi = ql_first(&a_ch->chi_ql);
    if (chi == NULL)
    {
	retval = TRUE;
	goto RETURN;
    }
    cw_check_ptr(chi);
    cw_dassert(chi->magic == CW_CHI_MAGIC);
    if (r_key != NULL)
    {
	*r_key = (void *) chi->key;
    }
    if (r_data != NULL)
    {
	*r_data = (void *) chi->data;
    }

    /* Rotate the list. */
    ql_first(&a_ch->chi_ql) = qr_next(ql_first(&a_ch->chi_ql), ch_link);

    retval = FALSE;
    RETURN:
    return retval;
}

cw_bool_t
ch_remove_iterate(cw_ch_t *a_ch, void **r_key, void **r_data, cw_chi_t **r_chi)
{
    cw_bool_t retval;
    cw_chi_t *chi;

    cw_check_ptr(a_ch);
    cw_dassert(a_ch->magic == CW_CH_MAGIC);

    chi = ql_first(&a_ch->chi_ql);
    if (chi == NULL)
    {
	retval = TRUE;
	goto RETURN;
    }
    cw_check_ptr(chi);
    cw_dassert(chi->magic == CW_CHI_MAGIC);

    /* Detach from the ch-wide list. */
    ql_remove(&a_ch->chi_ql, chi, ch_link);

    /* Detach from the slot list. */
    ql_remove(&a_ch->table[chi->slot], chi, slot_link);

    if (r_key != NULL)
    {
	*r_key = (void *) chi->key;
    }
    if (r_data != NULL)
    {
	*r_data = (void *) chi->data;
    }
    if (chi->is_malloced)
    {
	cw_opaque_dealloc(mema_dealloc_get(a_ch->mema),
			  mema_arg_get(a_ch->mema), chi, sizeof(cw_chi_t));
    }
    else if (r_chi != NULL)
    {
#ifdef CW_DBG
	memset(chi, 0x5a, sizeof(cw_chi_t));
#endif
	*r_chi = chi;
    }

    a_ch->count--;
#ifdef CW_CH_COUNT
    a_ch->num_removes++;
#endif

    retval = FALSE;
    RETURN:
    return retval;
}
#endif

cw_uint32_t
ch_string_hash(const void *a_key)
{
    cw_uint32_t retval, c;
    cw_uint8_t *str;

    cw_check_ptr(a_key);

    for (str = (cw_uint8_t *) a_key, retval = 5381; (c = *str) != 0; str++)
    {
	retval = ((retval << 5) + retval) + c;
    }

    return retval;
}

cw_uint32_t
ch_direct_hash(const void *a_key)
{
#if (SIZEOF_INT_P == 4)
    cw_uint32_t t = (cw_uint32_t) a_key;
#elif (SIZEOF_INT_P == 8)
    cw_uint64_t t = (cw_uint64_t) a_key;
#else
#error Unsupported pointer size
#endif

    /* Shift right until we've shifted one 1 bit off. */
#if (SIZEOF_INT_P == 8)
    t >>= 32 * !(t & 0xffffffff);
#endif
    t >>= 16 * !(t & 0xffff);
    t >>= 8 * !(t & 0xff);
    t >>= 4 * !(t & 0xf);
    t >>= 2 * !(t & 0x3);
    t >>= 1 * !(t & 0x1);
    t >>= 1;

    return t;
}

cw_bool_t
ch_string_key_comp(const void *a_k1, const void *a_k2)
{
    cw_check_ptr(a_k1);
    cw_check_ptr(a_k2);

    return strcmp((char *) a_k1, (char *) a_k2) ? FALSE : TRUE;
}

cw_bool_t
ch_direct_key_comp(const void *a_k1, const void *a_k2)
{
    return (a_k1 == a_k2) ? TRUE : FALSE;
}
