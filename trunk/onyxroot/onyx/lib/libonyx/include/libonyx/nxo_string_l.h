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

typedef struct cw_nxoe_string_s cw_nxoe_string_t;

struct cw_nxoe_string_s
{
    cw_nxoe_t nxoe;

#ifdef CW_THREADS
    /* Access is locked if this object has the locking bit set.  Indirect
     * strings aren't locked, but their parents are. */
    cw_mtx_t lock;
#endif

    /* Used for remembering the current state of reference iteration. */
    cw_uint32_t ref_iter;
    union
    {
	struct
	{
	    cw_nxo_t nxo;
	    cw_uint32_t beg_offset;
	    cw_uint32_t len;
	} i;
	struct
	{
	    cw_uint8_t *str;
	    cw_uint32_t len;
	    cw_uint32_t alloc_len;
	} s;
    } e;
};

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_string_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_string_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_STRING_C_))
CW_INLINE cw_bool_t
nxoe_l_string_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_string_t *string;

    string = (cw_nxoe_string_t *) a_nxoe;

    cw_check_ptr(string);
    cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(string->nxoe.type == NXOT_STRING);

    if (string->nxoe.indirect == FALSE && string->e.s.alloc_len > 0)
    {
	nxa_free(a_nxa, string->e.s.str, string->e.s.alloc_len);
    }

#ifdef CW_THREADS
    if (string->nxoe.locking && string->nxoe.indirect == FALSE)
    {
	mtx_delete(&string->lock);
    }
#endif

    nxa_free(a_nxa, string, sizeof(cw_nxoe_string_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_string_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_string_t *string;

    string = (cw_nxoe_string_t *) a_nxoe;

    if (a_reset)
    {
	string->ref_iter = 0;
    }

    if (a_nxoe->indirect == FALSE)
    {
	retval = NULL;
    }
    else if (string->ref_iter == 0)
    {
	retval = string->e.i.nxo.o.nxoe;
	string->ref_iter++;
    }
    else
    {
	retval = NULL;
    }

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_STRING_C_)) */
