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

#include <gtk/gtk.h>
#include "gtkdict.h"

static void
nx_gtk_object_destroyed (GtkObject *o, gpointer user_data)
{
  cw_nxoe_t *hook = gtk_object_get_data (o, "_cw_hook_object");
  cw_nxo_t nxo;

  printf ("destroyed GtkObject %p [%p]\n", o, hook);

  /*
   * XXX This is a gross hack, and a violation of the libonyx API.  This needs
   * to be fixed by using a malloced structure as the hook data and the gtk
   * object data.
   */
  nxo_p_new(&nxo, NXOT_HOOK);
  nxo.o.nxoe = (cw_nxoe_t *)hook;
  
  nxo_hook_data_set (&nxo, NULL);
}

static void
nx_gtk_object_delete (void *a_data, cw_nx_t *a_nx)
{
  if (a_data) {
    GtkObject *o = a_data;

    printf ("destroying GtkObject %p\n", o);

    gtk_signal_disconnect_by_func (o, nx_gtk_object_destroyed, NULL);

    gtk_object_destroy (o);
  }
}

static cw_nxoe_t *
nx_gtk_object_ref_iter (void *a_data, cw_bool_t a_reset)
{
  GtkObject *o = a_data;

  if (GTK_IS_WIDGET (o)) {
    GtkWidget *parent;

    if (a_reset) {
      gtk_object_set_data (o, "_cw_gtk_ref_iter", GTK_WIDGET(o)->parent);
    }
    
    parent = gtk_object_get_data (o, "_cw_gtk_container_iter");
    if (parent) {
      cw_nxoe_t *hook = gtk_object_get_data (GTK_OBJECT(parent), "_cw_hook_object");
      gtk_object_set_data (o, "_cw_gtk_container_iter", NULL);
      return hook;
    }
    else {
      return NULL;
    }
  }


  return NULL;
}

cw_nxo_t *
nx_gtk_object_attach_hook (cw_nx_t *nx, cw_nxo_t *stack, GtkObject *o)
{
  cw_nxo_t *hook;

  gtk_signal_connect (o, "destroy", nx_gtk_object_destroyed, NULL);

  hook = nxo_stack_push (stack);

  nxo_hook_new (hook, nx, o, NULL, nx_gtk_object_ref_iter, nx_gtk_object_delete);

  gtk_object_set_data (o, "_cw_hook_object", nxo_nxoe_get(hook));

  printf ("attaching hook to GtkObject %p [%p]\n", o, nxo_nxoe_get(hook));

  return hook;
}
