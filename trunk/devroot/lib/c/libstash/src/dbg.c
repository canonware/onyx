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
 ****************************************************************************/

#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include "libstash/mem_l.h"

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

#ifdef _LIBSTASH_DBG
  dbg_register(retval, "mem_error");
#endif
  
  return retval;
}

void
dbg_delete(cw_dbg_t * a_dbg)
{
  _cw_check_ptr(a_dbg);

  oh_delete(&a_dbg->flag_hash);
  _cw_free(a_dbg);
}

void
dbg_register(cw_dbg_t * a_dbg, const char * a_flag)
{
/*    _cw_check_ptr(a_dbg); */

  if (NULL != a_dbg)
  {
    /* Ignore the return value, since we don't care if the flag is already
     * registered.  Pass a NULL data pointer since we don't need any additional
     * info stored with the flag. */
    oh_item_insert(&a_dbg->flag_hash, (void *) a_flag, NULL);
  }
}

void
dbg_unregister(cw_dbg_t * a_dbg, const char * a_flag)
{
  char ** junk1 = NULL, ** junk2 = NULL;

/*    _cw_check_ptr(a_dbg); */

  if (NULL != a_dbg)
  {
    /* Ignore the return value, since we don't care if the flag isn't
     * registered.  Also, pass dummy variables junk1 and junk2, since we don't
     * care about their contents. */
    oh_item_delete(&a_dbg->flag_hash, a_flag, (void **) junk1, (void **) junk2);
  }
}

cw_bool_t
dbg_is_registered(cw_dbg_t * a_dbg, const char * a_flag)
{
  cw_bool_t retval;
  char * junk = NULL;

/*    _cw_check_ptr(a_dbg); */
  _cw_check_ptr(a_flag);

  if (NULL != a_dbg)
  {
    /* Pass dummy variable junk, since we don't care about the data pointer. */
    retval = ! oh_item_search(&a_dbg->flag_hash, a_flag, (void **) &junk);
  }
  else
  {
    retval = FALSE;
  }
  
  return retval;
}
