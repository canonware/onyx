/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 199 $
 * $Date: 1998-09-07 09:48:59 -0700 (Mon, 07 Sep 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#include <libstash.h>
#include <mem_priv.h>

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_mem_t *
mem_new()
{
  cw_mem_t * retval;

  /* Calling malloc() directly since we're bootstrapping. */
  retval = (cw_mem_t *) malloc(sizeof(cw_mem_t));
  _cw_check_ptr(retval);

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
mem_delete(cw_mem_t * a_mem_o)
{
  _cw_check_ptr(a_mem_o);

  /* Calling free() directly since we had to bootstrap the structure. */
  free(a_mem_o);
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void *
mem_malloc(cw_mem_t * a_mem_o, size_t a_size)
{
  void * retval;
  
  _cw_check_ptr(a_mem_o);

  retval = malloc(a_size);
  if (retval == NULL)
  {
    log_eprintf(g_log_o, __FILE__, __LINE__, "mem_malloc",
		"malloc(%d) returned NULL\n", a_size);
    abort();
  }

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void *
mem_calloc(cw_mem_t * a_mem_o, size_t a_number,
		  size_t a_size)
{
  void * retval;

  _cw_check_ptr(a_mem_o);

  retval = calloc(a_number, a_size);
  if (retval == NULL)
  {
    log_eprintf(g_log_o, __FILE__, __LINE__, "mem_calloc",
		"calloc(%d, %d) returned NULL\n", a_number, a_size);
    abort();
  }

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void *
mem_realloc(cw_mem_t * a_mem_o, void * a_ptr, size_t a_size)
{
  void * retval;

  _cw_check_ptr(a_mem_o);
  _cw_check_ptr(a_ptr);

  retval = realloc(a_ptr, a_size);
  if (retval == NULL)
  {
    log_eprintf(g_log_o, __FILE__, __LINE__, "mem_realloc",
		"realloc(%p, %d) returned NULL\n", a_ptr, a_size);
    abort();
  }
  
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
mem_free(cw_mem_t * a_mem_o, void * a_ptr)
{
  _cw_check_ptr(a_mem_o);
  _cw_check_ptr(a_ptr);

  free(a_ptr);
}
