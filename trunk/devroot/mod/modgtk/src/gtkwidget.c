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

void
nx_gtk_widget_show (cw_nxo_t *a_thread)
{
  cw_nxo_t    *ostack;
  cw_nxo_t    *w_hook;
  GtkWidget   *w;

  ostack = nxo_thread_ostack_get(a_thread);

  NXO_STACK_GET (w_hook, ostack, a_thread);
  if (nxo_type_get(w_hook) != NXOT_HOOK) {
    nxo_thread_nerror (a_thread, NXN_typecheck);
    return;
  }
  w = nxo_hook_data_get (w_hook);
  if (!GTK_IS_WIDGET (w)) {
    nxo_thread_nerror (a_thread, NXN_typecheck);
    return;
  }

  gtk_widget_show (w);

  NXO_STACK_POP (ostack, a_thread);
}

void
nx_gtk_widget_show_all (cw_nxo_t *a_thread)
{
  cw_nxo_t    *ostack;
  cw_nxo_t    *w_hook;
  GtkWidget   *w;

  ostack = nxo_thread_ostack_get(a_thread);

  NXO_STACK_GET (w_hook, ostack, a_thread);
  if (nxo_type_get(w_hook) != NXOT_HOOK) {
    nxo_thread_nerror (a_thread, NXN_typecheck);
    return;
  }
  w = nxo_hook_data_get (w_hook);
  if (!GTK_IS_WIDGET (w)) {
    nxo_thread_nerror (a_thread, NXN_typecheck);
    return;
  }

  gtk_widget_show_all (w);

  NXO_STACK_POP (ostack, a_thread);
}
