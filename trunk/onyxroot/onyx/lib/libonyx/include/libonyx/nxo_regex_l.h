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
    size_t studysize;

    /* Flags used when matching that determine where in the input string to
     * start searching. */
    cw_bool_t cont:1;
    cw_bool_t global:1;
};

#ifndef CW_USE_INLINES
void
nxo_l_regex_cache_new(cw_nxo_regex_cache_t *a_cache);

cw_nxoe_t *
nxo_l_regex_cache_ref_iter(cw_nxo_regex_cache_t *a_cache, cw_bool_t a_reset);

void
nxo_l_regex_cache_delete(cw_nxo_regex_cache_t *a_cache);

cw_bool_t
nxoe_l_regex_delete(cw_nxoe_t *a_nxoe cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_regex_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_REGEX_C_))
/* This function assumes that nxo_thread_new() already zeroed memory. */
CW_INLINE void
nxo_l_regex_cache_new(cw_nxo_regex_cache_t *a_cache)
{
    nxo_no_new(&a_cache->input);
    a_cache->mcnt = -1;
}

CW_INLINE cw_nxoe_t *
nxo_l_regex_cache_ref_iter(cw_nxo_regex_cache_t *a_cache, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so as long as two
     * interpreters aren't collecting simultaneously, using a static variable
     * works fine. */
    static cw_uint32_t ref_iter;

    if (a_reset)
    {
	ref_iter = 0;
    }

    for (retval = NULL; retval == NULL; ref_iter++)
    {
	switch (ref_iter)
	{
	    case 0:
	    {
		retval = nxo_nxoe_get(&a_cache->input);
		break;
	    }
	    default:
	    {
		retval = NULL;
		goto RETURN;
	    }
	}
    }

    RETURN:
    return retval;
}

CW_INLINE void
nxo_l_regex_cache_delete(cw_nxo_regex_cache_t *a_cache)
{
    if (a_cache->ovp != NULL)
    {
	nxa_free(a_cache->ovp, sizeof(int) * a_cache->ovcnt);
    }
}

CW_INLINE cw_bool_t
nxoe_l_regex_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter)
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
    nxa_l_count_adjust(-(cw_nxoi_t)(regex->size + regex->studysize));

    nxa_free(regex, sizeof(cw_nxoe_regex_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_regex_ref_iter(cw_nxoe_t *a_nxo, cw_bool_t a_reset)
{
    return NULL;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_REGEX_C_)) */
