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

#include <pcre.h>

typedef struct cw_nxoe_regex_s cw_nxoe_regex_t;

struct cw_nxoe_regex_s
{
    cw_nxoe_t nxoe;

    /* pcre data structures. */
    pcre *pcre;
    pcre_extra *extra;

    /* Number of elements (each element is an int) needed for output vectors
     * that are passed to pcre_exec(). */
    int ovcnt;

    /* Amount of memory allocated for the structures pointed to by pcre and
     * extra.  Unfortunately, it is prohibitively difficult to integrate pcre
     * with nxa's allocator API in a multi-threaded interpreter, so pcre and
     * extra are allocated with plain old malloc().  Thus, their sizes have to
     * be queried and the GC informed of their size. */
    size_t size;
    size_t extrasize;

    /* Maximum number of matches.  0 means unlimited. */
    cw_uint32_t limit;
};

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_regex_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_regex_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_REGEX_C_))
CW_INLINE cw_bool_t
nxoe_l_regex_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_regex_t *regex;

    regex = (cw_nxoe_regex_t *) a_nxoe;

    cw_check_ptr(regex);
    cw_dassert(regex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(regex->nxoe.type == NXOT_REGEX);

    /* Destroy pcre. */
    free(regex->pcre);
    if (regex->extra != NULL)
    {
	free(regex->extra);
    }
    /* Tell the GC that pcre has been deallocated. */
    nxa_l_count_adjust(a_nxa, -(regex->size + regex->extrasize));

    nxa_free(a_nxa, regex, sizeof(cw_nxoe_regex_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_regex_ref_iter(cw_nxoe_t *a_nxo, cw_bool_t a_reset)
{
    return NULL;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_REGEX_C_)) */
