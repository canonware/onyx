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

static void
nxo_p_regex_match(cw_nxoe_regex_t *a_regex, cw_nxo_t *a_thread,
		  cw_nxo_t *a_input, cw_bool_t *r_match)
{
    cw_nxo_t *tstack, *tnxo, *matches, *prev_input;
    cw_nx_t *nx;
    cw_uint32_t i;
    int *ovp, offset;
    int mcnt = -1;

    if (nxo_string_len_get(a_input) == 0)
    {
	*r_match = FALSE;
	return;
    }

    tstack = nxo_thread_tstack_get(a_thread);
    tnxo = nxo_stack_push(tstack);
    matches = nxo_thread_regex_matches_get(a_thread);
    nx = nxo_thread_nx_get(a_thread);

    /* Allocate a vector for passing to pcre_exec(). */
    ovp = cw_malloc(sizeof(int) * a_regex->ovcnt);

    /* Determine where in the string to start searching.  This depends on
     * whether $c or $g is set, as well as whether the previous match was
     * against the same string. */
    if (a_regex->cont || a_regex->global)
    {
	prev_input = nxo_l_thread_regex_input_get(a_thread);
	if (nxo_type_get(prev_input) == NXOT_STRING
	    && nxo_compare(prev_input, a_input) == 0)
	{
	    offset = nxo_l_thread_regex_cont_get(a_thread);
	    if (offset >= nxo_string_len_get(a_input))
	    {
		*r_match = FALSE;
		goto NOMATCH;
	    }
	}
	else
	{
	    offset = 0;
	}
    }
    else
    {
	offset = 0;
    }

    /* Look for a match. */
    nxo_string_lock(a_input);
    mcnt = pcre_exec(a_regex->pcre, a_regex->extra,
		     (char *) nxo_string_get(a_input),
		     (int) nxo_string_len_get(a_input),
		     offset, 0, ovp, a_regex->ovcnt);
    nxo_string_unlock(a_input);
    if (mcnt <= 0)
    {
	switch (mcnt)
	{
	    case 0:
	    case PCRE_ERROR_NOMATCH:
	    {
		/* No match found.  Not an error. */
		*r_match = FALSE;
		goto NOMATCH;
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
    *r_match = TRUE;

    NOMATCH:
    /* Store the location to start the next search from, if $c or $g is set. */
    if (a_regex->cont)
    {
	/* Only update the start location if this search was successful. */
	if (mcnt > 0)
	{
	    nxo_l_thread_regex_input_set(a_thread, a_input);
	    nxo_l_thread_regex_cont_set(a_thread, ovp[1]);
	}
    }
    else if (a_regex->global && mcnt > 0)
    {
	nxo_l_thread_regex_input_set(a_thread, a_input);
	nxo_l_thread_regex_cont_set(a_thread, ovp[1]);
    }
    else
    {
	/* Set to "no" object. */
	nxo_l_thread_regex_input_set(a_thread, tnxo);
    }

    /* Set all array elements to null. */
    nxo_null_new(tnxo);
    for (i = 0; i < 10; i++)
    {
	nxo_array_el_set(matches, tnxo, i);
    }

    /* Save the captured subpatterns.  Take care to set unused elements of the
     * array to null. */
    if (mcnt > 0)
    {
	for (i = 0; i < mcnt && i < 10; i++)
	{
	    if (ovp[i * 2] != -1)
	    {
		nxo_string_substring_new(tnxo, a_input, nx, ovp[i * 2],
					 ovp[i * 2 + 1] - ovp[i * 2]);
		nxo_array_el_set(matches, tnxo, i);
	    }
	}
    }

    nxo_stack_pop(tstack);

    /* Free the vector used with pcre_exec(). */
    cw_free(ovp);
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
    nxa_l_count_adjust(nxa, regex->size
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

    nxo_p_regex_match(&regex, a_thread, a_input, r_match);

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

    nxo_p_regex_match(regex, a_thread, a_input, r_match);
}
