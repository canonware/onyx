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

#include <dlfcn.h>	/* for modload operator. */

#include "onyx.h"

static cw_nxoe_t *
onyx_ops_modload_sym_ref_iter(void *a_data, cw_bool_t a_reset)
{
	return NULL;
}

static void
onyx_ops_modload_sym_delete(void *a_data, cw_nx_t *a_nx)
{
	dlclose(a_data);
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
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}

	/*
	 * Create a copy of the path with an extra byte to store a '\0'
	 * terminator.
	 */
	nxo = nxo_stack_push(tstack);
	nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(path) + 1);
	nxo_string_lock(path);
	nxo_string_set(nxo, 0, nxo_string_get(path),
	    nxo_string_len_get(path));
	nxo_string_el_set(nxo, '\0', nxo_string_len_get(nxo) - 1);
	nxo_string_unlock(path);

	str = nxo_string_get(nxo);

	/* Try to dlopen(). */
	_cw_out_put("dlopen(\"[s]\")\n", str);
	handle = dlopen(str, RTLD_LAZY);
	if (handle == NULL) {
		_cw_out_put("dlopen() error: [s]\n", dlerror());
		nxo_stack_pop(tstack);
		nxo_thread_error(a_thread, NXO_THREADE_INVALIDFILEACCESS);
		return;
	}

	/*
	 * Create a copy of sym with an extra byte to store a '\0' terminator.
	 */
	nxo_string_new(nxo, nxo_thread_nx_get(a_thread),
	    nxo_thread_currentlocking(a_thread), nxo_string_len_get(sym) + 1);
	nxo_string_lock(sym);
	nxo_string_set(nxo, 0, nxo_string_get(sym),
	    nxo_string_len_get(sym));
	nxo_string_el_set(nxo, '\0', nxo_string_len_get(nxo) - 1);
	nxo_string_unlock(sym);

	str = nxo_string_get(nxo);

	/* Look up symbol. */
	_cw_out_put("dlsym(\"[s]\")\n", str);
	symbol = dlsym(handle, str);

	/* Pop nxo. */
	nxo_stack_pop(tstack);

	if (symbol == NULL) {
		/* Couldn't find the init function. */
		dlclose(handle);
		nxo_thread_error(a_thread, NXO_THREADE_UNDEFINED);
		return;
	}

	/*
	 * Create a hook whose data pointer is the library handle, and whose
	 * evaluation function is the symbol we just looked up.
	 */
	nxo = nxo_stack_push(estack);
	nxo_hook_new(nxo, nxo_thread_nx_get(a_thread), handle, symbol,
	    onyx_ops_modload_sym_ref_iter,
	    onyx_ops_modload_sym_delete);
	nxo_dup(nxo_hook_tag_get(nxo), sym);
	nxo_attr_set(nxo, NXOA_EXECUTABLE);

	/* Pop the arguments before recursing. */
	nxo_stack_npop(ostack, 2);

	/* Recurse on the hook. */
	nxo_thread_loop(a_thread);
}

void
onyx_ops_init(cw_nxo_t *a_thread)
{
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
}
