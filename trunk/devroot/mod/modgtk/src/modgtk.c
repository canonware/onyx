/******************************************************************************
 *
 * <Copyright = toshok>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "libonyx/libonyx.h"
#include "gtk/gtk.h"
#include "gtkdict.h"

static const cw_uint8_t dict_name[] = "gtkdict";

void
_cw_modnxgtk_init (void *a_arg, cw_nxo_t *a_thread)
{
	cw_nxo_t *ostack, *tstack;
	cw_nx_t *nx;
	cw_nxo_t *name, *gtkdict;
	int fake_argc = 1;
	char **fake_argv;

	fake_argv = g_new (char*, 2);
	fake_argv[0] = "onyx";
	fake_argv[1] = NULL;

	gtk_init (&fake_argc, &fake_argv);

	gtk_signal_set_funcs (nx_gtk_signal_marshal,
			      nx_gtk_signal_destroy);

	ostack = nxo_thread_ostack_get(a_thread);
	tstack = nxo_thread_tstack_get(a_thread);
	nx = nxo_thread_nx_get(a_thread);

	/* create and populate our gtkdict */
	gtkdict = nxo_stack_push(tstack);
	nxo_dict_new(gtkdict, nx, TRUE, 128);
	gtkdict_l_populate (gtkdict, nx, a_thread);

	name = nxo_stack_push(tstack);
	nxo_name_new(name, nx, dict_name,
		     strlen(dict_name), TRUE);
	/* and place it into the systmdict */
	nxo_dict_def(nx_systemdict_get(nx),
		     nx, name, gtkdict);

	nxo_stack_npop(tstack, 2);
}
