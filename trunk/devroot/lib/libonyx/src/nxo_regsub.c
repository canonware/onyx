/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version = onyx>
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
nxo_p_regsub_init(cw_nxoe_regsub_t *a_regsub, cw_nxa_t *a_nxa,
		  const cw_uint8_t *a_pattern, cw_uint32_t a_plen,
		  cw_bool_t a_global, cw_bool_t a_insensitive,
		  cw_bool_t a_multiline, cw_bool_t a_singleline,
		  const cw_uint8_t *a_template, cw_uint32_t a_tlen)
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
    cw_uint32_t i, beg, end, voff;

    nxoe_l_new(&a_regsub->nxoe, NXOT_REGSUB, FALSE);

    /* Create a '\0'-terminated copy of a_pattern. */
    pattern = (char *) nxa_malloc(a_nxa, a_plen + 1);
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
    nxa_free(a_nxa, pattern, a_plen + 1);
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
#ifdef PCRE_INFO_EXTRASIZE
	|| (pcre_fullinfo(a_regsub->pcre, a_regsub->extra, PCRE_INFO_EXTRASIZE,
			  &a_regsub->extrasize) != 0)
#endif
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
    a_regsub->template = (cw_uint8_t *) nxa_malloc(a_nxa, a_tlen);
    memcpy(a_regsub->template, a_template, a_tlen);
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
//			fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__, __FUNCTION__);
			end = i + 1;
			tstate = TSTATE_BS_CONT;
			break;
		    }
		    default:
		    {
//			fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__, __FUNCTION__);
			break;
		    }
		}
		break;
	    }
	    case TSTATE_BS_CONT:
	    {
		switch (a_regsub->template[i])
		{
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    {
//			fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__, __FUNCTION__);
			/* Preceding plain text, if any. */
			if (end > beg)
			{
//			fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__, __FUNCTION__);
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
//			fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__, __FUNCTION__);
			/* Stay in this state (ignore extra leading '\'
			 * characters. */
			end = i + 1;
			break;
		    }
		    default:
		    {
//			fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__, __FUNCTION__);
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
//			fprintf(stderr, "%s:%d:%s()\n", __FILE__, __LINE__, __FUNCTION__);
	/* Normal characters after last subpattern substitution. */
	a_regsub->vlen++;
    }

    /* Initialize the vector, now that we know how big to make it. */
    a_regsub->vec
	= (cw_nxoe_regsub_telm_t *) nxa_malloc(a_nxa,
					       sizeof(cw_nxoe_regsub_telm_t)
					       * a_regsub->vlen);
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
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
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
			    = (cw_uint32_t) (a_regsub->template[i] - '0');
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

//CW_P_INLINE void
static void
nxo_p_regsub_append(cw_uint8_t **r_ostr, cw_uint32_t *r_omax,
		    cw_uint32_t *r_olen, const cw_uint8_t *a_istr,
		    cw_uint32_t a_ilen, cw_nxa_t *a_nxa)
{
    cw_uint32_t omax;

    /* Expand *r_ostr, if necessary. */
    for (omax = *r_omax; *r_olen + a_ilen > omax; omax *= 2)
    {
	/* Do nothing. */
    }
    if (omax != *r_omax)
    {
	*r_ostr = nxa_realloc(a_nxa, *r_ostr, omax, *r_omax);
	*r_omax = omax;
    }

/*      fprintf(stderr, "%s:%d:%s(): *r_ostr(%p): \"%s\"\n", __FILE__, __LINE__, */
/*  	    __FUNCTION__, *r_ostr, *r_ostr); */
/*      fprintf(stderr, "%s:%d:%s(): *r_olen: %u, *r_omax: %u\n", __FILE__, */
/*  	    __LINE__, __FUNCTION__, *r_olen, *r_omax); */
/*      fprintf(stderr, "%s:%d:%s(): a_istr(%p): \"%s\"\n", __FILE__, __LINE__, */
/*  	    __FUNCTION__, a_istr, a_istr); */
/*      fprintf(stderr, "%s:%d:%s(): a_ilen: %u\n", __FILE__, __LINE__, */
/*  	    __FUNCTION__, a_ilen); */

/*      fprintf(stderr, "%s:%d:%s(): memcpy(\"%s\", \"%s\", %u)\n", */
/*  	    __FILE__, __LINE__, __FUNCTION__, */
/*  	    &(*r_ostr)[*r_olen], a_istr, a_ilen); */


    /* Copy and adjust *r_olen. */
    memcpy(&(*r_ostr)[*r_olen], a_istr, a_ilen);
    *r_olen += a_ilen;
}

static void
nxo_p_regsub_subst(cw_nxoe_regsub_t *a_regsub, cw_nxo_t *a_thread,
		   cw_nxo_t *a_input, cw_nxo_t *r_output)
{
    cw_nxo_regex_cache_t *cache;
    cw_nx_t *nx;
    cw_nxa_t *nxa;
    cw_uint32_t scnt, ilen, ioff, olen, omax, v;
    cw_uint8_t *istr, *ostr;

    cache = nxo_l_thread_regex_cache_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    nxa = nx_nxa_get(nx);

    /* Allocate or extend the vector for passing to pcre_exec(), if
     * necessary. */
    if (cache->ovp == NULL)
    {
	cache->ovp = nxa_malloc(nxa, sizeof(int) * a_regsub->ovcnt);
	cache->ovcnt = a_regsub->ovcnt;
    }
    else if (cache->ovcnt < a_regsub->ovcnt)
    {
	cache->ovp = nxa_realloc(nxa, cache->ovp, sizeof(int) * a_regsub->ovcnt,
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
    ostr = nxa_malloc(nxa, omax);

    /* Iteratively look for matches. */
    for (scnt = ioff = 0;
	 ioff < ilen && (a_regsub->global || scnt < 1);
	 scnt++, ioff = (cw_uint32_t) cache->ovp[1])
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
	if (ioff < (cw_uint32_t) cache->ovp[0])
	{
	    nxo_p_regsub_append(&ostr, &omax, &olen,
				&istr[ioff],
				(cw_uint32_t) cache->ovp[0] - ioff, nxa);
	}

	/* Substitute. */
	for (v = 0; v < a_regsub->vlen; v++)
	{
	    if (a_regsub->vec[v].str != NULL)
	    {
		/* Copy from template string. */
		nxo_p_regsub_append(&ostr, &omax, &olen, a_regsub->vec[v].str,
				    a_regsub->vec[v].len, nxa);
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
					- cache->ovp[a_regsub->vec[v].len * 2],
					nxa);
		}
	    }
	}
    }
    DONE:
    /* If there are trailing bytes after the last match, copy them. */
    if (ioff < ilen)
    {
	nxo_p_regsub_append(&ostr, &omax, &olen, &istr[ioff], ilen - ioff, nxa);
    }

    /* Create an Onyx string and copy ostr to it. */
    nxo_string_new(r_output, nx, nxo_thread_currentlocking(a_thread), olen);
    if (olen > 0)
    {
	nxo_string_set(r_output, 0, ostr, olen);
    }

    /* Clean up. */
    nxa_free(nxa, ostr, omax);
}

cw_nxn_t
nxo_regsub_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, const cw_uint8_t *a_pattern,
	       cw_uint32_t a_plen, cw_bool_t a_global, cw_bool_t a_insensitive,
	       cw_bool_t a_multiline, cw_bool_t a_singleline,
	       const cw_uint8_t *a_template,
	       cw_uint32_t a_tlen)
{
    cw_nxn_t retval;
    cw_nxoe_regsub_t *regsub;
    cw_nxa_t *nxa;

    nxa = nx_nxa_get(a_nx);

    regsub = (cw_nxoe_regsub_t *) nxa_malloc(nxa, sizeof(cw_nxoe_regsub_t));

    retval = nxo_p_regsub_init(regsub, nxa, a_pattern, a_plen, a_global,
			       a_insensitive, a_multiline, a_singleline,
			       a_template, a_tlen);
    if (retval)
    {
	nxa_free(nxa, regsub, sizeof(cw_nxoe_regsub_t));
	goto RETURN;
    }

    /* Tell the GC about the space being taken up by regsub->pcre and
     * regsub->extra. */
    nxa_l_count_adjust(nxa, (cw_nxoi_t) regsub->size
#ifdef PCRE_INFO_EXTRASIZE
		       + regsub->extrasize
#endif
		       );

    /* Create a reference to the regsub object. */
    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) regsub;
    nxo_p_type_set(a_nxo, NXOT_REGSUB);

    /* Register the regsub object with the GC. */
    nxa_l_gc_register(nxa, (cw_nxoe_t *) regsub);

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

void
nxo_regsub_subst(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *a_input,
		 cw_nxo_t *r_output)
{
    cw_nxoe_regsub_t *regsub;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_REGSUB);

    regsub = (cw_nxoe_regsub_t *) a_nxo->o.nxoe;

    cw_check_ptr(regsub);
    cw_dassert(regsub->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(regsub->nxoe.type == NXOT_REGSUB);

    nxo_p_regsub_subst(regsub, a_thread, a_input, r_output);
}

/* Do a subst without creating a regsub object, in order to avoid putting
 * pressure on the GC. */
cw_nxn_t
nxo_regsub_nonew_subst(cw_nxo_t *a_thread, const cw_uint8_t *a_pattern,
		       cw_uint32_t a_plen, cw_bool_t a_global,
		       cw_bool_t a_insensitive, cw_bool_t a_multiline,
		       cw_bool_t a_singleline, const cw_uint8_t *a_template,
		       cw_uint32_t a_tlen, cw_nxo_t *a_input,
		       cw_nxo_t *r_output)
{
    cw_nxn_t retval;
    cw_nxoe_regsub_t regsub;
    cw_nx_t *nx;
    cw_nxa_t *nxa;

    nx = nxo_thread_nx_get(a_thread);
    nxa = nx_nxa_get(nx);

    retval = nxo_p_regsub_init(&regsub, nxa, a_pattern, a_plen, a_global,
			       a_insensitive, a_multiline, a_singleline,
			       a_template, a_tlen);
    if (retval)
    {
	goto RETURN;
    }

    nxo_p_regsub_subst(&regsub, a_thread, a_input, r_output);

    /* Clean up memory. */

    if (regsub.vec != NULL)
    {
	nxa_free(nxa, regsub.vec, sizeof(cw_nxoe_regsub_telm_t) * regsub.vlen);
    }

    if (regsub.template != NULL)
    {
	nxa_free(nxa, regsub.template, regsub.tlen);
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
