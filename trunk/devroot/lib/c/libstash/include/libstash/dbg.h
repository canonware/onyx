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
 * <<< Description >>>
 *
 * Public interface for the dbg class.
 *
 ****************************************************************************/

typedef struct cw_dbg_s cw_dbg_t;

cw_dbg_t *
dbg_new(void);

void
dbg_delete(cw_dbg_t * a_dbg);

cw_bool_t
dbg_register(cw_dbg_t * a_dbg, const char * a_flag);

void
dbg_unregister(cw_dbg_t * a_dbg, const char * a_flag);

cw_bool_t
dbg_is_registered(cw_dbg_t * a_dbg, const char * a_flag);
