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

#include "../include/libstil/libstil.h"

#define	NAME_anon_hook	"anon_hook"
#define	NAME_mark_hook	"mark_hook"

const cw_uint8_t *data = "hook data";

cw_stilo_threade_t
hook_eval(void *a_data, cw_stilo_t *a_thread)
{
	_cw_assert(a_data == data);

	_cw_stil_code(a_thread, "`Evaluated hook\n' print");

	return STILO_THREADE_NONE;
}

cw_stiloe_t *
hook_ref_iter(void *a_data, cw_bool_t a_reset)
{
	_cw_assert(a_data == data);

	return NULL;
}

void
hook_delete(void *a_data, cw_stil_t *a_stil)
{
	_cw_assert(a_data == data);
}

void
anon_hook(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *hook;

	ostack = stilo_thread_ostack_get(a_thread);
	hook = stilo_stack_push(ostack);
	stilo_hook_new(hook, stilo_thread_stil_get(a_thread), (void *)data,
	    hook_eval, hook_ref_iter, hook_delete);
	stilo_attrs_set(hook, STILOA_EXECUTABLE);
}

void
mark_hook(cw_stilo_t *a_thread)
{
	cw_stilo_t	*ostack, *hook, *tag;

	anon_hook(a_thread);

	ostack = stilo_thread_ostack_get(a_thread);
	hook = stilo_stack_get(ostack);
	tag = stilo_hook_tag_get(hook);
	stilo_mark_new(tag);
}

int
main(int argc, char **argv, char **envp)
{
	cw_stil_t	stil;
	cw_stilo_t	thread, *ostack, *name, *operator;

	libstash_init();
	out_put(out_err, "Test begin\n");

	_cw_assert(stil_new(&stil, argc, argv, envp, NULL, NULL, NULL, NULL,
	    NULL) == &stil);
	stilo_thread_new(&thread, &stil);

	/* Define anon_hook and mark_hook. */
	ostack = stilo_thread_ostack_get(&thread);

	name = stilo_stack_push(ostack);
	operator = stilo_stack_push(ostack);
	stilo_name_new(name, &stil, NAME_anon_hook, strlen(NAME_anon_hook),
	    FALSE);
	stilo_operator_new(operator, anon_hook, STILN_ZERO);
	stilo_attrs_set(operator, STILOA_EXECUTABLE);
	_cw_stil_code(&thread, "def");

	name = stilo_stack_push(ostack);
	operator = stilo_stack_push(ostack);
	stilo_name_new(name, &stil, NAME_mark_hook, strlen(NAME_mark_hook),
	    FALSE);
	stilo_operator_new(operator, mark_hook, STILN_ZERO);
	stilo_attrs_set(operator, STILOA_EXECUTABLE);
	_cw_stil_code(&thread, "def");

	_cw_stil_code(&thread, "
anon_hook 1 sprint
anon_hook eval
mark_hook 1 sprint
mark_hook eval
");
	stil_delete(&stil);

	out_put(out_err, "Test end\n");
	libstash_shutdown();
	return 0;
}
