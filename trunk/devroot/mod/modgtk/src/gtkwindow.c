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
nx_gtk_window_new (cw_nxo_t *a_thread)
{
  cw_nx_t     *nx;
  cw_nxo_t    *ostack;
  cw_nxo_t    *t;
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

  nx_gtk_object_attach_hook (nx, ostack, GTK_OBJECT (w));
}
