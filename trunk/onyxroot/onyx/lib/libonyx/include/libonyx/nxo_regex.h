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

/* One of these is embedded in cw_nxoe_thread_t, which provides a per-thread
 * regex cache. */
typedef struct
{
    /* A reference to the input string that was most recently matched against,
     * or a "no" object. */
    cw_nxo_t input;

    /* Offset into the input string to start the next match at.  This is only
     * used if the $c or $g flag is set. */
    int cont;

    /* Number of offset pairs in the offsets array that are valid.  0 or -1 is
     * used to indicate that there are no valid offset pairs. */
    int mcnt;

    /* Cached vector and its length.  This vector is repeatedly used for calls
     * to pcre_exec(), and is cached here in order to avoid repeated
     * allocations.
     *
     * The vector contains offset pairs into the input string of subpattern
     * matches.  Only the first mcnt pairs are valid.
     *
     * This vector starts out empty (ovp == NULL), and is grown as necessary,
     * but never shrunk. */
    int *ovp;
    int ovcnt;
} cw_nxo_regex_cache_t;

cw_nxn_t
nxo_regex_new(cw_nxo_t *a_nxo, const char *a_pattern, uint32_t a_len,
	      bool a_cont, bool a_global, bool a_insensitive,
	      bool a_multiline, bool a_singleline);

void
nxo_regex_match(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, cw_nxo_t *a_input,
		bool *r_match);

cw_nxn_t
nxo_regex_nonew_match(cw_nxo_t *a_thread, const char *a_pattern,
		      uint32_t a_len, bool a_cont, bool a_global,
		      bool a_insensitive, bool a_multiline,
		      bool a_singleline, cw_nxo_t *a_input,
		      bool *r_match);

void
nxo_regex_split(cw_nxo_t *a_nxo, cw_nxo_t *a_thread, uint32_t a_limit,
		cw_nxo_t *a_input, cw_nxo_t *r_array);

cw_nxn_t
nxo_regex_nonew_split(cw_nxo_t *a_thread, const char *a_pattern,
		      uint32_t a_len, bool a_insensitive,
		      bool a_multiline, bool a_singleline,
		      uint32_t a_limit, cw_nxo_t *a_input,
		      cw_nxo_t *r_array);

void
nxo_regex_submatch(cw_nxo_t *a_thread, uint32_t a_capture,
		   cw_nxo_t *r_match);
