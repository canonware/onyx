/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 88 $
 * $Date: 1998-06-23 17:42:27 -0700 (Tue, 23 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _DEPGN_H_
#define _DEPGN_H_

/* Pseudo-opaque type. */
typedef struct cw_depgn_s cw_depgn_t;

struct cw_depgn_s
{
  cw_bool_t is_malloced;
  cw_bool_t is_thread_safe;
  cw_rwl_t * rw_lock;
  void * data;
  void * object;
  cw_bool_t is_up_to_date;
  cw_bool_t is_transient;

  /* Data structures to keep track of dependencies. */
  cw_list_t * away_list;
  cw_oh_t * away_tab;

  /* Data structures to keep track of dependents. */
  cw_list_t * to_list;
  cw_oh_t * to_tab;
  
  /* Callbacks. */
  cw_bool_t (*update_callback)(void *);
  void (*delete_callback)(void *);
};

#define depgn_new _CW_NS_CMN(depgn_new)
#define depgn_delete _CW_NS_CMN(depgn_delete)
#define depgn_ _CW_NS_CMN(depgn_)

/* Typedefs to allow easy function pointer passing. */
typedef cw_bool_t depgn_update_callback_t(void *);
typedef cw_bool_t depgn_delete_callback_t(void *);

cw_depgn_t * depgn_new(cw_depgn_t * a_depgn_o, cw_bool_t a_is_thread_safe);
void depgn_delete(cw_depgn_t * a_depgn_o);

void depgn_wlock(cw_depgn_t * a_depgn_o);
void depgn_wunlock(cw_depgn_t * a_depgn_o);

cw_bool_t depgn_update(cw_depgn_t * a_depgn_o);
cw_bool_t depgn_is_parent_leaf(cw_depgn_t * a_depgn_o);

cw_bool_t depgn_get_is_transient(cw_depgn_t * a_depgn_o);
depgn_update_callback_t * depgn_get_update_callback(cw_depgn_t * a_depgn_o);
depgn_delete_callback_t * depgn_get_delete_callback(cw_depgn_t * a_depgn_o);

/* The following functions assume the wlock is currently held by the
 * caller if is_thread_safe is TRUE. */
cw_bool_t depgn_set_is_transient(cw_depgn_t * a_depgn_o,
				 cw_bool_t a_is_transient);
cw_bool_t depgn_set_update_callback(cw_depgn_t * a_depgn_o,
				    depgn_update_callback_t * a_new_callback);
cw_bool_t depgn_set_delete_callback(cw_depgn_t * a_depgn_o,
				    depgn_delete_callback_t * a_new_callback);
cw_bool_t depgn_add_dependent(cw_depgn_t * a_depgn_o, cw_depgn_t a_dependent);
cw_bool_t depgn_add_dependency(cw_depgn_t * a_depgn_o, cw_depgn_t a_dependency);
cw_bool_t depgn_rm_dependent(cw_depgn_t * a_depgn_o, cw_depgn_t a_dependent);
cw_bool_t depgn_rm_dependency(cw_depgn_t * a_depgn_o, cw_depgn_t a_dependency);





#endif /* _DEPGN_H_ */
