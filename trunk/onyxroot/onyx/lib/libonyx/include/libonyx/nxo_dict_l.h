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

/* This is private, but nxa needs to know its size.
 *
 * This structure is used as an element of a fixed size array for small
 * dicts. */
struct cw_nxoe_dicta_s
{
    cw_nxo_t key;
    cw_nxo_t val;
};

/* This is private, but nxa needs to know its size.
 *
 * This structure is used as an element for dch-based dicts. */
struct cw_nxoe_dicth_s
{
    cw_chi_t chi;
    ql_elm(cw_nxoe_dicth_t) link;

    cw_nxo_t key;
    cw_nxo_t val;
};

struct cw_nxoe_dict_s
{
    cw_nxoe_t nxoe;

#ifdef CW_THREADS
    /* Access is locked if this object has the locking bit set. */
    cw_mtx_t lock;
#endif

    /* If TRUE, the data are in the hash.  Otherwise, they are stored in the
     * array. */
    cw_bool_t is_hash:1;

    /* Iteration state variable for iterating over the data array.  The value is
     * always less than CW_LIBONYX_DICT_SIZE. */
    cw_uint32_t array_iter:31;

    union
    {
	struct
	{
	    /* Array of dicta's.  Searching is linear.  Invalid entries have a
	     * key of type NXOT_NO.  If the array is full and another insertion
	     * occurs, the array contents are converted to a hash, and are never
	     * converted back to an array, even if the array would be large
	     * enough. */
	    cw_nxoe_dicta_t array[CW_LIBONYX_DICT_SIZE];
	} a;
	struct
	{
	    /* Name/value pairs.  The keys are (cw_nxo_t *), and the values are
	     * (cw_nxoe_dicth_t *).  The nxo that the key points to resides in
	     * the nxoe_dicth (value) structure. */
	    cw_dch_t hash;

	    /* List of all dict items. */
	    ql_head(cw_nxoe_dicth_t) list;
	} h;
    } data;
};

cw_nxo_t *
nxo_l_dict_lookup(const cw_nxo_t *a_nxo, const cw_nxo_t *a_key);

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_dict_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_dict_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_DICT_C_))
CW_INLINE cw_bool_t
nxoe_l_dict_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter)
{
    cw_nxoe_dict_t *dict;

    dict = (cw_nxoe_dict_t *) a_nxoe;

    cw_check_ptr(dict);
    cw_dassert(dict->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(dict->nxoe.type == NXOT_DICT);

#ifdef CW_THREADS
    if (dict->nxoe.locking)
    {
	mtx_delete(&dict->lock);
    }
#endif
    if (dict->is_hash)
    {
	cw_nxoe_dicth_t *dicth;

	/* Set the dch non-shrinkable to avoid rehashes, which could be fatal
	 * if any of the objects this dict points to have already been swept. */
	dch_shrinkable_set(&dict->data.h.hash, FALSE);

	for (dicth = ql_first(&dict->data.h.list);
	     dicth != NULL;
	     dicth = ql_first(&dict->data.h.list))
	{
	    dch_chi_remove(&dict->data.h.hash, &dicth->chi);
	    ql_remove(&dict->data.h.list, dicth, link);
	    nxa_free(dicth, sizeof(cw_nxoe_dicth_t));
	}
	dch_delete(&dict->data.h.hash);
    }
    nxa_free(dict, sizeof(cw_nxoe_dict_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_dict_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_dict_t *dict;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so using a static
     * variable works fine. */
    static cw_uint32_t ref_iter;
    /* If non-NULL, the previous reference iteration returned the key of this
     * dict[ah], so the value of this dict[ah] is the next reference to
     * check. */
    static cw_nxoe_dicta_t *dicta;
    static cw_nxoe_dicth_t *dicth;

    dict = (cw_nxoe_dict_t *) a_nxoe;

    if (a_reset)
    {
	ref_iter = 0;
	dicta = NULL;
	dicth = NULL;
    }

    retval = NULL;
    if (dict->is_hash)
    {
	while (retval == NULL
	       && ref_iter < dch_count(&dict->data.h.hash))
	{
	    if (dicth == NULL)
	    {
		/* Key. */
		dicth = ql_first(&dict->data.h.list);
		cw_check_ptr(dicth);
		ql_first(&dict->data.h.list) = qr_next(dicth, link);
		retval = nxo_nxoe_get(&dicth->key);
	    }
	    else
	    {
		/* Value. */
		retval = nxo_nxoe_get(&dicth->val);
		ref_iter++;
		dicth = NULL;
	    }
	}
    }
    else
    {
	while (retval == NULL && ref_iter < CW_LIBONYX_DICT_SIZE)
	{
	    if (dicta == NULL)
	    {
		if (nxo_type_get(&dict->data.a.array[ref_iter].key)
		    != NXOT_NO)
		{
		    /* Key. */
		    dicta = &dict->data.a.array[ref_iter];
		    retval = nxo_nxoe_get(&dicta->key);
		}
		else
		{
		    /* Empty slot. */
		    ref_iter++;
		}
	    }
	    else
	    {
		/* Value. */
		retval = nxo_nxoe_get(&dicta->val);
		ref_iter++;
		dicta = NULL;
	    }
	}
    }

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_DICT_C_)) */
