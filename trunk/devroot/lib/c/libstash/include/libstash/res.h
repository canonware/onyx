/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * Description: 
 *              
 * Very simple resources class.  A resource looks like:
 *
 * <resource> ::= <^> <whitespaces> <name> <whitespaces> <linebreaks> <colon>
 *                <whitespaces> <linebreaks> <value> <comment>
 *              | <^> <whitespaces> <comment>
 * <name> ::= { <caps> | <lower> | <numbers> | <under> | <period> }+
 * <value> ::= { <caps> | <lower> | <numbers> | <under> | <period>
 *               | <backslash> <hash> | <whitespace> | <colon>
 *               | <backslash> <backslash> | <backslash> <n>
 *               | <legal_in_name> | <linebreak> }+
 *           | <e>
 * <comment> ::= <hash> { <caps> | <lower> | <numbers> | <under> | <period>
 *                        | <hash> | <whitespace> | <colon>
 *                        | <backslash> | <legal_in_name> }+
 *             | <hash> <e>
 * <linebreak> ::= <backslash> <whitespaces> <newline>
 * <linebreaks> ::= <linebreak> <linebreaks>
 *                | <e>
 * <^> ::= Bound to beginning of line.
 * <e> ::= Epsilon.
 * <n> ::= [n]
 * <caps> ::= [A-Z]
 * <lower> ::= [a-z]
 * <numbers> ::= [0-9]
 * <under> ::= [_]
 * <period> ::= [.]
 * <hash> ::= [#]
 * <whitespace> ::= [ \t] | <whitespace> [ \t]
 * <whitespaces> ::= <whitespace> <whitespaces>
 *                 | <e>
 * <colon> ::= [:]
 * <backslash> ::= [\\]
 * <legal_in_name> ::= [!"$%&'()*+,-/;<=>?@[]^`{|}~]
 * <newline> ::= [\n]
 * <null> ::= [\0]
 *
 * \ is a special character within <name>.  \ protects [#\\\n] and [ ]+[\n],
 * but a \ followed by anything else is an error.
 *              
 ****************************************************************************
 */

/* Pseudo-opaque type. */
typedef struct cw_res_s cw_res_t;

struct cw_res_s
{
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_rwl_t rw_lock;
#endif
  cw_oh_t hash;
  FILE * fd;
  char * str;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 *
 *
 * <<< Output(s) >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_res_t *
res_new(cw_res_t * a_res);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 *
 *
 * <<< Output(s) >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
res_delete(cw_res_t * a_res);

/****************************************************************************
 * <<< Description >>>
 *
 * Clears out all resources.
 *
 ****************************************************************************/
void
res_clear(cw_res_t * a_res);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 *
 *
 * <<< Output(s) >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
res_is_equal(cw_res_t * a_res, cw_res_t * a_resther);

/****************************************************************************
 * <<< Return Value >>>
 *
 * TRUE == error 
 *
 * <<< Description >>>
 *
 * Merges the resources contained in a_filename into the resource database.
 *
 ****************************************************************************/
cw_bool_t
res_merge_file(cw_res_t * a_res, const char * a_filename);

/****************************************************************************
 * <<< Arguments >>>
 *
 * ... : NULL-terminated list of resource name/value pair strings.
 *
 * <<< Return Value >>>
 *
 * TRUE == error
 *
 * <<< Description >>>
 *
 * Merges the resources into the resource database.
 *
 ****************************************************************************/
cw_bool_t
res_merge_list(cw_res_t * a_res, ...);

/****************************************************************************
 * <<< Return Value >>>
 *
 * NULL == error
 *
 * <<< Description >>>
 *
 * Returns the value associated with a_res_name if it exists.
 *
 ****************************************************************************/
const char *
res_get_res_val(cw_res_t * a_res, const char * a_res_name);

/****************************************************************************
 * <<< Description >>>
 *
 * Find a resource name/value pair, remove it from the resource database,
 * and set *a_res_name and *a_res_val to point it.  If the resource isn't
 * found, return TRUE.
 *
 ****************************************************************************/
cw_bool_t
res_extract_res(cw_res_t * a_res, char * a_res_key,
		char ** a_res_name, char ** a_res_val);

/****************************************************************************
 * <<< Description >>>
 *
 * Dump the resource database.  If a_filename is non-NULL, attempt to open
 * the specified file and write to it.  Otherwise, use g_log.
 *
 ****************************************************************************/
cw_bool_t
res_dump(cw_res_t * a_res, char * a_filename);
