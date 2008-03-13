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

// Table of primes that are approximately powers of 2 apart.
static const uint64_t primes[] =
{
    0x0000000000000001ULL, // 2^0
    0x0000000000000002ULL, // 2^1
    0x0000000000000005ULL, // 2^2
    0x000000000000000bULL, // 2^3
    0x0000000000000011ULL, // 2^4
    0x0000000000000025ULL, // 2^5
    0x0000000000000043ULL, // 2^6
    0x0000000000000083ULL, // 2^7
    0x0000000000000101ULL, // 2^8
    0x0000000000000209ULL, // 2^9
    0x0000000000000407ULL, // 2^10
    0x0000000000000805ULL, // 2^11
    0x0000000000001003ULL, // 2^12
    0x0000000000002011ULL, // 2^13
    0x000000000000401bULL, // 2^14
    0x0000000000008003ULL, // 2^15
    0x0000000000010001ULL, // 2^16
    0x000000000002001dULL, // 2^17
    0x0000000000040003ULL, // 2^18
    0x0000000000080015ULL, // 2^19
    0x0000000000100007ULL, // 2^20
    0x0000000000200011ULL, // 2^21
    0x000000000040000fULL, // 2^22
    0x0000000000800009ULL, // 2^23
    0x000000000100002bULL, // 2^24
    0x0000000002000023ULL, // 2^25
    0x000000000400000fULL, // 2^26
    0x000000000800001dULL, // 2^27
    0x0000000010000003ULL, // 2^28
    0x000000002000000bULL, // 2^29
    0x0000000040000003ULL, // 2^30
    0x000000008000000bULL, // 2^31
    0x000000010000000fULL, // 2^32
    0x0000000200000011ULL, // 2^33
    0x0000000400000019ULL, // 2^34
    0x0000000800000035ULL, // 2^35
    0x000000100000001fULL, // 2^36
    0x0000002000000009ULL, // 2^37
    0x0000004000000007ULL, // 2^38
    0x0000008000000017ULL, // 2^39
    0x000001000000000fULL, // 2^40
    0x000002000000001bULL, // 2^41
    0x000004000000000fULL, // 2^42
    0x000008000000001dULL, // 2^43
    0x0000100000000007ULL, // 2^44
    0x000020000000003bULL, // 2^45
    0x000040000000000fULL, // 2^46
    0x0000800000000005ULL, // 2^47
    0x0001000000000015ULL, // 2^48
    0x0002000000000045ULL, // 2^49
    0x0004000000000037ULL, // 2^50
    0x0008000000000015ULL, // 2^51
    0x0010000000000015ULL, // 2^52
    0x0020000000000005ULL, // 2^53
    0x004000000000009fULL, // 2^54
    0x0080000000000003ULL, // 2^55
    0x0100000000000051ULL, // 2^56
    0x0200000000000009ULL, // 2^57
    0x0400000000000045ULL, // 2^58
    0x0800000000000083ULL, // 2^59
    0x1000000000000021ULL, // 2^60
    0x200000000000000fULL, // 2^61
    0x4000000000000087ULL, // 2^62
    0x800000000000001dULL  // 2^63
};

/* Given the ch API, there is no way to both safely and efficiently transfer the
 * contents of one ch to another.  Therefore, this function mucks with ch
 * internals. */
