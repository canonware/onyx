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

void
nx_gtk_label_new (cw_nxo_t *a_thread)
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
  w = gtk_label_new (str);
  g_free (str);

  NXO_STACK_POP (ostack, a_thread);

  nx_gtk_object_attach_hook (nx, ostack, GTK_OBJECT (w));
}

void
nx_gtk_label_set_text(cw_nxo_t *a_thread)
{
  cw_nxo_t    *ostack;
  cw_nxo_t    *s, *w_hook;
  GtkWidget   *w;
  char        *str;
  cw_uint32_t  len;

  ostack = nxo_thread_ostack_get(a_thread);

  NXO_STACK_GET (w_hook, ostack, a_thread);
  if (nxo_type_get(w_hook) != NXOT_HOOK) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }
  w = nxo_hook_data_get (w_hook);
  if (!GTK_IS_WIDGET (w)) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  NXO_STACK_NGET (s, ostack, a_thread, 1);
  if (nxo_type_get(s) != NXOT_STRING) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  str = nxo_string_get (s);
  len = nxo_string_len_get (s);

  str = g_strndup (str, len);
  gtk_label_set_text (GTK_LABEL(w), str);
  g_free (str);

  NXO_STACK_POP (ostack, a_thread);
  NXO_STACK_POP (ostack, a_thread);
}

void
nx_gtk_label_set_justify(cw_nxo_t *a_thread)
{
  cw_nxo_t    *ostack;
  cw_nxo_t    *j, *w_hook;
  GtkWidget   *w;

  ostack = nxo_thread_ostack_get(a_thread);

  NXO_STACK_GET (w_hook, ostack, a_thread);
  if (nxo_type_get(w_hook) != NXOT_HOOK) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }
  w = nxo_hook_data_get (w_hook);
  if (!GTK_IS_LABEL (w)) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  NXO_STACK_NGET (j, ostack, a_thread, 1);
  if (nxo_type_get(j) != NXOT_INTEGER) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  gtk_label_set_justify (GTK_LABEL(w), nxo_integer_get (j));

  NXO_STACK_POP (ostack, a_thread);
  NXO_STACK_POP (ostack, a_thread);
}
