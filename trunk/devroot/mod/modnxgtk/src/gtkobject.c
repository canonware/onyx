
#include <gtk/gtk.h>
#include "gtkdict.h"

void
nx_gtk_object_destroyed (GtkObject *o, gpointer user_data)
{
  cw_nxo_t *hook = gtk_object_get_data (o, "_cw_hook_object");
  
#if notyet
  nxo_hook_data_set (hook, NULL);
#endif
}

void
nx_gtk_object_delete (void *a_data, cw_nx_t *a_nx)
{
  if (a_data) {
    GtkObject *o = a_data;

    printf ("destroying GtkObject %p\n", o);

    gtk_object_destroy (o);
  }
}
