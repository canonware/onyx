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
 *
 *
 ****************************************************************************/

typedef struct cw_dbg_s cw_dbg_t;

struct cw_dbg_s
{
  cw_oh_t flag_hash;
};

#define dbg_new _CW_NS_ANY(dbg_new)
#define dbg_delete _CW_NS_ANY(dbg_delete)
#define dbg_register _CW_NS_ANY(dbg_register)
#define dbg_unregister _CW_NS_ANY(dbg_unregister)
#define dbg_is_registered _CW_NS_ANY(dbg_is_registered)
#define dbg_clear _CW_NS_ANY(dbg_clear)

cw_dbg_t * dbg_new();
void dbg_delete(cw_dbg_t * a_dbg_o);
void dbg_register(cw_dbg_t * a_dbg_o, char * a_flag);
void dbg_unregister(cw_dbg_t * a_dbg_o, char * a_flag);
cw_bool_t dbg_is_registered(cw_dbg_t * a_dbg_o, char * a_flag);
