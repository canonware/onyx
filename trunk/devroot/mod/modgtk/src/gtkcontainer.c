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
nx_gtk_container_add (cw_nxo_t *a_thread)
{
  cw_nxo_t    *ostack;
  cw_nxo_t    *w_hook, *container_hook;
  GtkWidget   *w, *container;

  ostack = nxo_thread_ostack_get(a_thread);

  NXO_STACK_GET (container_hook, ostack, a_thread);
  if (nxo_type_get(container_hook) != NXOT_HOOK) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }
  container = nxo_hook_data_get (container_hook);
  if (!GTK_IS_CONTAINER (container)) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  NXO_STACK_NGET (w_hook, ostack, a_thread, 1);
  if (nxo_type_get(w_hook) != NXOT_HOOK) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }
  w = nxo_hook_data_get (w_hook);
  if (!GTK_IS_WIDGET (w)) {
    nxo_thread_error (a_thread, NXO_THREADE_TYPECHECK);
    return;
  }

  gtk_container_add (GTK_CONTAINER(container), w);

  NXO_STACK_POP (ostack, a_thread);
  NXO_STACK_POP (ostack, a_thread);
}
