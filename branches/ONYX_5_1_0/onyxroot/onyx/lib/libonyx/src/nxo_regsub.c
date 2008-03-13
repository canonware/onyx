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

#define CW_NXO_REGSUB_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_regex_l.h"
#include "../include/libonyx/nxo_regsub_l.h"
#include "../include/libonyx/nxo_thread_l.h"

/* Do the work of initializing a regsub, but don't do any of the typical
 * GC-related initialization, so that this function can be used for the case
 * where a regsub is temporarily constructed for a single subst. */
static cw_nxn_t
nxo_p_regsub_init(cw_nxoe_regsub_t *a_regsub, const char *a_pattern,
		  uint32_t a_plen, bool a_global,
		  bool a_insensitive, bool a_multiline,
		  bool a_singleline, const char *a_template,
		  uint32_t a_tlen)
{
    cw_nxn_t retval;
    char *pattern;
    const char *errptr;
    int options, erroffset, capturecount;
    enum
    {
	TSTATE_START,
	TSTATE_BS_CONT
    } tstate;
    uint32_t i, beg, end, voff;

    nxoe_l_new(&a_regsub->nxoe, NXOT_REGSUB, false);

    /* Create a '\0'-terminated copy of a_pattern. */
    pattern = (char *) nxa_malloc(a_plen + 1);
    memcpy(pattern, a_pattern, a_plen);
    pattern[a_plen] = '\0';

    /* Translate options to a format usable by pcre_compile(). */
    options = 0;
    /* $i. */
    if (a_insensitive)
    {
	options |= PCRE_CASELESS;
    }
    /* $m. */
    if (a_multiline)
    {
	options |= PCRE_MULTILINE;
    }
    /* $s. */
    if (a_singleline)
    {
	options |= PCRE_DOTALL;
    }

    /* Store the global flag.  This information is not needed in this function,
     * but gets used when actually doing substitutions. */
    a_regsub->global = a_global;

    /* Compile the regex. */
    a_regsub->pcre = pcre_compile(pattern, options, &errptr, &erroffset, NULL);
    nxa_free(pattern, a_plen + 1);
    if (a_regsub->pcre == NULL)
    {
	retval = NXN_regexerror;
	goto RETURN;
    }

    /* Call pcre_study(), which may improve matching performance. */
    a_regsub->extra = pcre_study(a_regsub->pcre, 0, &errptr);
    if (errptr != NULL)
    {
	free(a_regsub->pcre);
	retval = NXN_regexerror;
	goto RETURN;
    }

    /* Get capturecount and the amount of space that was allocated for
     * a_regsub->pcre and a_regsub->extra. */
    if ((pcre_fullinfo(a_regsub->pcre, a_regsub->extra, PCRE_INFO_CAPTURECOUNT,
		       &capturecount) != 0)
	|| (pcre_fullinfo(a_regsub->pcre, a_regsub->extra, PCRE_INFO_SIZE,
			  &a_regsub->size) != 0)
	|| (pcre_fullinfo(a_regsub->pcre, a_regsub->extra, PCRE_INFO_STUDYSIZE,
			  &a_regsub->studysize) != 0)
	)
    {
	free(a_regsub->pcre);
	if (a_regsub->extra != NULL)
	{
	    free(a_regsub->extra);
	}
	retval = NXN_regexerror;
	goto RETURN;
    }

    /* Use capturecount to calculate the size of vector needed for pcre_exec()
     * calls. */
    a_regsub->ovcnt = (capturecount + 1) * 3;

    /* Make a copy of a_template. */
    if (a_tlen > 0)
    {
	a_regsub->template = (char *) nxa_malloc(a_tlen);
	memcpy(a_regsub->template, a_template, a_tlen);
    }
    else
    {
	a_regsub->template = NULL;
    }
    a_regsub->tlen = a_tlen;

    /* Parse a_template and construct a vector from it.  Do this in two passes,
     * since having to reallocate is likely to be more expensive than parsing
     * twice. */
    for (i = beg = end = a_regsub->vlen = 0, tstate = TSTATE_START;
	 i < a_tlen;
	 i++)
    {
	switch (tstate)
	{
	    case TSTATE_START:
	    {
		switch (a_regsub->template[i])
		{
		    case '\\':
		    {
			end = i;
			tstate = TSTATE_BS_CONT;
			break;
		    }
		    default:
		    {
			break;
		    }
		}
		break;
	    }
	    case TSTATE_BS_CONT:
	    {
		switch (a_regsub->template[i])
		{
		    case '1': case '2': case '3': case '4': case '5': case '6':
		    case '7': case '8': case '9':
		    {
			/* Preceding plain text, if any. */
			if (end > beg)
			{
			    a_regsub->vlen++;
			}

			/* Subpattern substitution. */
			a_regsub->vlen++;
			beg = end = i + 1;
			tstate = TSTATE_START;
			break;
		    }
		    case '\\':
		    {
			/* Stay in this state (ignore extra leading '\'
			 * characters. */
			end = i;
			break;
		    }
		    default:
		    {
			/* Ignore. */
			tstate = TSTATE_START;
			break;
		    }
		}
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }
    if (beg < i)
    {
	/* Normal characters after last subpattern substitution. */
	a_regsub->vlen++;
    }

    /* Initialize the vector, now that we know how big to make it. */
    if (a_regsub->vlen > 0)
    {
	a_regsub->vec
	    = (cw_nxoe_regsub_telm_t *) nxa_malloc(sizeof(cw_nxoe_regsub_telm_t)
						   * a_regsub->vlen);
    }
    else
    {
	a_regsub->vec = NULL;
    }

    for (i = beg = end = voff = 0, tstate = TSTATE_START;
	 i < a_tlen;
	 i++)
    {
	switch (tstate)
	{
	    case TSTATE_START:
	    {
		switch (a_regsub->template[i])
		{
		    case '\\':
		    {
			end = i;
			tstate = TSTATE_BS_CONT;
			break;
		    }
		    default:
		    {
			break;
		    }
		}
		break;
	    }
	    case TSTATE_BS_CONT:
	    {
		switch (a_regsub->template[i])
		{
		    case '1': case '2': case '3': case '4': case '5': case '6':
		    case '7': case '8': case '9':
		    {
			/* Preceding plain text, if any. */
			if (end > beg)
			{
			    a_regsub->vec[voff].str = &a_regsub->template[beg];
			    a_regsub->vec[voff].len = end - beg;
			    voff++;
			}

			/* Subpattern substitution. */
			a_regsub->vec[voff].str = NULL;
			a_regsub->vec[voff].len
			    = (uint32_t) (a_regsub->template[i] - '0');
			voff++;
			beg = end = i + 1;
			tstate = TSTATE_START;
			break;
		    }
		    case '\\':
		    {
			/* Stay in this state (ignore extra leading '\'
			 * characters. */
			end = i;
			break;
		    }
		    default:
		    {
			/* Ignore. */
			tstate = TSTATE_START;
			break;
		    }
		}
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }
    if (beg < i)
    {
	/* Normal characters after last subpattern substitution. */
	a_regsub->vec[voff].str = &a_regsub->template[beg];
	a_regsub->vec[voff].len = i - beg;
#ifdef CW_DBG
	/* Make following assertion possible. */
	voff++;
#endif
    }
    cw_assert(voff == a_regsub->vlen);

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

CW_P_INLINE void
nxo_p_regsub_append(char **r_ostr, uint32_t *r_omax,
		    uint32_t *r_olen, const char *a_istr,
		    uint32_t a_ilen)
{
    uint32_t omax;

    /* Expand *r_ostr, if necessary. */
    for (omax = *r_omax; *r_olen + a_ilen > omax; omax *= 2)
    {
	/* Do nothing. */
    }
    if (omax != *r_omax)
    {
	*r_ostr = nxa_realloc(*r_ostr, omax, *r_omax);
	*r_omax = omax;
    }

    /* Copy and adjust *r_olen. */
    memcpy(&(*r_ostr)[*r_olen], a_istr, a_ilen);
    *r_olen += a_ilen;
}

static uint32_t
nxo_p_regsub_subst(cw_nxoe_regsub_t *a_regsub, cw_nxo_t *a_thread,
		   cw_nxo_t *a_input, cw_nxo_t *r_output)
{
    uint32_t retval = 0;
    cw_nxo_regex_cache_t *cache;
    uint32_t scnt, ilen, ioff, olen, omax, v;
    char *istr, *ostr;

    cache = nxo_l_thread_regex_cache_get(a_thread);

    /* Allocate or extend the vector for passing to pcre_exec(), if
     * necessary. */
    if (cache->ovp == NULL)
    {
	cache->ovp = nxa_malloc(sizeof(int) * a_regsub->ovcnt);
	cache->ovcnt = a_regsub->ovcnt;
    }
    else if (cache->ovcnt < a_regsub->ovcnt)
    {
	cache->ovp = nxa_realloc(cache->ovp, sizeof(int) * a_regsub->ovcnt,
				 sizeof(int) * cache->ovcnt);
	cache->ovcnt = a_regsub->ovcnt;
    }

    /* Allocate a temporary output string that is as large as the input string.
     * If ostr overflows, iteratively double its size.  omax tracks the current
     * allocation size of ostr, and olen tracks how full ostr is. */
    ilen = omax = nxo_string_len_get(a_input);
    olen = 0;
    if (omax == 0)
    {
	/* It is possible for a pattern to match the empty string, then
	 * substitute a non-empty string.  Therefore, handle the empty input
	 * string case. */
	omax = 8;
    }
    istr = nxo_string_get(a_input);
    ostr = nxa_malloc(omax);

    /* Iteratively look for matches. */
    for (scnt = ioff = 0;
	 ioff < ilen && (a_regsub->global || scnt < 1);
	 scnt++, ioff = (uint32_t) cache->ovp[1])
    {
	/* Look for a match. */
	nxo_string_lock(a_input);
	cache->mcnt = pcre_exec(a_regsub->pcre, a_regsub->extra, (char *) istr,
				ilen, ioff, 0, cache->ovp, cache->ovcnt);
	nxo_string_unlock(a_input);
	if (cache->mcnt <= 0)
	{
	    switch (cache->mcnt)
	    {
		case 0:
		case PCRE_ERROR_NOMATCH:
		{
		    /* No match found.  Not an error. */
		    goto DONE;
		}
		case PCRE_ERROR_NOMEMORY:
		{
		    xep_throw(CW_ONYXX_OOM);
		}
		case PCRE_ERROR_NULL:
		case PCRE_ERROR_BADOPTION:
		case PCRE_ERROR_BADMAGIC:
		case PCRE_ERROR_UNKNOWN_NODE:
		default:
		{
		    cw_not_reached();
		}
	    }
	}

	/* Copy any data between the end of the previous substitution and the
	 * beginning of the current substitution. */
	if (ioff < (uint32_t) cache->ovp[0])
	{
	    nxo_p_regsub_append(&ostr, &omax, &olen,
				&istr[ioff],
				(uint32_t) cache->ovp[0] - ioff);
	}

	/* Substitute. */
	for (v = 0; v < a_regsub->vlen; v++)
	{
	    if (a_regsub->vec[v].str != NULL)
	    {
		/* Copy from template string. */
		nxo_p_regsub_append(&ostr, &omax, &olen, a_regsub->vec[v].str,
				    a_regsub->vec[v].len);
	    }
	    else
	    {
		/* Substitute subpattern match, if the subpattern was
		 * matched. */
		if (a_regsub->vec[v].len < cache->mcnt
		    && cache->ovp[a_regsub->vec[v].len * 2] != -1)
		{

		    nxo_p_regsub_append(&ostr, &omax, &olen,
					&istr
					[cache->ovp[a_regsub->vec[v].len * 2]],
					cache->ovp[a_regsub->vec[v].len * 2 + 1]
					- cache->ovp[a_regsub->vec[v].len * 2]);
		}
	    }
	}

	/* Increment substitution count. */
	retval++;
    }
    DONE:
    /* If there are trailing bytes after the last match, copy them. */
    if (ioff < ilen)
    {
	nxo_p_regsub_append(&ostr, &omax, &olen, &istr[ioff], ilen - ioff);
    }

    /* Create an Onyx string and copy ostr to it. */
    if (retval > 0)
    {
	nxo_string_new(r_output, nxo_thread_currentlocking(a_thread), olen);
	if (olen > 0)
	{
	    nxo_string_set(r_output, 0, ostr, olen);
	}
    }
    else
    {
	/* No substitution done.  Dup the input string. */
	nxo_dup(r_output, a_input);
    }

    /* Clean up. */
    nxa_free(ostr, omax);

    return retval;
}

cw_nxn_t
nxo_regsub_new(cw_nxo_t *a_nxo, const char *a_pattern, uint32_t a_plen,
	       bool a_global, bool a_insensitive,
	       bool a_multiline, bool a_singleline,
	       const char *a_template, uint32_t a_tlen)
{
    cw_nxn_t retval;
    cw_nxoe_regsub_t *regsub;

    regsub = (cw_nxoe_regsub_t *) nxa_malloc(sizeof(cw_nxoe_regsub_t));

    retval = nxo_p_regsub_init(regsub, a_pattern, a_plen, a_global,
			       a_insensitive, a_multiline, a_singleline,
			       a_template, a_tlen);
    if (retval)
    {
	nxa_free(regsub, sizeof(cw_nxoe_regsub_t));
	goto RETURN;
    }

    /* Tell the GC about the space being taken up by regsub->pcre and
     * regsub->extra. */
    nxa_l_count_adjust((cw_nxoi_t) regsub->size + regsub->studysize);

    /* Create a reference to the regsub object. */
    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) regsub;
    nxo_p_type_set(a_nxo, NXOT_REGSUB);

    /* Register the regsub object with the GC. */
    nxa_l_gc_register((cw_nxoe_t *) regsub);

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

void
nxo_regsub_subst(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *a_input,
		 cw_nxo_t *r_output, uint32_t *r_count)
{
    cw_nxoe_regsub_t *regsub;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_REGSUB);

    regsub = (cw_nxoe_regsub_t *) a_nxo->o.nxoe;

    cw_check_ptr(regsub);
    cw_dassert(regsub->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(regsub->nxoe.type == NXOT_REGSUB);

    *r_count = nxo_p_regsub_subst(regsub, a_thread, a_input, r_output);
}

/* Do a subst without creating a regsub object, in order to avoid putting
 * pressure on the GC. */
cw_nxn_t
nxo_regsub_nonew_subst(cw_nxo_t *a_thread, const char *a_pattern,
		       uint32_t a_plen, bool a_global,
		       bool a_insensitive, bool a_multiline,
		       bool a_singleline, const char *a_template,
		       uint32_t a_tlen, cw_nxo_t *a_input,
		       cw_nxo_t *r_output, uint32_t *r_count)
{
    cw_nxn_t retval;
    cw_nxoe_regsub_t regsub;

    retval = nxo_p_regsub_init(&regsub, a_pattern, a_plen, a_global,
			       a_insensitive, a_multiline, a_singleline,
			       a_template, a_tlen);
    if (retval)
    {
	goto RETURN;
    }

    *r_count = nxo_p_regsub_subst(&regsub, a_thread, a_input, r_output);

    /* Clean up memory. */

    if (regsub.vec != NULL)
    {
	nxa_free(regsub.vec, sizeof(cw_nxoe_regsub_telm_t) * regsub.vlen);
    }

    if (regsub.template != NULL)
    {
	nxa_free(regsub.template, regsub.tlen);
    }

    free(regsub.pcre);
    if (regsub.extra != NULL)
    {
	free(regsub.extra);
    }

    retval = NXN_ZERO;
    RETURN:
    return retval;
}