CW_P_INLINE void
dch_p_insert(cw_ch_t *a_ch, cw_chi_t *a_chi)
{
    size_t slot;

    /* Initialize a_chi. */
    slot = a_ch->hash(a_chi->key) % a_ch->table_size;
    a_chi->slot = slot;

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

/* Given the ch API, there is no way to both safely and efficiently transfer the
 * contents of one ch to another.  Therefore, this function mucks with ch
 * internals. */
CW_P_INLINE void
dch_p_grow(cw_dch_t *a_dch)
{
    if ((ch_count(a_dch->ch) + 1)
	> (1 << a_dch->cur_power) - (1 << (a_dch->cur_power - 1)))
    {
	cw_ch_t *t_ch;
	cw_chi_t *chi;
	size_t i;

	/* Too small.  Create a new ch approximately twice as large and
	 * populate it. */
	t_ch = ch_new(NULL, a_dch->mema, primes[a_dch->cur_power + 1],
		      a_dch->hash, a_dch->key_comp);

	for (i = 0; i < a_dch->ch->table_size; i++)
	{
	    /* Use ql_last() to preserve chain order (LIFO). */
	    while ((chi = ql_last(&a_dch->ch->table[i], slot_link)) != NULL)
	    {
		ql_tail_remove(&a_dch->ch->table[i], cw_chi_t, slot_link);
		dch_p_insert(t_ch, chi);
	    }
	    /* Set to NULL to keep ch_delete from deleting this chain. */
	    ql_first(&a_dch->ch->table[i]) = NULL;
	}

	a_dch->cur_power++;
#ifdef CW_DCH_COUNT
	a_dch->num_grows++;
	t_ch->num_collisions += a_dch->ch->num_collisions;
	t_ch->num_inserts += a_dch->ch->num_inserts;
	t_ch->num_removes += a_dch->ch->num_removes;
	t_ch->num_searches += a_dch->ch->num_searches;
#endif
	ch_delete(a_dch->ch);
	a_dch->ch = t_ch;
    }
}

/* Given the ch API, there is no way to both safely and efficiently transfer the
 * contents of one ch to another.  Therefore, this function mucks with ch
 * internals. */
CW_P_INLINE void
dch_p_shrink(cw_dch_t *a_dch, const void *a_search_key)
{
    cw_ch_t *t_ch;
    cw_chi_t *chi;
    size_t count, i;

    count = ch_count(a_dch->ch);

    if ((count - 1 < (1 << (a_dch->cur_power - 1)))
	&& (a_dch->cur_power > a_dch->base_power)
	&& ch_search(a_dch->ch, a_search_key, NULL) == false)
    {
	/* Too big.  Create a new ch that is approximately half the size.  This
	 * new table will start out ~50% full. */

	t_ch = ch_new(NULL, a_dch->mema, a_dch->cur_power - 1,
		      a_dch->hash, a_dch->key_comp);

	for (i = 0; i < a_dch->ch->table_size; i++)
	{
	    /* Use ql_last() to preserve chain order (LIFO). */
	    while ((chi = ql_last(&a_dch->ch->table[i], slot_link)) != NULL)
	    {
		ql_tail_remove(&a_dch->ch->table[i], cw_chi_t, slot_link);
		dch_p_insert(t_ch, chi);
	    }
	    /* Set to NULL to keep ch_delete from deleting this chain. */
	    ql_first(&a_dch->ch->table[i]) = NULL;
	}

	a_dch->cur_power--;
#ifdef CW_DCH_COUNT
	a_dch->num_shrinks++;
	t_ch->num_collisions += a_dch->ch->num_collisions;
	t_ch->num_inserts += a_dch->ch->num_inserts;
	t_ch->num_removes += a_dch->ch->num_removes;
	t_ch->num_searches += a_dch->ch->num_searches;
#endif
	ch_delete(a_dch->ch);
	a_dch->ch = t_ch;
    }
}

cw_dch_t *
dch_new(cw_dch_t *a_dch, cw_mema_t *a_mema, size_t a_base_count,
	cw_ch_hash_t *a_hash, cw_ch_key_comp_t *a_key_comp)
{
    cw_dch_t *retval;
    size_t base_table_min;
    unsigned i;

    cw_check_ptr(a_mema);
    cw_check_ptr(mema_alloc_get(a_mema));
    cw_check_ptr(mema_calloc_get(a_mema));
    cw_check_ptr(mema_dealloc_get(a_mema));
    cw_check_ptr(a_hash);
    cw_check_ptr(a_key_comp);

    if (a_dch != NULL)
    {
	retval = a_dch;
	memset(retval, 0, sizeof(cw_dch_t));
	retval->is_malloced = false;
    }
    else
    {
	retval = (cw_dch_t *) cw_opaque_calloc(mema_calloc_get(a_mema),
					       mema_arg_get(a_mema), 1,
					       sizeof(cw_dch_t));
	retval->is_malloced = true;
    }

    retval->mema = a_mema;
    /* Find the minimum value in primes that is large enough to fit a_base_count
     * entries with a maximum fullness of approximately 75%. */
    base_table_min = ((a_base_count + (3 - (a_base_count % 3))) / 3) << 2;
    for (i = 0; i < (sizeof(primes) / sizeof(uint64_t)); i++)
    {
	if (base_table_min <= primes[i])
	{
	    break;
	}
    }
    retval->base_power = i;
    retval->cur_power = i;
    retval->shrinkable = true;
    retval->hash = a_hash;
    retval->key_comp = a_key_comp;

    xep_begin();
    volatile cw_dch_t *v_retval;
    xep_try
    {
	v_retval = retval;
	retval->ch = ch_new(NULL, a_mema, primes[retval->base_power],
			    retval->hash, retval->key_comp);
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
	    __func__, a_dch, a_dch->ch->num_collisions,
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

size_t
dch_count(cw_dch_t *a_dch)
{
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    return ch_count(a_dch->ch);
}

bool
dch_shrinkable_get(cw_dch_t *a_dch)
{
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    return a_dch->shrinkable;
}

void
dch_shrinkable_set(cw_dch_t *a_dch, bool a_shrinkable)
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

bool
dch_remove(cw_dch_t *a_dch, const void *a_search_key, void **r_key,
	   void **r_data, cw_chi_t **r_chi)
{
    bool retval;

    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    if (a_dch->shrinkable)
    {
	dch_p_shrink(a_dch, a_search_key);
    }
    if (ch_remove(a_dch->ch, a_search_key, r_key, r_data, r_chi))
    {
	retval = true;
	goto RETURN;
    }
    retval = false;
    RETURN:
    return retval;
}

void
dch_chi_remove(cw_dch_t *a_dch, cw_chi_t *a_chi)
{
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    if (a_dch->shrinkable)
    {
	dch_p_shrink(a_dch, a_chi->key);
    }
    ch_chi_remove(a_dch->ch, a_chi);
}

bool
dch_search(cw_dch_t *a_dch, const void *a_key, void **r_data)
{
    cw_check_ptr(a_dch);
    cw_dassert(a_dch->magic == CW_DCH_MAGIC);

    return ch_search(a_dch->ch, a_key, r_data);
}
