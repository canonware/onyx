/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
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

#include "libstash/dbg_p.h"
#include "libstash/mem_l.h"

cw_dbg_t *
dbg_new(void)
{
  cw_dbg_t * retval;

  retval = (cw_dbg_t *) _cw_malloc(sizeof(cw_dbg_t));
  if (NULL == retval)
  {
    goto RETURN;
  }

#ifdef _CW_REENTRANT
  if (NULL == oh_new_r(&retval->flag_hash))
#else
  if (NULL == oh_new(&retval->flag_hash))
#endif
  {
    _cw_free(retval);
    retval = NULL;
    goto RETURN;
  }

  RETURN:
  return retval;
}

void
dbg_delete(cw_dbg_t * a_dbg)
{
  _cw_check_ptr(a_dbg);

  oh_delete(&a_dbg->flag_hash);
  _cw_free(a_dbg);
}

cw_bool_t
dbg_register(cw_dbg_t * a_dbg, const char * a_flag)
{
  cw_bool_t retval;
  
  if ((NULL != a_dbg)
      && (-1 != oh_item_insert(&a_dbg->flag_hash, (void *) a_flag, NULL)))
  {
    retval = FALSE;
  }
  else
  {
    retval = TRUE;
  }
  
  return retval;
}

void
dbg_unregister(cw_dbg_t * a_dbg, const char * a_flag)
{
  if (NULL != a_dbg)
  {
    oh_item_delete(&a_dbg->flag_hash, a_flag, NULL, NULL);
  }
}

cw_bool_t
dbg_is_registered(cw_dbg_t * a_dbg, const char * a_flag)
{
  cw_bool_t retval;

  _cw_check_ptr(a_flag);

  if (NULL != a_dbg)
  {
    retval = ! oh_item_search(&a_dbg->flag_hash, a_flag, NULL);
  }
  else
  {
    retval = FALSE;
  }
  
  return retval;
}
