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
 * Public interface for the treen (tree node) class.
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_treen_s cw_treen_t;

struct cw_treen_s
{
#ifdef _CW_REENTRANT
  cw_bool_t is_thread_safe;
  cw_mtx_t lock;
#endif
  void * data;
  cw_uint32_t num_children;
  cw_treen_t ** children;
};

#define treen_new _CW_NS_ANY(treen_new)
#define treen_delete _CW_NS_ANY(treen_delete)
#define treen_get_num_children _CW_NS_ANY(treen_get_num_children)
#define treen_link_child _CW_NS_ANY(tree_ins_child)
#define treen_unlink_child _CW_NS_ANY(tree_del_child)
#define treen_get_child_ptr _CW_NS_ANY(treen_get_child_ptr)
#define treen_get_data_ptr _CW_NS_ANY(tree_get_data_ptr)
#define treen_set_data_ptr _CW_NS_ANY(tree_set_data_ptr)

cw_treen_t * treen_new(cw_bool_t a_is_thread_safe);
void treen_delete(cw_treen_t * a_treen_o);
cw_uint32_t treen_get_num_children(cw_treen_t * a_treen_o);
cw_bool_t treen_link_child(cw_treen_t * a_treen_o, cw_treen_t * a_child,
			   cw_uint32_t a_position);
cw_bool_t treen_unlink_child(cw_treen_t * a_treen_o, cw_uint32_t a_position,
			     cw_treen_t ** a_child);
cw_bool_t treen_get_child_ptr(cw_treen_t * a_treen_o, cw_uint32_t a_position,
			      cw_treen_t ** a_child);
void * treen_get_data_ptr(cw_treen_t * a_treen_o);
void * treen_set_data_ptr(cw_treen_t * a_treen_o, void * a_data);
