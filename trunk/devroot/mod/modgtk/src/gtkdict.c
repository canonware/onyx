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

#include "gtkdict.h"

#include "gtk/gtk.h"

#define ENTRY(name)	{#name, nx_##name}

struct cw_gtkdict_op_entry {
	const char      *op_n;
	cw_op_t		*op_f;
};

static const struct cw_gtkdict_op_entry gtkdict_ops[] = {
  /* GtkButton */
  ENTRY (gtk_button_new),
  ENTRY (gtk_button_new_with_label),

  /* GtkContainer */
  ENTRY (gtk_container_add),

  /* GtkLabel */
  ENTRY (gtk_label_new),
  ENTRY (gtk_label_set_text),
  ENTRY (gtk_label_set_justify),

  /* GtkWidget */
  ENTRY (gtk_widget_show),
  ENTRY (gtk_widget_show_all),

  /* GtkWindow */
  ENTRY (gtk_window_new),

  /* Gtk Main */
  ENTRY (gtk_main),
  ENTRY (gtk_main_quit),

  /* signals */
  ENTRY (gtk_signal_connect)
};
static int NOPS = sizeof (gtkdict_ops) / sizeof(gtkdict_ops[0]);

#define CONST(name)	{#name, name}

struct cw_gtkdict_const_entry {
	const char      *const_n;
	int		const_v;
};

static const struct cw_gtkdict_const_entry gtkdict_consts[] = {
  /* GtkWindowType */
  CONST (GTK_WINDOW_TOPLEVEL),
  CONST (GTK_WINDOW_DIALOG),
  CONST (GTK_WINDOW_POPUP)
};

static int NCONSTS = sizeof (gtkdict_consts) / sizeof(gtkdict_consts[0]);

void
gtkdict_l_populate (cw_nxo_t *gtk_dict, cw_nx_t *a_nx, cw_nxo_t *a_thread)
{
  int i;
  cw_nxo_t *name, *value;
  cw_nxo_t *tstack;

  tstack = nxo_thread_tstack_get(a_thread);

  for (i = 0; i < NOPS; i++) {
    name = nxo_stack_push (tstack);
    value = nxo_stack_push (tstack);
    nxo_name_new (name, a_nx,
		  gtkdict_ops[i].op_n, strlen (gtkdict_ops[i].op_n), FALSE);
    nxo_operator_new (value, gtkdict_ops[i].op_f,
		      NXN_ZERO);

    nxo_attr_set(value, NXOA_EXECUTABLE);

    nxo_dict_def(gtk_dict, a_nx, name, value);

    nxo_stack_npop(tstack, 2);
  }

  for (i = 0; i < NCONSTS; i ++) {
    name = nxo_stack_push (tstack);
    value = nxo_stack_push (tstack);

    nxo_name_new (name, a_nx,
		  gtkdict_consts[i].const_n, strlen (gtkdict_consts[i].const_n), FALSE);
    nxo_integer_new (value, gtkdict_consts[i].const_v);

    nxo_dict_def(gtk_dict, a_nx, name, value);

    nxo_stack_npop(tstack, 2);
  }
}
