#include "libonyx/libonyx.h"
#include "gtk/gtk.h"
#include "gtkdict.h"

void
nx_gtk_button_new (cw_nxo_t *a_thread)
{
  cw_nx_t     *nx;
  cw_nxo_t    *ostack;
  cw_nxo_t    *w_hook;
  GtkWidget   *w;

  nx = nxo_thread_nx_get(a_thread);

  ostack = nxo_thread_ostack_get(a_thread);

  w = gtk_button_new ();

  w_hook = nxo_stack_push (ostack);
  gtk_object_set_data (GTK_OBJECT (w), "_cw_hook_object", w_hook);

  nxo_hook_new (w_hook, nx, w, NULL, NULL, nx_gtk_object_delete);
}

void
nx_gtk_button_new_with_label (cw_nxo_t *a_thread)
{
  cw_nx_t     *nx;
  cw_nxo_t    *ostack;
  cw_nxo_t    *s, *w_hook;
  GtkWidget   *w;
  char        *str;
  cw_uint32_t  len;

  nx = nxo_thread_nx_get(a_thread);

  ostack = nxo_thread_ostack_get(a_thread);

  NXO_STACK_GET (s, ostack, a_thread);
  if (nxo_type_get(s) != NXOT_STRING) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  str = nxo_string_get (s);
  len = nxo_string_len_get (s);

  str = g_strndup (str, len);
  w = gtk_button_new_with_label (str);
  g_free (str);

  NXO_STACK_POP (ostack, a_thread);

  w_hook = nxo_stack_push (ostack);

  gtk_object_set_data (GTK_OBJECT (w), "_cw_hook_object", w_hook);

  nxo_hook_new (w_hook, nx, w, NULL, NULL, nx_gtk_object_delete);
}
