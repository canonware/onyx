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

#include "libstash/libstash.h"
#include "libonyx/libonyx.h"

#include "gtk/gtk.h"

void nx_gtk_button_new (cw_nxo_t *a_thread);
void nx_gtk_button_new_with_label (cw_nxo_t *a_thread);
void nx_gtk_container_add(cw_nxo_t *a_thread);
void nx_gtk_label_new(cw_nxo_t *a_thread);
void nx_gtk_label_set_text(cw_nxo_t *a_thread);
void nx_gtk_label_set_justify(cw_nxo_t *a_thread);
void nx_gtk_label_set_pattern(cw_nxo_t *a_thread);
void nx_gtk_label_set_line_wrap(cw_nxo_t *a_thread);
void nx_gtk_label_get(cw_nxo_t *a_thread);

void nx_gtk_signal_connect (cw_nxo_t *a_thread);

void nx_gtk_widget_show(cw_nxo_t *a_thread);
void nx_gtk_widget_show_all(cw_nxo_t *a_thread);
void nx_gtk_window_new(cw_nxo_t *a_thread);
void nx_gtk_main(cw_nxo_t *a_thread);
void nx_gtk_main_quit(cw_nxo_t *a_thread);

/* special hook handling function */
void nx_gtk_object_delete (void *a_data, cw_nx_t *a_nx);

/* special signal handling functions */
void nx_gtk_signal_marshal (GtkObject *object,
			    gpointer	  data,
			    guint	  nparams,
			    GtkArg	 *args,
			    GtkType	 *arg_types,
			    GtkType	  return_type);

void nx_gtk_signal_destroy (gpointer data);