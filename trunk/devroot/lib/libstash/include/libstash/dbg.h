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
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a dbg, or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_dbg_t *
dbg_new(void);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_dbg : Pointer to a dbg.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
dbg_delete(cw_dbg_t * a_dbg);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_dbg : Pointer to a dbg.
 *
 * a_flag : Pointer to a string that represents a debugging flag.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *               : a_dbg is NULL.
 *
 * <<< Description >>>
 *
 * Register a debug flag string (turn it on).
 *
 ****************************************************************************/
cw_bool_t
dbg_register(cw_dbg_t * a_dbg, const char * a_flag);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_dbg : Pointer to a dbg.
 *
 * a_flag : Pointer to a string that represents a debugging flag.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Unregister a flag (turn it off) if it is registered.
 *
 ****************************************************************************/
void
dbg_unregister(cw_dbg_t * a_dbg, const char * a_flag);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_dbg : Pointer to a dbg.
 *
 * a_flag : Pointer to a string that represents a debugging flag.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == not registered, TRUE == registered.
 *
 * <<< Description >>>
 *
 * Return TRUE if a_flag is registered, FALSE, otherwise.
 *
 ****************************************************************************/
cw_bool_t
dbg_is_registered(cw_dbg_t * a_dbg, const char * a_flag);
