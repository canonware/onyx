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
 * $Revision: 136 $
 * $Date: 1998-07-10 13:11:48 -0700 (Fri, 10 Jul 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#include <libstash.h>
#include <mem_priv.h>

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
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
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
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
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
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
    _cw_error("malloc() returned a NULL pointer.");
  }

  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
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
    _cw_error("calloc() returned a NULL pointer.");
  }

  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
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
    _cw_error("realloc() returned a NULL pointer.");
  }
  
  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
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
