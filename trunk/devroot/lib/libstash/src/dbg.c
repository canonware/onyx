/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Dynamic debug spew class.  The idea is to be able to turn various types
 * of debug spew on and off on the fly, without recompilation, without even
 * restarting the program.
 *
 ****************************************************************************/

#ifdef _CW_REENTRANT
#  include "libstash_r.h"
#else
#  include "libstash.h"
#endif

/****************************************************************************
 * <<< Description >>>
 *
 * dbg constructor.
 *
 ****************************************************************************/
cw_dbg_t *
dbg_new()
{
  cw_dbg_t * retval;

  retval = (cw_dbg_t *) _cw_malloc(sizeof(cw_dbg_t));

#ifdef _CW_REENTRANT
  oh_new(&retval->flag_hash, TRUE);
#else
  oh_new(&retval->flag_hash);
#endif

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * dbg destructor.
 *
 ****************************************************************************/
void
dbg_delete(cw_dbg_t * a_dbg_o)
{
  _cw_check_ptr(a_dbg_o);

  oh_delete(&a_dbg_o->flag_hash);
  _cw_free(a_dbg_o);
}

/****************************************************************************
 * <<< Description >>>
 *
 * Register a debug flag string (turn it on).
 *
 ****************************************************************************/
void
dbg_register(cw_dbg_t * a_dbg_o, char * a_flag)
{
  _cw_check_ptr(a_dbg_o);

  /* Ignore the return value, since we don't care if the flag is already
   * registered.  Pass a NULL data pointer since we don't need any additional
   * info stored with the flag. */
  oh_item_insert(&a_dbg_o->flag_hash, (void *) a_flag, NULL);
}

/****************************************************************************
 * <<< Description >>>
 *
 * Unregister a flag (turn it off) if it is registered.
 *
 ****************************************************************************/
void
dbg_unregister(cw_dbg_t * a_dbg_o, char * a_flag)
{
  char ** junk1, ** junk2;

  _cw_check_ptr(a_dbg_o);

  /* Ignore the return value, since we don't care if the flag isn't registered.
   * Also, pass dummy variables junk1 and junk2, since we don't care about their
   * contents. */
  oh_item_delete(&a_dbg_o->flag_hash, a_flag, (void **) junk1, (void **) junk2);
}

/****************************************************************************
 * <<< Description >>>
 *
 * Return TRUE if a_flag is registered.
 *
 ****************************************************************************/
cw_bool_t
dbg_is_registered(cw_dbg_t * a_dbg_o, char * a_flag)
{
  char ** junk;

  _cw_check_ptr(a_dbg_o);

  /* Pass dummy variable junk, since we don't care about the data pointer. */
  return ! oh_item_search(&a_dbg_o->flag_hash, a_flag, (void **) junk);
}
