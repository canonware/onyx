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

#define dbg_new _CW_NS_STASH(dbg_new)
cw_dbg_t *
dbg_new();

#define dbg_delete _CW_NS_STASH(dbg_delete)
void
dbg_delete(cw_dbg_t * a_dbg);

#define dbg_register _CW_NS_STASH(dbg_register)
void
dbg_register(cw_dbg_t * a_dbg, char * a_flag);

#define dbg_unregister _CW_NS_STASH(dbg_unregister)
void
dbg_unregister(cw_dbg_t * a_dbg, char * a_flag);

#define dbg_is_registered _CW_NS_STASH(dbg_is_registered)
cw_bool_t
dbg_is_registered(cw_dbg_t * a_dbg, char * a_flag);
