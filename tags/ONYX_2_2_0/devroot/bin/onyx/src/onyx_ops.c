/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "onyx.h"

#ifdef HAVE_DLOPEN
#include <dlfcn.h>	/* for modload operator. */
#endif

#ifdef HAVE_DLOPEN
static cw_nxoe_t *
onyx_ops_modload_sym_ref_iter(void *a_data, cw_bool_t a_reset)
{
	return NULL;
}

static cw_bool_t
onyx_ops_modload_sym_delete(void *a_data, cw_nx_t *a_nx, cw_uint32_t a_iter)
{
	cw_bool_t	retval;

	/*
	 * Don't dlclose() until the second GC sweep iteration, so that other
	 * objects that are part of the module can be deleted first.
	 */
	if (a_iter != 1) {
		retval = TRUE;
		goto RETURN;
	}

	dlclose(a_data);

	retval = FALSE;
	RETURN:
	return retval;
}

static void
onyx_ops_modload(cw_nxo_t *a_thread)
{
	cw_nxo_t	*ostack, *estack, *tstack;
	cw_nxo_t	*path, *sym, *nxo;
	cw_uint8_t	*str;
	void		*symbol, *handle = NULL;

	ostack = nxo_thread_ostack_get(a_thread);
	estack = nxo_thread_estack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	NXO_STACK_GET(sym, ostack, a_thread);
	NXO_STACK_DOWN_GET(path, ostack, a_thread, sym);
	if (nxo_type_get(path) != NXOT_STRING) {
		nxo_thread_nerror(a_thread, NXN_typecheck);
		return;
	}

	/* Create '\0'-terminated copy of path. */
	nxo = nxo_stack_push(tstack);
	nxo_string_cstring(nxo, path, a_thread);
	str = nxo_string_get(nxo);

	/* Try to dlopen(). */
/*  	fprintf(stderr, "dlopen(\"%s\")\n", str); */
	handle = dlopen(str, RTLD_LAZY);
	if (handle == NULL) {
#ifdef _CW_DBG
		fprintf(stderr, "dlopen() error: %s\n", dlerror());
#endif
		nxo_stack_pop(tstack);
		nxo_thread_nerror(a_thread, NXN_invalidfileaccess);
		return;
	}

	/* Create '\0'-terminated copy of sym. */
	nxo_string_cstring(nxo, sym, a_thread);
	str = nxo_string_get(nxo);

	/* Look up symbol. */
/*  	fprintf(stderr, "dlsym(\"%s\")\n", str); */
	symbol = dlsym(handle, str);

	/* Pop nxo. */
	nxo_stack_pop(tstack);

	if (symbol == NULL) {
		/* Couldn't find the symbol. */
#ifdef _CW_DBG
		fprintf(stderr, "dlsym() error: %s]\n", dlerror());
#endif
		dlclose(handle);
		nxo_thread_nerror(a_thread, NXN_undefined);
		return;
	}

	/*
	 * Create a hook whose data pointer is the library handle, and whose
	 * evaluation function is the symbol we just looked up.
	 */
	nxo = nxo_stack_push(estack);
	nxo_hook_new(nxo, nxo_thread_nx_get(a_thread), handle, symbol,
	    onyx_ops_modload_sym_ref_iter, onyx_ops_modload_sym_delete);
	nxo_dup(nxo_hook_tag_get(nxo), sym);
	nxo_attr_set(nxo, NXOA_EXECUTABLE);

	/* Pop the arguments before recursing. */
	nxo_stack_npop(ostack, 2);

	/* Recurse on the hook. */
	nxo_thread_loop(a_thread);
}
#endif

void
onyx_ops_init(cw_nxo_t *a_thread)
{
#ifdef HAVE_DLOPEN
	cw_nx_t			*nx;
	cw_nxo_t		*tstack, *name, *value;
	static const cw_uint8_t	onyx_name_modload[] = "modload";

	nx = nxo_thread_nx_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	name = nxo_stack_push(tstack);
	value = nxo_stack_push(tstack);

	/* modload. */
	nxo_name_new(name, nx, onyx_name_modload, sizeof(onyx_name_modload) - 1,
	    TRUE);
	nxo_operator_new(value, onyx_ops_modload, NXN_ZERO);
	nxo_attr_set(value, NXOA_EXECUTABLE);
	nxo_dict_def(nx_systemdict_get(nx), nx, name, value);

	nxo_stack_npop(tstack, 2);
#endif
}
