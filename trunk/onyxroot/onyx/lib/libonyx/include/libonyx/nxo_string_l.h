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

    union
    {
	struct
	{
	    cw_nxoe_string_t *string;
	    uint32_t beg_offset;
	    uint32_t len;
	} i;
	struct
	{
	    uint8_t *str;
	    uint32_t len;
	    uint32_t alloc_len;
	} s;
    } e;
};

#ifndef CW_USE_INLINES
bool
nxoe_l_string_delete(cw_nxoe_t *a_nxoe, uint32_t a_iter);

cw_nxoe_t *
nxoe_l_string_ref_iter(cw_nxoe_t *a_nxoe, bool a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_STRING_C_))
CW_INLINE bool
nxoe_l_string_delete(cw_nxoe_t *a_nxoe, uint32_t a_iter)
{
    cw_nxoe_string_t *string;

    string = (cw_nxoe_string_t *) a_nxoe;

    cw_check_ptr(string);
    cw_dassert(string->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(string->nxoe.type == NXOT_STRING);

    if (string->nxoe.indirect == false && string->e.s.alloc_len > 0)
    {
	nxa_free(string->e.s.str, string->e.s.alloc_len);
    }

#ifdef CW_THREADS
    if (string->nxoe.locking && string->nxoe.indirect == false)
    {
	mtx_delete(&string->lock);
    }
#endif

    nxa_free(string, sizeof(cw_nxoe_string_t));

    return false;
}

CW_INLINE cw_nxoe_t *
nxoe_l_string_ref_iter(cw_nxoe_t *a_nxoe, bool a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_string_t *string;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so using a static
     * variable works fine. */
    static uint32_t ref_iter;

    string = (cw_nxoe_string_t *) a_nxoe;

    if (a_reset)
    {
	ref_iter = 0;
    }

    if (a_nxoe->indirect == false)
    {
	retval = NULL;
    }
    else if (ref_iter == 0)
    {
	retval = (cw_nxoe_t *) string->e.i.string;
	ref_iter++;
    }
    else
    {
	retval = NULL;
    }

    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_STRING_C_)) */
