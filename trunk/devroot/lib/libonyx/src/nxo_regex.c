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

cw_nxn_t
nxo_regex_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, const cw_uint8_t *a_pattern,
	      cw_uint32_t a_len, cw_bool_t a_insensitive, cw_bool_t a_multiline,
	      cw_bool_t a_singleline, cw_uint32_t a_limit)
{
    cw_nxn_t retval;
    cw_nxoe_regex_t *regex;
    cw_nxa_t *nxa;
    char *pattern;
    const char *errptr;
    int options, erroffset;

    nxa = nx_nxa_get(a_nx);

    regex = (cw_nxoe_regex_t *) nxa_malloc(nxa, sizeof(cw_nxoe_regex_t));

    nxoe_l_new(&regex->nxoe, NXOT_REGEX, FALSE);

    /* Create a '\0'-terminated copy of a_pattern. */
    pattern = (char *) nxa_malloc(nxa, a_len + 1);
    memcpy(pattern, a_pattern, a_len);
    pattern[a_len] = '\0';

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

    regex->limit = a_limit;

    regex->pcre = pcre_compile(pattern, options, &errptr, &erroffset, NULL);
    nxa_free(nxa, pattern, a_len + 1);
    if (regex->pcre == NULL)
    {
	nxa_free(nxa, regex, sizeof(cw_nxoe_regex_t));
	retval = NXN_regexerror;
	goto RETURN;
    }

    /* XXX Call pcre_study(). */

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) regex;
    nxo_p_type_set(a_nxo, NXOT_REGEX);

    nxa_l_gc_register(nxa, (cw_nxoe_t *) regex);

    retval = NXN_ZERO;
    RETURN:
    return retval;
}
