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
nx_gtk_main (cw_nxo_t *a_thread)
{
  gtk_main();
}

void
nx_gtk_main_quit (cw_nxo_t *a_thread)
{
  gtk_main_quit();
}
