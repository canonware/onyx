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
 * Dynamic debug spew class.  The idea is to be able to turn various types
 * of debug spew on and off on the fly, without recompilation, without even
 * restarting the program.
 *
 ****************************************************************************/

typedef struct cw_dbg_s cw_dbg_t;

struct cw_dbg_s
{
  cw_oh_t flag_hash;
};

/****************************************************************************
 * <<< Description >>>
 *
 * dbg constructor.
 *
 ****************************************************************************/
cw_dbg_t *
dbg_new(void);

/****************************************************************************
 * <<< Description >>>
 *
 * dbg destructor.
 *
 ****************************************************************************/
void
dbg_delete(cw_dbg_t * a_dbg);

/****************************************************************************
 * <<< Description >>>
 *
 * Register a debug flag string (turn it on).
 *
 ****************************************************************************/
void
dbg_register(cw_dbg_t * a_dbg, const char * a_flag);

/****************************************************************************
 * <<< Description >>>
 *
 * Unregister a flag (turn it off) if it is registered.
 *
 ****************************************************************************/
void
dbg_unregister(cw_dbg_t * a_dbg, const char * a_flag);

/****************************************************************************
 * <<< Description >>>
 *
 * Return TRUE if a_flag is registered.
 *
 ****************************************************************************/
cw_bool_t
dbg_is_registered(cw_dbg_t * a_dbg, const char * a_flag);
