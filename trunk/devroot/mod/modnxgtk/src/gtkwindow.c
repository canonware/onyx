
#include "libonyx/libonyx.h"
#include "gtk/gtk.h"
#include "gtkdict.h"

void
nx_gtk_window_new (cw_nxo_t *a_thread)
{
  cw_nx_t     *nx;
  cw_nxo_t    *ostack;
  cw_nxo_t    *t, *w_hook;
  GtkWidget   *w;

  nx = nxo_thread_nx_get(a_thread);

  ostack = nxo_thread_ostack_get(a_thread);

  NXO_STACK_GET (t, ostack, a_thread);
  if (nxo_type_get(t) != NXOT_INTEGER) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  w = gtk_window_new (nxo_integer_get(t));

  NXO_STACK_POP (ostack, a_thread);

  w_hook = nxo_stack_push (ostack);
  gtk_object_set_data (GTK_OBJECT (w), "_cw_hook_object", w_hook);

  nxo_hook_new (w_hook, nx, w, NULL, NULL, nx_gtk_object_delete);
}
