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

typedef struct cw_nxoe_regsub_s cw_nxoe_regsub_t;

/* Substitution template element.  A substitution template is decomposed into a
 * vector of these. */
typedef struct
{
    /* String pointer and length for a plain string.  If str is NULL, then len
     * instead specifies a capturing subpattern. */
    cw_uint8_t *str;
    cw_uint32_t len;
} cw_nxoe_regsub_telm_t;

struct cw_nxoe_regsub_s
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
#ifdef PCRE_INFO_STUDYSIZE
    size_t studysize;
#endif

    /* Flag used when determining whether to substitute just one, or all
     * matches. */
    cw_bool_t global;

    /* Number of capturing subpatterns. */
    int capturecount;

    /* Template string and length.  vec elements point into this string. */
    cw_uint8_t *template;
    cw_uint32_t tlen;

    /* Vector of template elements. */
    cw_nxoe_regsub_telm_t *vec;
    cw_uint32_t vlen;
};

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_regsub_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_regsub_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_REGSUB_C_))
CW_INLINE cw_bool_t
nxoe_l_regsub_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_regsub_t *regsub;

    regsub = (cw_nxoe_regsub_t *) a_nxoe;

    cw_check_ptr(regsub);
    cw_dassert(regsub->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(regsub->nxoe.type == NXOT_REGSUB);

    if (regsub->vec != NULL)
    {
	nxa_free(a_nxa, regsub->vec,
		 sizeof(cw_nxoe_regsub_telm_t) * regsub->vlen);
    }

    if (regsub->template != NULL)
    {
	nxa_free(a_nxa, regsub->template, regsub->tlen);
    }

    /* Destroy pcre. */
    free(regsub->pcre);
    if (regsub->extra != NULL)
    {
	free(regsub->extra);
    }
    /* Tell the GC that pcre has been deallocated. */
    nxa_l_count_adjust(a_nxa, -(cw_nxoi_t)(regsub->size
#ifdef PCRE_INFO_STUDYSIZE
					   + regsub->studysize
#endif
					   ));

    nxa_free(a_nxa, regsub, sizeof(cw_nxoe_regsub_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_regsub_ref_iter(cw_nxoe_t *a_nxo, cw_bool_t a_reset)
{
    return NULL;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_REGSUB_C_)) */
