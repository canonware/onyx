/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 86 $
 * $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#include <config.h>
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
void *
mem_calloc(cw_mem_t * a_mem_o, size_t a_number,
		  size_t a_size)
{
  void * retval;

  _cw_check_ptr(a_mem_o);

  retval = calloc(a_number, a_size);
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
void *
mem_realloc(cw_mem_t * a_mem_o, void * a_ptr, size_t a_size)
{
  void * retval;

  _cw_check_ptr(a_mem_o);
  _cw_check_ptr(a_ptr);

  retval = realloc(a_ptr, a_size);
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
mem_free(cw_mem_t * a_mem_o, void * a_ptr)
{
  _cw_check_ptr(a_mem_o);
  _cw_check_ptr(a_ptr);

  free(a_ptr);
}
