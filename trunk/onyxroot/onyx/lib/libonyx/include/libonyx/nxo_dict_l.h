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

    /* Used for remembering the current state of reference iteration. */
    cw_uint32_t ref_iter;

    /* If non-NULL, the previous reference iteration returned the key of this
     * dicto, so the value of this dicto is the next reference to check. */
    cw_nxoe_dicto_t *dicto;

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

cw_bool_t
nxoe_l_dict_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_dict_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);

cw_nxo_t *
nxo_l_dict_lookup(const cw_nxo_t *a_nxo, const cw_nxo_t *a_key);
