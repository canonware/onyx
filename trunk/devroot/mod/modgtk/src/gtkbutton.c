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
nx_gtk_button_new (cw_nxo_t *a_thread)
{
  cw_nx_t     *nx;
  cw_nxo_t    *ostack;
  GtkWidget   *w;

  nx = nxo_thread_nx_get(a_thread);

  ostack = nxo_thread_ostack_get(a_thread);

  w = gtk_button_new ();

  nx_gtk_object_attach_hook (nx, ostack, GTK_OBJECT (w));
}

void
nx_gtk_button_new_with_label (cw_nxo_t *a_thread)
{
  cw_nx_t     *nx;
  cw_nxo_t    *ostack;
  cw_nxo_t    *s;
  GtkWidget   *w;
  char        *str;
  cw_uint32_t  len;

  nx = nxo_thread_nx_get(a_thread);

  ostack = nxo_thread_ostack_get(a_thread);

  NXO_STACK_GET (s, ostack, a_thread);
  if (nxo_type_get(s) != NXOT_STRING) {
    nxo_thread_nerror (a_thread, NXN_typecheck);
    return;
  }

  str = nxo_string_get (s);
  len = nxo_string_len_get (s);

  str = g_strndup (str, len);
  w = gtk_button_new_with_label (str);
  g_free (str);

  NXO_STACK_POP (ostack, a_thread);

  nx_gtk_object_attach_hook (nx, ostack, GTK_OBJECT (w));
}
