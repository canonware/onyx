
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

  NXO_STACK_POP (ostack, a_thread);
  NXO_STACK_POP (ostack, a_thread);
  NXO_STACK_POP (ostack, a_thread);
}
