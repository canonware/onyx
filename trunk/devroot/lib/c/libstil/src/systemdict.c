/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstil/libstil.h"

#include <errno.h>

/* Initial size of dictionaries created with the dict operator. */
#define	_CW_SYSTEMDICT_DICT_SIZE	16

#define soft_code(a_str) do {					\
	cw_stilts_t	stilts;						\
	static const cw_uint8_t	code[] = (a_str);			\
									\
	stilts_new(&stilts, a_stilt);					\
	stilt_interpret(a_stilt, &stilts, code, sizeof(code) - 1);	\
	stilt_flush(a_stilt, &stilts);					\
	stilts_delete(&stilts, a_stilt);				\
} while (0)

struct cw_systemdict_entry {
	cw_stiln_t	stiln;
	cw_op_t		*op_f;
};

#define ENTRY(name)	{STILN_##name, systemdict_##name}

/*
 * Array of operators in systemdict.
 */
static const struct cw_systemdict_entry systemdict_ops[] = {
	ENTRY(abort),
	ENTRY(abs),
	ENTRY(add),
	ENTRY(aload),
	ENTRY(anchorsearch),
	ENTRY(and),
	ENTRY(array),
	ENTRY(astore),
	ENTRY(begin),
	ENTRY(bind),
	ENTRY(bytesavailable),
	ENTRY(clear),
	ENTRY(cleardictstack),
	ENTRY(cleartomark),
	ENTRY(closefile),
	ENTRY(condition),
	ENTRY(copy),
	ENTRY(count),
	ENTRY(countdictstack),
	ENTRY(countexecstack),
	ENTRY(counttomark),
	ENTRY(currentcontext),
	ENTRY(currentdict),
	ENTRY(currentfile),
	ENTRY(currentglobal),
	ENTRY(cvlit),
	ENTRY(cvm),
	ENTRY(cvn),
	ENTRY(cvrs),
	ENTRY(cvs),
	ENTRY(cvx),
	ENTRY(def),
	ENTRY(defineresource),
	ENTRY(deletefile),
	ENTRY(detach),
	ENTRY(dict),
	ENTRY(dictstack),
	ENTRY(div),
	ENTRY(dup),
	ENTRY(end),
	ENTRY(eq),
	ENTRY(exch),
	ENTRY(exec),
	ENTRY(execstack),
	ENTRY(executeonly),
	ENTRY(executive),
	ENTRY(exit),
	ENTRY(exp),
	ENTRY(false),
	ENTRY(file),
	ENTRY(filenameforall),
	ENTRY(fileposition),
	ENTRY(filter),
	ENTRY(findresource),
	ENTRY(flush),
	ENTRY(flushfile),
	ENTRY(for),
	ENTRY(forall),
	ENTRY(fork),
	ENTRY(gcheck),
	ENTRY(ge),
	ENTRY(get),
	ENTRY(getinterval),
	ENTRY(gt),
	ENTRY(handleerror),
	ENTRY(if),
	ENTRY(ifelse),
	ENTRY(index),
	ENTRY(join),
	ENTRY(known),
	ENTRY(le),
	ENTRY(length),
	ENTRY(load),
	ENTRY(lock),
	ENTRY(loop),
	ENTRY(lt),
	ENTRY(mark),
	ENTRY(mod),
	ENTRY(mul),
	ENTRY(mutex),
	ENTRY(ne),
	ENTRY(neg),
	ENTRY(noaccess),
	ENTRY(not),
	ENTRY(notify),
	ENTRY(null),
	ENTRY(or),
	ENTRY(pop),
	ENTRY(print),
	ENTRY(product),
	ENTRY(prompt),
	ENTRY(promptstring),
	ENTRY(pstack),
	ENTRY(put),
	ENTRY(putinterval),
	ENTRY(quit),
	ENTRY(rand),
	ENTRY(rcheck),
	ENTRY(read),
	ENTRY(readonly),
	ENTRY(readstring),
	ENTRY(realtime),
	ENTRY(renamefile),
	ENTRY(repeat),
	ENTRY(resourceforall),
	ENTRY(resourcestatus),
	ENTRY(roll),
	ENTRY(run),
	ENTRY(search),
	ENTRY(setfileposition),
	ENTRY(setglobal),
	ENTRY(shift),
	ENTRY(srand),
	ENTRY(stack),
	ENTRY(start),
	ENTRY(status),
	ENTRY(stdin),
	ENTRY(stderr),
	ENTRY(stdout),
	ENTRY(stop),
	ENTRY(stopped),
	ENTRY(store),
	ENTRY(string),
	ENTRY(sub),
	ENTRY(sym_eq),
	ENTRY(sym_eq_eq),
	ENTRY(sym_gt_gt),
	{STILN_sym_lb, systemdict_mark},
	{STILN_sym_lt_lt, systemdict_mark},
	ENTRY(sym_rb),
	ENTRY(timedwait),
	ENTRY(token),
	ENTRY(true),
	ENTRY(trylock),
	ENTRY(type),
	ENTRY(undef),
	ENTRY(undefineresource),
	ENTRY(unlock),
	ENTRY(usertime),
	ENTRY(version),
	ENTRY(wait),
	ENTRY(wcheck),
	ENTRY(where),
	ENTRY(write),
	ENTRY(writestring),
	ENTRY(xcheck),
	ENTRY(xor),
	ENTRY(yield)
};

void
systemdict_populate(cw_stilo_t *a_dict, cw_stilt_t *a_stilt)
{
	cw_uint32_t	i;
	cw_stilo_t	name, operator;

#define	NEXTRA	2
#define NENTRIES							\
	(sizeof(systemdict_ops) / sizeof(struct cw_systemdict_entry))

	stilo_dict_new(a_dict, a_stilt, NENTRIES + NEXTRA);

	for (i = 0; i < NENTRIES; i++) {
		stilo_name_new(&name, a_stilt,
		    stiln_str(systemdict_ops[i].stiln),
		    stiln_len(systemdict_ops[i].stiln), TRUE);
		stilo_operator_new(&operator, systemdict_ops[i].op_f);
		stilo_attrs_set(&operator, STILOA_EXECUTABLE);

		stilo_dict_def(a_dict, a_stilt, &name, &operator);
	}

	/* Initialize entries that are not operators. */
	stilo_name_new(&name, a_stilt, stiln_str(STILN_globaldict),
	    stiln_len(STILN_globaldict), TRUE);
	stilo_dup(&operator, stilt_globaldict_get(a_stilt));
	stilo_dict_def(a_dict, a_stilt, &name, &operator);

	stilo_name_new(&name, a_stilt, stiln_str(STILN_systemdict),
	    stiln_len(STILN_systemdict), TRUE);
	stilo_dup(&operator, stilt_systemdict_get(a_stilt));
	stilo_dict_def(a_dict, a_stilt, &name, &operator);

#ifdef _LIBSTIL_DBG
	if (stilo_dict_count(a_dict) != NENTRIES + NEXTRA) {
		_cw_out_put_e("stilo_dict_count(a_dict) != NENTRIES + NEXTRA"
		    " ([i] != [i])\n", stilo_dict_count(a_dict), NENTRIES +
		    NEXTRA);
		_cw_error("Adjust NEXTRA");
	}
#endif
#undef NENTRIES
#undef NEXTRA
}

