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

/* This is private, but nxa needs to know its size. */
struct cw_nxoe_dicto_s
{
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
	/* Array of dicto's.  Searching is linear.  Invalid entries have a key
	 * of type NXOT_NO.  If the array is full and another insertion occurs,
	 * the array contents are converted to a hash, and are never converted
	 * back to an array, even if the array would be large enough. */
	cw_nxoe_dicto_t array[CW_LIBONYX_DICT_SIZE];

	/* Name/value pairs.  The keys are (cw_nxo_t *), and the values are
	 * (cw_nxoe_dicto_t *).  The nxo that the key points to resides in the
	 * nxoe_dicto (value) structure. */
	cw_dch_t hash;
    } data;
};

cw_nxo_t *
nxo_l_dict_lookup(const cw_nxo_t *a_nxo, const cw_nxo_t *a_key);

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_dict_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_dict_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_DICT_C_))
CW_INLINE cw_bool_t
nxoe_l_dict_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_dict_t *dict;
    cw_nxoe_dicto_t *dicto;
    cw_chi_t *chi;

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
	/* Set the dch non-shrinkable to avoid rehashes, which could be fatal
	 * if any of the objects this dict points to have already been swept. */
	dch_shrinkable_set(&dict->data.hash, FALSE);

	while (dch_remove_iterate(&dict->data.hash, NULL, (void **) &dicto,
				  &chi)
	       == FALSE)
	{
	    nxa_free(a_nxa, dicto, sizeof(cw_nxoe_dicto_t));
	    nxa_free(a_nxa, chi, sizeof(cw_chi_t));
	}
	dch_delete(&dict->data.hash);
    }
    nxa_free(a_nxa, dict, sizeof(cw_nxoe_dict_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_dict_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_dict_t *dict;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so as long as two
     * interpreters aren't collecting simultaneously, using a static variable
     * works fine. */
    static cw_uint32_t ref_iter;
    /* If non-NULL, the previous reference iteration returned the key of this
     * dicto, so the value of this dicto is the next reference to check. */
    static cw_nxoe_dicto_t *dicto;

    dict = (cw_nxoe_dict_t *) a_nxoe;

    if (a_reset)
    {
	ref_iter = 0;
	dicto = NULL;
    }

    retval = NULL;
    if (dict->is_hash)
    {
	while (retval == NULL
	       && ref_iter < dch_count(&dict->data.hash))
	{
	    if (dicto == NULL)
	    {
		/* Key. */
		dch_get_iterate(&dict->data.hash, NULL, (void **) &dicto);
		retval = nxo_nxoe_get(&dicto->key);
	    }
	    else
	    {
		/* Value. */
		retval = nxo_nxoe_get(&dicto->val);
		ref_iter++;
		dicto = NULL;
	    }
	}
    }
    else
    {
	while (retval == NULL && ref_iter < CW_LIBONYX_DICT_SIZE)
	{
	    if (dicto == NULL)
	    {
		if (nxo_type_get(&dict->data.array[ref_iter].key)
		    != NXOT_NO)
		{
		    /* Key. */
		    dicto = &dict->data.array[ref_iter];
		    retval = nxo_nxoe_get(&dicto->key);
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
		retval = nxo_nxoe_get(&dicto->val);
		ref_iter++;
		dicto = NULL;
	    }
	}
    }

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_DICT_C_)) */