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

#define CW_NXO_REGEX_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_regex_l.h"
#include "../include/libonyx/nxo_thread_l.h"

/* Do the work of initializing a regex, but don't do any of the typical
 * GC-related initialization, so that this function can be used for the case
 * where a regex is temporarily constructed for a single match. */
static cw_nxn_t
nxo_p_regex_init(cw_nxoe_regex_t *a_regex, cw_nxa_t *a_nxa,
		 const cw_uint8_t *a_pattern, cw_uint32_t a_len,
		 cw_bool_t a_cont, cw_bool_t a_global, cw_bool_t a_insensitive,
		 cw_bool_t a_multiline, cw_bool_t a_singleline)
{
    cw_nxn_t retval;
    char *pattern;
    const char *errptr;
    int options, erroffset, capturecount;

    nxoe_l_new(&a_regex->nxoe, NXOT_REGEX, FALSE);

    /* Create a '\0'-terminated copy of a_pattern. */
    pattern = (char *) nxa_malloc(a_nxa, a_len + 1);
    memcpy(pattern, a_pattern, a_len);
    pattern[a_len] = '\0';

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

    /* Store the cont and global flags.  This information is not needed in this
     * function, but gets used when actually doing matches. */
    a_regex->cont = a_cont;
    a_regex->global = a_global;

    /* Compile the regex. */
    a_regex->pcre = pcre_compile(pattern, options, &errptr, &erroffset, NULL);
    nxa_free(a_nxa, pattern, a_len + 1);
    if (a_regex->pcre == NULL)
    {
	retval = NXN_regexerror;
	goto RETURN;
    }

    /* Call pcre_study(), which may improve matching performance. */
    a_regex->extra = pcre_study(a_regex->pcre, 0, &errptr);
    if (errptr != NULL)
    {
	free(a_regex->pcre);
	retval = NXN_regexerror;
	goto RETURN;
    }

    /* Get capturecount and the amount of space that was allocated for
     * a_regex->pcre and a_regex->extra. */
    if ((pcre_fullinfo(a_regex->pcre, a_regex->extra, PCRE_INFO_CAPTURECOUNT,
		       &capturecount) != 0)
	|| (pcre_fullinfo(a_regex->pcre, a_regex->extra, PCRE_INFO_SIZE,
			  &a_regex->size) != 0)
#ifdef PCRE_INFO_EXTRASIZE
	|| (pcre_fullinfo(a_regex->pcre, a_regex->extra, PCRE_INFO_EXTRASIZE,
			  &a_regex->extrasize) != 0)
#endif
	)
    {
	free(a_regex->pcre);
	if (a_regex->extra != NULL)
	{
	    free(a_regex->extra);
	}
	retval = NXN_regexerror;
	goto RETURN;
    }

    /* Use capturecount to calculate the size of vector needed for pcre_exec()
     * calls. */
    a_regex->ovcnt = (capturecount + 1) * 3;

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

/* Returns TRUE if match is successful. */
static cw_bool_t
nxo_p_regex_match(cw_nxoe_regex_t *a_regex, cw_nxo_t *a_thread,
		  cw_nxo_t *a_input)
{
    cw_bool_t retval;
    cw_nxo_regex_cache_t *cache;
    cw_nx_t *nx;
    cw_nxa_t *nxa;
    int ioff;

    cache = nxo_l_thread_regex_cache_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    nxa = nx_nxa_get(nx);

    if (nxo_string_len_get(a_input) == 0)
    {
	cache->mcnt = -1;
	retval = FALSE;
	goto RETURN;
    }

    /* Allocate or extend the vector for passing to pcre_exec(), if
     * necessary. */
    if (cache->ovp == NULL)
    {
	cache->ovp = nxa_malloc(nxa, sizeof(int) * a_regex->ovcnt);
	cache->ovcnt = a_regex->ovcnt;
    }
    else if (cache->ovcnt < a_regex->ovcnt)
    {
	cache->ovp = nxa_realloc(nxa, cache->ovp, sizeof(int) * a_regex->ovcnt,
				 sizeof(int) * cache->ovcnt);
	cache->ovcnt = a_regex->ovcnt;
    }

    /* Determine where in the string to start searching.  This depends on
     * whether $c or $g is set, as well as whether the previous match was
     * against the same string. */
    if (a_regex->cont || a_regex->global)
    {
	if (nxo_type_get(&cache->input) == NXOT_STRING
	    && nxo_compare(&cache->input, a_input) == 0)
	{
	    ioff = cache->cont;
	    if ((cw_uint32_t) ioff >= nxo_string_len_get(a_input))
	    {
		cache->mcnt = -1;
		retval = FALSE;
		goto NOMATCH;
	    }
	}
	else
	{
	    ioff = 0;
	}
    }
    else
    {
	ioff = 0;
    }

    /* Look for a match. */
    nxo_string_lock(a_input);
    cache->mcnt = pcre_exec(a_regex->pcre, a_regex->extra,
			    (char *) nxo_string_get(a_input),
			    (int) nxo_string_len_get(a_input),
			    ioff, 0, cache->ovp, cache->ovcnt);
    nxo_string_unlock(a_input);
    if (cache->mcnt <= 0)
    {
	switch (cache->mcnt)
	{
	    case 0:
	    case PCRE_ERROR_NOMATCH:
	    {
		/* No match found.  Not an error. */
		retval = FALSE;
		break;
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
    else
    {
	retval = TRUE;
    }

    NOMATCH:
    /* Store the location to start the next search from, if $c or $g is set.
     * Also set the cached input string accordingly. */
    if (a_regex->cont)
    {
	if (cache->mcnt > 0)
	{
	    nxo_dup(&cache->input, a_input);
	    cache->cont = cache->ovp[1];
	}
    }
    else if (a_regex->global)
    {
	if (cache->mcnt > 0)
	{
	    nxo_dup(&cache->input, a_input);
	    cache->cont = cache->ovp[1];
	}
	else
	{
	    nxo_no_new(&cache->input);
	    cache->cont = 0;
	}
    }
    else
    {
	if (cache->mcnt > 0)
	{
	    nxo_dup(&cache->input, a_input);
	}
	else
	{
	    nxo_no_new(&cache->input);
	}
    }

    RETURN:
    return retval;
}

static void
nxo_p_regex_split(cw_nxoe_regex_t *a_regex, cw_nxo_t *a_thread,
		  cw_uint32_t a_limit, cw_nxo_t *a_input, cw_nxo_t *r_array)
{
    cw_nxo_regex_cache_t *cache;
    cw_nxo_t *tstack, *tnxo;
    cw_nx_t *nx;
    cw_nxa_t *nxa;
    cw_uint8_t *istr;
    int ilen, ioff;
    cw_uint32_t i, acnt;

    cache = nxo_l_thread_regex_cache_get(a_thread);
    tstack = nxo_thread_tstack_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);
    nxa = nx_nxa_get(nx);

    istr = nxo_string_get(a_input);
    ilen = (int) nxo_string_len_get(a_input);
    if (ilen == 0)
    {
	cache->mcnt = -1;
	acnt = 0;
	goto NOMATCH;
    }

    /* Allocate or extend the vector for passing to pcre_exec(), if
     * necessary. */
    if (cache->ovp == NULL)
    {
	cache->ovp = nxa_malloc(nxa, sizeof(int) * a_regex->ovcnt);
	cache->ovcnt = a_regex->ovcnt;
    }
    else if (cache->ovcnt < a_regex->ovcnt)
    {
	cache->ovp = nxa_realloc(nxa, cache->ovp, sizeof(int) * a_regex->ovcnt,
				 sizeof(int) * cache->ovcnt);
	cache->ovcnt = a_regex->ovcnt;
    }

    /* Iteratively search for matches with the splitting pattern and create
     * substrings until there is no more text, or the split limit has been
     * reached. */
    for (acnt = ioff = 0;
	 ioff < ilen && (acnt + 1 < a_limit || a_limit == 0);
	 )
    {
	/* Look for a match. */
	nxo_string_lock(a_input);
	cache->mcnt = pcre_exec(a_regex->pcre, a_regex->extra,
				(char *) istr, ilen, ioff, 0,
				cache->ovp, cache->ovcnt);
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

	/* Create a substring of the text that falls between the previous match
	 * and this match. */
	tnxo = nxo_stack_push(tstack);
	if (cache->ovp[0] < cache->ovp[1])
	{
	    /* Create a substring (normal case). */
	    nxo_string_substring_new(tnxo, a_input, nx, ioff,
				     cache->ovp[0] - ioff);
	    ioff = cache->ovp[1];
	}
	else
	{
	    /* The pattern matches the empty string, so split a single character
	     * to avoid an infinite loop. */
	    nxo_string_substring_new(tnxo, a_input, nx, ioff, 1);
	    ioff++;
	}
	acnt++;

	/* If there were capturing subpatterns that matched, create substrings
	 * of them.  If there are interspersed subpatterns that did not match,
	 * create null objects for them. */
	if (cache->mcnt > 1)
	{
	    for (i = 1; i < cache->mcnt; i++)
	    {
		tnxo = nxo_stack_push(tstack);
		nxo_string_substring_new(tnxo, a_input, nx,
					 cache->ovp[i * 2],
					 cache->ovp[i * 2 + 1]
					 - cache->ovp[i * 2]);
		acnt++;
	    }
	}
    }
    DONE:
    /* If there are trailing bytes after the last match, create a substring and
     * push it onto tstack. */
    if (ioff < ilen)
    {
	tnxo = nxo_stack_push(tstack);
	nxo_string_substring_new(tnxo, a_input, nx, ioff,
				 nxo_string_len_get(a_input)
				 - (cw_uint32_t) ioff);
	acnt++;
    }

    NOMATCH:
    /* Create an array that contains the substrings on tstack. */
    nxo_array_new(r_array, nx, nxo_thread_currentlocking(a_thread), acnt);

    /* Dup the substrings into r_matches. */
    for (i = 0, tnxo = nxo_stack_get(tstack);
	 i < acnt;
	 i++, tnxo = nxo_stack_down_get(tstack, tnxo))
    {
	nxo_array_el_set(r_array, tnxo, acnt - 1 - i);
    }

    /* Clean up tstack. */
    nxo_stack_npop(tstack, acnt);
}

cw_nxn_t
nxo_regex_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, const cw_uint8_t *a_pattern,
	      cw_uint32_t a_len, cw_bool_t a_cont, cw_bool_t a_global,
	      cw_bool_t a_insensitive, cw_bool_t a_multiline,
	      cw_bool_t a_singleline)
{
    cw_nxn_t retval;
    cw_nxoe_regex_t *regex;
    cw_nxa_t *nxa;

    nxa = nx_nxa_get(a_nx);

    regex = (cw_nxoe_regex_t *) nxa_malloc(nxa, sizeof(cw_nxoe_regex_t));

    retval = nxo_p_regex_init(regex, nxa, a_pattern, a_len, a_cont, a_global,
			      a_insensitive, a_multiline, a_singleline);
    if (retval)
    {
	nxa_free(nxa, regex, sizeof(cw_nxoe_regex_t));
	goto RETURN;
    }

    /* Tell the GC about the space being taken up by regex->pcre and
     * regex->extra. */
    nxa_l_count_adjust(nxa, (cw_nxoi_t) regex->size
#ifdef PCRE_INFO_EXTRASIZE
		       + regex->extrasize
#endif
		       );

    /* Create a reference to the regex object. */
    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) regex;
    nxo_p_type_set(a_nxo, NXOT_REGEX);

    /* Register the regex object with the GC. */
    nxa_l_gc_register(nxa, (cw_nxoe_t *) regex);

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

void
nxo_regex_match(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *a_input,
		cw_bool_t *r_match)
{
    cw_nxoe_regex_t *regex;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_REGEX);

    regex = (cw_nxoe_regex_t *) a_nxo->o.nxoe;

    cw_check_ptr(regex);
    cw_dassert(regex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(regex->nxoe.type == NXOT_REGEX);

    *r_match = nxo_p_regex_match(regex, a_thread, a_input);
}

/* Do a match without creating a regex object, in order to avoid putting
 * pressure on the GC. */
cw_nxn_t
nxo_regex_nonew_match(cw_nxo_t *a_thread, const cw_uint8_t *a_pattern,
		      cw_uint32_t a_len, cw_bool_t a_cont, cw_bool_t a_global,
		      cw_bool_t a_insensitive, cw_bool_t a_multiline,
		      cw_bool_t a_singleline, cw_nxo_t *a_input,
		      cw_bool_t *r_match)
{
    cw_nxn_t retval;
    cw_nxoe_regex_t regex;
    cw_nx_t *nx;
    cw_nxa_t *nxa;

    nx = nxo_thread_nx_get(a_thread);
    nxa = nx_nxa_get(nx);

    retval = nxo_p_regex_init(&regex, nxa, a_pattern, a_len, a_cont, a_global,
			      a_insensitive, a_multiline, a_singleline);
    if (retval)
    {
	goto RETURN;
    }

    *r_match = nxo_p_regex_match(&regex, a_thread, a_input);

    /* Clean up memory. */
    free(regex.pcre);
    if (regex.extra != NULL)
    {
	free(regex.extra);
    }

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

void
nxo_regex_split(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_uint32_t a_limit,
		cw_nxo_t *a_input, cw_nxo_t *r_array)
{
    cw_nxoe_regex_t *regex;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_REGEX);

    regex = (cw_nxoe_regex_t *) a_nxo->o.nxoe;

    cw_check_ptr(regex);
    cw_dassert(regex->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(regex->nxoe.type == NXOT_REGEX);

    nxo_p_regex_split(regex, a_thread, a_limit, a_input, r_array);
}

/* Do a split without creating a regex object, in order to avoid putting
 * pressure on the GC. */
cw_nxn_t
nxo_regex_nonew_split(cw_nxo_t *a_thread, const cw_uint8_t *a_pattern,
		      cw_uint32_t a_len, cw_bool_t a_insensitive,
		      cw_bool_t a_multiline, cw_bool_t a_singleline,
		      cw_uint32_t a_limit, cw_nxo_t *a_input, cw_nxo_t *r_array)
{
    cw_nxn_t retval;
    cw_nxoe_regex_t regex;
    cw_nx_t *nx;
    cw_nxa_t *nxa;

    nx = nxo_thread_nx_get(a_thread);
    nxa = nx_nxa_get(nx);

    retval = nxo_p_regex_init(&regex, nxa, a_pattern, a_len, FALSE, FALSE,
			      a_insensitive, a_multiline, a_singleline);
    if (retval)
    {
	goto RETURN;
    }

    nxo_p_regex_split(&regex, a_thread, a_limit, a_input, r_array);

    /* Clean up memory. */
    free(regex.pcre);
    if (regex.extra != NULL)
    {
	free(regex.extra);
    }

    retval = NXN_ZERO;
    RETURN:
    return retval;
}

void
nxo_regex_submatch(cw_nxo_t *a_thread, cw_uint32_t a_capture, cw_nxo_t *r_match)
{
    cw_nxo_regex_cache_t *cache;

    cache = nxo_l_thread_regex_cache_get(a_thread);

    if ((int) a_capture < cache->mcnt
	&& nxo_type_get(&cache->input) == NXOT_STRING
	&& cache->ovp[a_capture * 2] != -1)
    {
	/* Create a substring for the capturing subpattern. */
	nxo_string_substring_new(r_match, &cache->input,
				 nxo_thread_nx_get(a_thread),
				 cache->ovp[a_capture * 2],
				 cache->ovp[a_capture * 2 + 1]
				 - cache->ovp[a_capture * 2]);
    }
    else
    {
	/* This subpattern wasn't matched (possible when the capture is for an
	 * alternative). */
	nxo_null_new(r_match);
    }
}