void
systemdict_abort(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_abs(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a;

	ostack = stilt_ostack_get(a_stilt);
	
	a = stils_get(ostack, a_stilt);
	if (stilo_type_get(a) != STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_integer_abs(a, a);
}

void
systemdict_add(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	b = stils_get(ostack, a_stilt);
	a = stils_down_get(ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_integer_add(a, b, a);
	stils_pop(ostack, a_stilt);
}

void
systemdict_aload(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*array, *stilo;
	cw_uint32_t	i, len;

	ostack = stilt_ostack_get(a_stilt);

	array = stils_get(ostack, a_stilt);
	if (stilo_type_get(array) != STILOT_ARRAY)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	for (i = 0, len = stilo_array_len_get(array); i < len; i++) {
		stilo = stils_under_push(ostack, a_stilt, array);
		stilo_dup(stilo, stilo_array_el_get(array, a_stilt, i));
	}
}

void
systemdict_anchorsearch(cw_stilt_t *a_stilt)
{
	soft_code("
%/anchorsearch
%{
  exch dup 3 1 roll % Make a copy of the original string.
  search % -string- -seek- search -post- -match- -pre- true
         % -string- -seek- search -string- false
  {
    % Search string found.  Check if it was at the beginning.
    3 2 roll length 0 eq
    {
      % Success.
      3 2 roll pop % Get rid of original string.
      true
    }{
      % Nope.  Clean up
      pop pop
      false
    } ifelse
  }{
    % Search string not found at all.
    pop pop pop
    false
  } ifelse
%} bind def
");
}

void
systemdict_and(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_array(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_sint64_t	len;

	ostack = stilt_ostack_get(a_stilt);
	
	stilo = stils_get(ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	len = stilo_integer_get(stilo);
	if (len < 0)
		stilt_error(a_stilt, STILTE_RANGECHECK);

	stilo_array_new(stilo, a_stilt, len);
}

void
systemdict_astore(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*array, *stilo;
	cw_sint32_t	i, len;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	array = stils_get(ostack, a_stilt);
	if (stilo_type_get(array) != STILOT_ARRAY)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	/* Make sure there will be enough objects to fill the array. */
	len = stilo_array_len_get(array);
	if (len > stils_count(ostack) - 1)
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);

	stilo = stils_push(tstack, a_stilt);
	stilo_dup(stilo, array);
	stils_pop(ostack, a_stilt);
	array = stilo;

	/* Move ostack objects to the array. */
	for (i = len - 1; i >= 0; i--) {
		stilo_array_el_set(array, a_stilt, stils_get(ostack, a_stilt),
		    i);
		stils_pop(ostack, a_stilt);
	}

	/* Push the array back onto ostack. */
	stilo = stils_push(ostack, a_stilt);
	stilo_dup(stilo, array);
	stils_pop(tstack, a_stilt);
}

void
systemdict_begin(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*stilo, *dict;

	dstack = stilt_dstack_get(a_stilt);
	ostack = stilt_ostack_get(a_stilt);

	dict = stils_get(ostack, a_stilt);
	if (stilo_type_get(dict) != STILOT_DICT)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	
	stilo = stils_push(dstack, a_stilt);
	stilo_dup(stilo, dict);
	stils_pop(ostack, a_stilt);
}

void
systemdict_bind(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_bytesavailable(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file;
	cw_uint32_t	bytes;

	ostack = stilt_ostack_get(a_stilt);

	file = stils_get(ostack, a_stilt);
	
	if (stilo_type_get(file) != STILOT_FILE)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	bytes = stilo_file_buffer_count(file);
	stilo_integer_new(file, bytes);
}

void
systemdict_clear(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_uint32_t	count;

	ostack = stilt_ostack_get(a_stilt);
	count = stils_count(ostack);
	if (count > 0)
		stils_npop(ostack, a_stilt, count);
}

void
systemdict_cleardictstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*dstack;
	cw_uint32_t	count;

	dstack = stilt_dstack_get(a_stilt);
	count = stils_count(dstack);
	if (count > 3)
		stils_npop(dstack, a_stilt, count - 3);
}

void
systemdict_cleartomark(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_uint32_t	i, depth;

	ostack = stilt_ostack_get(a_stilt);

	for (i = 0, depth = stils_count(ostack), stilo = NULL; i < depth; i++) {
		stilo = stils_down_get(ostack, a_stilt, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth)
		stilt_error(a_stilt, STILTE_UNMATCHEDMARK);

	stils_npop(ostack, a_stilt, i + 1);
}

void
systemdict_closefile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);

	stilo = stils_get(ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_FILE)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_file_close(stilo, a_stilt);

	stils_pop(ostack, a_stilt);
}

void
systemdict_condition(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_copy(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_get(ostack, a_stilt);

	switch (stilo_type_get(stilo)) {
	case STILOT_INTEGER: {
		cw_stilo_t	*dup;
		cw_uint32_t	i;
		cw_sint64_t	count;

		/* Dup a range of the stack. */
		count = stilo_integer_get(stilo);
		if (count < 0)
			stilt_error(a_stilt, STILTE_RANGECHECK);
		if (count > stils_count(ostack) - 1)
			stilt_error(a_stilt, STILTE_STACKUNDERFLOW);
		stils_pop(ostack, a_stilt);

		/*
		 * Iterate down the stack, creating dup's along the way.  Since
		 * we're going down, it's necessary to use stils_under_push() to
		 * preserve order.
		 */
		for (i = 0, stilo = NULL, dup = NULL; i < count; i++) {
			stilo = stils_down_get(ostack, a_stilt, stilo);
			dup = stils_under_push(ostack, a_stilt, dup);
			stilo_dup(dup, stilo);
		}
		break;
	}
	case STILOT_ARRAY:
	case STILOT_DICT:
	case STILOT_HOOK:
	case STILOT_STRING: {
		cw_stilo_t	*orig;

		orig = stils_down_get(ostack, a_stilt, stilo);

		stilo_copy(stilo, orig, a_stilt);
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
	}
}

void
systemdict_count(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);

	stilo = stils_push(ostack, a_stilt);
	stilo_integer_new(stilo, stils_count(ostack) - 1);
}

void
systemdict_countdictstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*stilo;

	dstack = stilt_dstack_get(a_stilt);
	ostack = stilt_ostack_get(a_stilt);

	stilo = stils_push(ostack, a_stilt);
	stilo_integer_new(stilo, stils_count(dstack));
}

void
systemdict_countexecstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*stilo;

	estack = stilt_estack_get(a_stilt);
	ostack = stilt_ostack_get(a_stilt);

	stilo = stils_push(ostack, a_stilt);
	stilo_integer_new(stilo, stils_count(estack));
}

void
systemdict_counttomark(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_uint32_t	i, depth;

	ostack = stilt_ostack_get(a_stilt);

	for (i = 0, depth = stils_count(ostack), stilo = NULL; i < depth; i++) {
		stilo = stils_down_get(ostack, a_stilt, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth)
		stilt_error(a_stilt, STILTE_UNMATCHEDMARK);

	stilo = stils_push(ostack, a_stilt);
	stilo_integer_new(stilo, i);
}

void
systemdict_currentcontext(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_currentdict(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	dstack = stilt_dstack_get(a_stilt);

	stilo = stils_push(ostack, a_stilt);
	stilo_dup(stilo, stils_get(dstack, a_stilt));
}

void
systemdict_currentfile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*file, *stilo;
	cw_uint32_t	i, depth;

	estack = stilt_estack_get(a_stilt);
	ostack = stilt_ostack_get(a_stilt);

	file = stils_push(ostack, a_stilt);
	for (i = 0, depth = stils_count(estack), stilo = NULL; i < depth; i++) {
		stilo = stils_down_get(estack, a_stilt, stilo);
		if (stilo_type_get(stilo) == STILOT_FILE) {
			stilo_dup(file, stilo);
			break;
		}
	}
	if (i == depth)
		stilo_file_new(file, a_stilt);
}

void
systemdict_currentglobal(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_push(ostack, a_stilt);
	stilo_boolean_new(stilo, stilt_currentglobal(a_stilt));
}

void
systemdict_cvlit(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_get(ostack, a_stilt);
	stilo_attrs_set(stilo, STILOA_LITERAL);
}

void
systemdict_cvm(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_cvn(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_cvrs(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_cvs(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_cvx(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_get(ostack, a_stilt);
	stilo_attrs_set(stilo, STILOA_EXECUTABLE);
}

void
systemdict_def(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack;
	cw_stilo_t	*dict, *key, *val;

	ostack = stilt_ostack_get(a_stilt);
	dstack = stilt_dstack_get(a_stilt);

	dict = stils_get(dstack, a_stilt);
	val = stils_get(ostack, a_stilt);
	key = stils_down_get(ostack, a_stilt, val);

	stilo_dict_def(dict, a_stilt, key, val);

	stils_npop(ostack, a_stilt, 2);
}

void
systemdict_defineresource(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_deletefile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*string;
	cw_uint8_t	str[PATH_MAX];
	cw_uint32_t	nbytes;

	ostack = stilt_ostack_get(a_stilt);
	string = stils_get(ostack, a_stilt);

	if (stilo_type_get(string) != STILOT_STRING)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	if (stilo_string_len_get(string) < sizeof(str))
		nbytes = stilo_string_len_get(string);
	else
		nbytes = sizeof(str) - 1;
	memcpy(str, stilo_string_get(string), nbytes);
	str[nbytes] = '\0';

	stils_pop(ostack, a_stilt);

	if (unlink(str) == -1) {
		switch (errno) {
		case EACCES:
		case EPERM:
			stilt_error(a_stilt, STILTE_INVALIDFILEACCESS);
		case EIO:
		case EBUSY:
			stilt_error(a_stilt, STILTE_IOERROR);
		default:
			stilt_error(a_stilt, STILTE_UNDEFINEDFILENAME);
		}
	}
}

void
systemdict_detach(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_dict(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*dict;

	ostack = stilt_ostack_get(a_stilt);

	dict = stils_push(ostack, a_stilt);
	stilo_dict_new(dict, a_stilt, _CW_SYSTEMDICT_DICT_SIZE);
}

void
systemdict_dictstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *dstack, *tstack;
	cw_stilo_t	*array, *subarray, *stilo;
	cw_sint32_t	i, count;

	ostack = stilt_ostack_get(a_stilt);
	dstack = stilt_dstack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	array = stils_get(ostack, a_stilt);
	if (stilo_type_get(array) != STILOT_ARRAY)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	count = stils_count(dstack);
	if (count > stilo_array_len_get(array))
		stilt_error(a_stilt, STILTE_RANGECHECK);

	/* Move array to tstack, create subarray, and pop array. */
	stilo = stils_push(tstack, a_stilt);
	stilo_dup(stilo, array);
	subarray = array;
	array = stilo;
	stilo_array_subarray_new(subarray, array, a_stilt, 0, count);
	stils_pop(tstack, a_stilt);

	/* Copy dstack to subarray. */
	for (i = count - 1, stilo = NULL; i >= 0; i--) {
		stilo = stils_down_get(dstack, a_stilt, stilo);
		stilo_dup(stilo_array_el_get(subarray, a_stilt, i), stilo);
	}
}

void
systemdict_div(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	t_stilo, *a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	b = stils_get(ostack, a_stilt);
	a = stils_down_get(ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_integer_new(&t_stilo, stilo_integer_get(a));
	stilo_integer_div(&t_stilo, b, a);
	stils_pop(ostack, a_stilt);
}

void
systemdict_dup(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*orig, *dup;

	ostack = stilt_ostack_get(a_stilt);

	orig = stils_get(ostack, a_stilt);
	dup = stils_push(ostack, a_stilt);
	stilo_dup(dup, orig);
}

void
systemdict_end(cw_stilt_t *a_stilt)
{
	cw_stils_t	*dstack;

	dstack = stilt_dstack_get(a_stilt);

	/* threaddict, systemdict, globaldict, and userdict cannot be popped. */
	if (stils_count(dstack) <= 4)
		stilt_error(a_stilt, STILTE_DICTSTACKUNDERFLOW);

	stils_pop(dstack, a_stilt);
}

void
systemdict_eq(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_exch(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;

	ostack = stilt_ostack_get(a_stilt);

	stils_roll(ostack, a_stilt, 2, 1);
}

void
systemdict_exec(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*orig, *new;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);

	orig = stils_get(ostack, a_stilt);
	new = stils_push(estack, a_stilt);
	stilo_dup(new, orig);
	stils_pop(ostack, a_stilt);

	stilt_loop(a_stilt);
}

void
systemdict_execstack(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*array, *subarray, *stilo;
	cw_sint32_t	i, count;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	array = stils_get(ostack, a_stilt);
	if (stilo_type_get(array) != STILOT_ARRAY)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	count = stils_count(estack);
	if (count > stilo_array_len_get(array))
		stilt_error(a_stilt, STILTE_RANGECHECK);

	/* Move array to tstack, create subarray, and pop array. */
	stilo = stils_push(tstack, a_stilt);
	stilo_dup(stilo, array);
	subarray = array;
	array = stilo;
	stilo_array_subarray_new(subarray, array, a_stilt, 0, count);
	stils_pop(tstack, a_stilt);

	/* Copy estack to subarray. */
	for (i = count - 1, stilo = NULL; i >= 0; i--) {
		stilo = stils_down_get(estack, a_stilt, stilo);
		stilo_dup(stilo_array_el_get(subarray, a_stilt, i), stilo);
	}
}

void
systemdict_executeonly(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_executive(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*estilo, *tstilo;
	cw_uint32_t	edepth;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	/* Create a procedure that we can execute again and again. */
	soft_code("{//stdin 128 //string //readstring //cvx //exec}");
	tstilo = stils_push(tstack, a_stilt);
	stilo_dup(tstilo, stils_get(ostack, a_stilt));
	stils_pop(ostack, a_stilt);

	/*
	 * Cache the depth of the execution stack so that we can clean up later.
	 */
	edepth = stils_count(estack);

	xep_begin();
	xep_try {
		estilo = stils_push(estack, a_stilt);
		stilo_dup(estilo, tstilo);

		stilt_loop(a_stilt);
	}
	xep_catch(_CW_STILX_QUIT) {
		/*
		 * Pop objects off the exec stack, up to and including estilo.
		 */
		stils_npop(estack, a_stilt, stils_count(estack) - edepth);

		xep_handled();
	}
	/* XXX Set up exception handling. */
	xep_end();

	stils_pop(tstack, a_stilt);
}

void
systemdict_exit(cw_stilt_t *a_stilt)
{
	xep_throw(_CW_STILX_EXIT);
}

void
systemdict_exp(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	t_stilo, *a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	b = stils_get(ostack, a_stilt);
	a = stils_down_get(ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_integer_new(&t_stilo, stilo_integer_get(a));
	stilo_integer_exp(&t_stilo, b, a);
	stils_pop(ostack, a_stilt);
}

void
systemdict_false(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_push(ostack, a_stilt);
	stilo_boolean_new(stilo, FALSE);
}

void
systemdict_file(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *tstack;
	cw_stilo_t	*name, *flags, *file;

	ostack = stilt_ostack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);
	
	flags = stils_get(ostack, a_stilt);
	name = stils_down_get(ostack, a_stilt, flags);
	if (stilo_type_get(name) != STILOT_STRING || stilo_type_get(flags) !=
	    STILOT_STRING)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	file = stils_push(tstack, a_stilt);
	stilo_file_new(file, a_stilt);
	stilo_file_open(file, a_stilt, stilo_string_get(name),
	    stilo_string_len_get(name), stilo_string_get(flags),
	    stilo_string_len_get(flags));

	/* XXX Magic number. */
	stilo_file_buffer_size_set(file, 512);

	stils_pop(ostack, a_stilt);
	stilo_dup(name, file);
	stils_pop(tstack, a_stilt);
}

void
systemdict_filenameforall(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_fileposition(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file;
	cw_sint64_t	position;

	ostack = stilt_ostack_get(a_stilt);

	file = stils_get(ostack, a_stilt);
	
	if (stilo_type_get(file) != STILOT_FILE)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	position = stilo_file_position_get(file, a_stilt);
	stilo_integer_new(file, position);
}

void
systemdict_filter(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_findresource(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_flush(cw_stilt_t *a_stilt)
{
	stilo_file_buffer_flush(stilt_stdout_get(a_stilt), a_stilt);
}

void
systemdict_flushfile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file;

	ostack = stilt_ostack_get(a_stilt);

	file = stils_get(ostack, a_stilt);
	
	if (stilo_type_get(file) != STILOT_FILE)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_file_buffer_flush(file, a_stilt);
	
	stils_pop(ostack, a_stilt);
}

void
systemdict_for(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*exec, *ostilo, *estilo, *tstilo;
	cw_sint64_t	i, inc, limit, edepth;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	exec = stils_get(ostack, a_stilt);

	ostilo = stils_down_get(ostack, a_stilt, exec);
	if (stilo_type_get(ostilo) != STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	limit = stilo_integer_get(ostilo);

	ostilo = stils_down_get(ostack, a_stilt, ostilo);
	if (stilo_type_get(ostilo) != STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	inc = stilo_integer_get(ostilo);

	ostilo = stils_down_get(ostack, a_stilt, ostilo);
	if (stilo_type_get(ostilo) != STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	i = stilo_integer_get(ostilo);

	/* Move the object to be executed to tstack. */
	tstilo = stils_push(tstack, a_stilt);
	stilo_dup(tstilo, exec);
	stils_npop(ostack, a_stilt, 4);

	/* Record estack's depth so that we can clean up estack if necessary. */
	edepth = stils_count(estack);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		if (limit >= 0) {
			for (; i <= limit; i += inc) {
				/*
				 * Dup the object to execute onto the execution
				 * stack.
				 */
				estilo = stils_push(estack, a_stilt);
				stilo_dup(estilo, tstilo);

				/*
				 * Push the control variable onto the data
				 * stack.
				 */
				ostilo = stils_push(ostack, a_stilt);
				stilo_integer_new(ostilo, i);

				stilt_loop(a_stilt);
			}
		} else {
			for (; i >= limit; i += inc) {
				/*
				 * Dup the object to execute onto the execution
				 * stack.
				 */
				estilo = stils_push(estack, a_stilt);
				stilo_dup(estilo, tstilo);

				/*
				 * Push the control variable onto the data
				 * stack.
				 */
				ostilo = stils_push(ostack, a_stilt);
				stilo_integer_new(ostilo, i);

				stilt_loop(a_stilt);
			}
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		for (i = stils_count(estack); i > edepth; i--)
			stils_pop(estack, a_stilt);
	}
	xep_end();

	stils_pop(tstack, a_stilt);
}

void
systemdict_forall(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*stilo, *what, *proc;
	cw_sint64_t	i, count;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	proc = stils_get(ostack, a_stilt);
	what = stils_down_get(ostack, a_stilt, proc);

	xep_begin();
	xep_try {
		switch (stilo_type_get(what)) {
		case STILOT_ARRAY: {
			cw_stilo_t	*el;

			/* Move proc and array to tstack. */
			stilo = stils_push(tstack, a_stilt);
			stilo_dup(stilo, proc);
			proc = stilo;

			stilo = stils_push(tstack, a_stilt);
			stilo_dup(stilo, what);
			what = stilo;

			stils_npop(ostack, a_stilt, 2);

			/*
			 * Iterate through the array, push each element onto
			 * ostack, and execute proc.
			 */
			for (i = 0, count = stilo_array_len_get(what); i <
				 count; i++) {
				el = stilo_array_el_get(what, a_stilt, i);
				stilo = stils_push(ostack, a_stilt);
				stilo_dup(stilo, el);

				stilo = stils_push(estack, a_stilt);
				stilo_dup(stilo, proc);
				stilt_loop(a_stilt);
			}
			break;
		}
		case STILOT_DICT: {
			cw_stilo_t	*key, *val;

			/* Move proc and array to tstack. */
			stilo = stils_push(tstack, a_stilt);
			stilo_dup(stilo, proc);
			proc = stilo;

			stilo = stils_push(tstack, a_stilt);
			stilo_dup(stilo, what);
			what = stilo;

			stils_npop(ostack, a_stilt, 2);

			for (i = 0, count = stilo_dict_count(what); i <
				 count; i++) {
				/* Push key and val onto ostack. */
				key = stils_push(ostack, a_stilt);
				val = stils_push(ostack, a_stilt);

				/* Get next key. */
				stilo_dict_iterate(what, a_stilt, key);

				/* Use key to get val. */
				stilo_dict_lookup(what, a_stilt, key, val);

				/* Push proc onto estack and execute it. */
				stilo = stils_push(estack, a_stilt);
				stilo_dup(stilo, proc);
				stilt_loop(a_stilt);
			}
			break;
		}
		case STILOT_STRING: {
			cw_uint8_t	el;

			/* Move proc and array to tstack. */
			stilo = stils_push(tstack, a_stilt);
			stilo_dup(stilo, proc);
			proc = stilo;

			stilo = stils_push(tstack, a_stilt);
			stilo_dup(stilo, what);
			what = stilo;

			stils_npop(ostack, a_stilt, 2);

			/*
			 * Iterate through the string, push each element onto
			 * ostack, and execute proc.
			 */
			for (i = 0, count = stilo_array_len_get(what); i <
				 count; i++) {
				el = *stilo_string_el_get(what, a_stilt, i);
				stilo = stils_push(ostack, a_stilt);
				stilo_integer_new(stilo, (cw_sint64_t)el);

				stilo = stils_push(estack, a_stilt);
				stilo_dup(stilo, proc);
				stilt_loop(a_stilt);
			}
			break;
		}

		default:
			stilt_error(a_stilt, STILTE_TYPECHECK);
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up tstack. */
		stils_npop(tstack, a_stilt, 2);
	}
	xep_end();
}

void
systemdict_fork(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_gcheck(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_ge(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_get(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*from, *with;

	ostack = stilt_ostack_get(a_stilt);
	with = stils_get(ostack, a_stilt);
	from = stils_down_get(ostack, a_stilt, with);

	switch (stilo_type_get(from)) {
	case STILOT_ARRAY: {
		cw_sint64_t	index;

		if (stilo_type_get(with) != STILOT_INTEGER)
			stilt_error(a_stilt, STILTE_TYPECHECK);
		index = stilo_integer_get(with);

		stilo_dup(with, stilo_array_el_get(from, a_stilt, index));

		stils_roll(ostack, a_stilt, 2, 1);
		stils_pop(ostack, a_stilt);
		break;
	}
	case STILOT_DICT: {
		cw_stilo_t	*val;

		val = stils_push(ostack, a_stilt);
		if (stilo_dict_lookup(from, a_stilt, with, val)) {
			stils_pop(ostack, a_stilt);
			stilt_error(a_stilt, STILTE_UNDEFINED);
		}
		stils_roll(ostack, a_stilt, 3, 1);
		stils_npop(ostack, a_stilt, 2);
		break;
	}
	case STILOT_HOOK:
		_cw_error("XXX Not implemented");
		break;
	case STILOT_STRING: {
		cw_sint64_t	index;

		if (stilo_type_get(with) != STILOT_INTEGER)
			stilt_error(a_stilt, STILTE_TYPECHECK);
		index = stilo_integer_get(with);

		stilo_integer_set(with, (cw_sint64_t)*stilo_string_el_get(from,
		    a_stilt, index));

		stils_roll(ostack, a_stilt, 2, 1);
		stils_pop(ostack, a_stilt);
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
	}
}

void
systemdict_getinterval(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*from, *with, *count;
	cw_sint64_t	index, len;

	ostack = stilt_ostack_get(a_stilt);
	count = stils_get(ostack, a_stilt);
	with = stils_down_get(ostack, a_stilt, count);
	from = stils_down_get(ostack, a_stilt, with);

	if ((stilo_type_get(with) != STILOT_INTEGER) || (stilo_type_get(count)
	    != STILOT_INTEGER))
		stilt_error(a_stilt, STILTE_TYPECHECK);
	index = stilo_integer_get(with);
	len = stilo_integer_get(count);

	switch (stilo_type_get(from)) {
	case STILOT_ARRAY:
		stilo_array_subarray_new(count, from, a_stilt, index, len);
		break;
	case STILOT_HOOK:
		_cw_error("XXX Not implemented");
		break;
	case STILOT_STRING:
		stilo_string_substring_new(count, from, a_stilt, index, len);
		break;
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
	}
	stils_roll(ostack, a_stilt, 3, 1);
	stils_npop(ostack, a_stilt, 2);
}

void
systemdict_gt(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_handleerror(cw_stilt_t *a_stilt)
{
	cw_stils_t	*estack, *tstack;
	cw_stilo_t	*key, *errordict, *handleerror;

	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	/* Get errordict. */
	errordict = stils_push(tstack, a_stilt);
	key = stils_push(tstack, a_stilt);
	stilo_name_new(key, a_stilt, stiln_str(STILN_errordict),
	    stiln_len(STILN_errordict), TRUE);
	if (stilt_dict_stack_search(a_stilt, key, errordict)) {
		stils_npop(tstack, a_stilt, 2);
		xep_throw(_CW_STILX_ERRORDICT);
	}

	/* Get handleerror from errordict and push it onto estack. */
	handleerror = stils_push(estack, a_stilt);
	stilo_name_new(key, a_stilt, stiln_str(STILN_handleerror),
	    stiln_len(STILN_handleerror), TRUE);
	if (stilo_dict_lookup(errordict, a_stilt, key, handleerror)) {
		stils_pop(estack, a_stilt);
		stils_npop(tstack, a_stilt, 2);
		xep_throw(_CW_STILX_ERRORDICT);
	}
	stils_npop(tstack, a_stilt, 2);

	/* Execute handleerror. */
	stilt_loop(a_stilt);
}

void
systemdict_if(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*cond, *exec;

	ostack = stilt_ostack_get(a_stilt);
	exec = stils_get(ostack, a_stilt);
	cond = stils_down_get(ostack, a_stilt, exec);
	if (stilo_type_get(cond) != STILOT_BOOLEAN)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	if (stilo_boolean_get(cond)) {
		cw_stils_t	*estack;
		cw_stilo_t	*stilo;

		estack = stilt_estack_get(a_stilt);
		stilo = stils_push(estack, a_stilt);
		stilo_move(stilo, exec);
		stils_npop(ostack, a_stilt, 2);
		stilt_loop(a_stilt);
	} else
		stils_npop(ostack, a_stilt, 2);
}

void
systemdict_ifelse(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*cond, *exec_if, *exec_else, *stilo;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);

	exec_else = stils_get(ostack, a_stilt);

	exec_if = stils_down_get(ostack, a_stilt, exec_else);

	cond = stils_down_get(ostack, a_stilt, exec_if);
	if (stilo_type_get(cond) != STILOT_BOOLEAN)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo = stils_push(estack, a_stilt);
	if (stilo_boolean_get(cond))
		stilo_move(stilo, exec_if);
	else
		stilo_move(stilo, exec_else);
	stils_npop(ostack, a_stilt, 3);
	stilt_loop(a_stilt);
}

void
systemdict_index(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo, *orig;
	cw_sint64_t	index;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_get(ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	index = stilo_integer_get(stilo);
	if (index < 0)
		stilt_error(a_stilt, STILTE_RANGECHECK);

	orig = stils_nget(ostack, a_stilt, index + 1);
	stilo_no_new(stilo);
	stilo_dup(stilo, orig);
}

void
systemdict_join(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_known(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_le(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_length(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_sint64_t	len;

	ostack = stilt_ostack_get(a_stilt);
	
	stilo = stils_get(ostack, a_stilt);
	switch (stilo_type_get(stilo)) {
	case STILOT_ARRAY:
		len = stilo_array_len_get(stilo);
		break;
	case STILOT_DICT:
		len = stilo_dict_count(stilo);
		break;
	case STILOT_NAME:
		len = stilo_name_len_get(stilo);
		break;
	case STILOT_STRING:
		len = stilo_string_len_get(stilo);
		break;
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
	}

	stilo_integer_new(stilo, len);
}

void
systemdict_load(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_lock(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_loop(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack, *tstack;
	cw_stilo_t	*exec, *stilo, *tstilo;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	tstack = stilt_tstack_get(a_stilt);

	exec = stils_get(ostack, a_stilt);

	/* Move the object to be executed to tstack. */
	tstilo = stils_push(tstack, a_stilt);
	stilo_dup(tstilo, exec);
	stils_pop(ostack, a_stilt);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		for (;;) {
			stilo = stils_push(estack, a_stilt);
			stilo_dup(stilo, tstilo);
			stilt_loop(a_stilt);
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		while (stils_get(estack, a_stilt) != stilo)
			stils_pop(estack, a_stilt);
	}
	xep_end();

	stils_pop(estack, a_stilt);
	stils_pop(tstack, a_stilt);
}

void
systemdict_lt(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_mark(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_push(ostack, a_stilt);
	stilo_mark_new(stilo);
}

void
systemdict_mod(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	t_stilo, *a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	b = stils_get(ostack, a_stilt);
	a = stils_down_get(ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_integer_new(&t_stilo, stilo_integer_get(a));
	stilo_integer_mod(&t_stilo, b, a);
	stils_pop(ostack, a_stilt);
}

void
systemdict_mul(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	t_stilo, *a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	b = stils_get(ostack, a_stilt);
	a = stils_down_get(ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_integer_new(&t_stilo, stilo_integer_get(a));
	stilo_integer_mul(&t_stilo, b, a);
	stils_pop(ostack, a_stilt);
}

void
systemdict_mutex(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_ne(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_neg(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*a;

	ostack = stilt_ostack_get(a_stilt);
	
	a = stils_get(ostack, a_stilt);
	if (stilo_type_get(a) != STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_integer_neg(a, a);
}

void
systemdict_noaccess(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_not(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_notify(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_null(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_push(ostack, a_stilt);
	stilo_null_new(stilo);
}

void
systemdict_or(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_pop(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;

	ostack = stilt_ostack_get(a_stilt);

	stils_pop(ostack, a_stilt);
}

void
systemdict_print(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo, *stdout_stilo;

	ostack = stilt_ostack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	stilo = stils_get(ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_STRING)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_file_write(stdout_stilo, a_stilt, stilo_string_get(stilo),
	    stilo_string_len_get(stilo));
	stils_pop(ostack, a_stilt);
}

void
systemdict_product(cw_stilt_t *a_stilt)
{
	soft_code("(Canonware stil)");
}

void
systemdict_prompt(cw_stilt_t *a_stilt)
{
	soft_code("promptstring print flush");
}

void
systemdict_promptstring(cw_stilt_t *a_stilt)
{
	soft_code("(s> )");
}

void
systemdict_pstack(cw_stilt_t *a_stilt)
{
	cw_stilts_t	stilts;
	cw_stils_t	*ostack;
	cw_uint32_t	i, count;
	cw_uint8_t	code[] = "==";

	stilts_new(&stilts, a_stilt);
	ostack = stilt_ostack_get(a_stilt);
	count = stils_count(ostack);

	for (i = 0; i < count; i++) {
		systemdict_dup(a_stilt);
		stilt_interpret(a_stilt, &stilts, code, sizeof(code) - 1);
		stilt_flush(a_stilt, &stilts);
		stils_roll(ostack, a_stilt, count, 1);
	}
	stilts_delete(&stilts, a_stilt);
}

void
systemdict_put(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*into, *with, *what;

	ostack = stilt_ostack_get(a_stilt);
	what = stils_get(ostack, a_stilt);
	with = stils_down_get(ostack, a_stilt, what);
	into = stils_down_get(ostack, a_stilt, with);

	switch (stilo_type_get(into)) {
	case STILOT_ARRAY: {
		cw_sint64_t	index;

		if (stilo_type_get(with) != STILOT_INTEGER)
			stilt_error(a_stilt, STILTE_TYPECHECK);
		index = stilo_integer_get(with);

		stilo_array_el_set(into, a_stilt, what, index);
		break;
	}
	case STILOT_DICT: {
		stilo_dict_def(into, a_stilt, with, what);
		break;
	}
	case STILOT_HOOK:
		_cw_error("XXX Not implemented");
		break;
	case STILOT_STRING: {
		cw_sint64_t	index;
		cw_uint8_t	val;

		if ((stilo_type_get(with) != STILOT_INTEGER) ||
		    stilo_type_get(what) != STILOT_INTEGER)
			stilt_error(a_stilt, STILTE_TYPECHECK);
		index = stilo_integer_get(with);
		val = stilo_integer_get(what);
		
		stilo_string_el_set(into, a_stilt, val, index);
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
	}
	stils_npop(ostack, a_stilt, 3);
}

void
systemdict_putinterval(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*into, *with, *what;
	cw_sint64_t	index;

	ostack = stilt_ostack_get(a_stilt);
	what = stils_get(ostack, a_stilt);
	with = stils_down_get(ostack, a_stilt, what);
	into = stils_down_get(ostack, a_stilt, with);

	if (stilo_type_get(with) != STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	index = stilo_integer_get(with);

	switch (stilo_type_get(into)) {
	case STILOT_ARRAY: {
		cw_stilo_t	*arr;
		cw_uint32_t	len;

		arr = stilo_array_get(what);
		len = stilo_array_len_get(what);
		stilo_array_set(into, a_stilt, index, arr, len);
		break;
	}
	case STILOT_HOOK:
		_cw_error("XXX Not implemented");
		break;
	case STILOT_STRING: {
		cw_uint8_t	*str;
		cw_uint32_t	len;

		str = stilo_string_get(what);
		len = stilo_string_len_get(what);
		stilo_string_set(into, a_stilt, index, str, len);
		break;
	}
	default:
		stilt_error(a_stilt, STILTE_TYPECHECK);
	}
	stils_npop(ostack, a_stilt, 3);
}

void
systemdict_quit(cw_stilt_t *a_stilt)
{
	xep_throw(_CW_STILX_QUIT);
}

void
systemdict_rand(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*num;

	ostack = stilt_ostack_get(a_stilt);

	num = stils_push(ostack, a_stilt);
	stilo_cast(num, a_stilt, STILOT_INTEGER);
	stilo_integer_rand(num);
}

void
systemdict_rcheck(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_read(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *value, *code;
	cw_uint8_t	val;
	cw_sint32_t	nread;

	ostack = stilt_ostack_get(a_stilt);

	file = stils_get(ostack, a_stilt);
	if (stilo_type_get(file) != STILOT_FILE)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	value = stils_push(ostack, a_stilt);
	code = stils_push(ostack, a_stilt);

	nread = stilo_file_read(file, a_stilt, 1, &val);

	if (nread == 0) {
		stilo_integer_new(value, 0);
		stilo_boolean_new(code, FALSE);
	} else {
		stilo_integer_new(value, (cw_sint64_t)val);
		stilo_boolean_new(code, TRUE);
	}

	stils_roll(ostack, a_stilt, 3, 2);
	stils_pop(ostack, a_stilt);
}

void
systemdict_readonly(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_readstring(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *string;
	cw_sint32_t	nread;

	ostack = stilt_ostack_get(a_stilt);

	string = stils_get(ostack, a_stilt);
	file = stils_down_get(ostack, a_stilt, string);

	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(string) !=
	    STILOT_STRING)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	nread = stilo_file_read(file, a_stilt, stilo_string_len_get(string),
	    stilo_string_get(string));

	if (nread == 0) {
		/* EOF. */
		stilo_boolean_new(file, TRUE);
		stils_pop(ostack, a_stilt);
	} else if (nread < stilo_string_len_get(string)) {
		cw_stilo_t	*value, *code;

		/*
		 * We didn't fill the string, so we can't just use it as the
		 * result.  Create a copy.
		 */
		value = stils_under_push(ostack, a_stilt, file);
		stilo_string_substring_new(value, string, a_stilt, 0, nread);

		code = stils_under_push(ostack, a_stilt, file);
		stilo_boolean_new(code, FALSE);

		stils_npop(ostack, a_stilt, 2);
	} else {
		stilo_boolean_new(file, FALSE);
		stils_roll(ostack, a_stilt, 2, 1);
		stils_pop(ostack, a_stilt);
	}
}

void
systemdict_realtime(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_renamefile(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*string_from, *string_to;
	cw_uint8_t	str_from[PATH_MAX], str_to[1024];
	cw_uint32_t	nbytes;

	ostack = stilt_ostack_get(a_stilt);
	string_to = stils_get(ostack, a_stilt);
	string_from = stils_down_get(ostack, a_stilt, string_to);

	if (stilo_type_get(string_from) != STILOT_STRING ||
	    stilo_type_get(string_to) != STILOT_STRING)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	if (stilo_string_len_get(string_from) >= sizeof(str_from))
		stilt_error(a_stilt, STILTE_LIMITCHECK);
	nbytes = stilo_string_len_get(string_from);
	memcpy(str_from, stilo_string_get(string_from), nbytes);
	str_from[nbytes] = '\0';

	if (stilo_string_len_get(string_to) >= sizeof(str_to))
		stilt_error(a_stilt, STILTE_LIMITCHECK);
	nbytes = stilo_string_len_get(string_to);
	memcpy(str_to, stilo_string_get(string_to), nbytes);
	str_to[nbytes] = '\0';

	stils_npop(ostack, a_stilt, 2);

	if (rename(str_from, str_to) == -1) {
		switch (errno) {
		case EACCES:
		case EPERM:
		case EROFS:
		case EINVAL:
			stilt_error(a_stilt, STILTE_INVALIDFILEACCESS);
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			stilt_error(a_stilt, STILTE_UNDEFINEDFILENAME);
		default:
			stilt_error(a_stilt, STILTE_IOERROR);
		}
	}
}

void
systemdict_repeat(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*count, *exec, *stilo, *tstilo;
	cw_sint64_t	i, cnt;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);

	exec = stils_get(ostack, a_stilt);
	count = stils_down_get(ostack, a_stilt, exec);
	if (stilo_type_get(count) != STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	/*
	 * Use the execution stack for temporary storage of the stilo to be
	 * executed, in order to be GC-safe.
	 */
	tstilo = stils_push(estack, a_stilt);
	stilo_dup(tstilo, exec);

	cnt = stilo_integer_get(count);
	stils_npop(ostack, a_stilt, 2);

	/*
	 * Catch an exit exception, if thrown, but do not continue executing the
	 * loop.
	 */
	xep_begin();
	xep_try {
		for (i = 0; i < cnt; i++) {
			stilo = stils_push(estack, a_stilt);
			stilo_dup(stilo, tstilo);
			stilt_loop(a_stilt);
		}
	}
	xep_catch(_CW_STILX_EXIT) {
		xep_handled();

		/* Clean up whatever mess was left on the execution stack. */
		while (stils_get(estack, a_stilt) != tstilo)
			stils_pop(estack, a_stilt);
	}
	xep_end();

	stils_pop(estack, a_stilt);
}

void
systemdict_resourceforall(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_resourcestatus(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_roll(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_sint64_t	count, amount;

	ostack = stilt_ostack_get(a_stilt);

	stilo = stils_get(ostack, a_stilt);
	amount = stilo_integer_get(stilo);
	stils_pop(ostack, a_stilt);

	stilo = stils_get(ostack, a_stilt);
	count = stilo_integer_get(stilo);
	stils_pop(ostack, a_stilt);

	stils_roll(ostack, a_stilt, count, amount);
}

void
systemdict_run(cw_stilt_t *a_stilt)
{
	soft_code("(r) file cvx exec");
}

void
systemdict_search(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_setfileposition(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *position;

	ostack = stilt_ostack_get(a_stilt);

	position = stils_get(ostack, a_stilt);
	file = stils_down_get(ostack, a_stilt, position);
	
	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(position) !=
	    STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_file_position_set(file, a_stilt, stilo_integer_get(position));

	stils_npop(ostack, a_stilt, 2);
}

void
systemdict_setglobal(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_get(ostack, a_stilt);
	stilt_setglobal(a_stilt, stilo_boolean_get(stilo));
	stils_pop(ostack, a_stilt);
}

void
systemdict_shift(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_srand(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*seed;

	ostack = stilt_ostack_get(a_stilt);

	seed = stils_get(ostack, a_stilt);
	stilo_integer_srand(seed);
	stils_pop(ostack, a_stilt);
}

void
systemdict_stack(cw_stilt_t *a_stilt)
{
	cw_stilts_t	stilts;
	cw_stils_t	*ostack;
	cw_uint32_t	i, count;
	cw_uint8_t	code[] = "=";

	stilts_new(&stilts, a_stilt);
	ostack = stilt_ostack_get(a_stilt);
	count = stils_count(ostack);

	for (i = 0; i < count; i++) {
		systemdict_dup(a_stilt);
		stilt_interpret(a_stilt, &stilts, code, sizeof(code) - 1);
		stilt_flush(a_stilt, &stilts);
		stils_roll(ostack, a_stilt, count, 1);
	}
	stilts_delete(&stilts, a_stilt);
}

void
systemdict_start(cw_stilt_t *a_stilt)
{
	cw_stils_t	*estack;
	cw_stilo_t	*file;

	estack = stilt_estack_get(a_stilt);

	file = stils_push(estack, a_stilt);
	stilo_dup(file, stilt_stdin_get(a_stilt));
	stilo_attrs_set(file, STILOA_EXECUTABLE);

	xep_begin();
	xep_try {
		stilt_loop(a_stilt);
	}
	xep_catch(_CW_STILX_QUIT) {
		cw_stilo_t	*stilo;
		cw_uint32_t	i, depth;

		/*
		 * Pop objects off the exec stack, up to and including file.
		 */
		for (i = 0, depth = stils_count(estack), stilo = NULL; i <
		     depth; i++) {
			stilo = stils_down_get(estack, a_stilt, stilo);
			if (stilo == file)
				break;
		}
		_cw_assert(i < depth);
		stils_npop(estack, a_stilt, i + 1);

		xep_handled();
	}
	xep_catch(_CW_STILX_STOP) {
		xep_handled();

		soft_code("handleerror");
/*  		stilt_loop(a_stilt); */
	}
	/* XXX Set up exception handling. */
	xep_end();
}

void
systemdict_status(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_stdin(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_stdin;

	ostack = stilt_ostack_get(a_stilt);
	stilo_stdin = stils_push(ostack, a_stilt);

	stilo_dup(stilo_stdin, stilt_stdin_get(a_stilt));
}

void
systemdict_stderr(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_stderr;

	ostack = stilt_ostack_get(a_stilt);
	stilo_stderr = stils_push(ostack, a_stilt);

	stilo_dup(stilo_stderr, stilt_stderr_get(a_stilt));
}

void
systemdict_stdout(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo_stdout;

	ostack = stilt_ostack_get(a_stilt);
	stilo_stdout = stils_push(ostack, a_stilt);

	stilo_dup(stilo_stdout, stilt_stdout_get(a_stilt));
}

void
systemdict_stop(cw_stilt_t *a_stilt)
{
	xep_throw(_CW_STILX_STOP);
}

void
systemdict_stopped(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack, *estack;
	cw_stilo_t	*exec, *stilo;
	cw_bool_t	result = FALSE;

	ostack = stilt_ostack_get(a_stilt);
	estack = stilt_estack_get(a_stilt);
	
	exec = stils_get(ostack, a_stilt);
	stilo = stils_push(estack, a_stilt);
	stilo_dup(stilo, exec);
	stils_pop(ostack, a_stilt);

	/*
	 * Point exec to the copy on the execution stack, so that it can be used
	 * as a marker for execution stack cleanup if the stop operator is
	 * called.
	 */
	exec = stilo;

	/* Catch a stop exception, if thrown. */
	xep_begin();
	xep_try {
		stilt_loop(a_stilt);
	}
	xep_catch(_CW_STILX_STOP) {
		xep_handled();
		result = TRUE;

		/* Clean up whatever mess was left on the execution stack. */
		do {
			stilo = stils_get(estack, a_stilt);
			stils_pop(estack, a_stilt);
		} while (stilo != exec);
	}
	xep_end();

	stilo = stils_push(ostack, a_stilt);
	stilo_boolean_new(stilo, result);
}

void
systemdict_store(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_string(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_sint64_t	len;

	ostack = stilt_ostack_get(a_stilt);
	
	stilo = stils_get(ostack, a_stilt);
	if (stilo_type_get(stilo) != STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);
	len = stilo_integer_get(stilo);
	if (len < 0)
		stilt_error(a_stilt, STILTE_RANGECHECK);

	stilo_string_new(stilo, a_stilt, len);
}

void
systemdict_sub(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	t_stilo, *a, *b;

	ostack = stilt_ostack_get(a_stilt);
	
	b = stils_get(ostack, a_stilt);
	a = stils_down_get(ostack, a_stilt, b);
	if (stilo_type_get(a) != STILOT_INTEGER || stilo_type_get(b) !=
	    STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_integer_new(&t_stilo, stilo_integer_get(a));
	stilo_integer_sub(&t_stilo, b, a);
	stils_pop(ostack, a_stilt);
}

/* = */
void
systemdict_sym_eq(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo, *stdout_stilo;

	ostack = stilt_ostack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	stilo = stils_get(ostack, a_stilt);
	stilo_print(stilo, a_stilt, stdout_stilo, FALSE, TRUE);
	stilo_file_buffer_flush(stdout_stilo, a_stilt);
	stils_pop(ostack, a_stilt);
}

/* == */
void
systemdict_sym_eq_eq(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo, *stdout_stilo;

	ostack = stilt_ostack_get(a_stilt);
	stdout_stilo = stilt_stdout_get(a_stilt);

	stilo = stils_get(ostack, a_stilt);
	stilo_print(stilo, a_stilt, stdout_stilo, TRUE, TRUE);
	stilo_file_buffer_flush(stdout_stilo, a_stilt);
	stils_pop(ostack, a_stilt);
}

/* >> */
void
systemdict_sym_gt_gt(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

/* ] */
void
systemdict_sym_rb(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	t_stilo, *stilo, *arr;	/* XXX GC-unsafe.*/
	cw_sint32_t	nelements, i, depth;

	ostack = stilt_ostack_get(a_stilt);
	/* Find the mark. */
	for (i = 0, depth = stils_count(ostack), stilo = NULL; i < depth; i++) {
		stilo = stils_down_get(ostack, a_stilt, stilo);
		if (stilo_type_get(stilo) == STILOT_MARK)
			break;
	}
	if (i == depth)
		stilt_error(a_stilt, STILTE_UNMATCHEDMARK);

	/*
	 * i is the index of the mark, and stilo points to the mark.  Set
	 * nelements accordingly.  When we pop the stilo's off the stack, we'll
	 * have to pop (nelements + 1) stilo's.
	 */
	nelements = i;

	stilo_array_new(&t_stilo, a_stilt, nelements);
	arr = stilo_array_get(&t_stilo);

	/*
	 * Traverse down the stack, moving stilo's to the array.
	 */
	for (i = nelements - 1, stilo = NULL; i >= 0; i--) {
		stilo = stils_down_get(ostack, a_stilt, stilo);
		stilo_move(&arr[i], stilo);
	}

	/* Pop the stilo's off the stack now. */
	stils_npop(ostack, a_stilt, nelements + 1);

	/* Push the array onto the stack. */
	stilo = stils_push(ostack, a_stilt);
	stilo_move(stilo, &t_stilo);
}

void
systemdict_timedwait(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_token(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_true(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_push(ostack, a_stilt);
	stilo_boolean_new(stilo, TRUE);
}

void
systemdict_trylock(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_type(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_undef(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_undefineresource(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_unlock(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_usertime(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_version(cw_stilt_t *a_stilt)
{
	soft_code("(" _LIBSTIL_VERSION ")");
}

void
systemdict_wait(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_wcheck(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_where(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_write(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *value;
	cw_uint8_t	val;

	ostack = stilt_ostack_get(a_stilt);

	value = stils_get(ostack, a_stilt);
	file = stils_down_get(ostack, a_stilt, value);
	
	if (stilo_type_get(file) != STILOT_FILE || stilo_type_get(value) !=
	    STILOT_INTEGER)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	val = (cw_uint8_t)stilo_integer_get(value);
	stilo_file_write(file, a_stilt, &val, 1);

	stils_npop(ostack, a_stilt, 2);
}

void
systemdict_writestring(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*file, *string;

	ostack = stilt_ostack_get(a_stilt);
	string = stils_get(ostack, a_stilt);
	file = stils_down_get(ostack, a_stilt, string);

	if (stilo_type_get(string) != STILOT_STRING || stilo_type_get(file) !=
	    STILOT_FILE)
		stilt_error(a_stilt, STILTE_TYPECHECK);

	stilo_file_write(file, a_stilt, stilo_string_get(string),
	    stilo_string_len_get(string));

	stils_npop(ostack, a_stilt, 2);
}

void
systemdict_xcheck(cw_stilt_t *a_stilt)
{
	cw_stils_t	*ostack;
	cw_stilo_t	*stilo;
	cw_stiloa_t	attr;

	ostack = stilt_ostack_get(a_stilt);
	stilo = stils_get(ostack, a_stilt);
	
	attr = stilo_attrs_get(stilo);
	stilo_clobber(stilo);

	if (attr == STILOA_EXECUTABLE)
		stilo_boolean_new(stilo, TRUE);
	else
		stilo_boolean_new(stilo, FALSE);
}

void
systemdict_xor(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}

void
systemdict_yield(cw_stilt_t *a_stilt)
{
	_cw_error("XXX Not implemented");
}
