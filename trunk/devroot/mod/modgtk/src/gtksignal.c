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

#define	nxgtk_code(a_thread, a_code) do {				\
	cw_nxo_threadp_t	threadp;				\
									\
	nxo_threadp_new(&threadp);					\
	nxo_thread_interpret((a_thread), &threadp, a_code,		\
	    strlen(a_code));						\
	nxo_thread_flush((a_thread), &threadp);				\
	nxo_threadp_delete(&threadp, (a_thread));			\
} while (0)

typedef struct {
  cw_nxo_t *a_thread;
  char *wrapped_code;
} NxGtkSignalClosure;

void
nx_gtk_signal_marshal (GtkObject *object,
		       gpointer	  data,
		       guint	  nparams,
		       GtkArg	 *args,
		       GtkType	 *arg_types,
		       GtkType	  return_type)
{
  NxGtkSignalClosure *closure = data;
  cw_nxo_t *tstack;
  cw_nx_t *nx;

  nx = nxo_thread_nx_get(closure->a_thread);
  tstack = nxo_thread_tstack_get (closure->a_thread);

  if (nparams > 0) {
    cw_nxo_t *array;
    int i;

    array = nxo_stack_push (tstack);

    nxo_array_new (array, nx, FALSE, nparams);

    for (i = 0; i < nparams; i ++) {
      cw_nxo_t *el;

      printf ("arg_name = %s\n", args[i].name);

      switch (arg_types[i]) {
      case GTK_TYPE_CHAR:
	el = nxo_stack_push (tstack);
	nxo_integer_new (el, GTK_VALUE_CHAR(args[i]));
	break;
      case GTK_TYPE_UCHAR:
	el = nxo_stack_push (tstack);
	nxo_integer_new (el, GTK_VALUE_UCHAR(args[i]));
	break;
      case GTK_TYPE_BOOL:
	el = nxo_stack_push (tstack);
	nxo_boolean_new (el, GTK_VALUE_BOOL(args[i]));
	break;
      case GTK_TYPE_ENUM:
      case GTK_TYPE_INT:
	el = nxo_stack_push (tstack);
	nxo_integer_new (el, GTK_VALUE_INT(args[i]));
	break;
      case GTK_TYPE_FLAGS:
      case GTK_TYPE_UINT:
	el = nxo_stack_push (tstack);
	nxo_integer_new (el, GTK_VALUE_UINT(args[i]));
	break;
      case GTK_TYPE_LONG:
	el = nxo_stack_push (tstack);
	nxo_integer_new (el, GTK_VALUE_LONG(args[i]));
	break;
      case GTK_TYPE_ULONG:
	el = nxo_stack_push (tstack);
	nxo_integer_new (el, GTK_VALUE_ULONG(args[i]));
	break;
      case GTK_TYPE_FLOAT:
      case GTK_TYPE_DOUBLE:
	g_assert (0);
	break;
      case GTK_TYPE_BOXED:
      case GTK_TYPE_POINTER:
	g_assert (0);
	break;
      case GTK_TYPE_STRING:
	el = nxo_stack_push (tstack);
	nxo_string_new (el, nx, FALSE, strlen(GTK_VALUE_STRING(args[i])));
	nxo_string_set (el, 0, GTK_VALUE_STRING(args[i]), strlen(GTK_VALUE_STRING(args[i])));
	break;
      case GTK_TYPE_OBJECT: {
	  GtkObject *o = GTK_VALUE_OBJECT (args[i]);
	  cw_nxo_t *hook = gtk_object_get_data (o, "_cw_hook_object");

	  el = nxo_stack_push (tstack);
	  nxo_dup (el, hook);
	  break;
      }
      }

      nxo_array_el_set (array, el, i);
    }
  }

  nxgtk_code (closure->a_thread, closure->wrapped_code);
}

void
nx_gtk_signal_destroy (gpointer data)
{
  NxGtkSignalClosure *closure = data;

  g_free (closure->wrapped_code);
  g_free (closure);
}

void
nx_gtk_signal_connect (cw_nxo_t *a_thread)
{
  cw_nx_t     *nx;
  cw_nxo_t    *ostack;

  cw_nxo_t    *sig, *w_hook, *code;
  char        *sig_str, *code_str;
  int         sig_len, code_len;
  char        *s, *c;
  GtkWidget   *w;
  guint       signal_id;

  nx = nxo_thread_nx_get(a_thread);

  ostack = nxo_thread_ostack_get(a_thread);

  NXO_STACK_GET (w_hook, ostack, a_thread);
  if (nxo_type_get(w_hook) != NXOT_HOOK) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }
  w = nxo_hook_data_get (w_hook);
  if (!GTK_IS_OBJECT (w)) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  NXO_STACK_NGET (sig, ostack, a_thread, 1);
  if (nxo_type_get(sig) != NXOT_STRING) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  NXO_STACK_NGET (code, ostack, a_thread, 2);
  if (nxo_type_get(sig) != NXOT_STRING) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  sig_str = nxo_string_get (sig);
  sig_len = nxo_string_len_get (sig);
  s = g_strndup (sig_str, sig_len);

  code_str = nxo_string_get (code);
  code_len = nxo_string_len_get (code);
  c = g_strndup (code_str, code_len);

  /* figure out and hook up the signal here */
  signal_id = gtk_signal_lookup (s, GTK_OBJECT_TYPE (w));
  if (!signal_id) {
      g_warning ("nx_gtk_signal_connect(): could not find signal \"%s\" in the `%s' class ancestry",
		 s,
		 gtk_type_name (GTK_OBJECT_TYPE (w)));
  }
  else {
    char *wrapped_code;
    NxGtkSignalClosure *closure;

    wrapped_code = g_strdup_printf ("gtkdict begin %s end", c);

    closure = g_new (NxGtkSignalClosure, 1);
    closure->a_thread = a_thread;
    closure->wrapped_code = wrapped_code;

    gtk_signal_connect (GTK_OBJECT (w), s, NULL, closure);
  }

  g_free (s);
  g_free (c);

  nxo_stack_npop (ostack, 3);
}
